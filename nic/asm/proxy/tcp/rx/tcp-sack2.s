/*
 *	Implements the SACK stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */
	/* State needed only by SACK stage */

	ooo_head_cursor			: ADDRESS_WIDTH    ;
	ooo_tail_cursor			: ADDRESS_WIDTH	;
	delivered			: COUNTER32	;
	lost				: COUNTER32	;


	l_cell				: 8	;
	r_cell				: 8	;
	l_aol_start_seq			: SEQ_NUMBER_WIDTH	;
	aol_idx				: 8	;
	l_chop				: 8	;
	r_chop				: 8	;
	aol_page                        : ADDRESS_WIDTH    ;
	aol_scratch			: 64    ;
	aol_free_pending		: 1	;
	aol_valid			: 1	;
	aol_addr			: ADDRESS_WIDTH	;
	aol_offset			: OFFSET_WIDTH	;
	aol_len				: LEN_WIDTH	;

};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	syn				: 1 ;
	ece				: 1 ;
	cwr				: 1 ;
	ooo_rcv				: 1 ;
	rsvd				: 4 ;
	ca_event			: 4 ;
	num_sacks			: 8 ;
	sack_off			: 8 ;
	d_off				: 8 ;
	ts_off				: 8 ;
	ip_dsfield			: 8 ;
	pkts_acked			: 8  ;
	seq				: SEQ_NUMBER_WIDTH ;
	end_seq				: SEQ_NUMBER_WIDTH ;
	ack_seq				: SEQ_NUMBER_WIDTH ;
	window				: WINDOW_WIDTH ;
	process_ack_flag		: 16  ;
	descr_idx			: RING_INDEX_WIDTH ;
	page_idx			: RING_INDEX_WIDTH ;
	descr				: ADDRESS_WIDTH ;
	page				: ADDRESS_WIDTH ;
	aol_zero_scratch                : 1  ;
	aol_page_alloc                  : 1  ;
	aol_scratch                     : 64 ;
	aol_free_pending		: 1;
	aol_valid			: 1;
	aol_addr			: ADDRESS_WIDTH;
	aol_offset			: OFFSET_WIDTH;
	aol_len				: LEN_WIDTH;

	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
};

struct phv_ p	;
struct k_struct k	;
struct d_struct d	;

%%
	
flow_sack2_process_start:
	tblwr		d.aol_scratch, k.aol_scratch
	tblwr		d.aol_free_pending, k.aol_free_pending
	tblwr		d.aol_valid, k.aol_valid
	tblwr		d.aol_addr, k.aol_addr
	tblwr           d.aol_offset, k.aol_offset
	tblwr		d.aol_len, k.aol_len

	sne		c1, k.aol_zero_scratch, r0
	tblwr.c1	d.aol_scratch, 0
	sne		c2, k.aol_page_alloc, r0
	tblwr.c2	d.aol_page, k.page
	/*
	 * aol_idx = (head_idx + 
	 *      (cp->seq - (tp->rx.rcv_nxt - head_offset)) / NIC_PAGE_SIZE) & 
	 *      (MAX_ENTRIES_PER_DESC-1);
	 */
	sub		r1, k.seq, k.rcv_nxt
	srl		r1, r1, NIC_PAGE_SIZE_SHFT
	andi		r2, r1, MAX_ENTRIES_PER_DESC_MASK
	tblwr		d.aol_idx, r2 
	
	/* 
	 * Each page is divided into 32B cells of data. 
	 * Find the leftmost cell of segment received
	 *
 	 * l_aol_start_seq = ((cp->seq - (tp->rx.rcv_nxt - head_offset)) / NIC_PAGE_SIZE ) * NIC_PAGE_SIZE;
	 * l_cell =  (cp->seq -  l_aol_start_seq) / PAGE_CELL_SIZE;
	 *
	 * Check if leftmost cell is being partially filled, if so drop that part of segment *
	 * if (cp->seq != (l_aol_start_seq + PAGE_CELL_SIZE * l_cell)) {
	 *    l_cell++;
         *    l_chop = (cp->seq - (l_aol_start_seq + PAGE_CELL_SIZE * l_cell));
	 */

	/* l_aol_start_seq = ((cp->seq - (tp->rx.rcv_nxt - head_offset)) / NIC_PAGE_SIZE ) * NIC_PAGE_SIZE; */
	sll		r3, r1, NIC_PAGE_SIZE_SHFT
	tblwr		d.l_aol_start_seq, r3

	/* l_cell =  (cp->seq -  l_aol_start_seq) / PAGE_CELL_SIZE; */
	sub		r4, k.seq, d.l_aol_start_seq
	srl		r4, r4, PAGE_CELL_SIZE_SHFT
	tblwr		d.l_cell, r4

	/* if (cp->seq != (l_aol_start_seq + PAGE_CELL_SIZE * l_cell)) { */
	add		r5, d.l_cell, r0
	sll		r5, r5, PAGE_CELL_SIZE_SHFT
	/* r5 = l_cell(r4) * PAGE_CELL_SIZE */
	add		r5, r5, d.l_aol_start_seq
	/* r5 = l_aol_start_seq(r3) + l_cell * PAGE_CELL_SIZE */
	sne		c1, k.seq, r5
	addi.c1		r6, r0, 1
	/* l_cell++ */
	tbladd.c1	d.l_cell, r6
	/* l_chop = cp->seq - (l_aol_start_seq + PAGE_CELL_SIZE * l_cell) */
	sub		r6, k.seq, r5
	tblwr		d.l_chop, r6

	/* Find the rightmost cell of segment received 
	 * r_cell =  (cp->seq + cp->tcp_data_len - l_aol_start_seq) / PAGE_CELL_SIZE;
	 */
	/* r5 = cp->seq + cp->tcp_data_len - l_aol_start_seq */
	sub		r5, k.end_seq, d.l_aol_start_seq
	/* r_cell =  (cp->seq + cp->tcp_data_len - l_aol_start_seq) / PAGE_CELL_SIZE; */
	srl		r5, r5, PAGE_CELL_SIZE_SHFT
	tblwr		d.r_cell, r5
	/* r5 = r_cell */
	add		r6, d.r_cell, r0
	addi		r6, r6, 1
	sll		r6, r6, PAGE_CELL_SIZE_SHFT
	add		r6, r6, d.l_aol_start_seq
	/* r6 = (l_aol_start_seq + PAGE_CELL_SIZE * (r_cell + 1)) */
	sne		c1, k.end_seq, r6
	tbladd.c1	d.r_cell, -1

	sll		r6, d.r_cell, PAGE_CELL_SIZE_SHFT
	add		r6, r6, d.l_aol_start_seq
	sub		r5, k.end_seq, r6
	tblwr		d.r_chop, r5
	add		r6, d.aol_scratch, r0
	add		r3, d.r_cell, r0
	add		r4, d.l_cell, r0
	fsetv		r5, r6, r3, r4
	tblwr		d.aol_scratch, r5

dma_cmd_ooo_data:	
	addi		r5, r0, TCP_PHV_RXDMA_COMMANDS_START
	add		r1, r0, r5
	phvwr		p.p4_rxdma_intr_dma_cmd_ptr, r1

	/* Write A = page + offset + l_cell * PAGE_CELL_SIZE */
	add		r3, d.l_cell, r0
	sll		r3, r3, PAGE_CELL_SIZE_SHFT
	add		r2, d.aol_page, r3
	phvwr		p.dma_cmd0_dma_cmd_addr, r1

	/* Write L = (r_cell - l_cell + 1) * PAGE_CELL_SIZE */
	sub		r3, d.r_cell, d.l_cell
	sll		r3, r3, PAGE_CELL_SIZE_SHFT
	phvwr		p.dma_cmd0_dma_cmd_size, r3
	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM

	add		r1, r1, TCP_PHV_DMA_COMMAND_TOTAL_LEN

	bcf		[!c2], flow_sack2_process_done
	nop
dma_cmd_aol_entry:
	phvwr		p.aol_scratch, d.aol_scratch
        // TODO fix this
#if 0
	phvwr		p.aol_free_pending, d.aol_free_pending
	phvwr		p.aol_valid, d.aol_valid
	phvwr		p.aol_addr, d.aol_page
	phvwr		p.aol_offset, d.aol_offset
	phvwr		p.aol_len, d.aol_len
#endif

	add		r4, k.descr, r0
	addi		r4, r4, NIC_DESC_ENTRY_0_OFFSET

	phvwr		p.dma_cmd1_dma_cmd_addr, r4
	// TODO : phvwri		p.dma_cmd1_dma_cmd_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

	
	
flow_sack2_process_done:
table_read_SACK3:
#if 0
// TODO
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_sack3_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_SACK_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
#endif



