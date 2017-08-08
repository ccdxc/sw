/*
 *	Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	TCB_TX_SHARED_STATE
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	seqe_fid			: 16 ;
	seqe_desc			: ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%

flow_queue_sched_process_start:
	/* if (TAIL_DESC(cb,name) == NULL) { */
	seq		c1, d.retx_tail_desc, r0
	/*    TAIL_DESC(cb, name) = desc; */
	bcf		[c1], no_dma_cmd
	nop
dma_cmd_desc_entry_last:
	/* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
	
	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		r5, r5, d.retx_tail_desc
	phvwr		p.dma_cmd0_addr, r5

	phvwr		p.aol_addr, k.seqe_desc
	phvwri		p.aol_offset, 0
	phvwri		p.aol_len, 0

	phvwri		p.dma_cmd0_pad, TCP_PHV_AOL_START
	phvwri		p.dma_cmd0_size, TCP_PHV_AOL_SIZE
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM


no_dma_cmd:	
	tblwr.c1	d.retx_head_desc, k.seqe_desc
	tblwr.c1	d.retx_tail_desc, k.seqe_desc
tcp_retxq_produce:
	tbladd		d.retx_pi, 1
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TCP)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, k.seqe_fid, TCP_SCHED_RING_DATA, d.retx_pi)
	
	phvwr		p.dma_cmd1_addr, r4
	phvwr		p.db_data, r3
	phvwri		p.dma_cmd1_pad, TCP_PHV_DB_DATA_START
	phvwri		p.dma_cmd1_size, TCP_PHV_DB_DATA_SIZE
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM
