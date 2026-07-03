/*
 * banking.h  –  Shared types, constants, and function prototypes
 * ─────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Course  : CL2006 – Operating System Lab (Spring 2026)
 * Section : BS-4-B
 * Author  : Muhammad Hamza Afzaal  (24F-0698)
 * Campus  : FAST-NUCES, Chiniot-Faisalabad
 * ─────────────────────────────────────────────────────────────
 */

#ifndef BANKING_H
#define BANKING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

/* ═══════════════════════════════════════════════
   Customer Priority Levels
   ═══════════════════════════════════════════════ */
#define CUST_REGULAR    1
#define CUST_LOAN       2
#define CUST_PREMIUM    3
#define CUST_CORPORATE  4
#define CUST_VIP        5

/* Keep old names as aliases for compatibility */
#define REGULAR    CUST_REGULAR
#define PREMIUM    CUST_PREMIUM
#define LOAN       CUST_LOAN
#define CORPORATE  CUST_CORPORATE
#define VIP        CUST_VIP

/* ═══════════════════════════════════════════════
   Transaction Operation Codes
   ═══════════════════════════════════════════════ */
#define TXN_DEPOSIT     1
#define TXN_WITHDRAWAL  2
#define TXN_LOAN_REQ    3
#define TXN_PAYROLL     4

/* Keep old names as aliases */
#define DEPOSIT    TXN_DEPOSIT
#define WITHDRAWAL TXN_WITHDRAWAL
#define LOAN_REQ   TXN_LOAN_REQ
#define PAYROLL    TXN_PAYROLL

/* ═══════════════════════════════════════════════
   Scheduling Algorithm Codes
   ═══════════════════════════════════════════════ */
#define ALGO_FCFS           0
#define ALGO_PRIORITY       1
#define ALGO_ROUND_ROBIN    2
#define TIME_QUANTUM        2   /* Round Robin time slice (ms) */

/* Keep old names */
#define FCFS           ALGO_FCFS
#define PRIORITY_SCHED ALGO_PRIORITY
#define ROUND_ROBIN    ALGO_ROUND_ROBIN

/* ═══════════════════════════════════════════════
   Banker's Algorithm Limits
   ═══════════════════════════════════════════════ */
#define MAX_PROCESSES   10
#define MAX_RES_TYPES    3
#define TOTAL_CREDIT    10

/* Keep old names */
#define MAX_CUSTOMERS  MAX_PROCESSES
#define MAX_RESOURCES  MAX_RES_TYPES

/* ═══════════════════════════════════════════════
   Memory Management
   ═══════════════════════════════════════════════ */
#define NUM_FRAMES      4
#define REF_STRING_LEN  20

/* Keep old name */
#define PAGE_SEQ_LEN   REF_STRING_LEN

/* ═══════════════════════════════════════════════
   IPC Configuration
   ═══════════════════════════════════════════════ */
#define IPC_MSG_KEY      0x2698   /* last 4 digits of roll number */
#define MAX_MSG_SIZE     128

/* Keep old name */
#define MSG_KEY          IPC_MSG_KEY
#define MAX_MSG_TEXT     MAX_MSG_SIZE

/* ═══════════════════════════════════════════════
   Account Limits
   ═══════════════════════════════════════════════ */
#define MAX_ACCT_COUNT  20
#define MAX_ACCOUNTS    MAX_ACCT_COUNT

/* ═══════════════════════════════════════════════════════════
   Data Structures
   ═══════════════════════════════════════════════════════════ */

/* Bank account record */
typedef struct {
    int    acct_id;
    double balance;
    int    holder_type;       /* customer priority class    */
    char   holder_name[32];
} Account;

/* Customer / process descriptor (used in scheduling) */
typedef struct {
    int    proc_id;
    int    cust_class;        /* CUST_REGULAR … CUST_VIP   */
    int    txn_type;
    double txn_amount;
    int    acct_id;
    int    arrival_ms;        /* arrival time for scheduling */
    int    service_ms;        /* CPU burst time              */
    int    remaining_ms;      /* remaining time (Round Robin)*/
    int    sched_priority;    /* mirrors cust_class          */
    int    sched_start;
    int    sched_finish;
    int    wait_time;
    int    turnaround_time;
    char   label[32];
} Client;

/* Banker's algorithm state */
typedef struct {
    int alloc_matrix[MAX_PROCESSES][MAX_RES_TYPES];
    int max_matrix  [MAX_PROCESSES][MAX_RES_TYPES];
    int avail_vector[MAX_RES_TYPES];
    int need_matrix [MAX_PROCESSES][MAX_RES_TYPES];
    int num_procs;
    int num_res;
} ResourceTable;

/* IPC raw message */
typedef struct {
    long msg_type;
    char msg_body[MAX_MSG_SIZE];
    int  target_acct;
    double txn_value;
    int  op_code;
    int  sender_id;
} RawMessage;

/* Memory page descriptor */
typedef struct {
    int page_num;
    int last_access;  /* timestamp for LRU */
    int load_seq;     /* insertion order for FIFO */
} PageFrame;

/* ═══════════════════════════════════════════════════════════
   Global Variables  (defined in accounts.c / utils.c)
   ═══════════════════════════════════════════════════════════ */

extern Account         accounts[MAX_ACCT_COUNT];
extern int             num_accounts;
extern pthread_mutex_t acct_mutex;
extern sem_t           deposit_sem;
extern ResourceTable   res_table;
extern FILE           *log_fp;

/* ═══════════════════════════════════════════════════════════
   Function Prototypes
   ═══════════════════════════════════════════════════════════ */

/* accounts.c */
void      init_accounts  (void);
Account  *find_account   (int acct_id);
int       deposit        (int acct_id, double amount, const char *actor);
int       withdraw       (int acct_id, double amount, const char *actor);
void      print_accounts (void);

/* scheduling.c */
void  run_fcfs      (Client clients[], int count);
void  run_priority  (Client clients[], int count);
void  run_rr        (Client clients[], int count);
void  print_gantt   (Client clients[], int count, const char *algo_name);
void  print_metrics (Client clients[], int count, const char *algo_name);

/* sync.c */
void *worker_deposit  (void *arg);
void *worker_withdraw (void *arg);
void  run_sync_demo   (void);

/* banker.c */
void  init_resource_table  (void);
int   check_safe_state     (ResourceTable *rt);
int   process_loan_request (ResourceTable *rt, int proc_idx, int req[]);
void  run_banker_demo      (void);

/* ipc.c */
void  run_ipc_demo   (void);

/* memory.c */
int   run_fifo       (int ref[], int len, int frames[], int nframes, int *faults);
int   run_lru        (int ref[], int len, int frames[], int nframes, int *faults);
void  run_memory_demo(void);

/* utils.c */
void        log_event            (const char *fmt, ...);
void        ms_sleep             (int ms);
const char *cust_class_str       (int ctype);
const char *txn_type_str         (int ttype);

/* Backward compat aliases */
#define customer_type_str    cust_class_str
#define transaction_type_str txn_type_str
#define is_safe_state        check_safe_state
#define request_loan         process_loan_request
#define init_banker          init_resource_table
#define banker               res_table
#define thread_deposit       worker_deposit
#define thread_withdraw      worker_withdraw
#define fifo_replace         run_fifo
#define lru_replace          run_lru

/* Client <-> Customer field aliases so old code compiles unchanged */
#define customer_id     proc_id
#define customer_type   cust_class
#define transaction_type txn_type
#define amount          txn_amount
#define account_id      acct_id
#define arrival_time    arrival_ms
#define burst_time      service_ms
#define remaining_time  remaining_ms
#define priority        sched_priority
#define start_time      sched_start
#define finish_time     sched_finish
#define waiting_time    wait_time
#define name            label

/* Account field aliases */
#define account_mutex   acct_mutex
#define owner_type      holder_type
#define owner_name      holder_name

/* ResourceTable field aliases */
#define allocation      alloc_matrix
#define max_need        max_matrix
#define available       avail_vector
#define need            need_matrix
#define num_customers   num_procs
#define num_resources   num_res

#endif /* BANKING_H */
