/*
 * utils.c  –  Logging helpers and shared utility functions
 * ─────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 */

#include "banking.h"
#include <stdarg.h>

FILE *log_fp = NULL;

/* ═══════════════════════════════════════════════
   log_event()  –  timestamped logging to stdout + file
   ═══════════════════════════════════════════════ */
void log_event(const char *fmt, ...) {
    va_list args;
    time_t  now  = time(NULL);
    struct  tm *tm_info = localtime(&now);
    char    timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);

    /* Console output */
    printf("  [%s]  ", timestamp);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    /* File output */
    if (log_fp) {
        fprintf(log_fp, "[%s]  ", timestamp);
        va_start(args, fmt);
        vfprintf(log_fp, fmt, args);
        va_end(args);
        fprintf(log_fp, "\n");
        fflush(log_fp);
    }
}

/* ═══════════════════════════════════════════════
   ms_sleep()  –  sleep for given milliseconds
   ═══════════════════════════════════════════════ */
void ms_sleep(int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* ═══════════════════════════════════════════════
   cust_class_str()  –  human-readable customer type
   ═══════════════════════════════════════════════ */
const char *cust_class_str(int ctype) {
    switch (ctype) {
        case CUST_REGULAR:    return "Regular";
        case CUST_PREMIUM:    return "Premium";
        case CUST_LOAN:       return "Loan Applicant";
        case CUST_CORPORATE:  return "Corporate";
        case CUST_VIP:        return "VIP";
        default:              return "Unknown";
    }
}

/* ═══════════════════════════════════════════════
   txn_type_str()  –  human-readable transaction type
   ═══════════════════════════════════════════════ */
const char *txn_type_str(int ttype) {
    switch (ttype) {
        case TXN_DEPOSIT:     return "Deposit";
        case TXN_WITHDRAWAL:  return "Withdrawal";
        case TXN_LOAN_REQ:    return "LoanRequest";
        case TXN_PAYROLL:     return "Payroll";
        default:              return "Unknown";
    }
}
