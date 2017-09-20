/*
 * 	Implements the writing of barco odesc to SESQ
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"        

struct phv_ p	;
struct tx_table_s7_t0_k k	;
struct tx_table_s7_t0_tls_queue_brq7_d d ;
	
%%
	.param		BRQ_BASE
tls_dec_queue_brq_process:
    CAPRI_CLEAR_TABLE0_VALID
	/*   tlsp->next_tls_hdr_offset = md->next_tls_hdr_offset; */
	phvwr	    p.tls_global_phv_next_tls_hdr_offset, k.to_s7_next_tls_hdr_offset

dma_cmd_dec_data_len:
	/*   brq.odesc->data_len = tlsp->cur_tls_data_len; */
	add		    r5, r0, k.to_s7_odesc
	addi		r5, r5, NIC_DESC_DATA_LEN_OFFSET
	phvwr		p.dma_cmd1_dma_cmd_addr, r5
	/* Fill the data len */
	add		    r1, k.to_s7_cur_tls_data_len, TLS_HDR_SIZE
	phvwr		p.to_s7_cur_tls_data_len, k.to_s7_cur_tls_data_len

    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(to_s7_cur_tls_data_len)
	phvwri		p.dma_cmd1_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(to_s7_cur_tls_data_len)

	phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

	/*   tlsp->cur_tls_data_len = md->next_tls_hdr_offset - sizeof(tls_hdr_t); */
	add		    r1, r0, k.to_s7_next_tls_hdr_offset
	addi		r1, r1, -TLS_HDR_SIZE

	/*
	  SET_DESC_ENTRY(brq.odesc, 0, 
		 md->opage, 
		 NIC_PAGE_HEADROOM, 
		 tlsp->cur_tls_data_len);
	 */
dma_cmd_dec_desc_entry0:
	add		    r5, r0, k.to_s7_odesc
	addi		r5, r5, NIC_DESC_ENTRY0_OFFSET
	phvwr		p.dma_cmd2_dma_cmd_addr, r5

	phvwr		p.aol_A0, k.to_s7_odesc

	addi		r5, r0, NIC_PAGE_HEADROOM
	addi		r5, r5, TLS_HDR_SIZE
	phvwr		p.aol_O0, r5

	/* r1 = d.cur_tls_data_len + TLS_HDR_SIZE */
	phvwr		p.aol_L0, r1

    phvwri      p.dma_cmd2_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(aol_A0)
	phvwri		p.dma_cmd2_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(aol_L0)

    phvwri		p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
	
dma_cmd_dec_brq_slot:
	add		    r5, r0, d.pi_0
	tbladd		d.pi_0, 1
    sll		    r5, r5, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	addi		r1, r0, BRQ_BASE
	add		    r1, r1, r5

	phvwr		p.dma_cmd4_dma_cmd_addr,r1
	/* Fill the barco request */

    phvwri      p.dma_cmd4_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(barco_desc_input_list_address)
	phvwri		p.dma_cmd4_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(barco_desc_status_address)

    phvwri		p.dma_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    phvwri      p.dma_cmd4_dma_cmd_eop, 1
    phvwri      p.dma_cmd4_dma_cmd_wr_fence, 1
        
tls_queue_brq_dec_process_done:
	nop.e
	nop
