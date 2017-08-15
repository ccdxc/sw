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

        add r1, r0, k.p4_rxdma_intr_qstate_addr
        add r1, r0, k.p4_intr_global_lif_sbit0_ebit2
        add r1, r0, k.p4_intr_global_lif_sbit3_ebit10
        add r1, r0, k.p4_rxdma_intr_qid
        add r1, r0, k.tcp_app_header_p4plus_app_id
        add r1, r0, k.tcp_app_header_table0_valid
        add r1, r0, k.tcp_app_header_table1_valid
        add r1, r0, k.tcp_app_header_table2_valid
        add r1, r0, k.tcp_app_header_table3_valid
        add r1, r0, k.tcp_app_header_srcPort
        CAPRI_CLEAR_TABLE0_VALID
        add r1, r0, k.p4_rxdma_intr_qstate_addr

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
        add             r1, r0, k.tcp_app_header_ackNo_sbit24_ebit31
        sll             r1, r1, 24
        or              r1, r1, k.tcp_app_header_ackNo_sbit0_ebit23
	phvwr		p.to_s1_ack_seq, r1
	//phvwr		p.to_s1_ack_seq, k.tcp_app_header_ackNo
	add		r1, k.tcp_app_header_seqNo, k.tcp_app_header_payload_len
	phvwr		p.s1_s2s_end_seq, r1
	
	//phvwr		p.prr_out, d.prr_out
	phvwr		p.to_s1_snd_nxt, d.snd_nxt
	phvwr		p.to_s2_snd_nxt, d.snd_nxt
	//phvwr		p.ecn_flags_tx, d.ecn_flags_tx
	phvwr		p.common_phv_ecn_flags, d.ecn_flags_tx
	phvwr		p.s1_s2s_packets_out, d.packets_out
        phvwr           p.common_phv_qstate_addr, k.p4_rxdma_intr_qstate_addr


	phvwr		p.to_s6_payload_len, k.tcp_app_header_payload_len
#if 1
	phvwri		p.to_s6_payload_len, 24
	phvwri		p.to_s6_page, 0xa6ec6200
	phvwri		p.to_s6_descr, 0xa6ec7000
#endif

	
table_read_RX:	
	CAPRI_NEXT_TABLE0_READ(k.p4_rxdma_intr_qid, TABLE_LOCK_EN,
                            tcp_rx_process_stage1_start, k.p4_rxdma_intr_qstate_addr,
                            TCP_TCB_TABLE_ENTRY_SIZE_SHFT, TCP_TCB_RX_OFFSET,
                            TABLE_SIZE_512_BITS)
        nop.e
        nop
