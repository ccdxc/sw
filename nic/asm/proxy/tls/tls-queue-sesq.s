/*
 * 	Implements the writing of barco odesc to SESQ
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp-sched.h"
	
/* d is the data returned by lookup result */
struct d_struct {
	TLS_SHARED_STATE
};

/* SERQ consumer index */
struct k_struct {
	sesq_pidx			 : RING_INDEX_WIDTH ;
        desc_scratch                     :  28;
        desc_num_entries                 :  2;
        desc_data_len                    :  18;
        desc_head_idx                    :  2;
        desc_tail_idx                    :  2;
        desc_offset                      :  8;

        desc_aol0_scratch                :  64;
        desc_aol0_free_pending           :  1;
        desc_aol0_valid                  :  1;
        desc_aol0_addr                   :  30;
        desc_aol0_offset                 :  16;
        desc_aol0_len                    :  16;

        desc_aol1_scratch                :  64;
        desc_aol1_free_pending           :  1;
        desc_aol1_valid                  :  1;
        desc_aol1_addr                   :  30;
        desc_aol1_offset                 :  16;
        desc_aol1_len                    :  16;

	fid                              :  16;
	desc	                         :  ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
tls_queue_sesq_process_start:
	tblwr		d.qhead, k.desc_aol1_addr

dma_cmd_sesq_slot:
	add		r5, r0, k.sesq_pidx
	mincr		r5, 1, SESQ_TABLE_SIZE_SHFT
	sll		r5, r5, NIC_SESQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for SESQ slot */
	addi		r1, r0, SESQ_BASE
	add		r1, r1, r5

	phvwr		p.dma_cmd0_addr,r1
	phvwr		p.seqe_fid, k.fid
	phvwr		p.seqe_desc, k.desc
	phvwri		p.dma_cmd0_pad, TLS_PHV_SEQE_START
	phvwri		p.dma_cmd0_size, NIC_SERQ_ENTRY_SIZE
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TLS_PHV_DMA_COMMAND_TOTAL_LEN
tls_sesq_produce:

	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TCP)
	/* data will be in r3 */
	add		r1, k.fid, r0
	CAPRI_RING_DOORBELL_DATA(0, r1, TCP_SCHED_RING_SESQ, k.sesq_pidx)
	
	phvwr		p.dma_cmd1_addr, r4
	phvwr		p.db_data, r3
	phvwri		p.dma_cmd1_pad, TLS_PHV_DB_DATA_START
	phvwri		p.dma_cmd1_size, TLS_PHV_DB_DATA_SIZE
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

tls_queue_sesq_process_done:
	nop.e
	nop
