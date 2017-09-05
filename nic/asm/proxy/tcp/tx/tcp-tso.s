/*
 *	Implements the TSO stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p	;
struct tcp_tx_tso_k k	;
struct tcp_tx_tso_tso_d d	;
	

%%
        .align
	
tcp_tso_process_start:
        CAPRI_CLEAR_TABLE_VALID(0)
	/* check SESQ for pending data to be transmitted */
	or 		r1, k.to_s4_pending_tso_data, k.to_s4_pending_tso_retx
	or		r1, r1, k.to_s4_pending_ack_send
	sne		c1, r1, r0
        bal.c1          r7, tcp_write_xmit
        nop
        b		flow_tso_process_done
        nop

tcp_write_xmit:
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

	seq		c1, k.to_s4_xmit_cursor_addr, r0
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
	//phvwr		p.fid, d.fid
	//phvwr		p.source,d.source
	//phvwr		p.dest, d.dest
	//phvwr		p.seq, d.retx_xmit_seq
	//phvwr		p.ack_seq, k.rcv_nxt
	//phvwri		p.d_off, 5
	//phvwri		p.res1, 0
	//phvwri		p.ack,1
	//phvwr		p.window, k.rcv_wnd
	//phvwri		p.urg_ptr, 0

dma_cmd_intrinsic:	
        phvwri          p.p4_intr_global_tm_iport, 9
        phvwri          p.p4_intr_global_tm_oport, 11
        phvwri          p.p4_intr_global_tm_oq, 0
	addi		r5, r0, TCP_PHV_DMA_COMMANDS_START
	add		r1, r0, r5
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r1

	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
        phvwr           p.dma_cmd0_dma_cmd_phv_start_addr, TCP_PHV_INTRINSIC_START
        phvwr           p.dma_cmd0_dma_cmd_phv_end_addr, TCP_PHV_INTRINSIC_END
dma_cmd_p4plus_to_p4_app_header:
        phvwr           p.tcp_app_header_p4plus_app_id, 1 // TODO: P4PLUS_APP_P4PLUS_APP_TCP_PROXY_ID
	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
        phvwr           p.dma_cmd1_dma_cmd_phv_start_addr, TCP_PHV_TX_APP_HDR_START
        phvwr           p.dma_cmd1_dma_cmd_phv_end_addr, TCP_PHV_TX_APP_HDR_END
dma_cmd_hdr:	
	phvwri		p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
        add             r5, r0, k.common_phv_qstate_addr
        add             r6, r0, k.common_phv_fid, TCP_TCB_TABLE_ENTRY_SIZE_SHFT
        add             r5, r5, r6
        addi            r5, r5, TCP_TCB_HEADER_TEMPLATE_OFFSET
        phvwr           p.dma_cmd2_dma_cmd_addr, r5
	phvwri		p.dma_cmd2_dma_cmd_size, ETH_IP_TCP_HDR_SIZE

dma_cmd_data:
	phvwri		p.dma_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
        phvwri          p.dma_cmd3_dma_pkt_eop, 1
	seq		c2, k.to_s4_xmit_cursor_addr, r0
	/* r6 has tcp data len being sent */
	addi		r6, r0, 0
	/* We can end up taking this branch if we ended up here
	 * to send pure ack and there is really no data in retx queue
	 * to send
	 */
	bcf		[c2], tcp_write_xmit_done
	nop

	/* Write A = xmit_cursor_addr + xmit_cursor_offset */

	add		r2, k.to_s4_xmit_cursor_addr, k.to_s4_xmit_cursor_offset
	phvwr		p.dma_cmd3_dma_cmd_addr, r2

	/* Write L = min(mss, descriptor entry len) */
	slt		c1, k.to_s4_rcv_mss, k.to_s4_xmit_cursor_len
	add.c1		r6, k.to_s4_rcv_mss, r0
	add.!c1		r6, k.to_s4_xmit_cursor_len, r0
	phvwr		p.dma_cmd3_dma_cmd_size, r6

	/*
	 * if (tcp_in_cwnd_reduction(tp))
	 *    tp->cc.prr_out += 1
	 */
	and		r1, k.to_s4_ca_state, (TCPF_CA_CWR | TCPF_CA_Recovery)
	seq		c1, r1, r0
	addi.!c1	r1, r0, 1
	tbladd.!c1	d.prr_out, r1

dma_cmd_write_tx2rx_shared:
	/* Set the DMA_WRITE CMD for copying tx2rx shared data from phv to mem */
	phvwri		p.dma_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
        phvwri          p.dma_cmd4_dma_cmd_eop, 1
	add 		r5, r0, k.common_phv_qstate_addr
	add		r6, r0, k.common_phv_fid, TCP_TCB_TABLE_ENTRY_SIZE_SHFT
	add		r5, r5, r6
	addi		r5, r5, TCP_TCB_TX2RX_SHARED_WRITE_OFFSET
	phvwr		p.dma_cmd4_dma_cmd_addr, r5
	phvwri		p.dma_cmd4_dma_cmd_phv_start_addr, TCP_PHV_TX2RX_SHARED_START
	phvwri		p.dma_cmd4_dma_cmd_phv_end_addr, TCP_PHV_TX2RX_SHARED_END
	
	bcf		[c1], update_xmit_cursor
	nop
move_xmit_cursor:
	tbladd		d.retx_xmit_cursor, NIC_DESC_ENTRY_SIZE
	/* This clearing of xmit_cursor_addr will cause read of retx_xmit_cursor for
	 * next pass after tcp-tx stage
	 */
	phvwri		p.to_s4_xmit_cursor_addr, 0 // TODO : which stage needs this?
	b		tcp_read_xmit_cursor
	nop


update_xmit_cursor:
	/* Move offset of descriptor entry by xmit len */
	add		r4, k.to_s4_xmit_cursor_offset, r6

	addi		r2, r0, NIC_DESC_ENTRY_OFF_OFFSET	
	add		r1, d.retx_xmit_cursor, r2
	memwr.h		r1, r4

	/* Decrement length of descriptor entry by xmit len */
	sub		r4, k.to_s4_xmit_cursor_len, r6

	addi		r2, r0, NIC_DESC_ENTRY_LEN_OFFSET	
	add		r1, d.retx_xmit_cursor, r2
	memwr.h		r1, r4
	
	
tcp_write_xmit_done:
	/* Set the tot_len in ip header */
	addi		r1, r0, TCPIP_HDR_SIZE
	add		r1, r1, r6
	//phvwr		p.tot_len,r1 // TODO
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0


tcp_retx:
tcp_retx_done:

tcp_read_xmit_cursor:
#if 0
	/* Read the xmit cursor if we have zero xmit cursor addr */
	add		r1, d.retx_xmit_cursor, r0	

	phvwr		p.table_sel, TABLE_TYPE_RAW
	phvwr		p.table_mpu_entry_raw, flow_read_xmit_cursor
	phvwr		p.table_addr, r1
#endif
	
flow_tso_process_done:
	nop.e
	nop

