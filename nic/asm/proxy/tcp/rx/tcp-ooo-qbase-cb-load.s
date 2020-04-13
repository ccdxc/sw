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
#include "INGRESS_s5_t2_tcp_rx_k.h"

struct phv_ p;
struct s5_t2_tcp_rx_k_ k;
struct s5_t2_tcp_rx_ooo_qbase_cb_load_d d;

%%
    .param          tcp_rx_write_ooq_stage_start
    .param          TCP_PROXY_STATS
    .align

tcp_ooo_qbase_cb_load:
    add             r1, k.t2_s2s_ooo_queue_id, r0
    .brbegin
    br r1[1:0]
    nop
    .brcase 0
        seq         c1, d.ooo_qbase_addr0, r0
        tblwr.c1    d.ooo_qbase_addr0, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr0
        b           tcp_ooo_launch_dma
        nop
    .brcase 1
        seq         c1, d.ooo_qbase_addr1, r0
        tblwr.c1    d.ooo_qbase_addr1, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr1
        b           tcp_ooo_launch_dma
        nop
    .brcase 2
        seq         c1, d.ooo_qbase_addr2, r0
        tblwr.c1    d.ooo_qbase_addr2, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr2
        b           tcp_ooo_launch_dma
        nop
    .brcase 3
        seq         c1, d.ooo_qbase_addr3, r0
        tblwr.c1    d.ooo_qbase_addr3, k.t2_s2s_ooo_qbase_addr
        phvwr.!c1   p.t2_s2s_ooo_qbase_addr, d.ooo_qbase_addr3
        b           tcp_ooo_launch_dma
        nop
    .brend
tcp_ooo_launch_dma:
    CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS,
                tcp_rx_write_ooq_stage_start, k.common_phv_qstate_addr,
                TCP_TCB_RX_DMA_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

/*
 * We have received in-order data that requires dequeueing from OOQ
 */
.align
tcp_ooo_qbase_cb_load_in_order:
    CAPRI_CLEAR_TABLE_VALID(2)

    /*
     * k.s3_t2_s2s_ooo_rx2t2x_ready_(len and trim) has info on which queues to
     * dequeue. len != 0 means the queue has to be dequeued and trim is the
     * amount of bytes we need to trim from the head of the queue.
     *
     * r1 = phv offset of rx2tx ooq entry
     * r2 = number of entries
     * r4 = number of entries * 8
     * r5 = address in rx2tx queue to DMA into
     */
    add             r2, r0, r0

    /*
     * r2 uses ooq_rx2tx_queue_entry_t format
     */
    add             r1, r0, offsetof(struct phv_, ooq_rx2tx_queue_entry1_entry)

    sne             c1, k.s3_t2_s2s_ooo_rx2tx_ready_len0, 0
    add.c1          r3, k.s3_t2_s2s_ooo_rx2tx_ready_trim0, d.ooo_qbase_addr0, 30
    tblwr.c1        d.ooo_qbase_addr0, 0
    or.c1           r3, r3, k.s3_t2_s2s_ooo_rx2tx_ready_len0, 14
    phvwrp.c1       r1, 0, 64, r3
    sub.c1          r1, r1, 64
    add.c1          r2, r2, 1

    sne             c1, k.s3_t2_s2s_ooo_rx2tx_ready_len1, 0
    add.c1          r3, k.s3_t2_s2s_ooo_rx2tx_ready_trim1, d.ooo_qbase_addr1, 30
    tblwr.c1        d.ooo_qbase_addr1, 0
    or.c1           r3, r3, k.s3_t2_s2s_ooo_rx2tx_ready_len1, 14
    phvwrp.c1       r1, 0, 64, r3
    sub.c1          r1, r1, 64
    add.c1          r2, r2, 1

    sne             c1, k.s3_t2_s2s_ooo_rx2tx_ready_len2, 0
    add.c1          r3, k.s3_t2_s2s_ooo_rx2tx_ready_trim2, d.ooo_qbase_addr2, 30
    tblwr.c1        d.ooo_qbase_addr2, 0
    or.c1           r3, r3, k.s3_t2_s2s_ooo_rx2tx_ready_len2, 14
    phvwrp.c1       r1, 0, 64, r3
    sub.c1          r1, r1, 64
    add.c1          r2, r2, 1

    sne             c1, k.s3_t2_s2s_ooo_rx2tx_ready_len3, 0
    add.c1          r3, k.s3_t2_s2s_ooo_rx2tx_ready_trim3, d.ooo_qbase_addr3, 30
    tblwr.c1        d.ooo_qbase_addr3, 0
    or.c1           r3, r3, k.s3_t2_s2s_ooo_rx2tx_ready_len3, 14
    phvwrp.c1       r1, 0, 64, r3
    sub.c1          r1, r1, 64
    add.c1          r2, r2, 1

    seq             c1, r2, 0
    b.c1            tcp_ooo_qbase_cb_load_end       // ERROR

    /*
     * Check if we have space available in rx2tx queue
     * ( r2 <= (ci - (pi + 1)) )
     */
    add             r6, d.ooo_rx2tx_qbase_pi, 1
    sub             r6, d.ooo_rx2tx_ci, r6
    add             r6, r0, r6[ASIC_OOO_RX2TX_RING_SLOTS_SHIFT-1:0]
    sle             c1, r2, r6
    b.!c1           tcp_ooo_qbase_rx2tx_full        // ERROR

    /*
     * Setup DMA commands for rx2tx
     *
     * r2 = number of entries
     * r4 = number of entries * 8
     * r5 = address in rx2tx queue to DMA into
     */
    sll             r5, d.ooo_rx2tx_qbase_pi, NIC_OOQ_RX2TX_ENTRY_SIZE_SHIFT
    add             r5, r5, d.ooo_rx2tx_qbase
    sll             r4, r2, NIC_OOQ_RX2TX_ENTRY_SIZE_SHIFT
 
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(rx2tx_ooq_ready_dma_cmd, r5, ooq_rx2tx_queue_entry1_entry, r4)

    tblmincr       d.ooo_rx2tx_qbase_pi, ASIC_OOO_RX2TX_RING_SLOTS_SHIFT, r2
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_FENCE(rx2tx_ooq_doorbell_dma_cmd, LIF_TCP,
                        TCP_OOO_RX2TX_QTYPE, k.common_phv_fid,
                        TCP_OOO_RX2TX_RING0, d.ooo_rx2tx_qbase_pi,
                        rx2tx_ooq_ready_db_data_pid,
                        rx2tx_ooq_ready_db_data_index)

    
tcp_ooo_qbase_cb_load_end:
    nop.e
    nop

tcp_ooo_qbase_rx2tx_full:
    phvwri          p.p4_intr_global_drop, 1
    addui           r2, r0, hiword(TCP_PROXY_STATS)
    addi            r2, r2, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r2, TCP_PROXY_STATS_OOQ_RX2TX_FULL, 1)
    nop.e
    nop
