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
	
struct phv_ p;
struct tcp_tx_read_rx2tx_extra_k k;
struct tcp_tx_read_rx2tx_extra_read_rx2tx_extra_d d;
	
%%
    .align
    .param      tcp_tx_process_pending_stage2_start

tcp_tx_read_rx2tx_shared_extra_stage1_start:
    CAPRI_OPERAND_DEBUG(d.rcv_mss)
    CAPRI_OPERAND_DEBUG(k.common_phv_pending_rx2tx)
    phvwr           p.to_s4_rcv_mss, d.rcv_mss
    seq             c1, k.common_phv_pending_rx2tx, 1
    bcf             [c1], tcp_tx_start_pending
    nop
    nop.e
    nop
tcp_tx_start_pending:
    phvwr           p.common_phv_pending_ack_send, d.pending_ack_send

    // Debug : Don't send ack based on dol flag
    seq             c1, k.common_phv_debug_dol_dont_send_ack, 1
    bcf             [c1], tcp_tx_rx2tx_extra_end
    CAPRI_CLEAR_TABLE_VALID(0)

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_process_pending_stage2_start)
tcp_tx_rx2tx_extra_end:
    nop.e
    nop

