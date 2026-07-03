/*
 * ipc.c  –  Inter-Process Communication via POSIX System V Message Queues
 * ──────────────────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 *
 * Pattern: Producer-Consumer
 *   Producers  →  N customer processes sending transaction requests
 *   Consumer   →  1 bank server process reading & responding
 */

#include "banking.h"
#include <sys/wait.h>

#define IPC_CLIENT_COUNT   5
#define SERVER_MSG_TYPE    1L
#define CLIENT_RESP_BASE   200L   /* response type = CLIENT_RESP_BASE + client_id */

static int msg_queue_id = -1;

/* ═══════════════════════════════════════════════
   Message Structures
   ═══════════════════════════════════════════════ */
typedef struct {
    long   msg_type;
    int    client_id;
    int    target_acct;
    int    op_code;
    double txn_value;
    char   client_label[32];
} TxnRequest;

typedef struct {
    long   msg_type;
    int    client_id;
    int    status;           /* 1 = success, 0 = failed */
    double updated_balance;
    char   reply_text[72];
} TxnResponse;

/* ═══════════════════════════════════════════════
   Bank Server Process  (consumer)
   ═══════════════════════════════════════════════ */
static void bank_server_proc(void) {
    log_event("[SERVER] Bank server online – queue id=%d", msg_queue_id);

    int requests_served = 0;
    while (requests_served < IPC_CLIENT_COUNT) {
        TxnRequest req;
        if (msgrcv(msg_queue_id, &req, sizeof(req) - sizeof(long),
                   SERVER_MSG_TYPE, 0) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        log_event("[SERVER] Received %-10s from %-12s | Acct %-8d | Amt %.2f",
                  txn_type_str(req.op_code), req.client_label,
                  req.target_acct, req.txn_value);

        TxnResponse resp;
        resp.msg_type  = CLIENT_RESP_BASE + req.client_id;
        resp.client_id = req.client_id;

        int rc = (req.op_code == TXN_DEPOSIT)
                 ? deposit (req.target_acct, req.txn_value, req.client_label)
                 : withdraw(req.target_acct, req.txn_value, req.client_label);

        if (rc == 0) {
            resp.status = 1;
            Account *acc = find_account(req.target_acct);
            resp.updated_balance = acc ? acc->balance : 0.0;
            snprintf(resp.reply_text, sizeof(resp.reply_text),
                     "Transaction OK. Updated Balance: PKR %.2f",
                     resp.updated_balance);
        } else {
            resp.status          = 0;
            resp.updated_balance = 0.0;
            snprintf(resp.reply_text, sizeof(resp.reply_text),
                     "Transaction FAILED – check funds or account ID.");
        }

        msgsnd(msg_queue_id, &resp, sizeof(resp) - sizeof(long), 0);
        log_event("[SERVER] Reply sent to Client %d: %s",
                  req.client_id, resp.reply_text);
        requests_served++;
    }

    log_event("[SERVER] Served %d requests. Shutting down.", requests_served);
    exit(0);
}

/* ═══════════════════════════════════════════════
   Client Process descriptor
   ═══════════════════════════════════════════════ */
typedef struct {
    int    client_id;
    int    target_acct;
    int    op_code;
    double txn_value;
    char   label[32];
    int    send_delay_ms;
} ClientProc;

/* ═══════════════════════════════════════════════
   Customer Process  (producer)
   ═══════════════════════════════════════════════ */
static void client_proc(ClientProc *cp) {
    ms_sleep(cp->send_delay_ms);

    log_event("[CLIENT %d] %-10s sending %-10s request | Amount: %.2f",
              cp->client_id, cp->label,
              txn_type_str(cp->op_code), cp->txn_value);

    TxnRequest req;
    req.msg_type    = SERVER_MSG_TYPE;
    req.client_id   = cp->client_id;
    req.target_acct = cp->target_acct;
    req.op_code     = cp->op_code;
    req.txn_value   = cp->txn_value;
    strncpy(req.client_label, cp->label, sizeof(req.client_label) - 1);

    msgsnd(msg_queue_id, &req, sizeof(req) - sizeof(long), 0);

    /* Block until server sends response back */
    TxnResponse resp;
    long resp_type = CLIENT_RESP_BASE + cp->client_id;
    if (msgrcv(msg_queue_id, &resp,
               sizeof(resp) - sizeof(long), resp_type, 0) != -1) {
        log_event("[CLIENT %d] Response: %s",
                  cp->client_id, resp.reply_text);
    }
    exit(0);
}

/* ═══════════════════════════════════════════════
   Public: IPC Demo Entry Point
   ═══════════════════════════════════════════════ */
void run_ipc_demo(void) {
    printf("\n  ╔═══════════════════════════════════════════════════╗\n");
    printf("  ║       MODULE 4  –  IPC: Message Queue Demo        ║\n");
    printf("  ╚═══════════════════════════════════════════════════╝\n");

    /* Create System V message queue */
    msg_queue_id = msgget(IPC_MSG_KEY, IPC_CREAT | 0666);
    if (msg_queue_id == -1) {
        perror("msgget");
        printf("  [IPC] Cannot create message queue – skipping demo.\n");
        printf("  (Ensure IPC is enabled; try running with sudo on restricted systems.)\n");
        return;
    }

    /* Define client request data */
    ClientProc clients[IPC_CLIENT_COUNT] = {
        {1, 269801, TXN_DEPOSIT,     750.00, "Hamza",        40},
        {2, 269802, TXN_WITHDRAWAL,  300.00, "Zaryab",       80},
        {3, 269803, TXN_DEPOSIT,    1500.00, "Saad",          0},
        {4, 269804, TXN_DEPOSIT,    5000.00, "TechVentures", 160},
        {5, 269805, TXN_WITHDRAWAL, 4000.00, "Omar",         20},
    };

    /* Flush all buffered output before forking to prevent duplication */
    fflush(stdout);
    if (log_fp) fflush(log_fp);

    /* Fork bank server (consumer) */
    pid_t server_pid = fork();
    if (server_pid == 0) {
        bank_server_proc();   /* never returns */
    }
    log_event("[IPC] Bank server process  PID = %d", server_pid);

    /* Fork client processes (producers) */
    pid_t client_pids[IPC_CLIENT_COUNT];
    for (int i = 0; i < IPC_CLIENT_COUNT; i++) {
        client_pids[i] = fork();
        if (client_pids[i] == 0) {
            client_proc(&clients[i]);   /* never returns */
        }
    }

    /* Wait for all clients then server */
    for (int i = 0; i < IPC_CLIENT_COUNT; i++)
        waitpid(client_pids[i], NULL, 0);
    waitpid(server_pid, NULL, 0);

    /* Remove message queue */
    msgctl(msg_queue_id, IPC_RMID, NULL);
    msg_queue_id = -1;

    log_event("[IPC] Message queue removed. Demo complete.");

    printf("\n  ── Producer-Consumer Summary ──\n");
    printf("  Producers  : %d client processes (sent requests via queue)\n",
           IPC_CLIENT_COUNT);
    printf("  Consumer   : 1 bank server process (processed all requests)\n");
    printf("  IPC Channel: POSIX System V Message Queue  (key=0x%X)\n\n",
           IPC_MSG_KEY);
}
