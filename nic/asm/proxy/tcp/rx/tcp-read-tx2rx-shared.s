/*
 *	Implements the tx2rx shared state read stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_rx_read_tx2rx_k k;
struct tcp_rx_read_tx2rx_read_tx2rx_d d;
	
%%

// TODO check why it is splitting
#define tcp_app_header_gft_flow_id tcp_app_header_gft_flow_id_sbit4_ebit19

        .param          tcp_rx_process_stage1_start
	
tcp_rx_read_shared_stage0_start:
	/* Write all the tx to rx shared state from table data into phv */
	//phvwr		p.prr_out, d.prr_out
	phvwr		p.to_s1_snd_nxt, d.snd_nxt
	phvwr		p.to_s2_snd_nxt, d.snd_nxt
	//phvwr		p.ecn_flags_tx, d.ecn_flags_tx
	phvwr		p.common_phv_ecn_flags, d.ecn_flags_tx
	phvwr		p.s1_s2s_packets_out, d.packets_out
table_read_RX:	
	CAPRI_NEXT_TABLE0_READ(k.tcp_app_header_gft_flow_id, TABLE_LOCK_EN,
                            tcp_rx_process_stage1_start, TCP_TCB_TABLE_BASE,
                            TCP_TCB_TABLE_ENTRY_SIZE_SHFT, TCP_TCB_RX_OFFSET,
                            TABLE_SIZE_512_BITS)
