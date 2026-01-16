"""
M/M/1 queue simulator.
Simple event-driven simulation for a single-server queue (M/M/1).
"""

import sys
import os
import math
import random
import os
from dataclasses import dataclass
from typing import TextIO

# True constants
Q_LIMIT = 100
IDLE = 0
BUSY = 1


@dataclass
class SimulationStats:
    """Groups statistical counters."""
    num_custs_delayed: int = 0
    total_of_delays: float = 0.0
    area_num_in_q: float = 0.0
    area_server_status: float = 0.0
    arrived_counter: int = 0
    departed_counter: int = 0


@dataclass
class SimulationConfig:
    """Groups input parameters."""
    mean_interarrival: float
    mean_service: float
    num_delays_required: int
    verbose_flag: bool


@dataclass
class SimulationState:
    """Groups the dynamic state of the simulation."""
    sim_time: float = 0.0
    server_status: int = IDLE
    num_in_q: int = 0
    time_last_event: float = 0.0
    event_counter: int = 0


class MM1QueueSimulation:
    """Class to encapsulate the logic of the M/M/1 simulation."""

    def __init__(self, config: SimulationConfig):
        self.cfg = config
        self.stats = SimulationStats()
        self.state = SimulationState()

        # Event Lists
        self.time_next_event = [0.0, 0.0, float('inf')]
        self.time_arrival = [0.0] * (Q_LIMIT + 1)

        # Initialize first arrival
        self.time_next_event[1] = self.state.sim_time + self.expon(self.cfg.mean_interarrival)

    def expon(self, mean: float) -> float:
        """Return an exponentially distributed random variate."""
        u = max(random.random(), 1e-10)
        return -mean * math.log(u)

    def update_time_avg_stats(self) -> None:
        """Update time-average statistical accumulators."""
        duration = self.state.sim_time - self.state.time_last_event
        self.state.time_last_event = self.state.sim_time
        self.stats.area_num_in_q += self.state.num_in_q * duration
        self.stats.area_server_status += self.state.server_status * duration

    def timing(self, outfile: TextIO) -> None:
        """Advance the simulation clock to the next event."""
        next_type = 1 if self.time_next_event[1] < self.time_next_event[2] else 2
        self.state.event_counter += 1
        self.state.sim_time = self.time_next_event[next_type]

        if self.cfg.verbose_flag:
            if next_type == 1:
                self.stats.arrived_counter += 1
                outfile.write(f"{self.state.event_counter}. Next event: "
                              f"Customer {self.stats.arrived_counter} Arrival\n")
            else:
                self.stats.departed_counter += 1
                outfile.write(f"{self.state.event_counter}. Next event: "
                              f"Customer {self.stats.departed_counter} Departure\n")

        self.update_time_avg_stats()
        if next_type == 1:
            self.arrive(outfile)
        else:
            self.depart(outfile)

    def arrive(self, outfile: TextIO) -> None:
        """Process an arrival event."""
        self.time_next_event[1] = self.state.sim_time + self.expon(self.cfg.mean_interarrival)

        if self.state.server_status == BUSY:
            self.state.num_in_q += 1
            if self.state.num_in_q > Q_LIMIT:
                outfile.write(f"\nOverflow at time {self.state.sim_time}\n")
                sys.exit(2)
            self.time_arrival[self.state.num_in_q] = self.state.sim_time
        else:
            self.stats.num_custs_delayed += 1
            self.state.server_status = BUSY
            self.time_next_event[2] = self.state.sim_time + self.expon(self.cfg.mean_service)
            self.log_delay(outfile)

    def depart(self, outfile: TextIO) -> None:
        """Process a departure event."""
        if self.state.num_in_q == 0:
            self.state.server_status = IDLE
            self.time_next_event[2] = float('inf')
        else:
            self.state.num_in_q -= 1
            delay = self.state.sim_time - self.time_arrival[1]
            self.stats.total_of_delays += delay
            self.stats.num_custs_delayed += 1
            self.time_next_event[2] = self.state.sim_time + self.expon(self.cfg.mean_service)
            self.log_delay(outfile)

            for i in range(1, self.state.num_in_q + 1):
                self.time_arrival[i] = self.time_arrival[i + 1]

    def log_delay(self, outfile: TextIO) -> None:
        """Utility to log customer delay counts."""
        if self.cfg.verbose_flag:
            outfile.write(f"\n--------No. of customers delayed: "
                          f"{self.stats.num_custs_delayed}--------\n\n")

    def report(self, outfile: TextIO) -> None:
        """Write final results."""
        avg_delay = self.stats.total_of_delays / self.stats.num_custs_delayed
        avg_q = self.stats.area_num_in_q / self.state.sim_time
        util = self.stats.area_server_status / self.state.sim_time

        outfile.write("-" * 43 + "\n\n")
        outfile.write("Final Statistics:")
        outfile.write(f"\n\nAverage delay in queue{avg_delay:11.3f} minutes\n\n")
        outfile.write(f"Average number in queue{avg_q:10.3f}\n\n")
        outfile.write(f"Server utilization{util:15.3f}\n\n")
        outfile.write(f"Time simulation ended{self.state.sim_time:12.3f} minutes\n")


def main() -> None:
    """Main entry point."""
    script_dir = os.path.dirname(__file__)
    input_path = os.path.join(script_dir, 'in.txt')
    output_path = os.path.join(script_dir, 'out.txt')
    
    try:
        with open(input_path, 'r', encoding='utf-8') as infile:
            line = infile.read().split()
            m_inter, m_serv, n_delays = float(line[0]), float(line[1]), int(line[2])
    except FileNotFoundError:
        print("Error: input file 'in.txt' not found.", file=sys.stderr)
        return
    except IndexError:
        print(
            "Error: 'in.txt' must contain at least three values: "
            "mean interarrival time, mean service time, and number of customers.",
            file=sys.stderr,
        )
        return
    except ValueError as exc:
        print(f"Error: failed to parse numeric values from 'in.txt': {exc}", file=sys.stderr)
        return

    with open(output_path, 'w', encoding='utf-8') as outfile:
        outfile.write("Single-server queueing system\n\n")
        outfile.write(f"Mean interarrival time{m_inter:11.3f} minutes\n\n")
        outfile.write(f"Mean service time{m_serv:16.3f} minutes\n\n")
        outfile.write(f"Number of customers{n_delays:14d}\n\n")
        outfile.write("-" * 43 + "\n\n")

        config = SimulationConfig(m_inter, m_serv, n_delays, True)
        sim = MM1QueueSimulation(config)
        while sim.stats.num_custs_delayed < config.num_delays_required:
            sim.timing(outfile)

        sim.report(outfile)


if __name__ == "__main__":
    random.seed(2005021)
    main()
