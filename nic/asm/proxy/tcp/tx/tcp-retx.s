/*
 *    Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_tcp_tx_k.h"

struct phv_ p;
struct s3_t0_tcp_tx_k_ k;
struct s3_t0_tcp_tx_retx_d d;

%%
    .align
    .param          tcp_cc_and_fra_process_start
    .param          tcp_tx_read_nmdr_gc_idx_start

tcp_retx_process_start:
table_launch_cc_and_fra:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_cc_and_fra_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_CC_AND_FRA_OFFSET, TABLE_SIZE_512_BITS)

    smeqb           c1, k.common_phv_pending_retx_cleanup, \
                        PENDING_RETX_CLEANUP_TRIGGERED_FROM_RX, \
                        PENDING_RETX_CLEANUP_TRIGGERED_FROM_RX
    bcf             [c1], tcp_retx_snd_una_update_from_rx

    smeqb           c1, k.common_phv_pending_retx_cleanup, \
                        PENDING_RETX_CLEANUP_TRIGGERED_FROM_TX, \
                        PENDING_RETX_CLEANUP_TRIGGERED_FROM_TX
    bcf             [c1], tcp_retx_snd_una_update
    memwr.h.c1      d.sesq_ci_addr, k.to_s3_sesq_retx_ci

    seq             c1, k.common_phv_pending_rto, 1
    seq             c2, k.common_phv_pending_fast_retx, 1
    bcf             [c1 | c2], tcp_retx_retransmit

    seq             c1, k.common_phv_pending_sesq, 1
    seq.!c1         c1, k.common_phv_pending_asesq, 1

    b.c1            tcp_retx_enqueue
    nop

    nop.e
    nop


tcp_retx_enqueue:
    add             r2, k.to_s3_addr, k.to_s3_offset
    phvwr           p.to_s5_addr, r2
    phvwr           p.to_s5_offset, k.to_s3_offset

    // If sending RST, reschedule to cleanup retx queue
    seq             c1, k.common_phv_rst, 1
    tblwr.c1        d.tx_rst_sent, 1
    b.c1            tcp_retx_reschedule_tx
    phvwr           p.to_s5_len, k.to_s3_len

    nop.e
    nop

tcp_retx_snd_una_update_from_rx:
    seq             c2, d.tx_ring_scheduled, 1
    b.c2            tcp_retx_snd_una_update_from_rx_end_program

    /*
     * if we have data to be cleaned up,
     * schedule ourselves again
     */
    seq             c1, k.t0_s2s_state, TCP_RST
    seq.!c1         c1, d.tx_rst_sent, 1
    sle             c2, k.common_phv_snd_una, d.retx_snd_una
    bcf             [c2 & !c1], tcp_retx_snd_una_update_from_rx_end_program

    /*
     * reschedule
     */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
    tblwr           d.tx_ring_scheduled, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx        r4, r3

tcp_retx_snd_una_update_from_rx_end_program:
    CAPRI_CLEAR_TABLE_VALID(0)
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    nop

tcp_retx_snd_una_update:
    seq             c1, k.t0_s2s_state, TCP_RST
    seq             c2, d.tx_rst_sent, 1
    bcf             [c1 | c2], tcp_retx_rst_handling

    tblwr           d.tx_ring_scheduled, 0
    /*
     * We need to free sesq[sesq_retx_ci]
     */
    phvwr           p.t1_s2s_free_desc_addr, k.to_s3_sesq_desc_addr
    phvwr           p.t0_s2s_packets_out_decr, 1

    /*
     * TODO : We need to handle (atleast detect) the case when
     * peer acknowledges less than data in descriptor. This is more
     * of an error case. Under normal conditions this should not 
     * happen.
     *
     * Let's atleast increment stats for this case
     */
    sub             r1, k.common_phv_snd_una, d.retx_snd_una
    slt             c1, r1, k.to_s3_len
    tbladd.c1       d.partial_ack_cnt, 1

    tbladd          d.retx_snd_una, k.to_s3_len
    seq             c1, k.common_phv_fin, 1
    tbladd.c1       d.retx_snd_una, 1

    /*
     * if we still have more data to be cleaned up,
     * schedule ourselves again
     */
    sle             c1, k.common_phv_snd_una, d.retx_snd_una
    b.c1            free_descriptor

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
    tblwr           d.tx_ring_scheduled, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx        r4, r3
    phvwr           p.common_phv_partial_retx_cleanup, 1

free_descriptor:
    /*
     * TODO handle freeing pkts queued to asesq
     */
    seq             c1, k.common_phv_pending_asesq, 1
    b.c1            skip_free_descriptor

    seq             c1, k.common_phv_debug_dol_bypass_barco, 1
    b.c1            free_rnmdr
free_tnmdr:
    CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tcp_tx_read_nmdr_gc_idx_start,
                        TCP_TNMDR_GC_IDX, TABLE_SIZE_32_BITS)
    nop.e
    nop
free_rnmdr:
    CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tcp_tx_read_nmdr_gc_idx_start,
                        TCP_RNMDR_GC_IDX, TABLE_SIZE_32_BITS)
    nop.e
    nop

skip_free_descriptor:
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    nop

tcp_retx_retransmit:
    phvwr           p.t0_s2s_snd_nxt, d.retx_snd_una
    add             r2, k.to_s3_addr, k.to_s3_offset
    phvwr           p.to_s6_xmit_cursor_addr, r2
    phvwr           p.to_s6_xmit_cursor_len, k.to_s3_len
    phvwri          p.to_s6_pending_tso_data, 1
    nop.e
    nop

tcp_retx_rst_handling:
    /*
     * reschedule until we clean up all of retx queue
     */

     // We need to free sesq[sesq_retx_ci]
    phvwr           p.t1_s2s_free_desc_addr, k.to_s3_sesq_desc_addr
    phvwr           p.t0_s2s_packets_out_decr, 1

    tbladd          d.retx_snd_una, k.to_s3_len

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
    tblwr           d.tx_ring_scheduled, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx        r4, r3
    b               free_descriptor
    nop

tcp_retx_reschedule_tx:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
    tblwr           d.tx_ring_scheduled, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx.e      r4, r3
    nop

tcp_retx_end_program:
    // retransmission timer fired, but we have nothing to retransmit
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

