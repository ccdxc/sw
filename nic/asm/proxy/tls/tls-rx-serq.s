/*
 * 	Implements the receipt of tls request from SESQ 
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
/* d is the data returned by lookup result */
struct d_struct {
	TLS_SHARED_STATE
};

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
	next_tls_hdr_offset              :  16		  ;
	enc_flow                         :  1		  ;
};

struct phv_ p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	.param		tls_alloc_tnmdr_start
	.param		tls_alloc_tnmpr_start
tls_rx_serq_process_start:
	/*   if (IS_ENC_FLOW) { */
	sne		c1, r0, k.enc_flow
	bcf		[c1], tls_rx_serq_enc_process
	nop
tls_rx_serq_dec_process:
	/* ENQ_DESC(*dtlsp, dec, md->seqe.desc); */
	/* if (TAIL_DESC(cb,name) == NULL) { */
	seq		c1, d.qtail, r0
	/*    TAIL_DESC(cb, name) = desc; */
	bcf		[c1], no_dma_cmd
	nop
dma_cmd_dec_desc_entry_last:
	/* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
	
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		    r5, r5, d.qtail
	phvwr		p.dma_cmd0_dma_cmd_addr, r5

	phvwr		p.aol_next_addr, k.desc

    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, TLS_PHV_AOL_DESC_END
	phvwri		p.dma_cmd1_dma_cmd_phv_end_addr, TLS_PHV_AOL_DESC_START

	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd1_dma_cmd_eop, 0

no_dma_cmd:	
	tblwr.c1	d.qtail, k.desc
	tblwr.c1	d.qhead, k.desc
	
	/* dtlsp->dec_nxt.desc = TAIL_DESC(*dtlsp, dec); */
	tblwr		d.nxt_desc, k.desc

	/* Check if the next tls header is starting in this descriptor
	 * data, if so trigger a read to get that tls header and the 
	 * offset for the next tls header in the data stream 
	 */
	/* if (dtlsp->next_tls_hdr_offset < md->desc.data_len) { */
	slt		c1, d.next_tls_hdr_offset, k.desc_data_len
	bcf		[!c1], tls_no_read_header
	nop
	phvwr		p.tls_global_phv_next_tls_hdr_offset, d.next_tls_hdr_offset
	add		r2, r0, k.desc_aol0_addr
	add		r3, r0, k.desc_aol0_offset
	add		r3, r3, k.next_tls_hdr_offset
	add		r2, r2, r3

table_read_tls_header:	
    CAPRI_NEXT_TABLE0_READ(k.fid, TABLE_LOCK_DIS, tls_read_header_process, r2, 0, 0, TABLE_SIZE_8_BITS)
	b		tls_rx_serq_process_done
	nop
	/* md->next_tls_hdr_offset = dtlsp->next_tls_hdr_offset; */
	/* tls_read_header_process(&phv); */

tls_no_read_header:
	/* if (dtlsp->next_tls_hdr_offset == md->desc.data_len) { */
	seq		c1, d.next_tls_hdr_offset, k.desc_data_len
	
	bcf 		[!c1], tls_rx_serq_process_done
	nop

	b		table_read_queue_brq
	nop
	
tls_rx_serq_enc_process:
	/* ENQ_DESC(*etlsp, enc, md->seqe.desc); */
	/* if (TAIL_DESC(cb,name) == NULL) { */
	seq		c1, d.qtail, r0
	/*    TAIL_DESC(cb, name) = desc; */
	tblwr.c1		d.qtail, k.desc
dma_cmd_enc_desc_entry_last:
	/* SET_DESC_ENTRY(TAIL_DESC(cb,name), MAX_ENTRIES_PER_DESC - 1, desc, 0, 0); */
	
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		    r5, r5, d.qtail
	phvwr		p.dma_cmd0_dma_cmd_addr, r5

	phvwr		p.aol_next_addr, k.desc

    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, TLS_PHV_AOL_DESC_END
	phvwri		p.dma_cmd1_dma_cmd_phv_end_addr, TLS_PHV_AOL_DESC_START

	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd1_dma_cmd_eop, 0

	/* etlsp->enc_nxt.desc = TAIL_DESC(*etlsp, enc); */
	tblwr		d.nxt_desc, k.desc

table_read_queue_brq:
	phvwri		p.tls_global_phv_pending_queue_brq, 1
table_read_TNMDR_ALLOC_IDX:
	addi 		r3, r0, TNMDR_ALLOC_IDX
	CAPRI_NEXT_IDX0_READ(TABLE_LOCK_DIS, tls_alloc_tnmdr_start,
	                    r3, TABLE_SIZE_16_BITS)
table_read_TNMPR_ALLOC_IDX:
	addi 		r3, r0, TNMPR_ALLOC_IDX
	CAPRI_NEXT_IDX1_READ(TABLE_LOCK_DIS, tls_alloc_tnmpr_start,
	                    r3, TABLE_SIZE_16_BITS)

tls_rx_serq_process_done:
	nop.e
	nop


