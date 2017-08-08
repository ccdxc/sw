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
	
flow_read_rx2tx_shared_extra_process_start:
	/* Write the entire d-vector to p-vector for tx2rx shared state */
	//TODO: phvwr		p.prior_ssthresh, d.prior_ssthresh
	//TODO: phvwr		p.high_seq, d.high_seq
	//TODO: phvwr		p.{sacked_out...fackets_out}, d.{sacked_out...fackets_out}
	//TODO: phvwr		p.{ooo_datalen...undo_marker}, d.{ooo_datalen...undo_marker}
	//TODO: phvwr		p.undo_retrans, d.undo_retrans
	//TODO: phvwr.e         p.{snd_ssthresh...ecn_flags}, d.{snd_ssthresh...ecn_flags}
	nop
