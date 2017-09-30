/*
 * 	Implements the writing of request to BRQ
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"        
#include "tls-sched.h"

struct phv_ p	;
struct tx_table_s5_t0_k k	;
struct tx_table_s5_t0_d d;

	
%%
	.param		BRQ_BASE
    .param      tls_enc_pre_crypto_stats_process
        
tls_enc_queue_brq_process:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s6_debug_stage4_7_thread, CAPRI_MPU_STAGE_5, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

    /*   brq.odesc->data_len = brq.idesc->data_len + sizeof(tls_hdr_t); */
dma_cmd_enc_data_len:
	/*   brq.odesc->data_len = tlsp->cur_tls_data_len; */
	add		    r5, r0, k.to_s5_odesc
	addi		r5, r5, NIC_DESC_DATA_LEN_OFFSET

	/* Fill the data len */
	add		    r1, k.to_s5_cur_tls_data_len, TLS_HDR_SIZE
	phvwr		p.to_s5_cur_tls_data_len, k.to_s5_cur_tls_data_len

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r5, to_s5_cur_tls_data_len, to_s5_cur_tls_data_len)

	/*
	    SET_DESC_ENTRY(brq.odesc, 0, 
		 md->opage, 
		 NIC_PAGE_HEADROOM + sizeof(tls_hdr_t), 
		 brq.odesc->data_len);
         */
dma_cmd_enc_desc_entry0:
	add		    r5, r0, k.to_s5_odesc
	addi		r5, r5, PKT_DESC_AOL_OFFSET
	phvwr		p.dma_cmd1_dma_cmd_addr, r5

	phvwr		p.aol_A0, k.{to_s5_opage}.dx

	addi		r4, r0, NIC_PAGE_HEADROOM
	addi		r4, r4, TLS_HDR_SIZE
	phvwr		p.aol_O0, r4.wx

	/* r1 = d.cur_tls_data_len + TLS_HDR_SIZE */
    addi        r1, r0, 512
	phvwr		p.aol_L0, r1.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r5, aol_A0, aol_next_pkt)


dma_cmd_enc_tls_hdr:
	/* tlsh = (tls_hdr_t *)(u64)(md->opage + NIC_PAGE_HEADROOM); */
	add		    r5, r0, k.to_s5_opage
	addi		r5, r5, NIC_PAGE_HEADROOM


	/*
	  tlsh->type = NTLS_RECORD_DATA;
          tlsh->version_major = NTLS_TLS_1_2_MAJOR;
          tlsh->version_minor = NTLS_TLS_1_2_MINOR;
          tlsh->len = brq.idesc->data_len;
	 */
	
	

	phvwr		p.tls_global_phv_tls_hdr_len, k.to_s5_cur_tls_data_len

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r5,
                                tls_global_phv_tls_hdr_type, tls_global_phv_tls_hdr_len)    


dma_cmd_iv:
    add         r5, r0, k.to_s5_opage

    phvwr       p.barco_desc_iv_address, r5.dx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd3_dma_cmd, r5, crypto_iv_explicit_iv, crypto_iv_salt)    

dma_cmd_enc_brq_slot1:
    add         r7, r0, d.u.tls_queue_brq5_d.pi

    sll		    r5, r7, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	addi		r1, r0, BRQ_BASE
	add		    r1, r1, r5

	/* Fill the barco request */        
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd4_dma_cmd, r1, barco_desc_input_list_address,
                                barco_desc_application_tag)
        

dma_cmd_enc_brq_slot2:
    /* The remaining BRQ descriptor at offset 64 bytes (512 bits) */
    addi        r1, r1, 64

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd5_dma_cmd, r1, barco_desc_sector_num,
                                barco_desc_doorbell_data)

dma_cmd_output_list_addr:
	add		    r5, r0, k.to_s5_idesc
	addi		r5, r5, 4

    /* For Barco use output descriptor */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd6_dma_cmd, r5, barco_desc_output_list_address,
                                barco_desc_output_list_address)

    smeqb       c1, k.to_s5_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    bcf         [!c1], dma_cmd_ring_bsq_doorbell_skip
    nop

    /* for Barco bypass - use input descriptor */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd6_dma_cmd, r5, barco_desc_input_list_address,
                                barco_desc_input_list_address)
        
dma_cmd_ring_bsq_doorbell:

    CAPRI_DMA_CMD_RING_DOORBELL(dma_cmd7_dma_cmd, LIF_TLS, 0, k.tls_global_phv_fid, TLS_SCHED_RING_BSQ, 0,
                                crypto_iv_explicit_iv)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd7_dma_cmd)
    b           tls_queue_brq_process_done
    nop

dma_cmd_ring_bsq_doorbell_skip: 

dma_cmd_brq_doorbell:

    /* Barco DMA Channel PI in r7 */
    addi        r7, r7, 1
    phvwr       p.barco_dbell_pi, r7.wx


    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd7_dma_cmd, CAPRI_BARCO_MD_HENS_REG_PRODUCER_IDX,
                                  barco_dbell_pi, barco_dbell_pi)

    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd7_dma_cmd)


tls_queue_brq_process_done:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_pre_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_PRE_CRYPTO_STATS_OFFSET,
                        TABLE_SIZE_512_BITS)
	nop.e
	nop
