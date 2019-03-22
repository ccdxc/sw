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

#define QUEUE0          0
#define QUEUE1          1
#define QUEUE2          2
#define QUEUE3          3

#define QUEUE0_BIT      0
#define QUEUE1_BIT      1
#define QUEUE2_BIT      2
#define QUEUE3_BIT      3

#define QUEUE0_MASK     (1 << QUEUE0_BIT)
#define QUEUE1_MASK     (1 << QUEUE1_BIT)
#define QUEUE2_MASK     (1 << QUEUE2_BIT)
#define QUEUE3_MASK     (1 << QUEUE3_BIT)

%%
    .align
    .param          tcp_ooo_processing_launch_dummy1
    .param          tcp_ooq_alloc_idx_start
tcp_ooo_book_keeping:
    /*
     * Check if we need to queue to the end of an existing ring
     * TODO : handle duplicate OOQ packets
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
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE0

tcp_ooo_book_keeping_begin_use_ooo_queue1:
    tblwr           d.start_seq1, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq1, r1
    tbladd          d.tail_index1, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE1

tcp_ooo_book_keeping_begin_use_ooo_queue2:
    tblwr           d.start_seq2, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq2, r1
    tbladd          d.tail_index2, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE2

tcp_ooo_book_keeping_begin_use_ooo_queue3:
    tblwr           d.start_seq3, k.t2_s2s_seq
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    tblwr           d.end_seq3, r1
    tbladd          d.tail_index3, 1
    b               tcp_ooo_book_keeping_launch_alloc_queue
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE3

tcp_ooo_book_keeping_enqueue_tail_of_queue0:
    tbladd          d.end_seq0, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index0
    tbladd          d.tail_index0, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE0

tcp_ooo_book_keeping_enqueue_tail_of_queue1:
    tbladd          d.end_seq1, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index1
    tbladd          d.tail_index1, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE1

tcp_ooo_book_keeping_enqueue_tail_of_queue2:
    tbladd          d.end_seq2, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index2
    tbladd          d.tail_index2, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE2

tcp_ooo_book_keeping_enqueue_tail_of_queue3:
    tbladd          d.end_seq3, k.t2_s2s_payload_len
    phvwr           p.t2_s2s_ooo_tail_index, d.tail_index3
    tbladd          d.tail_index3, 1
    b               tcp_ooo_book_keeping_launch_dummy
    phvwr           p.t2_s2s_ooo_queue_id, QUEUE3

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
 *        - Handle queue alloc fail in subsequent stage (via write back)
 */


    .align
tcp_ooo_book_keeping_in_order:
    phvwr           p.{s3_t2_s2s_ooo_rx2tx_ready_trim0...s3_t2_s2s_ooo_rx2tx_ready_len3}, 0
    /*
     * r1 = max seq that is in in-order
     * r2 = bitmask of queues that become in-order
     */
    add             r1, k.t2_s2s_seq, k.t2_s2s_payload_len
    add             r2, r0, r0

/*
 * Check if the in-order packet caused one of the queues to become in-order
 */
tcp_ooo_book_keeping_in_order_check_queue0:
    /*
     * if start_seq <= r1, we can free the queue
     */
    sne             c1, d.tail_index0, 0
    scwle.c1        c1, d.start_seq0, r1
    b.!c1           tcp_ooo_book_keeping_in_order_check_queue1
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len0, d.tail_index0
    tblwr           d.tail_index0, 0
    sub             r3, r1, d.start_seq0
    phvwr           p.s3_t2_s2s_ooo_rx2tx_ready_trim0, r3
    // Set r1 = max(pkt.end_seq, queue.end_seq)
    scwlt           c1, r1, d.end_seq0
    add.c1          r1, r0, d.end_seq0
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    or              r2, r2, QUEUE0_MASK
tcp_ooo_book_keeping_in_order_check_queue1:
    sne             c1, d.tail_index1, 0
    scwle.c1        c1, d.start_seq1, r1
    b.!c1           tcp_ooo_book_keeping_in_order_check_queue2
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len1, d.tail_index1
    tblwr           d.tail_index1, 0
    sub             r3, r1, d.start_seq1
    phvwr           p.s3_t2_s2s_ooo_rx2tx_ready_trim1, r3
    // set r1 = max(pkt.end_seq, queue.end_seq)
    scwlt           c1, r1, d.end_seq1
    add.c1          r1, r0, d.end_seq1
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    or              r2, r2, QUEUE1_MASK
tcp_ooo_book_keeping_in_order_check_queue2:
    sne             c1, d.tail_index2, 0
    scwle.c1        c1, d.start_seq2, r1
    b.!c1           tcp_ooo_book_keeping_in_order_check_queue3
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len2, d.tail_index2
    tblwr           d.tail_index2, 0
    sub             r3, r1, d.start_seq2
    phvwr           p.s3_t2_s2s_ooo_rx2tx_ready_trim2, r3
    // set r1 = max(pkt.end_seq, queue.end_seq)
    scwlt           c1, r1, d.end_seq2
    add.c1          r1, r0, d.end_seq2
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    or              r2, r2, QUEUE2_MASK
tcp_ooo_book_keeping_in_order_check_queue3:
    sne             c1, d.tail_index3, 0
    scwle.c1        c1, d.start_seq3, r1
    b.!c1           tcp_ooo_book_keeping_done
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len3, d.tail_index3
    tblwr           d.tail_index3, 0
    sub             r3, r1, d.start_seq3
    phvwr           p.s3_t2_s2s_ooo_rx2tx_ready_trim3, r3
    // set r1 = max(pkt.end_seq, queue.end_seq)
    scwlt           c1, r1, d.end_seq3
    add.c1          r1, r0, d.end_seq3
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    or              r2, r2, QUEUE3_MASK

tcp_ooo_book_keeping_done:
    /*
     * No queue became in-order
     */

    /*
     * TODO : Send an ack here as we would have suppressed
     * sending of ack in tcp-rx for the new packet that came
     * in-order
     */
    CAPRI_CLEAR_TABLE_VALID(2)
    nop.e
    nop

/*
 * We need to make multiple passes through all queues, as dequeuing from one
 * queue can make packets in another queue in-order. So scan all queues until
 * no we reach a pass where no queue is dequeued from.
 */
tcp_ooo_book_keeping_in_order_scan_all_queues:
scan_queue0:
    smneb           c1, r2, QUEUE0_BIT, QUEUE0_BIT
    sne.c1          c1, d.tail_index0, 0
    scwlt.c1        c1, d.start_seq0, r1
    b.!c1           scan_queue1
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len0, d.tail_index0
    tblwr           d.tail_index0, 0
    or              r2, r2, QUEUE0_MASK
    // set r1 = max seq that is in-order
    scwlt           c1, r1, d.end_seq0
    add.c1          r1, r0, d.end_seq0
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    nop
scan_queue1:
    smneb           c1, r2, QUEUE1_BIT, QUEUE1_BIT
    sne             c1, d.tail_index1, 0
    scwlt.c1        c1, d.start_seq1, r1
    b.!c1           scan_queue2
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len1, d.tail_index1
    tblwr           d.tail_index1, 0
    or              r2, r2, QUEUE1_MASK
    // set r1 = max seq that is in-order
    scwlt           c1, r1, d.end_seq1
    add.c1          r1, r0, d.end_seq1
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    nop
scan_queue2:
    smneb           c1, r2, QUEUE2_BIT, QUEUE2_BIT
    sne             c1, d.tail_index2, 0
    scwlt.c1        c1, d.start_seq2, r1
    b.!c1           scan_queue3
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len2, d.tail_index2
    tblwr           d.tail_index2, 0
    or              r2, r2, QUEUE2_MASK
    // set r1 = max seq that is in-order
    scwlt           c1, r1, d.end_seq2
    add.c1          r1, r0, d.end_seq2
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    nop
scan_queue3:
    smneb           c1, r2, QUEUE3_BIT, QUEUE3_BIT
    sne             c1, d.tail_index3, 0
    scwlt.c1        c1, d.start_seq3, r1
    b.!c1           tcp_ooo_end_scan
    phvwr.c1        p.s3_t2_s2s_ooo_rx2tx_ready_len3, d.tail_index3
    tblwr           d.tail_index3, 0
    or              r2, r2, QUEUE3_MASK
    // set r1 = max seq that is in-order
    scwlt           c1, r1, d.end_seq3
    add.c1          r1, r0, d.end_seq3
    b               tcp_ooo_book_keeping_in_order_scan_all_queues
    nop

tcp_ooo_end_scan:
    // check if OOQ has become empty and inform stage 1 via memwr if so
    seq             c1, d.tail_index0, 0
    seq             c2, d.tail_index1, 0
    seq             c3, d.tail_index2, 0
    seq             c4, d.tail_index3, 0
    bcf             [!c1 | !c2 | !c3 | !c4], tcp_ooo_launch_dummy
    add             r1, k.common_phv_qstate_addr, TCP_TCB_RX_OOQ_NOT_EMPTY
    memwr.b         r1, 0
tcp_ooo_launch_dummy:
    b               tcp_ooo_book_keeping_launch_dummy
    nop
