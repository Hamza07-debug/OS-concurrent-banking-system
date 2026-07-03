/*
 * memory.c  –  Page Replacement: FIFO vs LRU with performance metrics
 * ─────────────────────────────────────────────────────────────────────
 * Concurrent Banking System Simulation
 * Author : Muhammad Hamza Afzaal  (24F-0698)
 * Course : CL2006 – OS Lab  |  Spring 2026  |  FAST-NUCES CFD
 *
 * Memory model:
 *   Each page represents a customer's account data block loaded into
 *   a fixed set of physical memory frames.
 *   Page ID → Customer:  1=Hamza, 2=Zaryab, 3=Saad, 4=TechVentures, 5=Omar
 */

#include "banking.h"

/* ═══════════════════════════════════════════════════════
   FIFO Page Replacement
   ═══════════════════════════════════════════════════════ */
int run_fifo(int ref_str[], int ref_len,
             int frames[], int num_frames, int *fault_count) {
    int mem[NUM_FRAMES];
    int insert_ptr = 0;
    int hit_count  = 0;
    *fault_count   = 0;

    for (int f = 0; f < num_frames; f++) mem[f] = -1;

    printf("\n  FIFO Simulation  (Frames = %d)\n", num_frames);
    printf("  ┌────────┬──────────────────────────────┬────────────┐\n");
    printf("  │  Page  │  Frame Contents              │  Result    │\n");
    printf("  ├────────┼──────────────────────────────┼────────────┤\n");

    for (int i = 0; i < ref_len; i++) {
        int pg   = ref_str[i];
        int hit  = 0;

        for (int f = 0; f < num_frames; f++)
            if (mem[f] == pg) { hit = 1; hit_count++; break; }

        if (!hit) {
            mem[insert_ptr] = pg;
            insert_ptr = (insert_ptr + 1) % num_frames;
            (*fault_count)++;
        }

        /* Build frame display string */
        char frame_buf[48] = {0};
        char cell[8];
        for (int f = 0; f < num_frames; f++) {
            if (mem[f] == -1) snprintf(cell, sizeof(cell), "[  ]");
            else              snprintf(cell, sizeof(cell), "[%2d]", mem[f]);
            strncat(frame_buf, cell,
                    sizeof(frame_buf) - strlen(frame_buf) - 1);
            if (f < num_frames - 1)
                strncat(frame_buf, " ",
                        sizeof(frame_buf) - strlen(frame_buf) - 1);
        }
        printf("  │  P%-4d │ %-28s │ %-10s │\n",
               pg, frame_buf, hit ? "HIT" : "PAGE FAULT");
    }
    printf("  └────────┴──────────────────────────────┴────────────┘\n");

    frames[0] = hit_count;   /* reuse output param for caller */
    return hit_count;
}

/* ═══════════════════════════════════════════════════════
   LRU Page Replacement
   ═══════════════════════════════════════════════════════ */
int run_lru(int ref_str[], int ref_len,
            int frames[], int num_frames, int *fault_count) {
    int mem       [NUM_FRAMES];
    int last_used [NUM_FRAMES];
    int hit_count  = 0;
    int tick       = 0;
    *fault_count   = 0;

    for (int f = 0; f < num_frames; f++) {
        mem      [f] = -1;
        last_used[f] =  0;
    }

    printf("\n  LRU Simulation  (Frames = %d)\n", num_frames);
    printf("  ┌────────┬──────────────────────────────┬────────────┐\n");
    printf("  │  Page  │  Frame Contents              │  Result    │\n");
    printf("  ├────────┼──────────────────────────────┼────────────┤\n");

    for (int i = 0; i < ref_len; i++) {
        int pg  = ref_str[i];
        int hit = 0;
        int victim = -1;
        tick++;

        for (int f = 0; f < num_frames; f++) {
            if (mem[f] == pg) {
                hit           = 1;
                hit_count++;
                last_used[f]  = tick;
                break;
            }
        }

        if (!hit) {
            /* First try to fill an empty frame */
            for (int f = 0; f < num_frames; f++)
                if (mem[f] == -1) { victim = f; break; }

            /* If no empty frame, evict least recently used */
            if (victim == -1) {
                int oldest_tick = last_used[0];
                victim = 0;
                for (int f = 1; f < num_frames; f++)
                    if (last_used[f] < oldest_tick) {
                        oldest_tick = last_used[f];
                        victim      = f;
                    }
            }
            mem      [victim] = pg;
            last_used[victim] = tick;
            (*fault_count)++;
        }

        /* Build frame display string */
        char frame_buf[48] = {0};
        char cell[8];
        for (int f = 0; f < num_frames; f++) {
            if (mem[f] == -1) snprintf(cell, sizeof(cell), "[  ]");
            else              snprintf(cell, sizeof(cell), "[%2d]", mem[f]);
            strncat(frame_buf, cell,
                    sizeof(frame_buf) - strlen(frame_buf) - 1);
            if (f < num_frames - 1)
                strncat(frame_buf, " ",
                        sizeof(frame_buf) - strlen(frame_buf) - 1);
        }
        printf("  │  P%-4d │ %-28s │ %-10s │\n",
               pg, frame_buf, hit ? "HIT" : "PAGE FAULT");
    }
    printf("  └────────┴──────────────────────────────┴────────────┘\n");

    frames[0] = hit_count;
    return hit_count;
}

/* ═══════════════════════════════════════════════════════
   Public: Memory Management Demo Entry Point
   ═══════════════════════════════════════════════════════ */
void run_memory_demo(void) {
    printf("\n  ╔═══════════════════════════════════════════════════╗\n");
    printf("  ║     MODULE 5  –  Memory Management Demo           ║\n");
    printf("  ╚═══════════════════════════════════════════════════╝\n");

    /*
     * Page reference string: 5 unique pages accessed repeatedly.
     * Page ID -> Customer: 1=Hamza, 2=Zaryab, 3=Saad, 4=TechVentures, 5=Omar
     */
    /* 5 unique pages with good locality – produces real hits at 3 frames */
    int ref_str[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3,
                     4, 5, 3, 2, 1, 4, 5, 2, 3, 1};
    int ref_len   = (int)(sizeof(ref_str) / sizeof(ref_str[0]));
    int buf[NUM_FRAMES];

    /* Print reference string */
    printf("\n  Page Reference String (%d accesses):\n  ", ref_len);
    for (int i = 0; i < ref_len; i++)
        printf("%d%s", ref_str[i], (i < ref_len - 1) ? " -> " : "\n");

    /* ─── 3-Frame test ─── */
    int num_frames  = 3;
    printf("\n  ══ Test A: %d Frames ══\n", num_frames);

    printf("\n  ── Algorithm: FIFO ──");
    int fifo_faults_3, fifo_hits_3;
    fifo_hits_3 = run_fifo(ref_str, ref_len, buf, num_frames, &fifo_faults_3);

    printf("\n  ── Algorithm: LRU ──");
    int lru_faults_3, lru_hits_3;
    lru_hits_3  = run_lru (ref_str, ref_len, buf, num_frames, &lru_faults_3);

    /* Comparison table */
    printf("\n  ┌──────────────────┬─────────────────┬─────────────────┐\n");
    printf("  │  Metric          │      FIFO       │      LRU        │\n");
    printf("  ├──────────────────┼─────────────────┼─────────────────┤\n");
    printf("  │  Page Faults     │ %-15d │ %-15d │\n",
           fifo_faults_3, lru_faults_3);
    printf("  │  Page Hits       │ %-15d │ %-15d │\n",
           fifo_hits_3,   lru_hits_3);
    printf("  │  Hit Ratio       │ %-13.1f%%  │ %-13.1f%%  │\n",
           100.0 * fifo_hits_3   / ref_len,
           100.0 * lru_hits_3    / ref_len);
    printf("  │  Fault Rate      │ %-13.1f%%  │ %-13.1f%%  │\n",
           100.0 * fifo_faults_3 / ref_len,
           100.0 * lru_faults_3  / ref_len);
    printf("  └──────────────────┴─────────────────┴─────────────────┘\n");

    const char *winner3 = (lru_faults_3  < fifo_faults_3)  ? "LRU"  :
                          (fifo_faults_3 < lru_faults_3)   ? "FIFO" : "Tie";
    printf("  Conclusion (%d frames): %s performs better.\n",
           num_frames, winner3);
    log_event("Memory (3 frames) – FIFO faults=%d  LRU faults=%d  Winner=%s",
              fifo_faults_3, lru_faults_3, winner3);

    /* ─── 4-Frame test ─── */
    num_frames = 4;
    printf("\n  ══ Test B: %d Frames ══\n", num_frames);

    printf("\n  ── Algorithm: FIFO ──");
    int fifo_faults_4, fifo_hits_4;
    fifo_hits_4 = run_fifo(ref_str, ref_len, buf, num_frames, &fifo_faults_4);

    printf("\n  ── Algorithm: LRU ──");
    int lru_faults_4, lru_hits_4;
    lru_hits_4  = run_lru (ref_str, ref_len, buf, num_frames, &lru_faults_4);

    printf("\n  ┌──────────────────┬─────────────────┬─────────────────┐\n");
    printf("  │  Metric          │      FIFO       │      LRU        │\n");
    printf("  ├──────────────────┼─────────────────┼─────────────────┤\n");
    printf("  │  Page Faults     │ %-15d │ %-15d │\n",
           fifo_faults_4, lru_faults_4);
    printf("  │  Page Hits       │ %-15d │ %-15d │\n",
           fifo_hits_4,   lru_hits_4);
    printf("  │  Hit Ratio       │ %-13.1f%%  │ %-13.1f%%  │\n",
           100.0 * fifo_hits_4   / ref_len,
           100.0 * lru_hits_4    / ref_len);
    printf("  │  Fault Rate      │ %-13.1f%%  │ %-13.1f%%  │\n",
           100.0 * fifo_faults_4 / ref_len,
           100.0 * lru_faults_4  / ref_len);
    printf("  └──────────────────┴─────────────────┴─────────────────┘\n");

    const char *winner4 = (lru_faults_4  < fifo_faults_4)  ? "LRU"  :
                          (fifo_faults_4 < lru_faults_4)   ? "FIFO" : "Tie";
    printf("  Conclusion (%d frames): %s performs better.\n\n",
           num_frames, winner4);
    log_event("Memory (4 frames) – FIFO faults=%d  LRU faults=%d  Winner=%s",
              fifo_faults_4, lru_faults_4, winner4);
}
