/*
 *	Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_tcp_tx_k.h"

struct phv_ p;
struct s2_t0_tcp_tx_k_ k;
struct s2_t0_tcp_tx_read_descr_d d;

%%
    .align
    .param          tcp_retx_process_start
    .param          tcp_clean_retx_process_start

tcp_tx_read_descr_start:

    CAPRI_OPERAND_DEBUG(d.A0)
    CAPRI_OPERAND_DEBUG(d.O0)
    CAPRI_OPERAND_DEBUG(d.L0)
    bbeq            k.common_phv_pending_retx_cleanup, 1, tcp_tx_asesq_cleanup
    add             r2, d.{A0}.dx, d.{O0}.wx
    phvwr           p.t0_s2s_addr, r2
    phvwr           p.t0_s2s_len, d.{L0}.wx

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_retx_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

tcp_tx_asesq_cleanup:
    phvwr           p.t0_s2s_clean_retx_len1, d.{L0}.wx
    CAPRI_NEXT_TABLE_READ_OFFSET_e(0, TABLE_LOCK_EN,
                        tcp_clean_retx_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)
    phvwr           p.t0_s2s_clean_retx_num_retx_pkts, 1
