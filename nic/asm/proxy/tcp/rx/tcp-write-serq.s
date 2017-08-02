/*
 *	Implements the RX stage of the RxDMA P4+ pipeline
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
	/* State needed only by RX stage */

	ato				: 4	;

	dup_ack_cnt			: COUNTER8	;
	pred_flags			: 8	;

	ca_flags			: 2	;
	fastopen_rsk			: 1	;
	delivered			: COUNTER8	;
	lost				: COUNTER8	;

	max_window			: WINDOW_WIDTH	;
	rcv_tstamp			: TS_WIDTH	;
	lrcv_time			: TS_WIDTH	;
	curr_ts				: TS_WIDTH	;
	rcv_tsval			: TS_WIDTH	;
	ts_recent			: TS_WIDTH	;
	ts_recent_tstamp		: TS_WIDTH	;
	bytes_rcvd			: COUNTER32	;
	bytes_acked			: COUNTER32	;
	nde_free_pending		: 1	;
	nde_valid			: 1	;
	nde_addr			: ADDRESS_WIDTH	;
	nde_offset			: OFFSET_WIDTH	;
	nde_len				: LEN_WIDTH	;
	
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
	serq_pidx			: 7		;
	write_serq                      : 1		;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

	
%%
	
flow_write_serq_process_start:
	/* r4 is loaded at the beginning of the stage with current timestamp value */
	tblwr		d.curr_ts, r4
	/* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
	sne		c1, k.write_serq, r0
	bcf		[!c1], flow_write_serq_process_done
	nop

dma_cmd_data:
	addi		r5, r0, TCP_PHV_DMA_COMMANDS_START
	add		r4, r5, r0
	phvwr		p.dma_cmd_ptr, r4

	/* Set the DMA_WRITE CMD for data */
	addi		r5, r0, NIC_PAGE_HDR_SIZE
	add		r1, k.page, r5
	tblwr.l		d.nde_addr,r1
	tblwr.l		d.nde_offset, NIC_PAGE_HEADROOM
	tblwr.l		d.nde_len, r2

	
	add		r1, d.nde_addr, d.nde_offset
	phvwr		p.dma_cmd0_addr, r1
	phvwri		p.dma_cmd0_pad, 0
	phvwr		p.dma_cmd0_size, r2
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PKT_TO_MEM
	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN
dma_cmd_descr:	
	/* Set the DMA_WRITE CMD for descr */
	add		r5, k.descr, r0
	addi		r1, r5, NIC_DESC_ENTRY_0_OFFSET
	phvwr		p.dma_cmd1_addr, r1

	phvwr		p.aol_scratch, r0
	phvwr		p.aol_free_pending,r0
	phvwri		p.aol_valid, 1
	phvwr		p.aol_addr, d.nde_addr
	phvwr		p.aol_offset,d.nde_offset
	phvwr		p.aol_len, d.nde_len

	phvwri		p.dma_cmd1_pad, TCP_PHV_AOL_START
	phvwri		p.dma_cmd1_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN

dma_cmd_serq_slot:
	add		r5, r0, k.serq_pidx
	mincr		r5, 1, SERQ_TABLE_SIZE_SHFT
	sll		r5, r5, NIC_SERQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SERQ slot */
	addi		r1, r0, SERQ_BASE
	add		r1, r1, r5

	phvwr		p.dma_cmd2_addr,r1
	phvwr		p.aol_desc, k.descr
	phvwri		p.dma_cmd2_pad, TCP_PHV_AOL_DESC_START
	phvwri		p.dma_cmd2_size, NIC_SERQ_ENTRY_SIZE
	phvwri		p.dma_cmd2_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN
dma_cmd_write_rx2tx_shared:
	/* Set the DMA_WRITE CMD for copying rx2tx shared data from phv to mem */
	addi		r5, r0, TCP_TCB_TABLE_BASE
	add		r6, r0, k.fid
	sll		r6, r6, TCP_TCB_TABLE_ENTRY_SIZE_SHFT
	add		r5, r5, r6
	add		r6, r0, TCP_TCB_RX2TX_SHARED_OFFSET
	add		r5, r5, r6
	phvwr		p.dma_cmd3_addr, r5
	phvwri		p.dma_cmd3_pad, TCP_PHV_RX2TX_SHARED_START
	phvwri		p.dma_cmd3_size, TCP_PHV_RX2TX_SHARED_SIZE
	phvwri		p.dma_cmd3_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM
	addi		r4, r4, TCP_PHV_DMA_COMMAND_TOTAL_LEN

tcp_serq_produce:

	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_GLOBALQ)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, SERQ_QID, 0, k.serq_pidx)
	
	phvwr		p.dma_cmd4_addr, r4
	phvwr		p.db_data, r3
	phvwri		p.dma_cmd4_pad, TCP_PHV_DB_DATA_START
	phvwri		p.dma_cmd4_size, TCP_PHV_DB_DATA_SIZE
	phvwri		p.dma_cmd4_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM
	
flow_write_serq_process_done:
table_read_RTT:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_rtt_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_RTT_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop

