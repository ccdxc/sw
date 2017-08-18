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

        .param          tcp_rx_process_stage1_start
	.align
tcp_rx_read_shared_stage0_start:
        CAPRI_CLEAR_TABLE0_VALID
        add r1, r0, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}

	/* Write all the tx to rx shared state from table data into phv */

	/* Set the flow id in the phv global area for the rest of
	 * the stages to use it
	 */
	phvwr		p.common_phv_fid, k.p4_rxdma_intr_qid
	and		r1, k.tcp_app_header_flags, TCPHDR_SYN
	phvwr		p.common_phv_syn, r1
	and		r1, k.tcp_app_header_flags, TCPHDR_ECE
	phvwr		p.common_phv_ece, r1

	//TODO: move to s2s phvwr		p.common_phv_rcv_tsecr, k.tcp_app_header_prev_echo_ts

	/* Setup the to-stage/stage-to-stage variables based
	 * on the p42p4+ app header info
	 */

	phvwr		p.to_s1_seq, k.tcp_app_header_seqNo
        //add             r1, r0, k.tcp_app_header_ackNo_sbit24_ebit31
        //sll             r1, r1, 24
        //or              r1, r1, k.tcp_app_header_ackNo_sbit0_ebit23
	//phvwr		p.to_s1_ack_seq, r1
	phvwr		p.to_s1_ack_seq, k.tcp_app_header_ackNo
	add		r1, k.tcp_app_header_seqNo, k.tcp_app_header_payload_len
	phvwr		p.s1_s2s_end_seq, r1
	
	//phvwr		p.prr_out, d.prr_out
	phvwr		p.to_s1_snd_nxt, d.snd_nxt
	phvwr		p.to_s2_snd_nxt, d.snd_nxt
	//phvwr		p.ecn_flags_tx, d.ecn_flags_tx
	phvwr		p.common_phv_ecn_flags, d.ecn_flags_tx
	phvwr		p.s1_s2s_packets_out, d.packets_out
        phvwr           p.common_phv_qstate_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}


	phvwr		p.to_s6_payload_len, k.tcp_app_header_payload_len
	
table_read_RX:	
	CAPRI_NEXT_TABLE0_READ(k.p4_rxdma_intr_qid, TABLE_LOCK_EN,
                            tcp_rx_process_stage1_start, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33},
                            TCP_TCB_TABLE_ENTRY_SIZE_SHFT, TCP_TCB_RX_OFFSET,
                            TABLE_SIZE_512_BITS)
        nop.e
        nop
