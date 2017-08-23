/*
 * 	Implements the writing of barco odesc to SESQ
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
	brq_pidx			 :  RING_INDEX_WIDTH ;
	odesc	                         :  ADDRESS_WIDTH ;
	opage	                         :  ADDRESS_WIDTH ;	
	next_tls_hdr_offset		 :  16		  ;
	enc_flow	  		 :  1		  ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	.param		BRQ_BASE
tls_queue_brq_process_start:
	/*   if (IS_ENC_FLOW) { */
	sne		c1, r0, k.enc_flow
	bcf		[c1], tls_queue_brq_enc_process
	nop
tls_queue_brq_dec_process:
	/*   tlsp->next_tls_hdr_offset = md->next_tls_hdr_offset; */
	tblwr		d.next_tls_hdr_offset, k.next_tls_hdr_offset

dma_cmd_dec_data_len:
	/*   brq.odesc->data_len = tlsp->cur_tls_data_len; */
	add		r5, r0, k.odesc
	addi		r5, r5, NIC_DESC_DATA_LEN_OFFSET
	phvwr		p.dma_cmd0_addr, r5
	/* Fill the data len */
	phvwr		p.cur_tls_data_len, d.cur_tls_data_len

	phvwri		p.dma_cmd0_pad, TLS_PHV_CUR_TLS_DATA_LEN_START
	phvwri		p.dma_cmd0_size, TLS_PHV_CUR_TLS_DATA_LEN_SIZE
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	/*   tlsp->cur_tls_data_len = md->next_tls_hdr_offset - sizeof(tls_hdr_t); */
	add		r1, r0, k.next_tls_hdr_offset
	addi		r1, r1, -TLS_HDR_SIZE

	/*
	  SET_DESC_ENTRY(brq.odesc, 0, 
		 md->opage, 
		 NIC_PAGE_HEADROOM, 
		 tlsp->cur_tls_data_len);
	 */
dma_cmd_dec_desc_entry0:
	addi		r5, r0, NIC_DESC_ENTRY0_OFFSET
	add		r5, r5, k.odesc
	phvwr		p.dma_cmd1_addr, r5

	phvwr		p.desc_aol0_addr, k.opage
	phvwri		p.desc_aol0_offset, NIC_PAGE_HEADROOM
	phvwr		p.desc_aol0_len, d.cur_tls_data_len

	phvwri		p.dma_cmd1_pad, TLS_PHV_AOL_START
	phvwri		p.dma_cmd1_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM
	
dma_cmd_dec_brq_slot:
	add		r5, r0, k.brq_pidx
	mincr		r5, 1, BRQ_TABLE_SIZE_SHFT
	sll		r5, r5, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	add		r1, r0, BRQ_BASE
	add		r1, r1, r5

	phvwr		p.dma_cmd2_addr,r1
	/* Fill the barco request */

	phvwr		p.brq_idesc, d.nxt_desc
	phvwr		p.brq_odesc, k.odesc
	phvwr		p.brq_key, d.key_addr
	phvwr		p.brq_iv, d.iv_addr
	phvwri		p.brq_auth_tag, 0
	phvwri 		p.brq_payload_offset, 0
	phvwri		p.brq_status, 0
	phvwri		p.brq_opaque_tag, 0
	phvwr		p.brq_rsvd, d.ofid

	phvwri		p.brq_cmd_core, BARCO_SYM_CMD_CORE_AES
	phvwri		p.brq_cmd_keysize, BARCO_SYM_CMD_KEYSIZE_AES128
	phvwri		p.brq_cmd_mode, BARCO_SYM_CMD_MODE_GCM
	phvwri		p.brq_cmd_hash, BARCO_SYM_CMD_HASH_SHA256
	phvwri		p.brq_cmd_op, BARCO_SYM_CMD_OP_DECRYPT
	
	phvwri		p.dma_cmd2_pad, TLS_PHV_BRQ_REQ_START
	phvwri		p.dma_cmd2_size, NIC_BRQ_ENTRY_SIZE
	phvwri		p.dma_cmd2_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TLS_PHV_DMA_COMMAND_TOTAL_LEN

	b		tls_queue_brq_process_done
	nop
tls_queue_brq_enc_process:
	/*   brq.odesc->data_len = brq.idesc->data_len + sizeof(tls_hdr_t); */
dma_cmd_enc_data_len:
	/*   brq.odesc->data_len = tlsp->cur_tls_data_len; */
	add		r5, r0, k.odesc
	addi		r5, r5, NIC_DESC_DATA_LEN_OFFSET
	phvwr		p.dma_cmd0_addr, r5
	/* Fill the data len */
	add		r1, d.cur_tls_data_len, TLS_HDR_SIZE
	phvwr		p.cur_tls_data_len, d.cur_tls_data_len

	phvwri		p.dma_cmd0_pad, TLS_PHV_CUR_TLS_DATA_LEN_START
	phvwri		p.dma_cmd0_size, TLS_PHV_CUR_TLS_DATA_LEN_SIZE
	phvwri		p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	/*
	    SET_DESC_ENTRY(brq.odesc, 0, 
		 md->opage, 
		 NIC_PAGE_HEADROOM + sizeof(tls_hdr_t), 
		 brq.odesc->data_len);
         */
dma_cmd_enc_desc_entry0:
	add		r5, r0, k.odesc
	addi		r5, r5, NIC_DESC_ENTRY0_OFFSET
	phvwr		p.dma_cmd1_addr, r5

	phvwr		p.desc_aol0_addr, k.odesc

	addi		r5, r0, NIC_PAGE_HEADROOM
	addi		r5, r5, TLS_HDR_SIZE
	phvwr		p.desc_aol0_offset, r5

	/* r1 = d.curr_tls_data_len + TLS_HDR_SIZE */
	phvwr		p.desc_aol0_len, r1

	phvwri		p.dma_cmd1_pad, TLS_PHV_AOL_START
	phvwri		p.dma_cmd1_size, NIC_DESC_ENTRY_SIZE
	phvwri		p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

dma_cmd_enc_tls_hdr:
	/* tlsh = (tls_hdr_t *)(u64)(md->opage + NIC_PAGE_HEADROOM); */
	add		r5, r0, k.opage
	addi		r5, r5, NIC_PAGE_HEADROOM
	phvwr		p.dma_cmd2_addr, r5

	/*
	  tlsh->type = NTLS_RECORD_DATA;
          tlsh->version_major = NTLS_TLS_1_2_MAJOR;
          tlsh->version_minor = NTLS_TLS_1_2_MINOR;
          tlsh->len = brq.idesc->data_len;
	 */
	
	
	phvwri		p.tls_hdr_type, NTLS_RECORD_DATA
	phvwri		p.tls_hdr_version_major, NTLS_TLS_1_2_MAJOR
	phvwri		p.tls_hdr_version_minor, NTLS_TLS_1_2_MINOR

	phvwr		p.tls_hdr_len, d.cur_tls_data_len
	phvwri		p.dma_cmd2_pad, TLS_PHV_TLSH_START
	phvwri		p.dma_cmd2_size, TLS_HDR_SIZE
	phvwri		p.dma_cmd2_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

dma_cmd_enc_brq_slot:
	add		r5, r0, k.brq_pidx
	mincr		r5, 1, BRQ_TABLE_SIZE_SHFT
	sll		r5, r5, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	addi		r1, r0, BRQ_BASE
	add		r1, r1, r5

	phvwr		p.dma_cmd2_addr,r1
	/* Fill the barco request */

	phvwr		p.brq_idesc, d.nxt_desc
	phvwr		p.brq_odesc, k.odesc
	phvwr		p.brq_key, d.key_addr
	phvwr		p.brq_iv, d.iv_addr
	phvwri		p.brq_auth_tag, 0
	phvwri 		p.brq_payload_offset, 0
	phvwri		p.brq_status, 0
	phvwri		p.brq_opaque_tag, 0
	phvwr		p.brq_rsvd, d.ofid

	phvwri		p.brq_cmd_core, BARCO_SYM_CMD_CORE_AES
	phvwri		p.brq_cmd_keysize, BARCO_SYM_CMD_KEYSIZE_AES128
	phvwri		p.brq_cmd_mode, BARCO_SYM_CMD_MODE_GCM
	phvwri		p.brq_cmd_hash, BARCO_SYM_CMD_HASH_SHA256
	phvwri		p.brq_cmd_op, BARCO_SYM_CMD_OP_DECRYPT
	
	phvwri		p.dma_cmd2_pad, TLS_PHV_BRQ_REQ_START
	phvwri		p.dma_cmd2_size, NIC_BRQ_ENTRY_SIZE
	phvwri		p.dma_cmd2_cmd, CAPRI_DMA_COMMAND_PHV_TO_MEM

	addi		r4, r4, TLS_PHV_DMA_COMMAND_TOTAL_LEN

tls_queue_brq_process_done:
	nop.e
	nop
