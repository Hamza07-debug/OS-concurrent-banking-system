# Concurrent Banking System

A multithreaded banking system simulation written in C, built to demonstrate core Operating Systems concepts — process/thread scheduling, synchronization, deadlock avoidance, inter-process communication, and memory management — through a realistic banking scenario.

## 📌 Overview

This project simulates a bank handling multiple concurrent customer requests (deposits, withdrawals, loan applications, payroll processing) using real OS-level mechanisms rather than simplified abstractions. Each customer type interacts with the system differently, allowing the project to showcase distinct OS concepts in a single, cohesive application.

## 👥 Customer Types

| Type | Behavior |
|---|---|
| Regular | Standard deposits/withdrawals |
| Premium | Higher scheduling priority |
| Loan Applicant | Requests evaluated via Banker's Algorithm |
| Corporate Client | Spawns multiple threads (e.g., payroll for multiple employees) |
| VIP | Highest scheduling priority, preempts other transactions |

## 🛠️ Core OS Concepts Implemented

### 1. Threading & Scheduling
Each customer request is handled as a separate thread. Execution order is determined by one of the following scheduling algorithms:
- **FCFS (First Come First Serve)**
- **Priority Scheduling** (VIP > Premium > Regular)
- **Round Robin** (fixed time-slice execution)

Includes Gantt chart generation and calculation of waiting time / turnaround time metrics.

### 2. Synchronization
Shared resources (account balances) are protected using:
- **Mutex locks** — ensures only one thread modifies a balance at a time
- **Semaphores** — allows a limited number of concurrent operations (e.g., simultaneous deposits)

### 3. Deadlock Handling
Loan requests are treated as resource allocation problems and evaluated using the **Banker's Algorithm**, which checks whether granting a loan would leave the system in an unsafe state before approving it.

### 4. Inter-Process Communication (IPC)
Customer processes communicate with the bank server using **message queues**, following a producer-consumer model where multiple customers (producers) send requests and the bank server (consumer) processes them sequentially.

### 5. Memory Management
Each customer has a simulated memory footprint (account data, transaction history). The system implements page replacement algorithms to manage limited memory frames:
- **FIFO (First In First Out)**
- **LRU (Least Recently Used)**

Includes tracking of page faults and hit ratios to compare algorithm efficiency.

## 📂 Project Structure

```
ConcurrentBankingSystem/
├── src/
│   ├── main.c          # Program entry point
│   ├── accounts.c      # Account creation and management
│   ├── banker.c         # Banker's Algorithm implementation
│   ├── banking.h        # Shared structs, constants, function declarations
│   ├── ipc.c             # Message queue-based IPC
│   ├── memory.c        # Page replacement (FIFO/LRU) simulation
│   ├── scheduling.c    # FCFS, Priority, Round Robin scheduling
│   ├── sync.c            # Mutex and semaphore-based synchronization
│   └── utils.c           # Helper/utility functions
├── charts/               # Gantt charts and scheduling performance graphs
├── logs/                 # IPC request/response and execution logs
├── report/               # Project report (implementation details, test cases, results)
├── Makefile
└── README.md
```

## ⚙️ Build & Run

**Requirements:** GCC, Make, Linux/Unix environment (uses POSIX threads and IPC).

```bash
# From the project root
make

# Run
./banking_system
```

## 📊 Deliverables

- Modular C codebase covering all five OS concepts
- Gantt charts and scheduling metrics (waiting/turnaround time)
- Safe vs. unsafe state test cases for the Banker's Algorithm
- IPC request/response logs
- Page fault and hit ratio comparisons (FIFO vs. LRU)

## 🎓 Course Context

Developed as a semester project for **CL2006 – Operating Systems Lab**, National University of Computer and Emerging Sciences (FAST-NUCES), Chiniot-Faisalabad Campus — Spring 2026.

## ✍️ Author

**Muhammad Hamza Afzaal**
[GitHub](https://github.com/Hamza07-debug)
