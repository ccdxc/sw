/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_read_rx2tx_extra_k k;
struct tcp_tx_read_rx2tx_extra_read_rx2tx_extra_d d;
	
%%
    .align
    .param      tcp_tx_sesq_read_stage2_start

tcp_tx_read_rx2tx_shared_extra_stage1_start:
	/* Write the entire d-vector to p-vector for tx2rx shared state */
    // TODO
    CAPRI_OPERAND_DEBUG(d.rcv_mss)
    phvwr           p.to_s4_rcv_mss, d.rcv_mss
    nop.e
    nop
