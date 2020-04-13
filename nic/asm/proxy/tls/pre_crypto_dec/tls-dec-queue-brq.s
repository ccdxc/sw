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
	.param		BRQ_GCM1_BASE
tls_dec_queue_brq_process:
    CAPRI_CLEAR_TABLE0_VALID

    smeqb       c5, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO

	phvwr		p.p4_txdma_intr_dma_cmd_ptr, (CAPRI_PHV_START_OFFSET(dma_cmd_aad_dma_cmd_type) / 16)

    seq         c1, k.tls_global_phv_write_arq, r0
    bcf         [!c1], tls_queue_brq_dec_process_done
    nop
        
dma_cmd_dec_odesc:
    /* Skip setting up of odesc when bypassing Barco offload */
    bcf         [c5], dma_cmd_iv
    phvwri.c5    p.dma_cmd_odesc_dma_cmd_type, 0

	add		    r5, r0, k.to_s6_odesc
	addi		r5, r5, PKT_DESC_AOL_OFFSET


	addi		r4, r0, (NIC_PAGE_HEADROOM - NTLS_AAD_SIZE)
    add         r6, r0, k.{to_s6_opage}
	phvwrpair   p.odesc_A0, r6.dx, \
	            p.odesc_O0, r4.wx

	/* r1 = d.cur_tls_data_len + TLS_HDR_SIZE */
    /* Includes the TLS header, explicit IV and authentication tag
     * Output from Barco includes the AAD
     */
    add         r4, r0, k.to_s6_cur_tls_data_len
    subi        r4, r4, (TLS_AES_GCM_AUTH_TAG_SIZE - NTLS_TLS_HEADER_SIZE)
	phvwr		p.odesc_L0, r4.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_odesc_dma_cmd, r5, odesc_A0, odesc_next_pkt)

dma_cmd_iv:
    /* Skip setting up of iv when bypassing Barco offload */
    bcf         [c5], dma_cmd_dec_brq_slot 
    phvwri.c5    p.dma_cmd_iv_dma_cmd_type, 0

    /* Use output page headroom for the IV */
    add         r5, r0, k.to_s6_opage

    phvwr       p.barco_desc_iv_address, r5.dx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_iv_dma_cmd, r5, crypto_iv_salt, crypto_iv_explicit_iv)    
	
dma_cmd_dec_brq_slot:
    /* Skip setting up of odesc when bypassing Barco offload */
    bcf         [c5], dma_cmd_idesc
    phvwri.c5    p.dma_cmd_brq_slot_dma_cmd_type, 0

	add             r7, r0, k.s4_s6_t0_phv_sw_barco_pi
        sll		r5, r7, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	addui		r1, r0, hiword(BRQ_GCM1_BASE)
	addi		r1, r1, loword(BRQ_GCM1_BASE)
	add		r1, r1, r5

   	/* Fill the barco request */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_brq_slot_dma_cmd, r1, barco_desc_input_list_address,
                                barco_desc_second_key_desc_index)

dma_cmd_idesc:
    /* Skip setting up of idesc when bypassing Barco offload */
    bcf         [c5], dma_cmd_output_list_addr
    phvwri.c5    p.dma_cmd_idesc_dma_cmd_type, 0

    add         r1, r0, k.to_s6_idesc
    add         r1, r1, PKT_DESC_AOL_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_idesc_dma_cmd, r1, idesc_A0, idesc_next_pkt)

dma_cmd_output_list_addr:
	add		    r5, r0, k.to_s6_idesc
	addi		r5, r5, 4

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_idesc_meta_dma_cmd, r5, odesc_dma_src_odesc, odesc_dma_src_odesc)

    //smeqb       c1, k.to_s6_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    //bcf         [!c1], dma_cmd_ring_bsq_doorbell_skip
    //nop


dma_cmd_brq_doorbell:

    /* Barco DMA Channel PI in r7 */
    //addi        r7, r7, 1
    mincr       r7, ASIC_BARCO_RING_SLOTS_SHIFT, 1
    phvwr       p.barco_dbell_pi, r7.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_dbell_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM1_PRODUCER_IDX,
                                  barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_dbell_dma_cmd)

tls_queue_brq_dec_process_done:
	nop.e
	nop
