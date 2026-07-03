/*
 * accounts.c  –  Bank account management with mutex & semaphore protection
 * ──────────────────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 */

#include "banking.h"

/* ═══════════════════════════════════════════════
   Global shared state
   ═══════════════════════════════════════════════ */
Account         accounts[MAX_ACCT_COUNT];
int             num_accounts   = 0;
pthread_mutex_t acct_mutex     = PTHREAD_MUTEX_INITIALIZER;
sem_t           deposit_sem;          /* max 2 concurrent deposits */

/* ─────────────────────────────────────────────── */
void init_accounts(void) {
    /*
     * Six accounts – one per customer class.
     * Account IDs follow roll-number pattern: 2698xx
     */
    struct {
        int         id;
        double      opening_bal;
        int         ctype;
        const char *holder;
    } acct_data[] = {
        { 269801,  7500.00, CUST_REGULAR,   "Hamza"     },
        { 269802, 14000.00, CUST_PREMIUM,   "Zaryab"    },
        { 269803,  3200.00, CUST_LOAN,      "Saad"      },
        { 269804, 22000.00, CUST_CORPORATE, "TechVentures" },
        { 269805, 18500.00, CUST_VIP,       "Omar"      },
        { 269806,  5100.00, CUST_REGULAR,   "Bilal"     },
    };

    num_accounts = (int)(sizeof(acct_data) / sizeof(acct_data[0]));

    for (int idx = 0; idx < num_accounts; idx++) {
        accounts[idx].acct_id      = acct_data[idx].id;
        accounts[idx].balance      = acct_data[idx].opening_bal;
        accounts[idx].holder_type  = acct_data[idx].ctype;
        strncpy(accounts[idx].holder_name, acct_data[idx].holder,
                sizeof(accounts[idx].holder_name) - 1);
    }

    /* Semaphore permits at most 2 simultaneous deposit operations */
    sem_init(&deposit_sem, 0, 2);

    log_event("Bank initialised – %d accounts loaded", num_accounts);
}

/* ─────────────────────────────────────────────── */
Account *find_account(int acct_id) {
    for (int i = 0; i < num_accounts; i++)
        if (accounts[i].acct_id == acct_id)
            return &accounts[i];
    return NULL;
}

/* ─────────────────────────────────────────────── */
int deposit(int acct_id, double amount, const char *actor) {
    /* Semaphore: throttle to max 2 concurrent deposits */
    sem_wait(&deposit_sem);
    pthread_mutex_lock(&acct_mutex);

    Account *acc = find_account(acct_id);
    if (!acc) {
        log_event("DEPOSIT FAILED  [%-12s] Account %d not found",
                  actor, acct_id);
        pthread_mutex_unlock(&acct_mutex);
        sem_post(&deposit_sem);
        return -1;
    }

    acc->balance += amount;
    log_event("DEPOSIT  OK     [%-12s] +%9.2f -> Acct %-8d | New Balance: %10.2f",
              actor, amount, acct_id, acc->balance);

    pthread_mutex_unlock(&acct_mutex);
    sem_post(&deposit_sem);
    return 0;
}

/* ─────────────────────────────────────────────── */
int withdraw(int acct_id, double amount, const char *actor) {
    pthread_mutex_lock(&acct_mutex);

    Account *acc = find_account(acct_id);
    if (!acc) {
        log_event("WITHDRAW FAILED [%-12s] Account %d not found",
                  actor, acct_id);
        pthread_mutex_unlock(&acct_mutex);
        return -1;
    }
    if (acc->balance < amount) {
        log_event("WITHDRAW FAILED [%-12s] Insufficient funds  "
                  "(Available: %.2f  Requested: %.2f)",
                  actor, acc->balance, amount);
        pthread_mutex_unlock(&acct_mutex);
        return -1;
    }

    acc->balance -= amount;
    log_event("WITHDRAW OK     [%-12s] -%9.2f -> Acct %-8d | New Balance: %10.2f",
              actor, amount, acct_id, acc->balance);

    pthread_mutex_unlock(&acct_mutex);
    return 0;
}

/* ─────────────────────────────────────────────── */
void print_accounts(void) {
    printf("\n  ┌────────────┬──────────────────┬───────────────┬──────────────┐\n");
    printf("  │  Acct ID   │  Account Holder  │  Class        │   Balance    │\n");
    printf("  ├────────────┼──────────────────┼───────────────┼──────────────┤\n");
    for (int i = 0; i < num_accounts; i++) {
        printf("  │ %-10d │ %-16s │ %-13s │ %10.2f   │\n",
               accounts[i].acct_id,
               accounts[i].holder_name,
               cust_class_str(accounts[i].holder_type),
               accounts[i].balance);
    }
    printf("  └────────────┴──────────────────┴───────────────┴──────────────┘\n\n");
}
