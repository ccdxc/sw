/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct s1_t2_tcp_tx_k k;
struct s1_t2_tcp_tx_read_xmit_d d;
	
%%
    .align

tcp_tx_process_read_xmit_start:
    CAPRI_CLEAR_TABLE_VALID(2)
    phvwr           p.to_s4_packets_out, d.packets_out
    phvwr           p.to_s4_sacked_out, d.sacked_out
    phvwr           p.to_s4_lost_out, d.lost_out
    phvwr           p.to_s4_retrans_out, d.retrans_out
    phvwr           p.to_s4_is_cwnd_limited, d.is_cwnd_limited
    nop.e
    nop

