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
struct s1_t0_read_rx2tx_extra_k k;
struct s1_t0_read_rx2tx_extra_read_rx2tx_extra_d d;
	
%%
    .align
    .param      tcp_tx_process_pending_start

tcp_tx_read_rx2tx_shared_extra_stage1_start:
    CAPRI_OPERAND_DEBUG(d.rcv_mss)
    CAPRI_OPERAND_DEBUG(k.common_phv_pending_rx2tx)
    phvwr           p.to_s5_rcv_mss, d.rcv_mss
    seq             c1, k.common_phv_pending_rx2tx, 1
    bcf             [c1], tcp_tx_start_pending
    nop
    nop.e
    nop
tcp_tx_start_pending:
    phvwr           p.common_phv_pending_ack_send, d.pending_ack_send
    phvwr           p.common_phv_pending_snd_una_update, d.pending_snd_una_update

    // Debug : Don't send ack based on dol flag
    seq             c1, k.common_phv_debug_dol_dont_send_ack, 1
    seq             c2, d.pending_snd_una_update, 1
    bcf             [c1 & !c2], tcp_tx_rx2tx_extra_end
    CAPRI_CLEAR_TABLE_VALID(0)

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tx_process_pending_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)
tcp_tx_rx2tx_extra_end:
    nop.e
    nop

