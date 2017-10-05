/*
 * 	Implements the submission of the decrypt request to Barco
 *  Stage 6, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"        

struct phv_                 p;
struct tx_table_s6_t0_k     k;
struct tx_table_s6_t0_d     d;
	
%%
	.param		BRQ_BASE
tls_dec_queue_brq_process:
    CAPRI_CLEAR_TABLE0_VALID

	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r5

    seq         c1, k.tls_global_phv_write_arq, r0
    bcf         [!c1], tls_queue_brq_dec_process_done
    nop
        
	/*   tlsp->next_tls_hdr_offset = md->next_tls_hdr_offset; */
//	phvwr	    p.tls_global_phv_next_tls_hdr_offset, k.to_s6_next_tls_hdr_offset

dma_cmd_dec_data_len:
	/*   brq.odesc->data_len = tlsp->cur_tls_data_len; */
	add		    r5, r0, k.to_s6_odesc
	addi		r5, r5, NIC_DESC_DATA_LEN_OFFSET

	/* Fill the data len */

    /* Setup plain-text size , RecordLen - IV - AuthTag*/
    add         r1, r0, k.to_s6_cur_tls_data_len
    sub         r1, r1, (NTLS_NONCE_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE)
    phvwr       p.to_s6_cur_tls_data_len, r1

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r5, to_s6_cur_tls_data_len,to_s6_cur_tls_data_len)    

	/*   tlsp->cur_tls_data_len = md->next_tls_hdr_offset - sizeof(tls_hdr_t); */
//	add		    r1, r0, k.to_s6_next_tls_hdr_offset

	/*
	  SET_DESC_ENTRY(brq.odesc, 0, 
		 md->opage, 
		 NIC_PAGE_HEADROOM, 
		 tlsp->cur_tls_data_len);
	 */
dma_cmd_dec_odesc:
	add		    r5, r0, k.to_s6_odesc
	addi		r5, r5, PKT_DESC_AOL_OFFSET

	phvwr		p.odesc_A0, k.{to_s6_opage}.dx

	addi		r4, r0, (NIC_PAGE_HEADROOM - NTLS_AAD_SIZE)
	phvwr		p.odesc_O0, r4.wx

	/* r1 = d.cur_tls_data_len + TLS_HDR_SIZE */
    /* Includes the TLS header, explicit IV and authentication tag
     * Output from Barco includes the AAD
     */
    add         r4, r0, k.to_s6_cur_tls_data_len
    subi        r4, r4, (TLS_AES_GCM_AUTH_TAG_SIZE - NTLS_TLS_HEADER_SIZE)
	phvwr		p.odesc_L0, r4.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r5, odesc_A0, odesc_next_pkt)

dma_cmd_iv:
    /* Use output page headroom for the IV */
    add         r5, r0, k.to_s6_opage

    phvwr       p.barco_desc_iv_address, r5.dx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd3_dma_cmd, r5, crypto_iv_salt, crypto_iv_explicit_iv)    
	
dma_cmd_dec_brq_slot:
	add		    r7, r0, d.{u.tls_queue_brq6_d.pi}.wx
    sll		    r5, r7, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	addi		r1, r0, BRQ_BASE
	add		    r1, r1, r5

   	/* Fill the barco request */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd4_dma_cmd, r1, barco_desc_input_list_address,
                                barco_desc_doorbell_data)

dma_cmd_idesc:
    add         r1, r0, k.to_s6_idesc
    add         r1, r1, PKT_DESC_AOL_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd5_dma_cmd, r1, idesc_A0, idesc_next_pkt)

dma_cmd_output_list_addr:
	add		    r5, r0, k.to_s6_idesc
	addi		r5, r5, 4

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd6_dma_cmd, r5, odesc_dma_src_odesc, odesc_dma_src_odesc)

    //smeqb       c1, k.to_s6_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    //bcf         [!c1], dma_cmd_ring_bsq_doorbell_skip
    //nop


dma_cmd_brq_doorbell:

    /* Barco DMA Channel PI in r7 */
    addi        r7, r7, 1
    phvwr       p.barco_dbell_pi, r7.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd7_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX,
                                  barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd7_dma_cmd)

tls_queue_brq_dec_process_done:
	nop.e
	nop
