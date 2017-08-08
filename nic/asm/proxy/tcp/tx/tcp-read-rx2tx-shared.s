/*
 *	Implements the rx2tx shared state read stage of the TxDMA P4+ pipeline
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
	
flow_read_rx2tx_shared_process_start:
	/* Trigger any pending timer bookkeeping from rx */
	sne		c1, d.pending_ft_clear, r0
	bcf		[c1], ft_clear
	nop
	sne		c2, d.pending_ft_reset, r0
	bcf		[c2], ft_reset
	nop
ft_clear:
	b		write_phv
	nop
ft_reset:	
	CAPRI_TIMER_START(LIF_TCP, 0, k.common_phv_fid, TCP_SCHED_RING_FT, d.rto_deadline)
write_phv:	
	/* Write the entire d-vector to p-vector for tx2rx shared state */
	//TODO: phvwr.e		p.{snd_una...rcv_wnd}, d.{snd_una...rcv_wnd}
	nop
