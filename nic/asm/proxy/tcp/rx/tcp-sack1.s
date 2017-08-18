/*
 *	Implements the SACK stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
	
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

	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

%%
	
tcp_rx_sack_stage2_start:
	/* Check if ooo rcv processing is needed by rx stage before */

	sne		c1, k.ooo_rcv, r0
	bcf		[!c1], tcp_ooo_rcv_done
	nop

tcp_ooo_rcv:
	sne		c1, d.ooo_tail_cursor, r0
	add.!c1		r1, k.descr, r0
	addi.!c1	r1, r1, NIC_DESC_ENTRY_0_OFFSET
	tblwr.!c1	d.ooo_tail_cursor, r1
	tblwr.!c1	d.ooo_head_cursor, d.ooo_tail_cursor
	sne		c1, d.ooo_tail_cursor, r0
	/* We don't have a tail cursor, drop this ooo packet  */
	bcf		[!c1], table_read_CC
	phvwri		p.drop,1

	/*
	 * aol_idx = (head_idx + 
	 *      (cp->seq - (tp->rx.rcv_nxt - head_offset)) / NIC_PAGE_SIZE) & 
	 *      (MAX_ENTRIES_PER_DESC-1);
	 */
	sub		r1, k.seq, k.rcv_nxt
	srl		r1, r1, NIC_PAGE_SIZE_SHFT
	andi		r1, r1, MAX_ENTRIES_PER_DESC_MASK
	tblwr		d.aol_idx, r1


table_read_setup_next2:
	add		r1, k.descr, r0
	addi		r1, r1, NIC_DESC_ENTRY_0_OFFSET
	add 		r2, d.aol_idx, r0
	sll		r2, r2, NIC_DESC_ENTRY_SIZE_SHIFT
	add		r1, r1, r2

	phvwr		p.table_sel, TABLE_TYPE_RAW
	phvwr		p.table_mpu_entry_raw, flow_read_aol_entry_process
	phvwr.e		p.table_addr, r1

tcp_ooo_rcv_done:
	
table_read_CC:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_cc_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_CC_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	


