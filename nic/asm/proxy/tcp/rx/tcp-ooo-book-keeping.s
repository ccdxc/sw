/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t2_tcp_rx_k.h"

struct phv_ p;
struct s2_t2_tcp_rx_k_ k;
struct s2_t2_tcp_rx_ooo_book_keeping_d d;

%%
    .align
    .param          tcp_ooo_processing_launch_dummy1
    .param          tcp_ooq_alloc_idx_start
tcp_ooo_book_keeping:
    /*
     * Check if we need to queue to the end of an existing ring
     */
    seq             c1, k.t2_s2s_seq, d.end_seq0
    sne             c2, d.tail_index0, 0
    sne             c3, d.tail_index0, TCP_OOO_QUEUE_NUM_ENTRIES
    bcf             [c1 & c2 & c3], tcp_ooo_book_keeping_enqueue_tail_of_queue0

    seq             c1, k.t2_s2s_seq, d.end_seq1
    sne             c2, d.tail_index1, 0
    sne             c2, d.tail_index1, TCP_OOO_QUEUE_NUM_ENTRIES
    bcf             [c1 & c2 & c3], tcp_ooo_book_keeping_enqueue_tail_of_queue1

    seq             c1, k.t2_s2s_seq, d.end_seq2
    sne             c2, d.tail_index2, 0
    sne             c2, d.tail_index2, TCP_OOO_QUEUE_NUM_ENTRIES
    bcf             [c1 & c2 & c3], tcp_ooo_book_keeping_enqueue_tail_of_queue2

    seq             c1, k.t2_s2s_seq, d.end_seq3
    sne             c2, d.tail_index3, 0
    sne             c2, d.tail_index3, TCP_OOO_QUEUE_NUM_ENTRIES
    bcf             [c1 & c2 & c3], tcp_ooo_book_keeping_enqueue_tail_of_queue3

    /*
     * We need to allocate a ring
     */
    seq             c1, d.tail_index0, r0
    bcf             [c1], tcp_ooo_book_keeping_begin_use_ooo_queue0

    seq             c1, d.tail_index1, r0
    bcf             [c1], tcp_ooo_book_keeping_begin_use_ooo_queue1

    seq             c1, d.tail_index2, r0
    bcf             [c1], tcp_ooo_book_keeping_begin_use_ooo_queue2

    seq             c1, d.tail_index3, r0
    bcf             [c1], tcp_ooo_book_keeping_begin_use_ooo_queue3
    nop

    tbladd.f        d.ooo_queue_full, 1
    phvwri.e        p.p4_intr_global_drop, 1
    phvwri          p.common_phv_ooo_alloc_fail, 1


tcp_ooo_book_keeping_begin_use_ooo_queue0:
    tblwr           d.start_seq0, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq0, r1
    tbladd          d.tail_index0, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, 0

tcp_ooo_book_keeping_begin_use_ooo_queue1:
    tblwr           d.start_seq1, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq1, r1
    tbladd          d.tail_index1, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, 1

tcp_ooo_book_keeping_begin_use_ooo_queue2:
    tblwr           d.start_seq2, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq2, r1
    tbladd          d.tail_index2, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, 2

tcp_ooo_book_keeping_begin_use_ooo_queue3:
    tblwr           d.start_seq3, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq3, r1
    tbladd          d.tail_index3, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, 3

tcp_ooo_book_keeping_enqueue_tail_of_queue0:
    tbladd          d.end_seq0, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index0
    tbladd          d.tail_index0, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, 0

tcp_ooo_book_keeping_enqueue_tail_of_queue1:
    tbladd          d.end_seq1, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index1
    tbladd          d.tail_index1, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, 1

tcp_ooo_book_keeping_enqueue_tail_of_queue2:
    tbladd          d.end_seq2, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index2
    tbladd          d.tail_index2, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, 2

tcp_ooo_book_keeping_enqueue_tail_of_queue3:
    tbladd          d.end_seq3, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index3
    tbladd          d.tail_index3, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, 3

tcp_ooo_book_keeping_launch_alloc_queue:
    CAPRI_NEXT_TABLE_READ_i(2, TABLE_LOCK_DIS, tcp_ooq_alloc_idx_start,
                        TCP_OOQ_ALLOC_IDX, TABLE_SIZE_64_BITS)
    nop.e
    nop

tcp_ooo_book_keeping_launch_dummy:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(2, tcp_ooo_processing_launch_dummy1)
    nop.e
    nop

/*
 * TODO : Yet to implement
 *        - Handle pkt in order, freeing up 1 or more queues
 *        - Handle queue alloc fail in subsequent stage (via write back)
 */
