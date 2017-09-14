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
//	aol_scratch			: 64    ;
	aol_free_pending		: 1	;
	aol_valid			: 1	;
	aol_addr			: ADDRESS_WIDTH	;
	aol_offset			: OFFSET_WIDTH	;
	aol_len				: LEN_WIDTH	;

//	rcv_aol_scratch			: 64    ;
	rcv_aol_free_pending		: 1	;
	rcv_aol_valid			: 1	;
	rcv_aol_addr			: ADDRESS_WIDTH	;
	rcv_aol_offset			: OFFSET_WIDTH	;
	rcv_aol_len			: LEN_WIDTH	;

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

	rcv_aol_scratch                 : 64 ;
	rcv_aol_free_pending		: 1;
	rcv_aol_valid			: 1;
	rcv_aol_addr			: ADDRESS_WIDTH;
	rcv_aol_offset			: OFFSET_WIDTH;
	rcv_aol_len			: LEN_WIDTH;

	serq_pidx			: 8		;
};

struct phv_ p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_sack4_process_start:
	tblwr		d.rcv_aol_free_pending, k.rcv_aol_free_pending
	tblwr		d.rcv_aol_valid, k.rcv_aol_valid
	tblwr		d.rcv_aol_addr, k.rcv_aol_addr
	tblwr           d.rcv_aol_offset, k.rcv_aol_offset
	tblwr		d.rcv_aol_len, k.rcv_aol_len

	ffcv		r1, k.rcv_aol_scratch, r0
	addi		r1, r1, 1
	seq		c1, r1, r0
	//bcf		[!c1], flow_sack4_process_done TODO
	nop

	addi		r5, r0, TCP_PHV_RXDMA_COMMANDS_START
	add		r4, r5, r0
	phvwr		p.p4_rxdma_intr_dma_cmd_ptr, r4


#if 0
// TODO
dma_cmd_descr:	
	/* Set the DMA_WRITE CMD for descr */
	add		r1, k.descr, r0
	addi		r1, r1 ,NIC_DESC_ENTRY_0_OFFSET
	phvwr		p.dma_cmd0_dma_cmd_addr, r1

	phvwr		p.aol_scratch, r0
	phvwr		p.aol_free_pending,r0
	phvwri		p.aol_valid, 1
	phvwr		p.aol_addr, d.rcv_aol_addr
	phvwr		p.aol_offset,d.rcv_aol_offset
	phvwr		p.aol_len, d.rcv_aol_len

	phvwri		p.dma_cmd0_pad, TCP_PHV_AOL_START

	phvwri		p.dma_cmd0_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN
dma_cmd_serq_slot:
	add		r5, r0, k.serq_pidx
	mincr		r5, 1, SERQ_TABLE_SIZE_SHFT
	sll		r5, r5, NIC_SERQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SERQ slot */
	addi		r1, r0, SERQ_BASE
	add		r1, r1, r5

	phvwr		p.dma_cmd1_addr, r1

	phvwr		p.aol_desc, k.descr
	phvwri		p.dma_cmd1_pad, TCP_PHV_AOL_DESC_START
	phvwri		p.dma_cmd1_size, NIC_SERQ_ENTRY_SIZE
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN
	
flow_sack4_process_done:
table_read_CC:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_cc_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_CC_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
#endif
