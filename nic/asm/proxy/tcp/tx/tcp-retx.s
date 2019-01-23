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
    .param          tcp_xmit_ack_process_start
    .param          tcp_xmit_idle_process_start
    .param          tcp_tx_read_nmdr_gc_idx_start

tcp_retx_process_start:
    bbeq            k.common_phv_pending_ack_send, 1, tcp_retx_ack_send
    bbeq            k.common_phv_pending_idle, 1, tcp_retx_idle
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_xmit_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)

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
    tblwr           d.last_snd_wnd, k.t0_s2s_snd_wnd

    // If sending RST, reschedule to cleanup retx queue
    seq             c1, k.common_phv_rst, 1
    b.c1            tcp_retx_reschedule_tx
    tblwr.c1        d.tx_rst_sent, 1

    nop.e
    nop

tcp_retx_retransmit:
    phvwr           p.t0_s2s_snd_nxt, d.retx_snd_una
    phvwri          p.to_s5_pending_tso_data, 1
    nop.e
    nop

tcp_retx_reschedule_tx:
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC,
                        DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid,
                        TCP_SCHED_RING_CLEAN_RETX, 0)
    memwr.dx.e      r4, r3
    nop

tcp_retx_ack_send:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_xmit_ack_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)
    /*
     * Check if we need to send ack, else clear table_valid
     */
    seq             c1, k.common_phv_pending_dup_ack_send, 1
    seq             c2, d.last_ack, k.t0_s2s_rcv_nxt
    setcf           c3, [!c1 & c2]
    tblwr.e         d.last_ack, k.t0_s2s_rcv_nxt
    phvwri.c3       p.app_header_table0_valid, 0

tcp_retx_idle:
    CAPRI_NEXT_TABLE_READ_OFFSET_e(0, TABLE_LOCK_EN,
                        tcp_xmit_idle_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)
    nop
