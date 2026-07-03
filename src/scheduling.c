/*
 * scheduling.c  –  CPU Scheduling: FCFS, Priority, Round Robin + Gantt charts
 * ─────────────────────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 */

#include "banking.h"

/* ═══════════════════════════════════════════════════
   Internal: FCFS  –  sort by arrival, then compute
   ═══════════════════════════════════════════════════ */
static void compute_fcfs(Client q[], int cnt) {
    /* Bubble-sort by arrival time (ascending) */
    for (int i = 0; i < cnt - 1; i++)
        for (int j = i + 1; j < cnt; j++)
            if (q[j].arrival_ms < q[i].arrival_ms) {
                Client tmp = q[i]; q[i] = q[j]; q[j] = tmp;
            }

    int clock = 0;
    for (int i = 0; i < cnt; i++) {
        if (clock < q[i].arrival_ms) clock = q[i].arrival_ms;
        q[i].sched_start    = clock;
        q[i].sched_finish   = clock + q[i].service_ms;
        q[i].wait_time      = q[i].sched_start  - q[i].arrival_ms;
        q[i].turnaround_time = q[i].sched_finish - q[i].arrival_ms;
        clock               = q[i].sched_finish;
    }
}

/* ═══════════════════════════════════════════════════
   Internal: Priority  –  sort by priority DESC / arrival ASC
   ═══════════════════════════════════════════════════ */
static void compute_priority(Client q[], int cnt) {
    for (int i = 0; i < cnt - 1; i++)
        for (int j = i + 1; j < cnt; j++) {
            int do_swap = 0;
            if (q[j].sched_priority > q[i].sched_priority)
                do_swap = 1;
            else if (q[j].sched_priority == q[i].sched_priority &&
                     q[j].arrival_ms < q[i].arrival_ms)
                do_swap = 1;
            if (do_swap) { Client tmp = q[i]; q[i] = q[j]; q[j] = tmp; }
        }

    int clock = 0;
    for (int i = 0; i < cnt; i++) {
        if (clock < q[i].arrival_ms) clock = q[i].arrival_ms;
        q[i].sched_start    = clock;
        q[i].sched_finish   = clock + q[i].service_ms;
        q[i].wait_time      = q[i].sched_start  - q[i].arrival_ms;
        q[i].turnaround_time = q[i].sched_finish - q[i].arrival_ms;
        clock               = q[i].sched_finish;
    }
}

/* ═══════════════════════════════════════════════════
   Internal: Round Robin  –  time-slice cycling
   ═══════════════════════════════════════════════════ */
typedef struct { int client_idx; int slice_start; int slice_end; } TimeSlice;
#define MAX_SLICES 256

static TimeSlice rr_schedule[MAX_SLICES];
static int       rr_slice_count;

static void compute_rr(Client q[], int cnt) {
    rr_slice_count = 0;
    int remaining[MAX_ACCT_COUNT];
    int finished_flag[MAX_ACCT_COUNT];

    for (int i = 0; i < cnt; i++) {
        remaining[i]      = q[i].service_ms;
        finished_flag[i]  = 0;
        q[i].sched_start  = -1;
    }

    int clock      = 0;
    int done_count = 0;

    while (done_count < cnt) {
        int made_progress = 0;
        for (int i = 0; i < cnt; i++) {
            if (finished_flag[i] || q[i].arrival_ms > clock) continue;
            if (remaining[i] <= 0) { finished_flag[i] = 1; continue; }

            made_progress = 1;
            if (q[i].sched_start == -1) q[i].sched_start = clock;

            int run_duration = (remaining[i] < TIME_QUANTUM)
                               ? remaining[i] : TIME_QUANTUM;

            if (rr_slice_count < MAX_SLICES) {
                rr_schedule[rr_slice_count].client_idx  = i;
                rr_schedule[rr_slice_count].slice_start = clock;
                rr_schedule[rr_slice_count].slice_end   = clock + run_duration;
                rr_slice_count++;
            }

            clock        += run_duration;
            remaining[i] -= run_duration;

            if (remaining[i] <= 0) {
                finished_flag[i]     = 1;
                q[i].sched_finish    = clock;
                q[i].wait_time       = q[i].sched_finish
                                       - q[i].arrival_ms
                                       - q[i].service_ms;
                q[i].turnaround_time = q[i].sched_finish - q[i].arrival_ms;
                done_count++;
            }
        }
        if (!made_progress) clock++;  /* CPU idle cycle */
    }
}

/* ═══════════════════════════════════════════════════
   Public: Gantt Chart (ASCII)
   ═══════════════════════════════════════════════════ */
void print_gantt(Client q[], int cnt, const char *algo_name) {
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf("  ║  Gantt Chart  ─  %-34s║\n", algo_name);
    printf("  ╚══════════════════════════════════════════════════════╝\n");

    if (strcmp(algo_name, "Round Robin") == 0) {
        printf("  |");
        for (int i = 0; i < rr_slice_count; i++)
            printf(" %-6s |", q[rr_schedule[i].client_idx].label);
        printf("\n  %d", rr_schedule[0].slice_start);
        for (int i = 0; i < rr_slice_count; i++)
            printf("        %d", rr_schedule[i].slice_end);
        printf("\n\n");
    } else {
        printf("  |");
        for (int i = 0; i < cnt; i++)
            printf(" %-6s |", q[i].label);
        printf("\n  %d",
               (q[0].arrival_ms > q[0].sched_start)
               ? q[0].arrival_ms : q[0].sched_start);
        for (int i = 0; i < cnt; i++)
            printf("        %d", q[i].sched_finish);
        printf("\n\n");
    }
}

/* ═══════════════════════════════════════════════════
   Public: Metrics Table
   ═══════════════════════════════════════════════════ */
void print_metrics(Client q[], int cnt, const char *algo_name) {
    printf("  ┌───────────────────────────────────────────────────────────────────┐\n");
    printf("  │  Scheduling Metrics  ─  %-44s│\n", algo_name);
    printf("  ├───────────┬───────────┬───────────┬───────────┬───────────────────┤\n");
    printf("  │  Client   │ Priority  │  Arrival  │   Wait    │   Turnaround      │\n");
    printf("  ├───────────┼───────────┼───────────┼───────────┼───────────────────┤\n");

    double total_wait = 0.0, total_tat = 0.0;
    for (int i = 0; i < cnt; i++) {
        printf("  │ %-9s │ %-9d │ %-9d │ %-9d │ %-17d │\n",
               q[i].label,
               q[i].sched_priority,
               q[i].arrival_ms,
               q[i].wait_time,
               q[i].turnaround_time);
        total_wait += q[i].wait_time;
        total_tat  += q[i].turnaround_time;
    }
    printf("  ├───────────┴───────────┴───────────┼───────────┼───────────────────┤\n");
    printf("  │                         AVERAGE    │ %-9.2f │ %-17.2f │\n",
           total_wait / cnt, total_tat / cnt);
    printf("  └────────────────────────────────────┴───────────┴───────────────────┘\n\n");

    log_event("[%-20s] Avg Wait = %.2f ms  |  Avg Turnaround = %.2f ms",
              algo_name, total_wait / cnt, total_tat / cnt);
}

/* ═══════════════════════════════════════════════════
   Public run wrappers  (work on local copies)
   ═══════════════════════════════════════════════════ */
void run_fcfs(Client q[], int cnt) {
    Client local[MAX_ACCT_COUNT];
    memcpy(local, q, cnt * sizeof(Client));
    compute_fcfs(local, cnt);
    print_gantt (local, cnt, "FCFS");
    print_metrics(local, cnt, "FCFS");
}

void run_priority(Client q[], int cnt) {
    Client local[MAX_ACCT_COUNT];
    memcpy(local, q, cnt * sizeof(Client));
    compute_priority(local, cnt);
    print_gantt (local, cnt, "Priority Scheduling");
    print_metrics(local, cnt, "Priority Scheduling");
}

void run_rr(Client q[], int cnt) {
    Client local[MAX_ACCT_COUNT];
    memcpy(local, q, cnt * sizeof(Client));
    compute_rr(local, cnt);
    print_gantt (local, cnt, "Round Robin");
    print_metrics(local, cnt, "Round Robin");
}
