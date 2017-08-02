/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed by TX but updated by RX pipeline */
	TCB_RX2TX_SHARED_EXTRA_STATE
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

	
%%
	
flow_read_rx2tx_shared_extra_process_start:
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
	CAPRI_TIMER_START(LIF_TCP, 0, k.fid, TCP_SCHED_RING_FT, d.rto_deadline)
write_phv:	
	/* Write the entire d-vector to p-vector for tx2rx shared state */

	phvwr		p.pending_challenge_ack_send, d.pending_challenge_ack_send
	phvwr		p.pending_ack_send, d.pending_ack_send
	phvwr		p.pending_sync_mss, d.pending_sync_mss
	phvwr		p.pending_tso_keepalive, d.pending_tso_keepalive
	phvwr		p.pending_tso_pmtu_probe, d.pending_tso_pmtu_probe
	phvwr		p.pending_tso_data, d.pending_tso_data
	phvwr		p.pending_tso_probe_data, d.pending_tso_probe_data
	phvwr		p.pending_tso_probe, d.pending_tso_probe
	phvwr		p.pending_ooo_se_recv, d.pending_ooo_se_recv
	phvwr		p.pending_tso_retx, d.pending_tso_retx
	phvwr		p.pending_ft_clear, d.pending_ft_clear
	phvwr		p.pending_ft_reset, d.pending_ft_reset
	phvwr		p.pending_rexmit, d.pending_rexmit
	phvwr		p.pending, d.pending

	phvwr		p.ack_blocked, d.ack_blocked
	phvwr		p.ack_pending, d.ack_pending
	phvwr		p.snd_wscale, d.snd_wscale
	phvwr		p.rcv_mss_shft, d.rcv_mss_shft
	phvwr		p.rcv_mss, d.rcv_mss
	phvwr		p.rto, d.rto

	phvwr		p.ca_state, d.ca_state
	phvwr		p.ato_deadline, d.ato_deadline
	phvwr		p.rto_deadline, d.rto_deadline

	phvwr		p.retx_head_ts, d.retx_head_ts
	phvwr		p.srtt_us, d.srtt_us

	phvwr		p.rcv_wnd, d.rcv_wnd

        phvwr		p.prior_ssthresh, d.prior_ssthresh
	phvwr		p.high_seq, d.high_seq
	phvwr		p.{sacked_out...fackets_out}, d.{sacked_out...fackets_out}
	phvwr		p.{ooo_datalen...undo_marker}, d.{ooo_datalen...undo_marker}
	phvwr		p.undo_retrans, d.undo_retrans
	phvwr.e         p.{snd_ssthresh...ecn_flags}, d.{snd_ssthresh...ecn_flags}
	nop
