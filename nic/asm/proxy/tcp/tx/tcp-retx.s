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

    smeqb           c1, k.common_phv_rx_flag, FLAG_SND_UNA_ADVANCED, FLAG_SND_UNA_ADVANCED
    seq             c2, k.common_phv_pending_ack_send, 1
    bcf             [c1 & c2], tcp_retx_reschedule_and_quit
    nop
    bcf             [c1], tcp_retx_snd_una_update

    seq             c1, k.common_phv_pending_rto, 1
    seq             c2, k.common_phv_pending_fast_retx, 1
    bcf             [c1 | c2], tcp_retx_retransmit

    seq             c1, k.common_phv_pending_sesq, 1
    memwr.h.c1      d.sesq_ci_addr, k.to_s3_sesq_retx_ci
    seq.!c1         c1, k.common_phv_pending_asesq, 1

    b.c1            tcp_retx_enqueue
    nop

    nop.e
    nop


tcp_retx_enqueue:
    add             r2, k.to_s3_addr, k.to_s3_offset
    phvwr           p.to_s5_addr, r2
    phvwr           p.to_s5_offset, k.to_s3_offset
    phvwr           p.to_s5_len, k.to_s3_len

    nop.e
    nop


tcp_retx_snd_una_update:
    seq             c1, k.common_phv_snd_una, d.retx_snd_una
    b.c1            tcp_retx_end_program

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
     */
    tbladd          d.retx_snd_una, k.to_s3_len
    seq             c1, k.common_phv_fin, 1
    tbladd.c1       d.retx_snd_una, 1

    /*
     * if we still have more data to be cleaned up,
     * schedule ourselves again
     */
    sub             r1, k.common_phv_snd_una, d.retx_snd_una
    sle             c1, r1, r0
    b.c1            free_descriptor

    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
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

    CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tcp_tx_read_nmdr_gc_idx_start,
                        TCP_NMDR_GC_IDX, TABLE_SIZE_32_BITS)
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

tcp_retx_end_program:
    // retransmission timer fired, but we have nothing to retransmit
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

tcp_retx_reschedule_and_quit:
    phvwr           p.common_phv_partial_retx_cleanup, 1
    tbladd          d.tx_ring_pi, 1
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx.e      r4, r3
    nop.e
    nop
