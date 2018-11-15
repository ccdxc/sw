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
    .param          tcp_retx_reschedule_tx

tcp_retx_process_start:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_cc_and_fra_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_CC_AND_FRA_OFFSET, TABLE_SIZE_512_BITS)

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
    j.c1            tcp_retx_reschedule_tx
    phvwr           p.to_s5_len, k.to_s3_len

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

