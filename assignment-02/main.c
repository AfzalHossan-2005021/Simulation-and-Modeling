/* External definitions for inventory system. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcgrand.h" /* Header file for random-number generator. */
#include "table.h"   /* Header file for Table helper. */
#include "utils.h"   /* Header file for string utilities. */

int amount, bigs, initial_inv_level, inv_level, next_event_type, num_events,
    num_months, num_values_demand, smalls, policy_no;
float area_holding, area_shortage, holding_cost, incremental_cost, maxlag,
    mean_interdemand, minlag, prob_distrib_demand[26], setup_cost,
    shortage_cost, sim_time, time_last_event, time_next_event[5],
    total_ordering_cost;
FILE *infile, *outfile;
void report_inputs(void);
void report_costs(void);
void initialize(void);
void timing(void);
void order_arrival(void);
void demand(void);
void evaluate(void);
void report(Table *table);
void update_time_avg_stats(void);
float expon(float mean);
int random_integer(float prob_distrib[]);
float uniform(float a, float b);

int main() /* Main function. */
{
  int i, num_policies;

  /* Open input and output files. */
  infile = fopen("in.txt", "r");
  outfile = fopen("out.txt", "w");

  /* Specify the number of events for the timing function. */
  num_events = 4;

  /* Read input parameters. */
  fscanf(infile, "%d %d %d %d %f %f %f %f %f %f %f",
         &initial_inv_level, &num_months, &num_policies, &num_values_demand,
         &mean_interdemand, &setup_cost, &incremental_cost, &holding_cost,
         &shortage_cost, &minlag, &maxlag);
  for (i = 1; i <= num_values_demand; ++i)
    fscanf(infile, "%f", &prob_distrib_demand[i]);

  /* Write report heading. */
  fprintf(outfile, "------Single-Product Inventory System------\n\n");

  /* Report input parameters. */
  report_inputs();

  /* Report cost parameters. */
  report_costs();

  /* Table for policies */
  const char *colnames[] = {"No", "Policy", "Avg_total_cost", "Avg_ordering_cost", "Avg_holding_cost", "Avg_shortage_cost"};
  Table *policies_table = table_create("Policies", 6, colnames);

  /* Begin the simulations for the different (s,S) policies. */
  for (i = 1; i <= num_policies; ++i)
  {
    /* Read the inventory policy, and initialize the simulation. */
    fscanf(infile, "%d %d", &smalls, &bigs);
    policy_no = i;
    initialize();

    /* Run the simulation until it terminates after an end-simulation event (type 3) occurs. */
    do
    {
      /* Determine the next event. */
      timing();

      /* Update time-average statistical accumulators. */
      update_time_avg_stats();

      /* Invoke the appropriate event function. */
      switch (next_event_type)
      {
      case 1:
        order_arrival();
        break;
      case 2:
        demand();
        break;
      case 4:
        evaluate();
        break;
      case 3:
        report(policies_table);
        break;
      }
      /* If the event just executed was not the end-simulation event (type 3),
         continue simulating. Otherwise, end the simulation for the current
         (s,S) pair and go on to the next pair (if any). */
    } while (next_event_type != 3);
  }
  /* End the simulations. */

  /* Print the policies table */
  table_print(policies_table, outfile);
  table_free(policies_table);

  /* Close input and output files. */
  fclose(infile);
  fclose(outfile);
  return 0;
}

void report_inputs(void) /* Function to report input parameters. */
{
  /* Use the Table helper to print a neat two-column table */
  const char *colnames[] = {"Parameter", "Value"};
  Table *t = table_create("Input Parameters", 2, colnames);

  table_add_row(t, (const char *[]){"Initial inventory level", concat_strings(int_to_string(initial_inv_level), " items")});
  table_add_row(t, (const char *[]){"Number of demand sizes", int_to_string(num_values_demand)});
  table_add_row(t, (const char *[]){"Distribution function of demand sizes", array_to_string(prob_distrib_demand, num_values_demand)});
  table_add_row(t, (const char *[]){"Mean interdemand time", concat_strings(float_to_string(mean_interdemand), " months")});
  table_add_row(t, (const char *[]){"Delivery lag range", concat_strings(range_to_string(minlag, maxlag), " months")});
  table_add_row(t, (const char *[]){"Length of the simulation", concat_strings(int_to_string(num_months), " months")});

  table_print(t, outfile);
  fprintf(outfile, "\n");
  table_free(t);
}

void report_costs(void) /* Function to report cost parameters. */
{
  const char *colnames[] = {"Parameter", "Value"};
  Table *t = table_create("Cost Parameters", 2, colnames);

  table_add_row(t, (const char *[]){"K", float_to_string(setup_cost)});
  table_add_row(t, (const char *[]){"i", float_to_string(incremental_cost)});
  table_add_row(t, (const char *[]){"h", float_to_string(holding_cost)});
  table_add_row(t, (const char *[]){"pi", float_to_string(shortage_cost)});

  table_print(t, outfile);
  fprintf(outfile, "\n");
  table_free(t);
}

void initialize(void) /* Initialization function. */
{                     /* Initialize the simulation clock. */
  sim_time = 0.0;
  /* Initialize the state variables. */
  inv_level = initial_inv_level;
  time_last_event = 0.0;

  /* Initialize the statistical counters. */
  total_ordering_cost = 0.0;
  area_holding = 0.0;
  area_shortage = 0.0;
  amount = 0.0;

  /* Initialize the event list. Since no order is outstanding, the order-
     arrival event is eliminated from consideration. */
  time_next_event[1] = 1.0e+30;
  time_next_event[2] = sim_time + expon(mean_interdemand);
  time_next_event[3] = num_months;
  time_next_event[4] = 0.0;
}

void timing(void) /* Timing function. */
{
  int i;
  float min_time_next_event = 1.0e+29;
  next_event_type = 0;

  /* Determine the event type of the next event to occur. */
  for (i = 1; i <= num_events; ++i)
    if (time_next_event[i] < min_time_next_event)
    {
      min_time_next_event = time_next_event[i];
      next_event_type = i;
    }

  /* Check to see whether the event list is empty. */
  if (next_event_type == 0)
  {
    /* The event list is empty, so stop the simulation. */
    fprintf(outfile, "\nEvent list empty at time %f", sim_time);
    exit(1);
  }

  /* The event list is not empty, so advance the simulation clock. */
  sim_time = min_time_next_event;
}

void order_arrival(void) /* Order arrival event function. */
{
  /* Increment the inventory level by the amount ordered. */
  inv_level += amount;

  /* Since no order is now outstanding, eliminate the order-arrival event from consideration. */
  time_next_event[1] = 1.0e+30;
}

void demand(void) /* Demand event function. */
{
  /* Decrement the inventory level by a generated demand size. */
  inv_level -= random_integer(prob_distrib_demand);

  /* Schedule the time of the next demand. */
  time_next_event[2] = sim_time + expon(mean_interdemand);
}

void evaluate(void) /* Inventory-evaluation event function. */
{
  /* Check whether the inventory level is less than smalls. */
  if (inv_level < smalls)
  {
    /* The inventory level is less than smalls, so place an order for the appropriate amount. */
    amount = bigs - inv_level;
    total_ordering_cost += setup_cost + incremental_cost * amount;

    /* Schedule the arrival of the order. */
    time_next_event[1] = sim_time + uniform(minlag, maxlag);
  }

  /* Regardless of the place-order decision, schedule the next inventory evaluation. */
  time_next_event[4] = sim_time + 1.0;
}

void report(Table *table) /* Report generator function. */
{
  /* Compute and write estimates of desired measures of performance. */
  float avg_holding_cost, avg_ordering_cost, avg_shortage_cost, avg_total_cost;
  avg_ordering_cost = total_ordering_cost / num_months;
  avg_holding_cost = holding_cost * area_holding / num_months;
  avg_shortage_cost = shortage_cost * area_shortage / num_months;
  avg_total_cost = avg_ordering_cost + avg_holding_cost + avg_shortage_cost;
  const char *row[] = {int_to_string(policy_no), pair_to_string(smalls, bigs),
                       float_to_string(avg_total_cost), float_to_string(avg_ordering_cost),
                       float_to_string(avg_holding_cost), float_to_string(avg_shortage_cost)};
  table_add_row(table, row);
}

void update_time_avg_stats(void) /* Update area accumulators for time-average statistics. */
{
  float time_since_last_event;

  /* Compute time since last event, and update last-event-time marker. */
  time_since_last_event = sim_time - time_last_event;
  time_last_event = sim_time;

  /* Determine the status of the inventory level during the previous interval.
     If the inventory level during the previous interval was negative, update
     area_shortage. If it was positive, update area_holding. If it was zero,
     no update is needed. */
  if (inv_level < 0)
    area_shortage -= inv_level * time_since_last_event;
  else if (inv_level > 0)
    area_holding += inv_level * time_since_last_event;
}

float expon(float mean) /* Exponential variate generation function. */
{
  /* Return an exponential random variate with mean "mean". */
  return -mean * log(lcgrand(1));
}

int random_integer(float prob_distrib[]) /* Random integer generation function. */
{
  int i;
  float u;

  /* Generate a U(0,1) random variate. */
  u = lcgrand(1);

  /* Return a random integer in accordance with the (cumulative) distribution function prob_distrib. */
  for (i = 1; u >= prob_distrib[i]; ++i)
    ;
  return i;
}

float uniform(float a, float b) /* Uniform variate generation function. */
{
  /* Return a U(a,b) random variate. */
  return a + lcgrand(1) * (b - a);
}
