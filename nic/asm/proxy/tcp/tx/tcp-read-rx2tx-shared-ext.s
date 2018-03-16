/*
 *      Implements the rx2tx shared state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_tcp_tx_k.h"
	
struct phv_ p;
struct s0_t0_tcp_tx_k_ k;
struct s0_t0_tcp_tx_read_rx2tx_d d;
	
%%
    .align

tcp_tx_read_rx2tx_shared_process_ext:
    phvwrpair       p.common_phv_fid, k.p4_txdma_intr_qid, \
                        p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr
    phvwr           p.common_phv_snd_una, d.snd_una
    CAPRI_OPERAND_DEBUG(d.snd_wnd)
    phvwrpair       p.t0_s2s_snd_wnd, d.snd_wnd, \
                        p.t0_s2s_rto, d.rto
    phvwr           p.to_s5_state, d.state
    phvwr.f         p.to_s6_rcv_nxt, d.rcv_nxt
    nop.e
    nop
