/*
 *	Implements the rx2tx shared state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	CAPRI_QSTATE_HEADER_COMMON

	CAPRI_QSTATE_HEADER_RING(0)

	CAPRI_QSTATE_HEADER_RING(1)

	CAPRI_QSTATE_HEADER_RING(2)

	CAPRI_QSTATE_HEADER_RING(3)

	CAPRI_QSTATE_HEADER_RING(4)

	CAPRI_QSTATE_HEADER_RING(5)
	
	
	/* State needed by RX but updated by TX pipeline */
	TCB_RX2TX_SHARED_STATE
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

	
%%
	
flow_read_rx2tx_shared_process_start:
	/* Write the entire d-vector to p-vector for tx2rx shared state */
	phvwr.e		p.{snd_una...snd_cwnd}, d.{snd_una...snd_cwnd}
	nop
#if 0
	.brbegin
	brpri		r7[5:0], [0,1,2,3,4,5]
	nop
	.brcase 0
	b tcp_sesq_process
	nop
	.brcase 1
	b tcp_pending_process
	nop
	.brcase 2
	b tcp_fast_timer_process
	nop
	.brcase 3
	b tcp_slow_timer_process
	nop
	.brcase 4
	b tcp_tso_process
	nop
	.brcase 5
	b tcp_ooo_process
	nop
	.brcase 6
	nop
	nop.e
	.brend
tcp_sesq_process:
tcp_pending_process:
tcp_fast_timer_process:
tcp_slow_timer_process:
tcp_tso_process:
tcp_ooo_process:
#endif
	nop
	nop.e
