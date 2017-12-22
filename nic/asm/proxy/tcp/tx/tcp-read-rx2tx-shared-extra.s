/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_tcp_tx_k.h"
	
struct phv_ p;
struct s1_t0_tcp_tx_k_ k;
struct s1_t0_tcp_tx_read_rx2tx_extra_d d;
	
%%
    .align
    .param      tcp_tx_process_pending_start
    .param      tcp_tx_s2_bubble_start

tcp_tx_read_rx2tx_shared_extra_stage1_start:
    CAPRI_OPERAND_DEBUG(d.rcv_mss)
    CAPRI_OPERAND_DEBUG(k.common_phv_pending_rx2tx)
    phvwr           p.to_s6_rcv_mss, d.rcv_mss
    phvwr           p.t0_s2s_snd_ssthresh, d.snd_ssthresh
    phvwri          p.tcp_header_flags, TCPHDR_ACK
    seq             c1, k.common_phv_pending_rx2tx, 1
    bcf             [c1], tcp_tx_start_pending
    seq             c1, k.common_phv_pending_rto, 1
    bcf             [c1], tcp_tx_start_s2_bubble
    nop
    nop.e
    nop
tcp_tx_start_pending:
    // Debug : Don't send ack based on dol flag
    seq             c1, k.common_phv_debug_dol_dont_send_ack, 1
    smeqb           c2, k.common_phv_rx_flag, FLAG_SND_UNA_ADVANCED, FLAG_SND_UNA_ADVANCED

    /*
     * For snd_una_update, the next stage is launched by pending stage,
     * so skip launching the next stage here
     */
    phvwri.c1       p.app_header_table0_valid, 0
    bcf             [c1 | c2], tcp_tx_rx2tx_extra_end

    /*
     * For pending_ack_send, we need to launch the bubble stage
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s2_bubble_start)

tcp_tx_rx2tx_extra_end:
    nop.e
    nop

tcp_tx_start_s2_bubble:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s2_bubble_start)
    nop.e
    nop

