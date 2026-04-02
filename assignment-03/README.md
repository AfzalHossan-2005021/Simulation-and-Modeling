# Multiteller Bank Simulation (Assignment 03)

This repository implements a bank queueing simulation in two modeling styles:

1. Event Scheduling (`event_scheduling/mtbank.c`)
2. Process Interaction (`process_interaction/mtbank.cpp`)

The problem statement is in `Problem_Statement.md` and the benchmark data is in `in/mtbank.in`.

---

## 🎯 Objective

Simulate a bank with separate queues for each teller, jockeying, and exponential interarrival/service times:

- arrival mean 1.0 minute
- service mean 4.5 minutes
- bank open for 8 hours (480 min)
- queues: 4, 5, 6, 7 tellers
- output metrics:
  - Avg # in queue (time-average)
  - Avg delay in queue
  - Max delay in queue

---

## 📁 Repository Structure

```
assignment-03/
├── Problem_Statement.md
├── event_scheduling/
│   ├── mtbank.c
│   ├── simulation.sh
│   ├── simulation.ps1
│   └── simulation.bat
├── process_interaction/
│   ├── mtbank.cpp
│   ├── simulation.sh
│   ├── simulation.ps1
│   └── simulation.bat
├── figures/
│   └── figure_1.jpeg
├── in/
│   └── mtbank.in
├── lib/
│   ├── lcgrand.h
│   ├── simlib.c
│   ├── simlib.h
│   └── simlibdefs.h
├── log/
│   └── pi_simulation.log
├── out/
│   ├── es_mtbank.out
│   ├── pi_mtbank.out
│   └── sample_mtbank.out
├── FLOWCHART.md
└── README.md
```

---

## 🔧 Input

`in/mtbank.in` (current):

```
4 7 1.0 4.5 8.0
```

Meaning:

- 4 ≤ tellers ≤ 7
- meanInterarrival = 1.0 min
- meanService = 4.5 min
- door-open duration = 8.0 hours

---

## ▶️ How to run

### Linux / macOS

```bash
cd event_scheduling
./simulation.sh
cd ../process_interaction
./simulation.sh
```

### Windows (CMD)

```bat
cd event_scheduling
simulation.bat
cd ..\process_interaction
simulation.bat
```

### Windows (PowerShell)

```powershell
cd event_scheduling
.\simulation.ps1
cd ..\process_interaction
.\simulation.ps1
```

---

## 📤 Output files

- `out/es_mtbank.out` - event scheduling report
- `out/pi_mtbank.out` - process interaction report
- `log/pi_simulation.log` - detailed process-interaction trace

---

## 🗺️ Visual Architecture Diagram (Process Interaction Method)

Here's the complete system architecture:

```
═══════════════════════════════════════════════════════════════════════════════
                        MULTI-TELLER BANK SIMULATION
                        (Process Interaction Method)
═══════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────┐
│                            USER'S PROGRAM (main)                            │
│                                                                             │
│  1. Read parameters (min/max tellers, arrival/service rates)                │
│  2. For each num_tellers (4 to 7):                                          │
│     ├─ Initialize simlib & process system                                   │
│     ├─ Create teller processes (one per teller)                             │
│     ├─ Schedule first customer arrival                                      │
│     ├─ Schedule bank closing (8 hours)                                      │
│     └─ Run TIMING ROUTINE ───────────────┐                                  │
│  3. Generate report                      │                                  │
└──────────────────────────────────────────┼──────────────────────────────────┘
                                           │
                                           ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                             TIMING ROUTINE                                  │
│                         (Main Simulation Loop)                              │
│                                                                             │
│  while (FES not empty):                                                     │
│    ┌─────────────────────────────────────────────────────────────┐          │
│    │ 1. Check CES for processes to activate (if any)             │          │
│    │    └─> Tellers waiting for customers                        │          │
│    │                                                             │          │
│    │ 2. Get next event from FES (timing())                       │          │
│    │    └─> EVENT_ARRIVAL or EVENT_SERVICE_DONE                  │          │
│    │                                                             │          │
│    │ 3. Advance simulation time                                  │          │
│    │                                                             │          │
│    │ 4. Execute process based on event type:                     │          │
│    │    ├─ EVENT_ARRIVAL ────────> arrival_process()             │          │
│    │    │                              │                         │          │
│    │    │                              └─> customer_process()    │          │
│    │    │                                                        │          │
│    │    ├─ EVENT_SERVICE_DONE ───> teller_process(teller_num)    │          │
│    │    │                                                        │          │
│    │    └─ EVENT_CLOSE_DOORS ───> Stop new arrivals              │          │
│    │                                                             │          │
│    │ 5. Update process states (Active/Scheduled/Conditional)     │          │
│    │                                                             │          │
│    │ 6. Continue loop                                            │          │
│    └─────────────────────────────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════
                              PROCESS TYPES
═══════════════════════════════════════════════════════════════════════════════

┌────────────────────────────────┬─────────────────────────────────────────┐
│   CUSTOMER PROCESS             │        TELLER PROCESS                   │
│   (One per arrival)            │        (One per teller, persistent)     │
├────────────────────────────────┼─────────────────────────────────────────┤
│                                │                                         │
│     ┌──────────────────┐       │  ┌──────────────────┐                   │
│     │ Create customer  │       │  │  Teller created  │                   │
│     │     record       │       │  │ at initialization│                   │
│     │  (PCB with ID,   │       │  │   (PCB stored)   │                   │
│     │  arrival time)   │       │  └────────┬─────────┘                   │
│     └────────┬─────────┘       │           │                             │
│              ↓                 │           ↓                             │
│     ┌──────────────────┐       │  ┌──────────────────┐                   │
│     │ Find idle teller │       │  │ Check queue      │◄──────────┐       │
│     │ (scan all        │       │  │   empty?         │           │       │
│     │  tellers)        │       │  └────────┬─────────┘           │       │
│     └────────┬─────────┘       │           ↓                     │       │
│              ↓                 |      ┌─────────────┐            |       │
|       ┌─────────────┐          │ Empty│    Not Empty│            │       │
│ Found │        None |          │      │             │            │       │
│       ↓             ↓          │      ↓             ↓            │       │
│  ┌──────────┐ ┌──────────┐     │  ┌──────────┐ ┌──────────┐      │       │
│  │Activate  │ │   Join   │     │  │ Put in   │ │ Remove   │      │       │
│  │ teller   │ │ shortest │     │  │   CES    │ │ customer │      │       │
│  │          │ │  queue   │     │  │  (wait)  │ │from queue│      │       │
│  └────┬─────┘ └────┬─────┘     │  └────┬─────┘ └────┬─────┘      │       │
│       │            │           │       │            │            │       │
│       │            │           │       ↓            ↓            │       │
│       │            │           │  [Customer      ┌──────────┐    │       │
│       │            │           │   activates] ──>│Determine │    │       │
│       │            │           │                 │ service  │    │       │
│       │            │           │                 │   time   │    │       │
│       └──────┬─────┘           │                 └────┬─────┘    │       │
│              ↓                 │                      ↓          │       │
│    ┌──────────────────┐        │           ┌──────────────────┐  │       │
│    │ Wait for service │        │           │ Schedule service │  │       │
│    │   completion     │        │           │   completion     │  │       │
│    │  (implicit via   │        │           │   (FES event)    │  │       │
│    │  delay recording)│        │           └────┬─────────────┘  │       │
│    └────────┬─────────┘        │                ↓                │       │
│             ↓                  │      ┌──────────────────┐       │       │
│    ┌──────────────────┐        │      │ Wait for service │       │       │
│    │ Service complete │        │      │   completion     │       │       │
│    │ (delay recorded) │        │      └─────────┬────────┘       │       │
│    └────────┬─────────┘        │                ↓                │       │
│             ↓                  │      [Service completes]        │       │
│    ┌──────────────────┐        │                ↓                │       │
│    │Destroy customer  │        │      ┌──────────────────┐       │       │
│    │     record       │        │      │ Activate customer│       │       │
│    │      (PCB)       │        │      │  (implicit via   │       │       │
│    └──────────────────┘        │      │  delay record)   │       │       │
│             │                  │      └─────────┬────────┘       │       │
│           [END]                │                ↓                │       │
│                                |      ┌──────────────────┐       │       │
│                                │      │  Check jockeying │       │       │
│                                │      │  (balance queues)│       │       │
│                                │      └─────────┬────────┘       │       │
│                                │                │                │       │
│                                │                └────────────────┘       │
│                                │                                         │
└────────────────────────────────┴─────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════
                           PROCESS STATE MANAGEMENT
═══════════════════════════════════════════════════════════════════════════════

┌────────────────────────────────────────────────────────────────────────────┐
│                      PROCESS CONTROL BLOCK (PCB)                           │
│                                                                            │
│  ┌───────────────────────────────────────────────────────────────┐         │
│  │ struct ProcessControlBlock {                                  │         │
│  │    int   process_id;         // Unique identifier             │         │
│  │    int   process_type;       // CUSTOMER, TELLER, ARRIVAL     │         │
│  │    int   state;              // Current state                 │         │
│  │    float reactivation_time;  // When to wake up               │         │
│  │    int   teller_number;      // For teller processes          │         │
│  │    float arrival_time;       // For customer processes        │         │
│  │    int   resume_point;       // Where to continue execution   │         │
│  │ }                                                             │         │
│  └───────────────────────────────────────────────────────────────┘         │
│                                                                            │
│  PROCESS STATES:                                                           │
│                                                                            │
│   ┌──────────────┐                                                         │
│   │   ACTIVE     │  Currently executing code                               │
│   └──────┬───────┘                                                         │
│          │                                                                 │
│          ├─────────────────────┬────────────────────┐                      │
│          ↓                     ↓                    ↓                      │
│   ┌──────────────┐      ┌──────────────┐    ┌──────────────┐               │
│   │  SCHEDULED   │      │ CONDITIONAL  │    │ TERMINATED   │               │
│   │              │      │              │    │              │               │
│   │  In FES      │      │  In CES      │    │  Completed   │               │
│   │  Waiting for │      │  Waiting for │    │              │               │
│   │  time event  │      │  activation  │    └──────────────┘               │
│   └──────┬───────┘      └──────┬───────┘                                   │
│          │                     │                                           │
│          └──────────┬──────────┘                                           │
│                     ↓                                                      │
│              ┌──────────────┐                                              │
│              │   ACTIVE     │  Reactivated                                 │
│              └──────────────┘                                              │
└─────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════
                            DATA STRUCTURES
═══════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────┐
│                         SIMLIB LISTS USAGE                                  │
│                                                                             │
│  List 1:             Queue for Teller 1                                     │
│  List 2:             Queue for Teller 2                                     │
│  List 3:             Queue for Teller 3                                     │
│  ...                 ...                                                    │
│  List num_tellers:   Queue for Teller num_tellers                           │
│                                                                             │
│  List (num_tellers+1): Teller 1 busy indicator                              │
│  List (num_tellers+2): Teller 2 busy indicator                              │
│  ...                   ...                                                  │
│  List (2*num_tellers): Teller num_tellers busy indicator                    │
│                                                                             │
│  LIST_CES:           Conditional Event Set (idle tellers)                   │
│  LIST_EVENT:         Future Event Set (simlib managed)                      │
│                                                                             │
│                                                                             │
│  EXAMPLE WITH 5 TELLERS:                                                    │
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────┐          │
│  │  List 1: [Cust A] [Cust B] [Cust C] ◄─ Teller 1 Queue         │          │
│  │  List 2: [Cust D]                    ◄─ Teller 2 Queue        │          │
│  │  List 3: []                          ◄─ Teller 3 Queue        │          │
│  │  List 4: [Cust E] [Cust F]           ◄─ Teller 4 Queue        │          │
│  │  List 5: []                          ◄─ Teller 5 Queue        │          │
│  ├───────────────────────────────────────────────────────────────┤          │
│  │  List 6: [Busy]                      ◄─ Teller 1 Status       │          │
│  │  List 7: [Busy]                      ◄─ Teller 2 Status       │          │
│  │  List 8: []                          ◄─ Teller 3 Status (idle)│          │
│  │  List 9: [Busy]                      ◄─ Teller 4 Status       │          │
│  │  List 10: []                         ◄─ Teller 5 Status (idle)│          │
│  ├───────────────────────────────────────────────────────────────┤          │
│  │  LIST_CES: [Teller 3 PCB] [Teller 5 PCB] ◄─ Waiting tellers   │          │
│  ├───────────────────────────────────────────────────────────────┤          │
│  │  LIST_EVENT (FES):                                            │          │
│  │    [time=485.3, EVENT_ARRIVAL]        ◄─ Next customer        │          │
│  │    [time=487.1, EVENT_SERVICE_DONE]   ◄─ Teller 1 finishes    │          │
│  │    [time=489.5, EVENT_SERVICE_DONE]   ◄─ Teller 2 finishes    │          │
│  │    [time=491.2, EVENT_SERVICE_DONE]   ◄─ Teller 4 finishes    │          │
│  └───────────────────────────────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════
                           SYNCHRONIZATION EXAMPLE
═══════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────┐
│  SCENARIO: Customer arrives, Teller 3 is idle                               │
│                                                                             │
│  Time = 485.3                                                               │
│                                                                             │
│  1. EVENT_ARRIVAL triggers                                                  │
│     └─> arrival_process()                                                   │
│         └─> customer_process()                                              │
│             │                                                               │
│             ├─> find_idle_teller()                                          │
│             │   Returns: Teller 3 PCB (found in CES)                        │
│             │                                                               │
│             ├─> sampst(0.0, DELAYS)  // Zero delay                          │
│             │                                                               │
│             ├─> list_file(FIRST, 8)  // Make Teller 3 busy                  │
│             │                                                               │
│             ├─> remove_from_ces(Teller 3 PCB)                               │
│             │   // Remove Teller 3 from CES                                 │
│             │                                                               │
│             └─> event_schedule(485.3 + 4.5, EVENT_SERVICE_DONE)             │
│                 transfer[3] = 3  // Teller number                           │
│                                                                             │
│  2. Customer process completes (PCB destroyed)                              │
│                                                                             │
│  3. Timing routine continues                                                │
│                                                                             │
│  Time = 489.8 (485.3 + 4.5)                                                 │
│                                                                             │
│  4. EVENT_SERVICE_DONE triggers                                             │
│     └─> teller_process(3)                                                   │
│         │                                                                   │
│         ├─> Check queue 3: empty                                            │
│         │                                                                   │
│         ├─> list_remove(FIRST, 8)  // Make Teller 3 idle                    │
│         │                                                                   │
│         ├─> put_in_ces(Teller 3 PCB)                                        │
│         │   // Put Teller 3 back in CES to wait                             │
│         │                                                                   │
│         └─> jockey(3)                                                       │
│             // Check if customers should jockey to Teller 3                 │
│                                                                             │
│  5. Teller 3 now waiting in CES for next customer                           │
└─────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════
                              KEY FUNCTIONS
═══════════════════════════════════════════════════════════════════════════════

┌────────────────────────────────┬──────────────────────────────────────────┐
│  PROCESS MANAGEMENT            │  SIMULATION FUNCTIONS                    │
├────────────────────────────────┼──────────────────────────────────────────┤
│ create_process()               │ timing()          - Get next event       │
│ destroy_process()              │ event_schedule()  - Schedule event       │
│ schedule_process()             │ list_file()       - Add to list          │
│ activate_process_now()         │ list_remove()     - Remove from list     │
│ wait_conditional()             │ sampst()          - Record statistic     │
│ put_in_ces()                   │ filest()          - Queue statistic      │
│ remove_from_ces()              │ expon()           - Random number        │
│ find_idle_teller()             │                                          │
│ get_teller_process()           │                                          │
└────────────────────────────────┴──────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════
                             EXECUTION FLOW
═══════════════════════════════════════════════════════════════════════════════

Bank opens (time = 0)
  │
  ├─> Create 4-7 teller processes (depending on configuration)
  │   Each teller put in CES (waiting for customers)
  │
  ├─> Schedule first customer arrival (time = expon(1.0))
  │
  └─> Schedule bank closing (time = 480 minutes = 8 hours)

Main Loop (until FES empty):
  │
  ├─> Get next event (timing())
  │
  ├─> Advance time to event time
  │
  └─> Process event:
      │
      ├─> ARRIVAL:
      │   ├─> Customer arrives
      │   ├─> Schedule next arrival
      │   ├─> Find idle teller or join shortest queue
      │   └─> Start service if teller idle
      │
      ├─> SERVICE_DONE:
      │   ├─> Teller completes service
      │   ├─> Check queue for next customer
      │   ├─> Start next service or go idle
      │   └─> Check for jockeying opportunities
      │
      └─> CLOSE_DOORS:
          └─> Stop scheduling new arrivals

Bank closes (time = 480)
  │
  └─> Continue until all customers served

Generate report
  │
  ├─> Average number in queue (across all tellers)
  ├─> Average delay in queue
  └─> Maximum delay observed

═══════════════════════════════════════════════════════════════════════════════
```

---

## 🧭 Simulation Flowchart (Process Interaction Method)

Based on the attached `FLOWCHART.md`, here's the detailed flowchart:

### 1. MAIN PROGRAM (USER'S PROGRAM ROUTINE)

```
                    ┌──────────────────┐
                    │   START MAIN     │
                    └────────┬─────────┘
                             ↓
                    ┌──────────────────┐
                    │ Read Parameters: │
                    │ - min_tellers    │
                    │ - max_tellers    │
                    │ - mean_arrival   │
                    │ - mean_service   │
                    │ - num_customers  │
                    └────────┬─────────┘
                             ↓
                    ┌──────────────────┐
                    │   Open Input &   │
                    │   Output Files   │
                    └────────┬─────────┘
                             ↓
        ┌──────────────────────┴─────────────────────┐
        │FOR num_tellers = min_tellers TO max_tellers│
        │                                            │
        │  ┌──────────────────────────────────────┐  │
        │  │ Initialize simlib                    │  │
        │  │ - init_simlib()                      │  │
        │  │ - create_event_lists()               │  │
        │  │ - init_process_system()              │  │
        │  └──────────────────┬───────────────────┘  │
        │                     ↓                      │
        │  ┌──────────────────────────────────────┐  │
        │  │ Create num_tellers TELLER PROCESSES  │  │
        │  │ FOR each teller (1 to num_tellers):  │  │
        │  │  - Create teller PCB                 │  │
        │  │  - Put teller in CES (waiting)       │  │
        │  └──────────────────┬───────────────────┘  │
        │                     ↓                      │
        │  ┌──────────────────────────────────────┐  │
        │  │ Schedule Initial Events:             │  │
        │  │  - Event: FIRST CUSTOMER ARRIVAL     │  │
        │  │    Time: expon(mean_arrival)         │  │
        │  │  - Event: DOORS CLOSE                │  │
        │  │    Time: 60 * length_doors_open      │  │
        │  └──────────────────┬───────────────────┘  │
        │                     ↓                      │
        │  ┌──────────────────────────────────────┐  │
        │  │    CALL TIMING ROUTINE               │  │
        │  │    (Main Simulation Loop)            │  │
        │  │    [see section 4 for details]       │  │
        │  └──────────────────┬───────────────────┘  │
        │                     ↓                      │
        │  ┌──────────────────────────────────────┐  │
        │  │ PRODUCE SUMMARY STATISTICS           │  │
        │  │ - Total delays                       │  │
        │  │ - Average delay                      │  │
        │  │ - Teller utilization                 │  │
        │  │ - Total customers served             │  │
        │  └──────────────────┬───────────────────┘  │
        │                     │                      │
        └─────────────────────┴──────────────────────┘
                              ↓
                    ┌──────────────────┐
                    │  Close Files     │
                    └────────┬─────────┘
                             ↓
                    ┌──────────────────┐
                    │      END         │
                    └──────────────────┘
```

### 2. TIMING ROUTINE (MAIN EVENT LOOP)

```
        ┌─────────────────────────────────┐
        │   TIMING ROUTINE / EVENT LOOP   │
        └────────────┬────────────────────┘
                     ↓
        ┌─────────────────────────────────         ┐
        │  WHILE (FES not empty)                   │
        │  [Event list has events]                 │
        │                                          │
        │  ┌───────────────────────────┐           │
        │  │ Select next event from    │           │
        │  │ Future Event Schedule     │           │
        │  │ Remove it from FES        │           │
        │  └────────┬──────────────────┘           │
        │           ↓                              │
        │  ┌───────────────────────────┐           │
        │  │ Advance simulation clock: │           │
        │  │ sim_time = event_time     │           │
        │  └────────┬──────────────────┘           │
        │           ↓                              │
        │  ┌───────────────────────────┐           │
        │  │  SWITCH (event_type)      │           │
        │  └────────┬──────────────────┘           │
        │           ↓                              │
        │  ┌────────────────────────────────────┐  │
        │  │ EVENT_ARRIVAL                      │  │
        │  │ ├─ Decrement customers remaining   │  │
        │  │ ├─ IF (doors still open)           │  │
        │  │ │   Schedule next arrival          │  │
        │  │ ├─ Create CUSTOMER PROCESS (PCB)   │  │
        │  │ ├─ CALL customer_process()         │  │
        │  │ │  [see section 3]                 │  │
        │  │ └─ Customer process continues...   │  │
        │  └─────────┬──────────────────────────┘  │
        │            ↓                             │
        │  ┌────────────────────────────────────┐  │
        │  │ EVENT_SERVICE_DONE                 │  │
        │  │ ├─ Get teller_num from transfer[3] │  │
        │  │ ├─ Get teller PCB                  │  │
        │  │ ├─ CALL teller_process(teller_num) │  │
        │  │ │  [see section 4]                 │  │
        │  │ └─ Teller continues...             │  │
        │  └─────────┬──────────────────────────┘  │
        │            ↓                             │
        │  ┌────────────────────────────────────┐  │
        │  │ EVENT_CLOSE_DOORS                  │  │
        │  │ ├─ Set doors_open = FALSE          │  │
        │  │ ├─ Cancel remaining arrivals       │  │
        │  │ └─ Tellers finish remaining work   │  │
        │  └─────────┬──────────────────────────┘  │
        │            ↓                             │
        │  ┌───────────────────────────┐           │
        │  │ END SWITCH                │           │
        │  └────────┬──────────────────┘           │
        │           ↓                              │
        │  ┌───────────────────────────┐           │
        │  │ Check process states:     │           │
        │  │ - Conditional wait (CES)  │           │
        │  │ - Time wait (FES)         │           │
        │  │ - Process termination     │           │
        │  └────────┬──────────────────┘           │
        │           │                              │
        └───────────┴──────────────────────────────┘
                    ↓
        ┌─────────────────────────────────┐
        │  RETURN from Timing Routine     │
        └─────────────────────────────────┘
```

### 3. CUSTOMER ARRIVAL PROCESS

```
        ┌──────────────────────────────┐
        │   EVENT_ARRIVAL TRIGGERED    │
        │   CALL arrival_process()     │
        └────────┬─────────────────────┘
                 ↓
        ┌──────────────────────────────┐
        │ Determine next arrival time: │
        │ next_arrival_time =          │
        │   sim_time + expon(          │
        │     mean_arrival,            │
        │     STREAM_ARRIVAL)          │
        └────────┬─────────────────────┘
                 ↓
        ┌──────────────────────────────┐
        │ Schedule next arrival event  │
        │ (if doors still open)        │
        │                              │
        │ event_schedule(              │
        │   next_arrival_time,         │
        │   EVENT_ARRIVAL)             │
        └────────┬─────────────────────┘
                 ↓
        ┌──────────────────────────────┐
        │ CREATE CUSTOMER PROCESS:     │
        │ - Create PCB for customer    │
        │ - Record arrival time        │
        │ - Set customer ID            │
        └────────┬─────────────────────┘
                 ↓
        ┌──────────────────────────────┐
        │ CALL customer_process()      │
        │ [See next section]           │
        └──────────────────────────────┘
```

### 4. CUSTOMER PROCESS ROUTINE

```
        ┌──────────────────────────────┐
        │   CUSTOMER PROCESS CREATED   │
        └────────┬─────────────────────┘
                 ↓
        ┌──────────────────────────────┐
        │ Record Customer Data:        │
        │ - arrival_time = sim_time    │
        │ - customer_id                │
        │ - Store in PCB               │
        └────────┬─────────────────────┘
                 ↓
        ┌──────────────────────────────┐
        │   CHECK FOR IDLE TELLER:     │
        │   idle_teller =              │
        │   find_idle_teller()         │
        │   [scans all tellers]        │
        └────────┬─────────────────────┘
                 ↓
        ┌─────────────────┐
        │  Idle Teller?   │
        └────────┬────────┘
                 ↓
    ┌────────────────────────────┐
    │                            │
   YES                          NO
    │                            │
    ↓                            ↓
┌─────────────────────┐  ┌───────────────────────┐
│ YES: Teller Idle    │  │ NO: Join Queue        │
├─────────────────────┤  ├───────────────────────┤
│ ┌─────────────────┐ │  │ ┌───────────────────┐ │
│ │ Find idle teller│ │  │ │Find shortest queue│ │
│ │ by number       │ │  │ │among all tellers  │ │
│ └────────┬────────┘ │  │ │(load balancing)   │ │
│          ↓          │  │ └────────┬──────────┘ │
│ ┌─────────────────┐ │  │          ↓            │
│ │ Record delay=0  │ │  │ ┌──────────────────┐  │
│ │ sampst(0.0)     │ │  │ │ Move customer to │  │
│ └────────┬────────┘ │  │ │ selected queue:  │  │
│          ↓          │  │ │ list_file(LAST)  │  │
│ ┌─────────────────┐ │  │ │                  │  │
│ │ Store customer  │ │  │ │ Set status to:   │  │
│ │ info in         │ │  │ │ WAITING_IN_QUEUE │  │
│ │ transfer array  │ │  │ └────────┬─────────┘  │
│ └────────┬────────┘ │  │          ↓            │
│          ↓          │  │ ┌──────────────────┐  │
│ ┌─────────────────┐ │  │ │WAIT to be removed│  │
│ │ Remove teller   │ │  │ │from queue by     │  │
│ │ from CES        │ │  │ │teller process    │  │
│ │ (activate it)   │ │  │ │                  │  │
│ └────────┬────────┘ │  │ │[Suspends until   │  │
│          ↓          │  │ │teller removes    │  │
│ ┌─────────────────┐ │  │ │customer]         │  │
│ │ Determine       │ │  │ └──────────────────┘  │
│ │ service time:   │ │  │          │            │
│ │ service_time =  │ │  │          ↓            │
│ │ expon(mean)     │ │  │ ┌──────────────────┐  │
│ └────────┬────────┘ │  │ │When teller       │  │
│          ↓          │  │ │removes customer: │  │
│ ┌─────────────────┐ │  │ │ Record delay:    │  │
│ │ Schedule service│ │  │ │ sim_time -       │  │
│ │ completion:     │ │  │ │ arrival_time     │  │
│ │ event_schedule( │ │  │ └──────────────────┘  │
│ │  sim_time +     │ │  │          │            │
│ │  service_time,  │ │  │          ↓            │
│ │  EVENT_SERVICE_ │ │  │ ┌──────────────────┐  │
│ │  DONE)          │ │  │ │ Receive service  │  │
│ └────────┬────────┘ │  │ │ Schedule with    │  │
│          ↓          │  │ │ teller           │  │
│ ┌─────────────────┐ │  │ └──────────────────┘  │
│ │ WAIT for service│ │  │          │            │
│ │ completion      │ │  │          ↓            │
│ │ (Suspends)      │ │  │ ┌──────────────────┐  │
│ └────────┬────────┘ │  │ │ TERMINATE        │  │
│          ↓          │  │ │ CUSTOMER PROCESS │  │
│ ┌─────────────────┐ │  │ │                  │  │
│ │ Service Event   │ │  │ │ [PCB destroyed]  │  │
│ │ Scheduled →     │ │  │ └──────────────────┘  │
│ │ EVENT_SERVICE_  │ │  └───────────────────────┘
│ │ DONE triggers   │ │
│ │[Teller process] │ │
│ └────────┬────────┘ │
│          ↓          │
│ ┌─────────────────┐ │
│ │ TERMINATE       │ │
│ │ CUSTOMER        │ │
│ │ PROCESS         │ │
│ │                 │ │
│ │[PCB destroyed]  │ │
│ └─────────────────┘ │
└─────────────────────┘
```

### 5. TELLER PROCESS ROUTINE

```
        ┌──────────────────────────────────┐
        │   TELLER PROCESS CREATED         │
        │   Initialized in CES (waiting)   │
        └────────┬─────────────────────────┘
                 ↓
        ┌──────────────────────────────────┐
        │  EVENT_SERVICE_DONE or           │
        │  Customer activates idle teller  │
        │  [Teller activated from CES]     │
        └────────┬─────────────────────────┘
                 ↓
        ┌──────────────────────────────────┐
        │   TELLER ACTIVE:                 │
        │   Check own queue                │
        │   [list_size[teller_num]]        │
        └────────┬─────────────────────────┘
                 ↓
        ┌─────────────────────────┐
        │   Queue Empty?          │←──────────────────┐
        └────────┬────────────────┘                   │
                 ↓                                    │
    ┌────────────────────────────┐                    │
    │                            │                    │
   YES                          NO                    │
    │                            │                    │
    ↓                            ↓                    │
┌────────────────────────┐ ┌───────────────────────┐  │
│ YES: Queue Empty       │ │ NO: Customers Waiting │  │
├────────────────────────┤ ├───────────────────────┤  │
│ ┌────────────────────┐ │ │ ┌───────────────────┐ │  │
│ │ Make teller idle:  │ │ │ │ Remove FIRST      │ │  │
│ │ Remove from busy   │ │ │ │ customer from     │ │  │
│ │ list               │ │ │ │ queue:            │ │  │
│ │ list_remove(FIRST) │ │ │ │ list_remove(FIRST)│ │  │
│ └────────┬───────────┘ │ │ └────────┬──────────┘ │  │
│          ↓             │ │          ↓            │  │
│ ┌────────────────────┐ │ │ ┌───────────────────┐ │  │
│ │ Put teller in CES: │ │ │ │ Record customer's │ │  │
│ │ put_in_ces()       │ │ │ │ delay in system:  │ │  │
│ │                    │ │ │ │                   │ │  │
│ │ [Conditional wait] │ │ │ │ delay = sim_time -│ │  │
│ │ [Suspends until    │ │ │ │         arrival   │ │  │
│ │  customer arrives] │ │ │ │                   │ │  │
│ │                    │ │ │ │ sampst(delay,     │ │  │
│ │                    │ │ │ │ SAMPST_DELAYS)    │ │  │
│ └────────┬───────────┘ │ │ └────────┬──────────┘ │  │
│          ↓             │ │          ↓            |  │
│ ┌────────────────────┐ │ │ ┌───────────────────┐ │  │
│ │ Wait in CES...     │ │ │ │ Determine service │ │  │
│ │                    │ │ │ │ time:             │ │  │
│ │ [Next arrival or   │ │ │ │                   │ │  │
│ │  jockey event]     │ │ │ │ service_time =    │ │  │
│ │                    │ │ │ │expon(mean_service)│ │  │
│ │ [Return here when] │ │ │ │                   │ │  │
│ │ [reactivated]      │ │ │ │                   │ │  │
│ │                    │ │ │ │                   │ │  │
│ │ [Loop back to      │ │ │ └────────┬──────────┘ │  │
│ │  check queue again]│ │ │          ↓            │  │
│ └────────────────────┘ │ │ ┌───────────────────┐ │  │
└──────────┬─────────────┘ │ │ Schedule service  │ │  │
           │               │ │ completion:       │ │  │
           │               │ │                   │ │  │
           │               │ │ event_schedule(   │ │  │
           │               │ │  sim_time +       │ │  │
           │               │ │  service_time,    │ │  │
           │               │ │  EVENT_SERVICE_   │ │  │
           │               │ │  DONE)            │ │  │
           │               │ └────────┬──────────┘ │  │
           │               │          ↓            │  │
           │               │ ┌───────────────────┐ │  │
           │               │ │ WAIT for service  │ │  │
           │               │ │ completion        │ │  │
           │               │ │ (event scheduled) │ │  │
           │               │ │                   │ │  │
           │               │ │ [Suspends: returns│ │  │
           │               │ │  to timing loop]  │ │  │
           │               │ └────────┬──────────┘ │  │
           │               │          ↓            │  │
           │               │ ┌────────────────────┐│  │
           │               │ │ SERVICE COMPLETES: ││  │
           │               │ │ EVENT_SERVICE_DONE ││  │
           │               │ │                    ││  │
           │               │ │[Teller reactivated]││  │
           │               │ │                    ││  │
           │               │ │ Loop back: Check   ││  │
           │               │ │ queue again        ││  │
           │               │ └────────────────────┘│  │
           │               └──────────┬────────────┘  │
           └────────┬─────────────────┘               │
                    │                                 │
                    ↓                                 │
           ┌────────────────────┐                     │
           │ CHECK for JOCKEYING│                     │
           │                    │                     │
           │ After service:     │                     │
           │ Check if customers │                     │
           │ in other queues    │                     │
           │ should move to     │                     │
           │ this queue         │                     │
           │                    │                     │
           │ [Optional feature] │                     │
           └─────────┬──────────┘                     │
                     └────────────────────────────────┘

```

### 6. PROCESS INTERACTION DIAGRAM

```
                    ┌─────────────────┐
                    │  TIMING ROUTINE │
                    │  (Main Loop)    │
                    └────────┬────────┘
                             │
                ┌────────────┼────────────┐
                │            │            │
                ↓            ↓            ↓
        ┌─────────────┐ ┌───────────┐ ┌─────────────┐
        │  EVENT_     │ │  EVENT_   │ │ EVENT_CLOSE │
        │ ARRIVAL     │ │ SERVICE_  │ │ _DOORS      │
        │             │ │ DONE      │ │             │
        └──────┬──────┘ └─────┬─────┘ └──────┬──────┘
               │              │              │
               ↓              │              │
        ┌─────────────────┐   │              │
        │  ARRIVAL_       │   │              │
        │ PROCESS()       │   │              │
        │                 │   │              │
        │ - Schedule next │   │              │
        │   arrival       │   │              │
        │ - Create customer   │              │
        │ - Call customer_    │              │
        │   process()         │              │
        └─────────────────┘   │              │
               │              │              │
               ├──────────────┤              │
               │              │              │
               ↓              ↓              │
        ┌──────────────────────────────┐     │
        │ CUSTOMER PROCESS ROUTINE     │     │
        │                              │     │
        │ 1. Check for idle teller     │     │
        │ 2a. If idle:                 │     │
        │  - Activate teller now       │     │
        │  - Schedule service          │     │
        │  - WAIT (suspended)          │     │
        │ 2b. If no idle:              │     │
        │  - Join shortest queue       │     │
        │  - WAIT in queue (suspended) │     │
        │ 3. Receive service from      │     │
        │    teller (when removed)     │     │
        │ 4. TERMINATE                 │     │
        └──────────┬───────────────────┘     │
                   │                         │
                   │                         │
        ┌──────────┴──────────────────────┐  │
        │                                 │  │
        ↓                                 │  │
   ┌─────────────────────┐                │  │
   │ TELLER PROCESS      │                │  │
   │ (Multiple instances)│                │  │
   │                     │                │  │
   │ Loop:               │                │  │
   │ 1. Check queue      │←───────────────┘  │
   │ 2a. If empty:       │       (from set)  │
   │  - Get from CES     │    doors_open=0   │
   │  - WAIT             │                   │
   │ 2b. If customers:   │                   │
   │  - Receive customer │                   │
   │  - Determine service│                   │
   │  - Schedule event   │                   │
   │  - WAIT             │                   │
   │ 3. Loop back        │                   │
   └─────────────────────┘                   │
                                             │
                                             ↓
                                     ┌──────────────┐
                                     │ doors_open   │
                                     │ = FALSE      │
                                     │              │
                                     │ Stop arrivals│
                                     └──────────────┘
```

### 7. PROCESS STATE TRANSITIONS

```
                   CUSTOMER PROCESS

                  ┌──────────────┐
                  │  CREATED     │
                  └──────┬───────┘
                         ↓
                    ┌─────────────┐
                    │   ACTIVE    │
                    │ (Checking   │
                    │  tellers)   │
                    └──────┬──────┘
                           │
        ┌──────────────────┴──────────────────┐
        │                                     │
        ↓                                     ↓
   ┌──────────────────┐              ┌──────────────┐
   │ TIME WAIT (FES)  │              │ TIME WAIT    │
   │ [Idle teller]    │              │ [In queue]   │
   │                  │              │              │
   │ Service scheduled│              │ Waiting to   │
   │ in FES           │              │ be removed   │
   │                  │              │              │
   │ [Suspended]      │              │ [Suspended]  │
   │                  │              │              │
   │ When service     │              │ When teller  │
   │ completes:       │              │ removes:     │
   └────────┬─────────┘              └────────┬─────┘
            │                                 │
            │                                 │
            ↓                                 ↓
        ┌──────────────────┐          ┌──────────────┐
        │  TERMINATED      │          │  TERMINATED  │
        │  [PCB destroyed] │          │  [PCB freed] │
        └──────────────────┘          └──────────────┘


                   TELLER PROCESS

                  ┌──────────────┐
                  │  CREATED     │
                  │ (Init)       │
                  └──────┬───────┘
                         ↓
              ┌───────────────────────┐
              │ CONDITIONAL WAIT (CES)│
              │ [Initial: waiting]    │
              └──────┬────────────────┘
                     │
        ┌────────────┴────────────┐
        │                         │
        ↓                         │
   ┌────────────────────┐         │
   │ ACTIVE             │         │
   │ (Customer arrived) │         │
   └──────┬─────────────┘         │
          │                       │
          ├──────┬────────────────┤
          │      │                │
       Queue  Empty  Not Empty    │
       check  │        │          │
          │   │        │          │
          ↓   ↓        ↓          │
   ┌──────────────────────────┐   │
   │ CONDITIONAL WAIT (CES)   │───┘
   │ or TIME WAIT (FES)       │
   │                          │
   │ IF queue empty:          │
   │  → CES (wait for arrival)│
   │                          │
   │ IF queue has customers:  │
   │  → FES (wait for service │
   │       completion)        │
   │                          │
   │ [Suspended: returns to   │
   │  timing loop]            │
   │                          │
   │ When reactivated:        │
   └────────────┬─────────────┘
                │
                │ Loop back to
                │ check queue
                │
                ↓
        ┌──────────────────────┐
        │ SERVICE COMPLETES    │
        │ EVENT_SERVICE_DONE   │
        │ triggered            │
        └──────────────────────┘
```

### 8. KEY DATA STRUCTURES

```
LISTS (used for queues):
├─ list 1: Teller 1 queue
├─ list 2: Teller 2 queue
├─ list 3: Teller 3 queue
├─ ...
├─ list N: Teller N queue
├─ list (N+1): Busy teller 1
├─ list (N+2): Busy teller 2
├─ ...
└─ list (2N): Busy teller N

CES (Conditional Event Set):
├─ All idle tellers waiting for customers
└─ Customers waiting in queues (implicitly in lists)

FES (Future Event Set):
├─ Scheduled customer arrivals
├─ Scheduled service completions
└─ Scheduled door closure

TRANSFER ARRAY (Inter-process Communication):
├─ transfer[1] = Customer arrival time
├─ transfer[3] = Teller number
└─ Other fields as needed

PROCESS CONTROL BLOCKS (PCBs):
├─ Customer PCBs: One per customer
│  └─ Stores: arrival_time, customer_id
├─ Teller PCBs: One per teller
│  └─ Stores: teller_num, idle_status
└─ Used to track process state
```

### 9. SIMULATION STATISTICS COLLECTION

```
SAMPST - samples taken:
├─ SAMPST_DELAYS
│  └─ Customer delay in system (queue + service)
└─ SAMPST_TELLER_UTIL
   └─ Teller utilization

EVENT_COUNT - events recorded:
├─ Arrivals
├─ Service completions
└─ Door closures

SUMMARY STATISTICS calculated:
├─ Average delay per customer
├─ Maximum delay observed
├─ Minimum delay observed
├─ Total customers served
├─ Total time in system
├─ Teller utilization percentages
└─ For each configuration (1-7 tellers)
```

---

## 💡 Tips

- Default configuration is 4 to 7 tellers, with 8 hour bank day.
- The sample output file (`out/sample_mtbank.out`) demonstrates results already generated.
- If you need multi-run averaging, run the script repeatedly and aggregate outputs externally.
