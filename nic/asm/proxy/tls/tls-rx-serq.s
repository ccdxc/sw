/*
 * 	Implements the receipt of tls request from SESQ 
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
	
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

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
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
	
	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		r5, r5, d.qtail
	phvwr		p.dma_cmd3_addr, r5

	phvwr		p.desc_aol1_addr, k.desc
	phvwri		p.desc_aol1_offset, 0
	phvwri		p.desc_aol1_len, 0

	phvwri		p.dma_cmd3_pad, TLS_PHV_AOL1_START
	phvwri		p.dma_cmd3_size, TLS_PHV_CUR_TLS_DATA_LEN_SIZE
	phvwri		p.dma_cmd3_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

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
	phvwr		p.next_tls_hdr_offset, d.next_tls_hdr_offset
	add		r2, r0, k.desc_aol0_addr
	add		r3, r0, k.desc_aol0_offset
	add		r3, r3, k.next_tls_hdr_offset
	add		r2, r2, r3

	
	phvwr		p.desc, k.desc
	phvwr		p.desc_aol0_addr, k.desc_aol0_addr
	phvwr		p.desc_aol0_offset, k.desc_aol0_offset
	phvwr		p.desc_aol0_len, k.desc_aol0_len

	phvwr		p.desc_aol1_addr, k.desc_aol0_addr
	phvwr		p.desc_aol1_offset, r3
	phvwr		p.desc_aol1_len, k.desc_aol0_len

table_read_tls_header:	
	TLS_READ_ADDR(r2, TABLE_TYPE_RAW, tls_read_header_process)
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
	
	addi		r5, r0, NIC_DESC_ENTRY_L_OFFSET
	add		r5, r5, d.qtail
	phvwr		p.dma_cmd3_addr, r5

	phvwr		p.desc_aol1_addr, k.desc
	phvwri		p.desc_aol1_offset, 0
	phvwri		p.desc_aol1_len, 0

	phvwri		p.dma_cmd3_pad, TLS_PHV_AOL1_START
	phvwri		p.dma_cmd3_size, TLS_PHV_CUR_TLS_DATA_LEN_SIZE
	phvwri		p.dma_cmd3_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	/* etlsp->enc_nxt.desc = TAIL_DESC(*etlsp, enc); */
	tblwr		d.nxt_desc, k.desc

table_read_queue_brq:
	phvwri		p.pending_queue_brq, 1
table_read_alloc_rnmdr:
	TLS_READ_IDX(RNMDR_ALLOC_IDX, TABLE_TYPE_RAW, tls_alloc_rnmdr_process)
/*
	TLS_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, tls_queue_brq_process,
	                    TLS_TCB_TABLE_BASE, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TLS_TCB_TABLE_ENTRY_SIZE)
*/

tls_rx_serq_process_done:
	nop.e
	nop


