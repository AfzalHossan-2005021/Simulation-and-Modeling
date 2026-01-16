# M/M/1 Queue Simulation — Problem Specification ✅

## Overview

Simulate a single-server (M/M/1) queue using a discrete-event simulation. Customers arrive according to a Poisson process (exponential interarrival times) and receive service with exponential service times. The simulator must collect time-average and per-customer statistics and stop after a specified number of customers have been delayed (served).

---

## Input 📥

- Input is read from a single file named `in.txt` (or from standard input). It contains a single line with three values separated by whitespace:
  1. **mean interarrival time** (float) — mean time between arrivals
  2. **mean service time** (float) — mean service time
  3. **number of customers** (integer) — number of customers to be delayed (i.e., count of completed customers used as termination)

Example `in.txt`:

```
1.0 0.5 1000
```

---

## Output 📤

- Output is written to `out.txt`. It must contain:
  - A short header that echoes the input parameters
  - A numbered event log listing each next event (arrival or departure) and the relevant customer number
  - Periodic lines indicating the total number of customers delayed (after events that increase that count)
  - A final summary with these metrics (formatted with 3 decimal digits where applicable):
    - Average delay in queue (minutes)
    - Average number in queue
    - Server utilization
    - Time simulation ended (minutes)

Example summary block:

```
Average delay in queue   0.345 minutes
Average number in queue  0.123
Server utilization        0.432
Time simulation ended  991.024 minutes
```

---

## Behavior & Requirements 🔧

- Simulate **two** event types: arrival (1) and departure (2).
- Maintain an event list with `timeNextEvent[1]` (next arrival) and `timeNextEvent[2]` (next departure).
- Generate exponential random variates using the inversion method: if U ~ Uniform(0,1) then X = -mean \* ln(U).
- State variables to maintain:
  - `simTime` — simulation clock
  - `serverStatus` — IDLE or BUSY
  - `numInQ` — number of customers in queue
  - `timeArrival[]` — arrival times for queued customers (for delay calculation)
  - `timeNextEvent[]` — scheduled times for next events
- Time-average statistics:
  - `areaNumInQ` — area under number-in-queue curve (used to compute average queue length)
  - `areaServerStatus` — area under server-busy indicator (used for utilization)

---

## Event Logic & Statistics

- Arrival:
  - Schedule the next arrival: `timeNextEvent[1] = simTime + expon(meanInterarrival)`.
  - If server is IDLE: start service immediately, record zero delay, increment `numCustsDelayed`, set server to BUSY, schedule departure.
  - If server is BUSY: increment `numInQ`, store arrival time in `timeArrival[numInQ]`. Check for queue overflow and exit with message if queue limit exceeded.
- Departure:
  - If queue is empty: set server to IDLE and set `timeNextEvent[2]` = +infinity.
  - Otherwise: remove front customer from queue, compute delay = `simTime - timeArrival[front]`, add to `totalOfDelays`, increment `numCustsDelayed`, schedule next departure, and shift queue entries (or use a FIFO structure).
- Update time-average statistics each event: multiply time-since-last-event by current `numInQ` and `serverStatus`, and accumulate.
- Termination: stop when `numCustsDelayed` equals the requested number of customers.

---

## Edge Cases & Robustness ⚠️

- Implement a queue capacity (e.g., `Q_LIMIT`) and print an overflow message and terminate if exceeded.
- If the event list becomes empty (no next events), terminate with an error message.
- Optionally support a verbose flag to toggle per-event logging for very large runs.

---

## Reproducibility & Testing ✅

- Allow setting a random seed (e.g., `random.seed(...)`) to make runs deterministic for testing.
- Test small cases (e.g., small number of customers) and extreme rates:
  - Low arrival rate (λ << μ): mostly idle server, almost zero queue.
  - High arrival rate (λ close to μ but < μ): queue grows; performance aligns with theory for long runs.
- Verify simulated results against theoretical M/M/1 formulas (when ρ = λ/μ < 1):
  - Utilization ≈ ρ
  - E[L] = ρ / (1 - ρ)
  - E[W] = ρ / (μ \* (1 - ρ))

---

## Output Formatting & Scoring (for assignment) 📝

- Correctness (60%): simulation metrics are plausible and program terminates after the requested number of customers.
- Output format (20%): header, event log, and final summary match required layout and numeric precision.
- Robustness & style (10%): handles queue overflow, event-list errors, and code is modular/readable.
- Tests & reproducibility (10%): include at least two test cases and support for RNG seed.

---

## Hints & Implementation Notes 💡

- Use constants for `Q_LIMIT`, `IDLE`, `BUSY` for clarity.
- Use a small function `expon(mean)` that returns `-mean * ln(random.random())`.
- Keep event selection logic isolated (e.g., a `getNextEvent()` function that returns the type of next event and advances the clock).
- Prefer a FIFO queue (collections.deque) for clarity instead of manual array shifting, but array-shifting is acceptable for small limits.
- Use clear logging format for easy grading; allow a `--quiet` flag to suppress verbose per-event output.

---

## Deliverables

- Source file(s) (e.g., `2005xxx.xyz`) that run from project root and read `in.txt`.
- At least one example `in.txt` test case and its expected-type output.

---
