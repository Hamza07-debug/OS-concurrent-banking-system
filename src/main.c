/*
 * main.c  –  Concurrent Banking System: Entry Point
 * ─────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author  : Muhammad Hamza Afzaal  (24F-0698)
 * Section : BS-4-B  |  Roll: 24F-0698
 * Course  : CL2006 – Operating System Lab  |  Spring 2026
 * Campus  : FAST-NUCES, Chiniot-Faisalabad
 * ─────────────────────────────────────────────────────────────
 * Covers five OS concepts:
 *   1. CPU Scheduling        (FCFS, Priority, Round Robin)
 *   2. Synchronization       (Mutex, Semaphore)
 *   3. Deadlock Avoidance    (Banker's Algorithm)
 *   4. IPC                   (Message Queues, Producer-Consumer)
 *   5. Memory Management     (FIFO, LRU Page Replacement)
 */

#include "banking.h"

/* ─── Helper: build a Client descriptor ──────── */
static Client make_client(int pid, const char *lbl, int cclass,
                           int arr_ms, int burst_ms) {
    Client cl  = {0};
    cl.proc_id         = pid;
    cl.cust_class      = cclass;
    cl.sched_priority  = cclass;   /* priority = customer class value */
    cl.arrival_ms      = arr_ms;
    cl.service_ms      = burst_ms;
    cl.remaining_ms    = burst_ms;
    strncpy(cl.label, lbl, sizeof(cl.label) - 1);
    return cl;
}

/* ════════════════════════════════════════════════════════════
   main()
   ════════════════════════════════════════════════════════════ */
int main(void) {

    /* Open log file */
    log_fp = fopen("logs/banking_system.log", "w");
    if (!log_fp)
        log_fp = fopen("/tmp/cbs_hamza.log", "w");

    /* ─── Banner ─────────────────────────────────────── */
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════╗\n");
    printf("  ║          CONCURRENT BANKING SYSTEM SIMULATION               ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════╣\n");
    printf("  ║  Concepts : Scheduling | Sync | Deadlock | IPC | Memory     ║\n");
    printf("  ║  Course   : CL2006 – OS Lab        Spring 2026              ║\n");
    printf("  ║  Author   : Muhammad Hamza Afzaal  (24F-0698)               ║\n");
    printf("  ║  Section  : BS-4-B   |   FAST-NUCES Chiniot-Faisalabad      ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════╝\n\n");

    /* ════════════════════════════════════════════
       Initialise shared state
       ════════════════════════════════════════════ */
    init_accounts();
    init_resource_table();

    printf("\n  ── Opening Account Balances ──\n");
    print_accounts();

    /* ════════════════════════════════════════════
       MODULE 1 – CPU Scheduling
       ════════════════════════════════════════════ */
    printf("\n  ╔══════════════════════════════════════════════════════════════╗\n");
    printf("  ║             MODULE 1  –  CPU Scheduling Demo                ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════╝\n");

    /*
     * Six clients representing all customer types.
     * Arrival times and burst times are staggered so each
     * algorithm produces visibly different behaviour.
     *
     * label       class         arrival(ms)  burst(ms)
     * ─────────   ──────────    ──────────   ─────────
     * Hamza       REGULAR  (1)     0            8
     * Zaryab      PREMIUM  (3)     2            5
     * Saad        LOAN     (2)     4            3
     * TechVentr   CORPORATE(4)     1            6
     * Omar        VIP      (5)     3            4
     * Bilal       REGULAR  (1)     5            7
     */
    Client sched_queue[6] = {
        make_client(1, "Hamza",      CUST_REGULAR,   0, 8),
        make_client(2, "Zaryab",     CUST_PREMIUM,   2, 5),
        make_client(3, "Saad",       CUST_LOAN,      4, 3),
        make_client(4, "TechVentr",  CUST_CORPORATE, 1, 6),
        make_client(5, "Omar",       CUST_VIP,       3, 4),
        make_client(6, "Bilal",      CUST_REGULAR,   5, 7),
    };
    int queue_size = 6;

    printf("\n  Clients queued for scheduling:\n");
    printf("  %-12s %-10s %-10s %-10s\n",
           "Name","Priority","Arrival","Burst");
    printf("  %-12s %-10s %-10s %-10s\n",
           "────────────","────────","───────","─────");
    for (int i = 0; i < queue_size; i++)
        printf("  %-12s %-10d %-10d %-10d\n",
               sched_queue[i].label,
               sched_queue[i].sched_priority,
               sched_queue[i].arrival_ms,
               sched_queue[i].service_ms);

    run_fcfs    (sched_queue, queue_size);
    run_priority(sched_queue, queue_size);
    run_rr      (sched_queue, queue_size);

    /* ════════════════════════════════════════════
       MODULE 2 – Synchronization
       ════════════════════════════════════════════ */
    run_sync_demo();

    /* ════════════════════════════════════════════
       MODULE 3 – Deadlock (Banker's Algorithm)
       ════════════════════════════════════════════ */
    run_banker_demo();

    /* ════════════════════════════════════════════
       MODULE 4 – IPC (Message Queues)
       ════════════════════════════════════════════ */
    run_ipc_demo();

    /* ════════════════════════════════════════════
       MODULE 5 – Memory Management
       ════════════════════════════════════════════ */
    run_memory_demo();

    /* ─── Final Summary ──────────────────────────── */
    printf("\n  ╔══════════════════════════════════════════════════════════════╗\n");
    printf("  ║                    SIMULATION COMPLETE                      ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════╣\n");
    printf("  ║  Module 1  ─  CPU Scheduling (FCFS / Priority / RR)    ✓  ║\n");
    printf("  ║  Module 2  ─  Synchronization (Mutex & Semaphore)       ✓  ║\n");
    printf("  ║  Module 3  ─  Deadlock Avoidance (Banker's Algorithm)   ✓  ║\n");
    printf("  ║  Module 4  ─  IPC (Message Queues, Producer-Consumer)   ✓  ║\n");
    printf("  ║  Module 5  ─  Memory Management (FIFO & LRU)            ✓  ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════╣\n");
    printf("  ║  Author    :  Muhammad Hamza Afzaal  (24F-0698)             ║\n");
    printf("  ║  Log File  :  logs/banking_system.log                       ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════╝\n\n");

    if (log_fp) fclose(log_fp);
    sem_destroy(&deposit_sem);
    return 0;
}
