/*
 *	Implements the TSO stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	retx_snd_una			: SEQ_NUMBER_WIDTH ;
	retx_snd_nxt			: SEQ_NUMBER_WIDTH ;
	retx_head_desc			: ADDRESS_WIDTH ;
	retx_snd_una_cursor		: ADDRESS_WIDTH ;
	retx_tail_desc			: ADDRESS_WIDTH ;
	retx_snd_nxt_cursor		: ADDRESS_WIDTH ;
	retx_xmit_cursor		: ADDRESS_WIDTH ;
	retx_xmit_seq			: SEQ_NUMBER_WIDTH ;

	rcv_wup				: WINDOW_WIDTH ;

	xmit_cursor_flags		: 8  ;
	xmit_cursor_addr		: ADDRESS_WIDTH ;
	xmit_cursor_offset		: OFFSET_WIDTH ;
	xmit_cursor_len			: LEN_WIDTH ;
	
	/* ethhdr fields from hdr template */
//	h_dest				: 48 ;
//	h_source			: 48 ;

	/* iphdr fields from hdr template page */
//	saddr				: 32 ;
//	daddr				: 32 ;
	/* tcphdr fields from hdr template page */
	fid                             : 32 ;
	source				: 16 ;
	dest				: 16 ;
	pending_ack_tx			: 1  ;
	pending_delayed_ack_tx		: 1  ;

	/* State needed by RX and TX pipelines
	 * This has to be at the end.
	 * Each of these fields will be written by Rx only or Tx only
	 */

        prr_out				: COUNTER32	        ;\

};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	xmit_cursor_flags		: 8  ;
	xmit_cursor_addr		: ADDRESS_WIDTH ;
	xmit_cursor_offset		: OFFSET_WIDTH ;
	xmit_cursor_len			: LEN_WIDTH ;


	pending_challenge_ack_send	: 1	                ;\
	pending_ack_send		: 1	                ;\
	pending_sync_mss		: 1	                ;\
	pending_tso_keepalive           : 1                     ;\
	pending_tso_pmtu_probe          : 1                     ;\
	pending_tso_data		: 1                     ;\
	pending_tso_probe_data		: 1                     ;\
	pending_tso_probe		: 1                     ;\
	pending_ooo_se_recv		: 1                     ;\
	pending_tso_retx	        : 1                     ;\
	pending_rexmit		        : 2                     ;\

	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
	rcv_wnd				: WINDOW_WIDTH          ;\
        rcv_mss                         : 8                     ;\
	ca_state			: 8	                ;\

};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	

%%
	
flow_tso_process_start:
	/* check SESQ for pending data to be transmitted */
	or 		r1, k.pending_tso_data, k.pending_tso_retx
	or		r1, r1, k.pending_ack_send
	sne		c1, r1, r0
	jal.c1		r7, tcp_write_xmit
	nop
	b		flow_tso_process_done
	nop

tcp_write_xmit_start:
	/* Get the point where we are supposed to send next from */
	seq		c1, d.retx_xmit_cursor, r0
	/* If the retx was all cleaned up , then reinit the xmit
	 * cursor to snd_una cursor which is the head of data that
         * can be sent
	 */
	
	tblwr.c1	d.retx_xmit_cursor, d.retx_snd_una_cursor
	nop
	/* Even after all this retx_xmit_cursor has no data, then
	 * there is no data to send
	 */
	seq		c1, d.retx_xmit_cursor, r0
	bcf		[c1], tcp_write_xmit_done
	nop

	seq		c1, k.xmit_cursor_addr, r0
	bcf		[c1], tcp_write_xmit_done
	nop

	/* Write the eth header */
//eth_start:
//	phvwr		p.h_dest, d.h_dest
//	phvwr		p.h_source, d.h_source
//	phvwri		p.h_proto, ETH_P_IP
	/* Write the ip header */
//ip_start:	
//	phvwri		p.ihl, 5
//	phvwri		p.version, 4
//	phvwri		p.ip_dsfield, 0
//	phvwri		p.id, 0
//	phvwri		p.frag_off,0
//	phvwri		p.ttl, 255
//	phvwri		p.protocol, IPPROTO_TCP,
//	phvwr		p.saddr, d.saddr
//	phvwr		p.daddr, d.daddr
	/* Write the tcp header */
tcp_start:
	phvwr		p.fid, d.fid
	phvwr		p.source,d.source
	phvwr		p.dest, d.dest
	phvwr		p.seq, d.retx_xmit_seq
	phvwr		p.ack_seq, k.rcv_nxt
	phvwri		p.d_off, 5
	phvwri		p.res1, 0
	phvwri		p.ack,1
	phvwr		p.window, k.rcv_wnd
	phvwri		p.urg_ptr, 0

dma_cmd_hdr:	
	addi		r5, r0, TCP_PHV_DMA_COMMANDS_START
	add		r1, r0, r5
	phvwr		p.dma_cmd_ptr, r1

	phvwri		p.dma_cmd0_addr, TCP_PHV_ETH_START
	phvwri		p.dma_cmd0_pad, 0
	phvwri		p.dma_cmd0_size, TCP_HDR_SIZE
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_PKT

        addi            r1, r1, TCP_PHV_DMA_COMMAND_TOTAL_LEN

dma_cmd_data:
	seq		c2, k.xmit_cursor_addr, r0
	/* r6 has tcp data len being sent */
	addi		r6, r0, 0
	/* We can end up taking this branch if we ended up here
	 * to send pure ack and there is really no data in retx queue
	 * to send
	 */
	bcf		[c2], tcp_write_xmit_done
	nop

	/* Write A = xmit_cursor_addr + xmit_cursor_offset */

	add		r2, k.xmit_cursor_addr, k.xmit_cursor_offset
	phvwr		p.dma_cmd1_addr, r2
	phvwri		p.dma_cmd1_pad, 0

	/* Write L = min(mss, descriptor entry len) */
	slt		c1, k.rcv_mss, k.xmit_cursor_len
	add.c1		r6, k.rcv_mss, r0
	add.!c1		r6, k.xmit_cursor_len, r0
	phvwr		p.dma_cmd1_size, r6
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_PKT

        addi            r1, r1, TCP_PHV_DMA_COMMAND_TOTAL_LEN
	
	/*
	 * if (tcp_in_cwnd_reduction(tp))
	 *    tp->cc.prr_out += 1
	 */
	and		r1, k.ca_state, (TCPF_CA_CWR | TCPF_CA_Recovery)
	seq		c1, r1, r0
	addi.!c1	r1, r0, 1
	tbladd.!c1	d.prr_out, r1

dma_cmd_write_tx2rx_shared:
	/* Set the DMA_WRITE CMD for copying tx2rx shared data from phv to mem */
	addi		r5, r0, TCP_TCB_TABLE_BASE
	add		r6, r0, k.fid
	sll		r6, r6, TCP_TCB_TABLE_ENTRY_SIZE_SHFT
	add		r5, r5, r6
	add		r6, r0, TCP_TCB_TX2RX_SHARED_OFFSET
	add		r5, r5, r6
	phvwr		p.dma_cmd2_addr, r5
	phvwri		p.dma_cmd2_pad, TCP_PHV_TX2RX_SHARED_START
	phvwri		p.dma_cmd2_size, TCP_PHV_TX2RX_SHARED_SIZE
	phvwri		p.dma_cmd2_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM
	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN
	
	bcf		[c1], update_xmit_cursor
	nop
move_xmit_cursor:
	tbladd		d.retx_xmit_cursor, NIC_DESC_ENTRY_SIZE
	/* This clearing of xmit_cursor_addr will cause read of retx_xmit_cursor for
	 * next pass after tcp-tx stage
	 */
	phvwri		p.xmit_cursor_addr, 0
	b		tcp_read_xmit_cursor
	nop


update_xmit_cursor:
	/* Move offset of descriptor entry by xmit len */
	add		r4, k.xmit_cursor_offset, r6

	addi		r2, r0, NIC_DESC_ENTRY_OFF_OFFSET	
	add		r1, d.retx_xmit_cursor, r2
	memwr.h		r1, r4

	/* Decrement length of descriptor entry by xmit len */
	sub		r4, k.xmit_cursor_len, r6

	addi		r2, r0, NIC_DESC_ENTRY_LEN_OFFSET	
	add		r1, d.retx_xmit_cursor, r2
	memwr.h		r1, r4
	
	
tcp_write_xmit_done:
	/* Set the tot_len in ip header */
	addi		r1, r0, TCPIP_HDR_SIZE
	add		r1, r1, r6
	phvwr		p.tot_len,r1
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0


tcp_retx:
tcp_retx_done:

tcp_read_xmit_cursor:
	/* Read the xmit cursor if we have zero xmit cursor addr */
	add		r1, d.retx_xmit_cursor, r0	

	phvwr		p.table_sel, TABLE_TYPE_RAW
	phvwr		p.table_mpu_entry_raw, flow_read_xmit_cursor
	phvwr		p.table_addr, r1


	
	
	
flow_tso_process_done:
	nop.e
	nop.e

	
