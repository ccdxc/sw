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
    .param          tcp_xmit_process_start
    .param          tcp_tx_read_nmdr_gc_idx_start
    .param          TCP_PROXY_STATS

tcp_clean_retx_process_start:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_xmit_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)

tcp_retx_snd_una_update:
    seq             c1, k.t0_s2s_clean_retx_state, TCP_RST
    seq             c2, d.tx_rst_sent, 1
    bcf             [c1 | c2], tcp_retx_rst_handling

    /*
     * retx_snd_una should never go beyond snd_una
     */
    add             r1, d.retx_snd_una, k.t0_s2s_clean_retx_len1
    scwlt           c1, k.common_phv_snd_una, r1
    b.c1            tcp_retx_cleaningup_more_error

    /*
     * Handle the case where the peer has acknowledged less data
     * than in the descriptor
     */
    sub             r1, k.common_phv_snd_una, d.retx_snd_una
    slt             c1, r1[31:0], k.t0_s2s_clean_retx_len1
    b.c1            tcp_retx_stretch_ack

tcp_retx_calc_num_pkts_freed_start:
    // r1 = snd_una - retx_snd_una
    // r4 = total bytes freed (minimum = len1)
    // r5 = number of descriptors freed (minimum 1)
    // r6 = num pkts in retx queue that can be freed
    add             r5, r0, 1
    add             r4, r0, k.t0_s2s_clean_retx_len1
    add             r6, r0, k.t0_s2s_clean_retx_num_retx_pkts

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    nop
    add             r4, r4, k.t0_s2s_clean_retx_len2
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len2
    add.c1          r5, r5, 1

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    add             r4, r4, k.t0_s2s_clean_retx_len3
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len3
    add.c1          r5, r5, 1

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    add             r4, r4, k.t0_s2s_clean_retx_len4
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len4
    add.c1          r5, r5, 1

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    add             r4, r4, k.t0_s2s_clean_retx_len5
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len5
    add.c1          r5, r5, 1

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    add             r4, r4, k.t0_s2s_clean_retx_len6
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len6
    add.c1          r5, r5, 1

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    add             r4, r4, k.t0_s2s_clean_retx_len7
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len7
    add.c1          r5, r5, 1

    beq             r6, r5, tcp_retx_calc_num_pkts_freed_done
    add             r4, r4, k.t0_s2s_clean_retx_len8
    slt             c1, r4, r1[31:0]
    b.!c1           tcp_retx_calc_num_pkts_freed_done
    sub.!c1         r4, r4, k.t0_s2s_clean_retx_len8
    add.c1          r5, r5, 1
tcp_retx_calc_num_pkts_freed_done:

    seq             c1, k.common_phv_fin, 1
    tbladd          d.retx_snd_una, r4
    tbladd.c1       d.retx_snd_una, 1
    phvwr           p.t1_s2s_num_pkts_freed, r5
    phvwr           p.t0_s2s_packets_out_decr, r5

    seq             c1, k.common_phv_pending_asesq, 1
    b.!c1           tcp_retx_cleanup_sesq
    // get retx_ci offset in TCP CB
    add             r1, k.common_phv_qstate_addr, TCP_TCB_RX2TX_RETX_CI_OFFSET
tcp_retx_cleanup_asesq:
    // increment retx_ci before writing
    add             r2, k.to_s3_sesq_retx_ci, 0
    mincr           r2, CAPRI_ASESQ_RING_SLOTS_SHIFT, r5
    // write new asesq retx ci value into TCP CB
    add             r1, r1, 2
    memwr.h         r1, r2
    // remove barrier
    add             r1, r1, 2
    memwr.b         r1, 0
    b               tcp_retx_cleanup_sesq_end
tcp_retx_cleanup_sesq:
    // increment retx_ci before writing
    add             r2, k.to_s3_sesq_retx_ci, 0
    mincr           r2, CAPRI_SESQ_RING_SLOTS_SHIFT, r5
    // write new sesq retx ci value into TCP CB
    memwr.h         r1, r2
    // remove barrier
    add             r1, r1, 4
    memwr.b         r1, 0
    // write new sesq retx ci into TCP producer (TLS for example)
    add             r1, r0, d.sesq_ci_addr
    memwr.h         r1, r2
tcp_retx_cleanup_sesq_end:

    /*
     * if we have more work to do, free descriptor and continue
     * otherwise, set clean_retx_ci to current clean_retx_pi
     */
    scwlt           c1, d.retx_snd_una, k.common_phv_snd_una
    b.c1            free_descriptor
    phvwr.c1        p.common_phv_partial_retx_cleanup, 1

    // if window is not restricted currently, ring clean_retx doorbell
    // and continue, else we need to schedule ourselves to transmit packets
    // that are held in sesq, as window may have opened up
    bbeq            k.to_s3_window_not_restricted, 1, tcp_retx_clean_retx_doorbell

tcp_retx_handle_window_opening_up:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx        r4, r3

tcp_retx_clean_retx_doorbell:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    //tbladd          d.tx_ring_pi, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_CLEAN_RETX, k.to_s3_clean_retx_pi)
    memwr.dx        r4, r3

free_descriptor:
    /*
     * TODO handle freeing pkts queued to asesq
     */
    seq             c1, k.common_phv_pending_asesq, 1
    b.c1            skip_free_descriptor

    seq             c1, k.common_phv_debug_dol_bypass_barco, 1
    b.c1            free_rnmdr
free_tnmdr:
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, tcp_tx_read_nmdr_gc_idx_start,
                        d.gc_base, TABLE_SIZE_32_BITS)
    nop.e
    nop
free_rnmdr:
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, tcp_tx_read_nmdr_gc_idx_start,
                        d.gc_base, TABLE_SIZE_32_BITS)
    nop.e
    nop

skip_free_descriptor:
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    nop

tcp_retx_rst_handling:
    /*
     * reschedule until we clean up all of retx queue
     */

     // We need to free sesq[sesq_retx_ci]
    phvwr           p.t0_s2s_packets_out_decr, 1

    tbladd          d.retx_snd_una, k.t0_s2s_clean_retx_len1
    phvwr           p.t1_s2s_num_pkts_freed, 1

    seq             c1, k.common_phv_pending_asesq, 1
    b.!c1           tcp_retx_rst_cleanup_sesq
    // get retx_ci offset in TCP CB
    add             r1, k.common_phv_qstate_addr, TCP_TCB_RX2TX_RETX_CI_OFFSET
tcp_retx_rst_cleanup_asesq:
    // increment retx_ci before writing
    add             r2, k.to_s3_sesq_retx_ci, 0
    mincr           r2, CAPRI_ASESQ_RING_SLOTS_SHIFT, 1
    // write new asesq retx ci value into TCP CB
    add             r1, r1, 2
    memwr.h         r1, r2
    // remove barrier
    add             r1, r1, 2
    memwr.h         r1, 0
    b               tcp_retx_cleanup_sesq_end
tcp_retx_rst_cleanup_sesq:
    // increment retx_ci before writing
    add             r2, k.to_s3_sesq_retx_ci, 0
    mincr           r2, CAPRI_SESQ_RING_SLOTS_SHIFT, 1
    // write new sesq retx ci value into TCP CB
    memwr.h         r1, r2
    // remove barrier
    add             r1, r1, 4
    memwr.h         r1, 0
    // write new sesq retx ci into TCP producer (TLS for example)
    add             r1, r0, d.sesq_ci_addr
    memwr.h         r1, r2
tcp_retx_rst_cleanup_sesq_end:

    b               free_descriptor
    nop

tcp_retx_end_program:
    // retransmission timer fired, but we have nothing to retransmit
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

tcp_retx_stretch_ack:
    tbladd          d.stretch_ack_cnt, 1
    addui           r2, r0, hiword(TCP_PROXY_STATS)
    addi            r2, r2, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r2, TCP_PROXY_STATS_RETX_STRETCH_ACK, 1)
    b               tcp_retx_remove_barrier_and_end_program

/*
 * We reach here for trying to clean up more than current snd_una or if
 * snd_wnd has changed (window advertised from peer)
 */
tcp_retx_cleaningup_more_error:
    addui           r2, r0, hiword(TCP_PROXY_STATS)
    addi            r2, r2, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r2, TCP_PROXY_STATS_RETX_NOP_SCHEDULE, 1)

    // We can reach here for window change from peer without advancing snd_una
    // in which case, check if window is restricted and we can send more
    // packets
    // 
    // We can also reach here when limited_transmit != 0 (on receipt of the
    // first 2 dupacks) in which case we want to transmit even if there is no
    // window

    // if window is not restricted currently, quit
    // else if current window is greater than old window,
    // we need to schedule ourselves to transmit packets
    // that are held in sesq
    seq             c1, k.to_s3_window_not_restricted, 1
    sle             c2, k.t0_s2s_snd_wnd, d.last_snd_wnd
    seq.c2          c2, k.t0_s2s_clean_retx_limited_transmit, 0
    bcf             [c1 | c2], tcp_retx_remove_barrier_and_end_program

tcp_retx_handle_window_change_or_limited_transmit:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    tbladd          d.tx_ring_pi, 1
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_PENDING_TX, d.tx_ring_pi)
    memwr.dx        r4, r3

tcp_retx_remove_barrier_and_end_program:
    phvwri          p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE_VALID(0)

    // ring doorbell to set clean_retx_ci
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_CLEAN_RETX, k.to_s3_clean_retx_pi)
    memwr.dx        r4, r3

    // get barrier offset in TCP CB (retx_ci + 4)
    add             r1, k.common_phv_qstate_addr, TCP_TCB_RX2TX_RETX_CI_OFFSET + 4
    // remove barrier
    memwr.b.e       r1, 0
    nop
