/*
 * banker.c  –  Deadlock Avoidance via Banker's Algorithm
 * ─────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 *
 * Resources: [Credit Units, Bank Tellers, Vault Access Slots]
 */

#include "banking.h"

ResourceTable res_table;

/* ═══════════════════════════════════════════════════════
   init_resource_table()
   Populate allocation / max / need matrices and available
   vector with realistic banking-scenario values.
   ═══════════════════════════════════════════════════════ */
void init_resource_table(void) {
    res_table.num_procs = 5;
    res_table.num_res   = 3;

    /* Available resource vector: [Credit Units, Tellers, Vault Slots] */
    int avail_init[] = {4, 2, 3};
    for (int r = 0; r < res_table.num_res; r++)
        res_table.avail_vector[r] = avail_init[r];

    /*
     * Maximum resource needs per process:
     *   P0 – Hamza      (Regular)    : [6, 4, 2]
     *   P1 – Zaryab     (Premium)    : [3, 2, 1]
     *   P2 – Saad       (Loan)       : [8, 1, 2]
     *   P3 – TechVentures(Corporate) : [2, 3, 3]
     *   P4 – Omar       (VIP)        : [5, 2, 4]
     */
    int max_init[5][3] = {
        {6, 4, 2},
        {3, 2, 1},
        {8, 1, 2},
        {2, 3, 3},
        {5, 2, 4},
    };

    /*
     * Current allocations (already granted):
     *   P0 – Hamza      : [0, 1, 0]
     *   P1 – Zaryab     : [2, 0, 1]
     *   P2 – Saad       : [3, 0, 1]
     *   P3 – TechVentures: [1, 2, 1]
     *   P4 – Omar       : [0, 0, 2]
     */
    int alloc_init[5][3] = {
        {0, 1, 0},
        {2, 0, 1},
        {3, 0, 1},
        {1, 2, 1},
        {0, 0, 2},
    };

    for (int proc = 0; proc < res_table.num_procs; proc++) {
        for (int r = 0; r < res_table.num_res; r++) {
            res_table.alloc_matrix[proc][r] = alloc_init[proc][r];
            res_table.max_matrix  [proc][r] = max_init  [proc][r];
            res_table.need_matrix [proc][r] = max_init  [proc][r]
                                            - alloc_init[proc][r];
        }
    }

    log_event("Resource table initialised: %d processes, %d resource types",
              res_table.num_procs, res_table.num_res);
}

/* ═══════════════════════════════════════════════════════
   check_safe_state()
   Safety Algorithm – returns 1 if system is in safe state.
   Prints the safe execution sequence when found.
   ═══════════════════════════════════════════════════════ */
int check_safe_state(ResourceTable *rt) {
    int work  [MAX_RES_TYPES];
    int done  [MAX_PROCESSES];
    int seq   [MAX_PROCESSES];
    int seq_len = 0;

    for (int r = 0; r < rt->num_res;   r++) work[r] = rt->avail_vector[r];
    for (int p = 0; p < rt->num_procs; p++) done[p] = 0;

    while (seq_len < rt->num_procs) {
        int found = 0;
        for (int p = 0; p < rt->num_procs; p++) {
            if (done[p]) continue;
            /* Check: need[p] <= work */
            int feasible = 1;
            for (int r = 0; r < rt->num_res; r++)
                if (rt->need_matrix[p][r] > work[r]) { feasible = 0; break; }

            if (feasible) {
                /* Simulate process p completing and releasing resources */
                for (int r = 0; r < rt->num_res; r++)
                    work[r] += rt->alloc_matrix[p][r];
                done[p]        = 1;
                seq[seq_len++] = p;
                found          = 1;
            }
        }
        if (!found) break;   /* no progress – unsafe */
    }

    if (seq_len == rt->num_procs) {
        printf("  Safe Sequence Found:  ");
        for (int i = 0; i < seq_len; i++)
            printf("P%d%s", seq[i], (i < seq_len - 1) ? "  ->  " : "");
        printf("\n");
        return 1;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════
   process_loan_request()
   Resource-Request Algorithm  –  returns 1 if approved.
   Tentatively allocates then rolls back if unsafe.
   ═══════════════════════════════════════════════════════ */
int process_loan_request(ResourceTable *rt, int proc_idx, int req[]) {
    const char *proc_names[] = {
        "Hamza", "Zaryab", "Saad", "TechVentures", "Omar"
    };
    const char *pname = proc_names[proc_idx];

    printf("\n  [Loan Request]  %s (P%d) requesting [%d, %d, %d]\n",
           pname, proc_idx,
           req[0], req[1], req[2]);

    /* Step 1 – Request must not exceed declared maximum need */
    for (int r = 0; r < rt->num_res; r++) {
        if (req[r] > rt->need_matrix[proc_idx][r]) {
            printf("  ✗ DENIED  –  Request exceeds maximum declared need.\n");
            log_event("LOAN DENIED [%s]: request > max need", pname);
            return 0;
        }
    }

    /* Step 2 – Requested resources must be currently available */
    for (int r = 0; r < rt->num_res; r++) {
        if (req[r] > rt->avail_vector[r]) {
            printf("  ✗ DENIED  –  Resources unavailable; process must wait.\n");
            log_event("LOAN DENIED [%s]: resources unavailable", pname);
            return 0;
        }
    }

    /* Step 3 – Tentative allocation */
    for (int r = 0; r < rt->num_res; r++) {
        rt->avail_vector  [r]            -= req[r];
        rt->alloc_matrix  [proc_idx][r]  += req[r];
        rt->need_matrix   [proc_idx][r]  -= req[r];
    }

    /* Step 4 – Safety check */
    if (check_safe_state(rt)) {
        printf("  ✓ APPROVED  –  System remains in a SAFE state.\n");
        log_event("LOAN APPROVED [%s]", pname);
        return 1;
    }

    /* Rollback – allocation would cause unsafe state */
    for (int r = 0; r < rt->num_res; r++) {
        rt->avail_vector  [r]            += req[r];
        rt->alloc_matrix  [proc_idx][r]  -= req[r];
        rt->need_matrix   [proc_idx][r]  += req[r];
    }
    printf("  ✗ DENIED  –  Allocation leads to UNSAFE state (deadlock risk).\n");
    log_event("LOAN DENIED [%s]: unsafe state", pname);
    return 0;
}

/* ─── Helper: print resource table ──────────────── */
static void print_resource_table(ResourceTable *rt) {
    const char *proc_names[] = {
        "Hamza", "Zaryab", "Saad", "TechVentures", "Omar"
    };
    printf("\n  %-14s %-20s %-20s %-20s\n",
           "Process", "Allocation [C,T,V]", "Max Need  [C,T,V]", "Need      [C,T,V]");
    printf("  %-14s %-20s %-20s %-20s\n",
           "─────────────","────────────────────",
           "────────────────────","────────────────────");

    for (int p = 0; p < rt->num_procs; p++) {
        char al[24], mx[24], nd[24];
        snprintf(al, sizeof(al), "[%d, %d, %d]",
                 rt->alloc_matrix[p][0], rt->alloc_matrix[p][1],
                 rt->alloc_matrix[p][2]);
        snprintf(mx, sizeof(mx), "[%d, %d, %d]",
                 rt->max_matrix[p][0],   rt->max_matrix[p][1],
                 rt->max_matrix[p][2]);
        snprintf(nd, sizeof(nd), "[%d, %d, %d]",
                 rt->need_matrix[p][0],  rt->need_matrix[p][1],
                 rt->need_matrix[p][2]);
        printf("  %-14s %-20s %-20s %-20s\n",
               proc_names[p], al, mx, nd);
    }
    printf("  Available:     [%d, %d, %d]  "
           "(C=Credit Units, T=Tellers, V=Vault Slots)\n",
           rt->avail_vector[0], rt->avail_vector[1], rt->avail_vector[2]);
}

/* ═══════════════════════════════════════════════════════
   Public: Banker Demo Entry Point
   ═══════════════════════════════════════════════════════ */
void run_banker_demo(void) {
    printf("\n  ╔═══════════════════════════════════════════════════╗\n");
    printf("  ║   MODULE 3  –  Banker's Algorithm / Deadlock      ║\n");
    printf("  ╚═══════════════════════════════════════════════════╝\n");

    print_resource_table(&res_table);

    printf("\n  ── Initial Safety Check ──\n");
    if (check_safe_state(&res_table))
        printf("  System is currently in a SAFE state.\n");
    else
        printf("  System is currently in an UNSAFE state!\n");

    /* Test 1: Safe request from Zaryab (P1) */
    printf("\n  ── Test Case 1: Safe Loan Request ──\n");
    int req1[] = {1, 0, 0};
    process_loan_request(&res_table, 1, req1);

    /* Test 2: Unsafe request from Saad (P2) – would cause deadlock */
    printf("\n  ── Test Case 2: Unsafe Request (expect DENIAL) ──\n");
    int req2[] = {4, 1, 0};
    process_loan_request(&res_table, 2, req2);

    /* Test 3: Request exceeding declared max need */
    printf("\n  ── Test Case 3: Request Exceeds Maximum Need ──\n");
    int req3[] = {9, 9, 9};
    process_loan_request(&res_table, 0, req3);

    /* Test 4: Safe VIP request from Omar (P4) */
    printf("\n  ── Test Case 4: VIP Loan Request ──\n");
    int req4[] = {2, 1, 1};
    process_loan_request(&res_table, 4, req4);

    printf("\n  ── Final Resource Table State ──\n");
    print_resource_table(&res_table);
}
