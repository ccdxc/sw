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
	phvwr		p.dma_cmd0_dma_cmd_addr, r5
	/* Fill the data len */
	add		    r1, k.to_s5_cur_tls_data_len, TLS_HDR_SIZE
	phvwr		p.to_s5_cur_tls_data_len, k.to_s5_cur_tls_data_len

    phvwri      p.dma_cmd0_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(to_s5_cur_tls_data_len)
	phvwri		p.dma_cmd0_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(to_s5_cur_tls_data_len)

	phvwri		p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_eop, 0

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

	addi		r5, r0, NIC_PAGE_HEADROOM
	addi		r5, r5, TLS_HDR_SIZE
	phvwr		p.aol_O0, r5.wx

	/* r1 = d.cur_tls_data_len + TLS_HDR_SIZE */
    addi        r1, r0, 512
	phvwr		p.aol_L0, r1.wx

    /* For now clear the rest of the descriptor */
    phvwr       p.aol_A1, r0
    phvwr       p.aol_O1, r0
    phvwr       p.aol_L1, r0
    phvwr       p.aol_A2, r0
    phvwr       p.aol_O2, r0
    phvwr       p.aol_L2, r0
    phvwr       p.aol_next_addr, r0
    phvwr       p.aol_next_pkt, r0

    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(aol_A0)
    phvwri		p.dma_cmd1_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(aol_next_pkt)
        
    phvwri		p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd1_dma_cmd_eop, 0


dma_cmd_enc_tls_hdr:
	/* tlsh = (tls_hdr_t *)(u64)(md->opage + NIC_PAGE_HEADROOM); */
	add		    r5, r0, k.to_s5_opage
	addi		r5, r5, NIC_PAGE_HEADROOM
	phvwr		p.dma_cmd2_dma_cmd_addr, r5

	/*
	  tlsh->type = NTLS_RECORD_DATA;
          tlsh->version_major = NTLS_TLS_1_2_MAJOR;
          tlsh->version_minor = NTLS_TLS_1_2_MINOR;
          tlsh->len = brq.idesc->data_len;
	 */
	
	

	phvwr		p.tls_global_phv_tls_hdr_len, k.to_s5_cur_tls_data_len

    phvwri      p.dma_cmd2_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(tls_global_phv_tls_hdr_type)
	phvwri		p.dma_cmd2_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(tls_global_phv_tls_hdr_len)

    phvwri		p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd2_dma_cmd_eop, 0


dma_cmd_iv:
    add         r5, r0, k.to_s5_opage
    phvwr       p.dma_cmd3_dma_cmd_addr, r5

    phvwr       p.barco_desc_iv_address, r5.dx

    phvwri      p.dma_cmd3_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(crypto_iv_explicit_iv)
    phvwri      p.dma_cmd3_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(crypto_iv_salt)

    phvwri      p.dma_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd3_dma_cmd_eop, 0

dma_cmd_enc_brq_slot1:
#   TBD : Comment this once BRQ QPCB is setup
    tblwr       d.u.tls_queue_brq5_d.pi_0, 0
    nop
	add		    r5, r0, d.u.tls_queue_brq5_d.pi_0
	tbladd		d.u.tls_queue_brq5_d.pi_0, 1

    sll		    r5, r5, NIC_BRQ_ENTRY_SIZE_SHIFT
	/* Set the DMA_WRITE CMD for BRQ slot */
	addi		r1, r0, BRQ_BASE
	add		    r1, r1, r5

	phvwr		p.dma_cmd4_dma_cmd_addr,r1
	/* Fill the barco request */

    phvwri      p.dma_cmd4_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(barco_desc_input_list_address)
    phvwri		p.dma_cmd4_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(barco_desc_application_tag_1)

    phvwri		p.dma_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd4_dma_cmd_eop, 0
        

dma_cmd_enc_brq_slot2:
    /* The remaining BRQ descriptor at offset 64 bytes (512 bits) */
    addi        r1, r1, 64
	phvwr		p.dma_cmd5_dma_cmd_addr,r1

    phvwri      p.dma_cmd5_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(barco_desc_application_tag_0)
    phvwri		p.dma_cmd5_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(barco_desc_rsvd)

    phvwri		p.dma_cmd5_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd5_dma_cmd_eop, 0

dma_cmd_output_list_addr:
	add		    r5, r0, k.to_s5_idesc
	addi		r5, r5, 4
	phvwr		p.dma_cmd6_dma_cmd_addr, r5

    
    /* For Barco use output descriptor */
    phvwri      p.dma_cmd6_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(barco_desc_output_list_address)
	phvwri		p.dma_cmd6_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(barco_desc_output_list_address)

    phvwri		p.dma_cmd6_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd6_dma_cmd_eop, 0

    addi        r1, r0, TLS_DDOL_BYPASS_BARCO
    seq         c1, r1, k.to_s5_debug_dol
    bcf         [!c1], dma_cmd_ring_bsq_doorbell_skip
    nop

    /* for Barco bypass - use input descriptor */
    phvwri      p.dma_cmd6_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(barco_desc_input_list_address)
	phvwri		p.dma_cmd6_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(barco_desc_input_list_address)

    phvwri		p.dma_cmd6_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd6_dma_cmd_eop, 0
        
dma_cmd_ring_bsq_doorbell:
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
	phvwr		p.dma_cmd7_dma_cmd_addr, r4
    CAPRI_OPERAND_DEBUG(r4)

	CAPRI_RING_DOORBELL_DATA(0, k.tls_global_phv_fid, TLS_SCHED_RING_BSQ, 0)
    /* HACK Alert: Using crypto_iv_explicit_iv field as the source of the DMA req
     * since we need a byte aligned field and barco_desc_doorbell_data is not
     */
    phvwr       p.crypto_iv_explicit_iv, r3.dx
    CAPRI_OPERAND_DEBUG(r3.dx)
	    

	phvwri		p.dma_cmd7_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(crypto_iv_explicit_iv)
    phvwri		p.dma_cmd7_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri		p.dma_cmd7_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(crypto_iv_explicit_iv)
    phvwri      p.dma_cmd7_dma_cmd_eop, 1
    phvwri      p.dma_cmd7_dma_cmd_wr_fence, 1
    b           tls_queue_brq_process_done
    nop

dma_cmd_ring_bsq_doorbell_skip: 

dma_cmd_brq_doorbell:

    /* FIXME: */
    addi        r1, r0, 1
    phvwr       p.barco_dbell_pi, r1.wx
    phvwri      p.dma_cmd7_dma_cmd_addr, CAPRI_BARCO_MD_HENS_REG_PRODUCER_IDX
    phvwri      p.dma_cmd7_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(barco_dbell_pi)
    phvwri		p.dma_cmd7_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(barco_dbell_pi)

    phvwri		p.dma_cmd7_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd7_dma_cmd_eop, 1
    phvwri      p.dma_cmd7_dma_cmd_wr_fence, 1


tls_queue_brq_process_done:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_pre_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_PRE_CRYPTO_STATS_OFFSET,
                        TABLE_SIZE_512_BITS)
	nop.e
	nop
