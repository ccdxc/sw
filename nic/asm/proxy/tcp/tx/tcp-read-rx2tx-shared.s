/*
 *      Implements the rx2tx shared state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_read_rx2tx_k k;
struct tcp_tx_read_rx2tx_read_rx2tx_d d;
	
%%
        .align
        .param      tcp_tx_read_rx2tx_shared_extra_stage1_start
        .param      tcp_tx_sesq_read_ci_stage1_start
	
tcp_tx_read_rx2tx_shared_process:
        phvwr           p.common_phv_fid, k.p4_txdma_intr_qid
        phvwr           p.common_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
#if 0
        /* Trigger any pending timer bookkeeping from rx */
        sne             c1, d.pending_ft_clear, r0
        bcf             [c1], ft_clear
        nop
        sne             c2, d.pending_ft_reset, r0
        bcf             [c2], ft_reset
        nop
ft_clear:
        b               write_phv
        nop
ft_reset:	
        CAPRI_TIMER_START(LIF_TCP, 0, k.p4_txdma_intr_qid, TCP_SCHED_RING_FT, d.rto_deadline)
write_phv:	
#endif
        /* Write the entire d-vector to p-vector for tx2rx shared state */
        phvwr           p.{rx2tx_snd_una...rx2tx_snd_cwnd}, d.{snd_una...snd_cwnd}

        CAPRI_OPERAND_DEBUG(d.snd_wnd)
        CAPRI_OPERAND_DEBUG(d.snd_wl1)
        CAPRI_OPERAND_DEBUG(d.snd_cwnd)
        phvwr           p.t0_s2s_snd_cwnd, d.snd_cwnd
        phvwr           p.t0_s2s_snd_wnd, d.snd_wnd

        // TODO check pi != ci
        phvwr           p.to_s2_sesq_cidx, d.{ci_0}.hx
        add             r3, d.{sesq_base}.wx, d.{ci_0}.hx, NIC_SESQ_ENTRY_SIZE_SHIFT
        phvwr           p.to_s1_sesq_ci_addr, r3

        CAPRI_NEXT_TABLE0_READ(k.p4_txdma_intr_qid, TABLE_LOCK_EN,
                            tcp_tx_read_rx2tx_shared_extra_stage1_start,
                            k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33},
                            TCP_TCB_TABLE_ENTRY_SIZE_SHFT, TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET,
                            TABLE_SIZE_512_BITS)
        CAPRI_NEXT_IDX1_READ(TABLE_LOCK_DIS, tcp_tx_sesq_read_ci_stage1_start,
                         r3, TABLE_SIZE_64_BITS)
        nop.e
        nop
