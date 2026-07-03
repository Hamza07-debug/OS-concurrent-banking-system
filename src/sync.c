/*
 * sync.c  –  Thread Synchronization: Mutex locks & Semaphores
 * ─────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 */

#include "banking.h"

/* ═══════════════════════════════════════════════
   Thread argument for deposit / withdrawal tasks
   ═══════════════════════════════════════════════ */
typedef struct {
    int    target_acct;
    double txn_value;
    char   worker_name[32];
    int    op_type;       /* TXN_DEPOSIT or TXN_WITHDRAWAL */
    int    delay_ms;      /* simulated scheduling delay    */
} WorkerArg;

/* ─── Thread: deposit ─────────────────────────── */
void *worker_deposit(void *arg) {
    WorkerArg *w = (WorkerArg *)arg;
    ms_sleep(w->delay_ms);
    log_event("[THREAD] %-12s  >>  Deposit  %.2f  (Acct %d)",
              w->worker_name, w->txn_value, w->target_acct);
    deposit(w->target_acct, w->txn_value, w->worker_name);
    return NULL;
}

/* ─── Thread: withdrawal ──────────────────────── */
void *worker_withdraw(void *arg) {
    WorkerArg *w = (WorkerArg *)arg;
    ms_sleep(w->delay_ms);
    log_event("[THREAD] %-12s  >>  Withdraw %.2f  (Acct %d)",
              w->worker_name, w->txn_value, w->target_acct);
    withdraw(w->target_acct, w->txn_value, w->worker_name);
    return NULL;
}

/* ═══════════════════════════════════════════════
   Corporate Payroll: N threads, one per employee
   ═══════════════════════════════════════════════ */
typedef struct {
    int    target_acct;
    double salary;
    int    emp_number;
} PayrollTask;

static void *payroll_worker(void *arg) {
    PayrollTask *pt = (PayrollTask *)arg;
    char tag[32];
    snprintf(tag, sizeof(tag), "Staff-%02d", pt->emp_number);
    ms_sleep(10);
    deposit(pt->target_acct, pt->salary, tag);
    free(pt);
    return NULL;
}

static void run_payroll_demo(void) {
    printf("\n  ── Corporate Payroll Demo: 10 employee threads ──\n");
    const int  NUM_STAFF    = 10;
    const int  PAYROLL_ACCT = 269804;      /* TechVentures account */
    const double MONTHLY_SALARY = 650.00;

    pthread_t tids[10];
    for (int i = 0; i < NUM_STAFF; i++) {
        PayrollTask *pt = malloc(sizeof(PayrollTask));
        pt->target_acct = PAYROLL_ACCT;
        pt->salary      = MONTHLY_SALARY;
        pt->emp_number  = i + 1;
        pthread_create(&tids[i], NULL, payroll_worker, pt);
    }
    for (int i = 0; i < NUM_STAFF; i++)
        pthread_join(tids[i], NULL);

    log_event("Payroll complete – %d staff members paid from Acct %d",
              NUM_STAFF, PAYROLL_ACCT);
}

/* ═══════════════════════════════════════════════
   Public: Synchronization Demo Entry Point
   ═══════════════════════════════════════════════ */
void run_sync_demo(void) {
    printf("\n  ╔═══════════════════════════════════════════════════╗\n");
    printf("  ║       MODULE 2  –  Synchronization Demo           ║\n");
    printf("  ╚═══════════════════════════════════════════════════╝\n");

    /* ── Test 1: Mutex – two concurrent withdrawals, same account ── */
    printf("\n  [Test 1] Mutex Guard: Two simultaneous withdrawals from Acct 269801\n");
    pthread_t th1, th2;
    WorkerArg w1 = {269801, 1200.00, "Hamza-T1", TXN_WITHDRAWAL, 0};
    WorkerArg w2 = {269801,  800.00, "Hamza-T2", TXN_WITHDRAWAL, 5};
    pthread_create(&th1, NULL, worker_withdraw, &w1);
    pthread_create(&th2, NULL, worker_withdraw, &w2);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    printf("  --> Mutex ensures serialised balance updates. No race condition.\n");

    /* ── Test 2: Semaphore – three deposits throttled to two at once ── */
    printf("\n  [Test 2] Semaphore(2): Three concurrent deposits – third must wait\n");
    pthread_t th3, th4, th5;
    WorkerArg w3 = {269802, 350.00, "Zaryab-D1",  TXN_DEPOSIT, 0};
    WorkerArg w4 = {269802, 450.00, "Saad-D1",    TXN_DEPOSIT, 0};
    WorkerArg w5 = {269802, 275.00, "Bilal-D1",   TXN_DEPOSIT, 0};
    pthread_create(&th3, NULL, worker_deposit, &w3);
    pthread_create(&th4, NULL, worker_deposit, &w4);
    pthread_create(&th5, NULL, worker_deposit, &w5);
    pthread_join(th3, NULL);
    pthread_join(th4, NULL);
    pthread_join(th5, NULL);
    printf("  --> Semaphore(2) blocked third deposit until a slot was released.\n");

    /* ── Test 3: Corporate payroll ── */
    run_payroll_demo();

    /* ── Test 4: VIP executes before Regular (priority ordering) ── */
    printf("\n  [Test 4] Priority Order: VIP withdrawal executes before Regular deposit\n");
    WorkerArg vip_arg = {269805, 3000.00, "Omar-VIP",   TXN_WITHDRAWAL, 0};
    WorkerArg reg_arg = {269801,  500.00, "Bilal-Reg",  TXN_DEPOSIT,    5};
    pthread_t th_vip, th_reg;
    pthread_create(&th_vip, NULL, worker_withdraw, &vip_arg);
    pthread_create(&th_reg, NULL, worker_deposit,  &reg_arg);
    pthread_join(th_vip, NULL);
    pthread_join(th_reg, NULL);

    printf("\n  ── Account Balances After Synchronization Tests ──\n");
    print_accounts();
}
