#include <stdio.h>
#include <cmath>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <vector>
#include <climits>
#include <iomanip>
#include "../lib/lcgrand.h"

using namespace std;

enum EntityState
{
    WAITING,
    READY
};

enum TellerState
{
    STARTED_NOW,
    WAITING_FOR_ACTIVATION,
    SERVICE_IN_PROGRESS,
    FINISHED_SERVICE
};

enum CustomerState
{
    ARRIVED_NOW,
    WAITING_IN_THE_QUEUE,
    WAITING_FOR_SERVICE_COMPLETION
};

/* Forward declarations */
class Teller;
class Customer;
struct Statistics;

int TotalCustomers = 0;
int min_tellers, max_tellers;
float mean_interarrival, mean_service, length_doors_open;

float sim_time = 0.0;   /* Global simulation clock */
bool bank_open = true;  /* Flag for bank operating status */
FILE *simlog = nullptr; /* Log file for simulation trace output */

class Entity
{
public:
    bool waiting_for_reactivation = true; /* Flag to indicate if the entity is waiting for activation */
    bool conditional_waiting = true;      /* Flag to indicate if the entity is waiting conditionally */
    float reactivation_time = 0.0;        /* Time at which the entity should be reactivated */
    EntityState CES_state = WAITING;
    virtual ~Entity() = default;
};

set<Entity *> CES;              /* Conditional Event Set */
multimap<float, Entity *> FES;  /* Future Event Set (allow simultaneous events) */
vector<Teller *> tellers;       /* List of all tellers in the simulation */

class Teller : public Entity
{
public:
    int teller_index;
    TellerState state = STARTED_NOW;
    deque<Customer *> queue;
    Customer *current_customer = nullptr;
    float last_queue_change_time = 0.0;
    Teller(int index) : teller_index(index) {}

    void create_record();
    bool is_queue_empty();
    void wait_until_activated();
    void receive_customer_from_queue();
    float determine_service_time();
    void schedule_service_completion(float service_time);
    void wait_until_service_completion();
    void activate_customer_now();
    void undertake_jockey();
    void teller_process();
};

/* Statistics tracking structure */
struct Statistics
{
    vector<float> delays;
    float total_queue_length = 0.0;

    void record_delay(float delay);
    void record_queue_length(Teller *teller);
    void flush_queues();

    float get_average_delay();
    float get_max_delay();
    float get_min_delay();
    float get_average_queue_length();

    void reset();
};

/* global stats instance */
Statistics stats;

class Customer : public Entity
{
public:
    int customer_id; /* Optional: can be used for debugging or tracking */
    float arrival_time;
    CustomerState next_event = ARRIVED_NOW;
    Customer(int id, float arrival_time) : customer_id(id), arrival_time(arrival_time) {}

    void create_record()
    {
        fprintf(simlog, "Customer %d arrives at time %f\n", customer_id, sim_time);
        waiting_for_reactivation = false; /* Customer is active now, so it's not waiting for reactivation */
    }

    float determine_next_arrival_time()
    {
        return sim_time + expon(mean_interarrival, 1); /* Stream 1 for arrivals */
    }
    void schedule_next_arrival(float next_arrival_time)
    {
        if (next_arrival_time <= length_doors_open * 60)
        {
            FES.insert({next_arrival_time, new Customer(TotalCustomers++, next_arrival_time)});
        }
    }
    Teller *find_tell_with_shortest_queue()
    {
        Teller *selected_teller = tellers[0];

        int min_queue_length = selected_teller->queue.size();
        if (selected_teller->CES_state == READY)
            min_queue_length += 1;

        for (Teller *teller : tellers)
        {
            int queue_length = teller->queue.size();
            if (teller->CES_state == READY)
                queue_length += 1;
            if (queue_length < min_queue_length)
            {
                min_queue_length = queue_length;
                selected_teller = teller;
            }
        }
        return selected_teller;
    }
    bool is_teller_idle(Teller *teller)
    {
        return teller->CES_state == WAITING;
    }
    void activate_teller_now(Teller *teller, Customer *customer)
    {
        /* Mark teller as busy */
        teller->current_customer = customer;
        teller->CES_state = READY;
        teller->state = WAITING_FOR_ACTIVATION;
        fprintf(simlog, "Customer %d finds Teller %d idle and starts service immediately at time %f\n",
                customer_id, teller->teller_index, sim_time);
    }
    void join_queue(Teller *teller)
    {
        stats.record_queue_length(teller);
        teller->queue.push_back(this);
        fprintf(simlog, "Customer %d finds Teller %d busy and joins the queue at time %f\n",
                customer_id, teller->teller_index, sim_time);
    }
    void jockey(Teller *source_teller, Teller *destination_teller)
    {
        source_teller->queue.pop_back();

        fprintf(simlog, "Customer %d jockeys from Teller %d to Teller %d at time %f\n",
                this->customer_id, source_teller->teller_index, destination_teller->teller_index, sim_time);

        if (destination_teller->is_queue_empty())
        {
            fprintf(simlog, "Teller %d was idle and jockeying customer %d directly joins the teller at time %f\n",
                    destination_teller->teller_index, this->customer_id, sim_time);
            /* If this teller is idle, activate the jockey immediately */
            destination_teller->current_customer = this;
            this->next_event = WAITING_FOR_SERVICE_COMPLETION;
            destination_teller->state = WAITING_FOR_ACTIVATION; /* Set state to waiting for activation */
        }
        else
        {
            fprintf(simlog, "Teller %d has customers waiting, so jockeying customer %d joins the queue at time %f\n",
                    destination_teller->teller_index, this->customer_id, sim_time);
            /* Otherwise, add jockey to this teller's queue */
            destination_teller->queue.push_back(this);
        }
    }
    void wait_until_service_completion()
    {
        fprintf(simlog, "Customer %d starts waiting for service completion at time %f\n", customer_id, sim_time);
        next_event = WAITING_FOR_SERVICE_COMPLETION;
        CES_state = WAITING;
        waiting_for_reactivation = true;
        conditional_waiting = true;
    }
    void wait_in_the_queue()
    {
        fprintf(simlog, "Customer %d starts waiting in the queue at time %f\n", customer_id, sim_time);
        next_event = WAITING_IN_THE_QUEUE;
        CES_state = WAITING;
        waiting_for_reactivation = true;
        conditional_waiting = true;
    }
    void record_delay_in_system()
    {
        float delay = sim_time - arrival_time;
        stats.record_delay(delay);
        fprintf(simlog, "Customer %d experiences a total delay in system of %f minutes at sim time %f\n",
                customer_id, delay, sim_time);
    }
    void destroy_record()
    {
        this->waiting_for_reactivation = false;
        fprintf(simlog, "Customer %d departs at time %f with total delay in system %f\n",
                customer_id, sim_time, sim_time - arrival_time);
    }
    void customer_process()
    {
        if (next_event == ARRIVED_NOW)
        {
            create_record();
            float next_arrival_time = determine_next_arrival_time();
            schedule_next_arrival(next_arrival_time);
            Teller *selected_teller = find_tell_with_shortest_queue();
            if (is_teller_idle(selected_teller))
            {
                /* Idle teller found - activate immediately */
                activate_teller_now(selected_teller, this);
                wait_until_service_completion();
            }
            else
            {
                /* All tellers busy - join queue */
                join_queue(selected_teller);
                wait_in_the_queue();
            }
        }
        else if (next_event == WAITING_FOR_SERVICE_COMPLETION)
        {
            /* Customer process completes here */
            destroy_record();
        }
    }
};

/* Definitions for Statistics methods */

void Statistics::record_delay(float delay)
{
    delays.push_back(delay);
}

void Statistics::record_queue_length(Teller *teller)
{
    /* Only count customers WAITING in queue, NOT the one being served
       (mirrors simlib filest(teller) which tracks the waiting list only) */
    float queue_size = (float)teller->queue.size();
    float observation_duration = sim_time - teller->last_queue_change_time;
    total_queue_length += queue_size * observation_duration;
    teller->last_queue_change_time = sim_time; /* Update checkpoint */
}

float Statistics::get_average_delay()
{
    if (delays.empty())
        return 0.0;
    float sum = 0.0;
    for (float d : delays)
        sum += d;
    return sum / delays.size();
}

float Statistics::get_max_delay()
{
    if (delays.empty())
        return 0.0;
    float max_val = delays[0];
    for (float d : delays)
        if (d > max_val)
            max_val = d;
    return max_val;
}

float Statistics::get_min_delay()
{
    if (delays.empty())
        return 0.0;
    float min_val = delays[0];
    for (float d : delays)
        if (d < min_val)
            min_val = d;
    return min_val;
}

float Statistics::get_average_queue_length()
{
    /* Divide by sim_time (actual end time) — equivalent to summing filest() for each
       teller's waiting list, which is what mtbank.c's report() does. */
    return total_queue_length / sim_time;
}

void Statistics::flush_queues()
{
    /* Record the final time-segment for every teller's waiting queue */
    for (Teller *t : tellers)
        record_queue_length(t);
}

void Statistics::reset()
{
    delays.clear();
    total_queue_length = 0.0;
}

void Teller::create_record()
{
    fprintf(simlog, "Teller %d starts process at time %f\n", teller_index, sim_time);
    /* This function can be used to initialize any additional state for the teller if needed */
    waiting_for_reactivation = true; /* Teller starts in a waiting state until activated by a customer or by timing routine */
    conditional_waiting = true;      /* Teller starts waiting conditionally for activation */
    CES.insert(this);                /* Register teller in CES to wait for activation */
}

bool Teller::is_queue_empty()
{
    return this->queue.empty();
}

void Teller::wait_until_activated()
{
    /* Teller waits conditionally for next customer */
    this->CES_state = WAITING;
    this->waiting_for_reactivation = true;
    this->conditional_waiting = true;
    fprintf(simlog, "Teller %d becomes idle at time %f\n", teller_index, sim_time);
}

void Teller::receive_customer_from_queue()
{
    stats.record_queue_length(this); /* Record area BEFORE removing customer from queue */
    current_customer = queue.front();
    queue.pop_front();
    /* last_queue_change_time already updated inside record_queue_length */
    current_customer->next_event = WAITING_FOR_SERVICE_COMPLETION;
    fprintf(simlog, "Teller %d receives customer %d at time %f with delay %f\n",
            teller_index, current_customer->customer_id, sim_time, sim_time - current_customer->arrival_time);
}

float Teller::determine_service_time()
{
    return expon(mean_service, 2); /* Stream 2 for service times */
}

void Teller::schedule_service_completion(float service_time)
{
    float service_completion_time = sim_time + service_time;
    this->reactivation_time = service_completion_time;
    stats.record_delay(sim_time - current_customer->arrival_time); /* Record delay in queue */
}

void Teller::wait_until_service_completion()
{
    this->state = SERVICE_IN_PROGRESS;
    this->waiting_for_reactivation = true;
    this->conditional_waiting = false; /* Wait for specific reactivation time (service completion) */
    fprintf(simlog, "Teller %d starts service for customer %d at time %f, scheduled to complete at time %f\n",
            teller_index, current_customer->customer_id, sim_time, this->reactivation_time);
}

/* Implementation of Teller::activate_customer_now() - after Customer class is defined */
inline void Teller::activate_customer_now()
{
    fprintf(simlog, "Teller %d completes service and activates customer %d at time %f\n",
            teller_index, current_customer->customer_id, sim_time);
    current_customer->CES_state = READY;

    /* Service completion - current customer is done */
    current_customer = nullptr;
    this->state = FINISHED_SERVICE; /* Set to finished service to trigger idle state in next iteration */
}

void Teller::undertake_jockey()
{
    /* Calculate total customers at destination teller (waiting + being served) */
    int ni = queue.size() + (current_customer != nullptr ? 1 : 0);

    int jumper_index = -1;
    int min_distance = INT_MAX;

    /* Search for the best queue to jockey from */
    for (int j = 0; j < (int)tellers.size(); j++)
    {
        Teller *teller_j = tellers[j];
        if (teller_j == this)
            continue; /* Skip self */

        /* Calculate nj: total customers at teller j */
        int nj = teller_j->queue.size();
        if (teller_j->current_customer != nullptr)
            nj++; /* Add 1 if teller j is currently serving a customer */

        /* Check if this queue satisfies jockeying condition: nj > ni + 1 */
        if (nj > ni + 1)
        {
            /* Calculate distance (to find closest/leftmost) */
            int distance = abs(this->teller_index - j);

            /* Select this queue if:
               1. It's closer than previous best, OR
               2. Same distance but more to the left (smaller index) */
            if (distance < min_distance ||
                (distance == min_distance && j < jumper_index))
            {
                min_distance = distance;
                jumper_index = j;
            }
        }
    }

    /* If a customer should jockey, move them */
    if (jumper_index != -1 && !tellers[jumper_index]->queue.empty())
    {
        /* Remove customer from TAIL (back) of source queue */
        Customer *jockey_customer = tellers[jumper_index]->queue.back();

        /* Record area BEFORE removing customer from source queue */
        stats.record_queue_length(tellers[jumper_index]);

        /* Record area BEFORE modifying destination queue */
        stats.record_queue_length(this);

        /* Activate the jockeying customer */
        jockey_customer->jockey(tellers[jumper_index], this);
    }
}

/* Implementation of Teller::teller_process() - after Customer class is defined */
void Teller::teller_process()
{
    while (true)
    {

        if (state == STARTED_NOW)
        {
            create_record();
            if (is_queue_empty())
            {
                /* Teller waits conditionally for next customer */
                wait_until_activated();
                break; /* Exit loop to wait for activation event */
            }
            else
            {
                /* Receive customer from queue and start service immediately */
                receive_customer_from_queue();
                float service_time = determine_service_time();
                schedule_service_completion(service_time);
                wait_until_service_completion();
                break; /* Exit loop to wait for service completion event */
            }
        }
        else if (state == FINISHED_SERVICE)
        {
            if (is_queue_empty())
            {
                /* Teller waits conditionally for next customer */
                wait_until_activated();
                break; /* Exit loop to wait for activation event */
            }
            else
            {
                /* Receive customer from queue and start service immediately */
                receive_customer_from_queue();
                float service_time = determine_service_time();
                schedule_service_completion(service_time);
                wait_until_service_completion();
                break; /* Exit loop to wait for service completion event */
            }
        }
        else if (state == WAITING_FOR_ACTIVATION)
        {
            float service_time = determine_service_time();
            schedule_service_completion(service_time);
            wait_until_service_completion();
            break; /* Exit loop to wait for service completion event */
        }
        else if (state == SERVICE_IN_PROGRESS)
        {
            /* Activate the customer who was just served */
            activate_customer_now();

            /* Check for jockeying */
            undertake_jockey();
        }
    }
}

void initialize(int num_tellers)
{
    /* Clear all state */
    CES.clear();
    FES.clear();
    tellers.clear();
    stats.reset();
    sim_time = 0.0;
    bank_open = true;
    TotalCustomers = 0;

    /* Initialize tellers */
    for (int i = 0; i < num_tellers; i++)
    {
        Teller *teller = new Teller(i);
        teller->teller_process();
        tellers.push_back(teller);
    }

    /* Schedule first customer arrival */
    float first_arrival_time = sim_time + expon(mean_interarrival, 1); /* Stream 1 for arrivals */
    FES.insert({first_arrival_time, new Customer(TotalCustomers++, first_arrival_time)});
}

void timing_routine()
{
    while (true)
    {
        /* Search CES for active entities */
        fprintf(simlog, "Processing CES at time %f\n", sim_time);
        if (!CES.empty())
        {
            /* If there are entities with CES_state == READY then remove them from CES and insert them into FES with sim_time as the current simulation time */
            for (auto it = CES.begin(); it != CES.end();)
            {
                Entity *entity = *it;
                if (entity->CES_state == READY)
                {
                    entity->conditional_waiting = false; /* No longer waiting conditionally */
                    FES.insert({sim_time, entity});
                    it = CES.erase(it);
                    // print whether it's a teller or customer being moved from CES to FES
                    if (Teller *teller = dynamic_cast<Teller *>(entity))
                    {
                        fprintf(simlog, "Moving Teller %d from CES to FES at time %f\n", teller->teller_index, sim_time);
                    }
                    else if (Customer *customer = dynamic_cast<Customer *>(entity))
                    {
                        fprintf(simlog, "Moving Customer %d from CES to FES at time %f\n", customer->customer_id, sim_time);
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
        else
        {
            fprintf(simlog, "No entities in CES at time %f\n", sim_time);
        }

        /* If FES is not empty, process the next event */
        fprintf(simlog, "Processing FES at time %f\n", sim_time);
        if (!FES.empty())
        {
            auto it = FES.begin();
            Entity *entity = it->second;
            float event_time = it->first;
            FES.erase(it);
            entity->waiting_for_reactivation = false; /* Entity is now active, so it's not waiting for reactivation */

            /* Advance time */
            sim_time = event_time;

            if (Teller *teller = dynamic_cast<Teller *>(entity))
            {
                fprintf(simlog, "Processing Teller %d event from FES at time %f\n", teller->teller_index, sim_time);
                teller->teller_process();
            }
            else if (Customer *customer = dynamic_cast<Customer *>(entity))
            {
                fprintf(simlog, "Processing Customer %d event from FES at time %f\n", customer->customer_id, sim_time);
                customer->customer_process();
            }
            if (entity->waiting_for_reactivation)
            {
                if (entity->conditional_waiting)
                {
                    fprintf(simlog, "Entity is waiting conditionally for reactivation at time %f\n", sim_time);
                    CES.insert(entity); /* Insert back into CES to wait for next reactivation event */
                }
                else
                {
                    fprintf(simlog, "Entity is waiting for reactivation at time %f\n", sim_time);
                    FES.insert({entity->reactivation_time, entity}); /* Insert back into FES to wait for next reactivation event */
                }
            }
            else
            {
                if (Teller *teller = dynamic_cast<Teller *>(entity))
                {
                    fprintf(simlog, "Destroying Teller %d record at time %f\n", teller->teller_index, sim_time);
                }
                else if (Customer *customer = dynamic_cast<Customer *>(entity))
                {
                    fprintf(simlog, "Destroying Customer %d record at time %f\n", customer->customer_id, sim_time);
                }
                delete entity; /* Clean up entity if it's not waiting for reactivation */
            }
        }
        else
        {
            fprintf(simlog, "No events in FES at time %f\n", sim_time);
            break;
        }
    }
}

void report(FILE *outfile, int num_tellers)
{
    fprintf(outfile, "\n\nWith %d tellers, average number in queue = %10.3f\n",
            num_tellers, stats.get_average_queue_length());

    fprintf(outfile, "\nDelays in queue, in minutes:\n\n");

    fprintf(outfile, " sampst                         Number\n");
    fprintf(outfile, "variable                          of\n");
    fprintf(outfile, " number       Average           values          Maximum          Minimum\n");
    fprintf(outfile, "________________________________________________________________________\n\n");

    fprintf(outfile, "    1 %#15.6G  %#15.6G  %#15.6G  %#15.6G \n",
            stats.get_average_delay(),
            (float)stats.delays.size(),
            stats.get_max_delay(),
            stats.get_min_delay());

    fprintf(outfile, "________________________________________________________________________\n\n\n");
}

int main(void)
{
    /* Open input and output files */
    FILE *infile = fopen("../in/mtbank.in", "r");
    FILE *outfile = fopen("../out/pi_mtbank.out", "w");

    /* Open simulation trace log */
    simlog = fopen("../log/pi_simulation.log", "w");
    if (simlog)
    {
        /* Use line buffering so logs are flushed per-line on most platforms */
        setvbuf(simlog, NULL, _IOLBF, 0);
    }

    /* Read input parameters */
    fscanf(infile, "%d %d %f %f %f", &min_tellers, &max_tellers,
           &mean_interarrival, &mean_service, &length_doors_open);

    /* Write report heading and input parameters */
    fprintf(outfile, "#################################################\n");
    fprintf(outfile, "#                                               #\n");
    fprintf(outfile, "#     Process Interaction Method Simulation     #\n");
    fprintf(outfile, "#                                               #\n");
    fprintf(outfile, "#################################################\n");
    fprintf(outfile, "\n\nMultiteller bank with separate queues & jockeying\n\n");
    fprintf(outfile, "Number of tellers%16d to%3d\n\n", min_tellers, max_tellers);
    fprintf(outfile, "Mean interarrival time%11.3f minutes\n\n", mean_interarrival);
    fprintf(outfile, "Mean service time%16.3f minutes\n\n", mean_service);
    fprintf(outfile, "Bank closes after%16.3f hours\n\n\n\n", length_doors_open);
    fflush(outfile);

    for (int num_tellers = min_tellers; num_tellers <= max_tellers; ++num_tellers)
    {
        /* Initialize simulation for this number of tellers */
        initialize(num_tellers);
        /* Run the timing routine to process events */
        timing_routine();
        /* Flush outstanding queue-area for the last time-segment of each teller */
        stats.flush_queues();
        /* Generate report for this number of tellers */
        report(outfile, num_tellers);

        /* Clean up tellers */
        for (Teller *teller : tellers)
        {
            fprintf(simlog, "Destroying Teller %d record at time %f\n", teller->teller_index, sim_time);
            delete teller;
        }
    }

    if (simlog)
        fclose(simlog);

    return 0;
}
