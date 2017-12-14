/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * 	Implements the writing of request to BRQ
 *  Stage 5, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"        
#include "tls_common.h"

struct phv_ p	;
struct tx_table_s5_t0_k k;
struct tx_table_s5_t0_d d;

	
%%
	.param	    BRQ_BASE
        .param      tls_mac_pre_crypto_stats_process
        
tls_mac_queue_brq_process:
    smeqb       c5, k.to_s5_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO

    CAPRI_SET_DEBUG_STAGE4_7(p.to_s6_debug_stage4_7_thread, CAPRI_MPU_STAGE_5, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID
    addi	r5, r0, TLS_PHV_DMA_COMMANDS_START
    add		r4, r5, r0
    phvwr	p.p4_txdma_intr_dma_cmd_ptr, r4

dma_cmd_enc_brq_slot:
    add         r7, r0, d.{u.tls_queue_brq5_d.pi}.wx

    sll		r5, r7, NIC_BRQ_ENTRY_SIZE_SHIFT

    /* Set the DMA_WRITE CMD for BRQ slot */
    addi	r1, r0, BRQ_BASE
    add		r1, r1, r5

    /* Fill the barco request */        
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r1, barco_desc_input_list_address,
                                barco_desc_second_key_desc_index)
dma_cmd_idesc:
    /* Already setup in Stage 2, Table 0 */
    add         r1, r0, k.to_s5_idesc
    addi        r1, r1, PKT_DESC_AOL_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd3_dma_cmd, r1, idesc_A0, idesc_next_pkt)

    /* NOOP if we bypass Barco */
    phvwri.c5   p.dma_cmd5_dma_cmd_type, 0
        
dma_cmd_output_list_addr:
    add		r5, r0, k.to_s5_idesc
    addi	r5, r5, 4

    bcf         [!c5], dma_cmd_ring_bsq_doorbell_skip
    nop

    /* for Barco bypass - use input descriptor */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd4_dma_cmd, r5, barco_desc_input_list_address,
                                barco_desc_input_list_address)
        
dma_cmd_ring_bsq_doorbell:

    CAPRI_DMA_CMD_RING_DOORBELL(dma_cmd5_dma_cmd, LIF_TLS, 0, k.tls_global_phv_fid, TLS_SCHED_RING_BSQ, 0,
                                barco_desc_second_key_desc_index)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd5_dma_cmd)
    b           tls_queue_brq_process_done
    nop

dma_cmd_ring_bsq_doorbell_skip: 

dma_cmd_brq_doorbell:

    /* Barco DMA Channel PI in r7 */
    addi        r7, r7, 1
    phvwr       p.barco_dbell_pi, r7.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd5_dma_cmd, CAPRI_BARCO_MP_MPNS_REG_MPP2_PRODUCER_IDX,
                                  barco_dbell_pi, barco_dbell_pi)

    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd5_dma_cmd)


tls_queue_brq_process_done:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_mac_pre_crypto_stats_process,
                    k.tls_global_phv_qstate_addr, TLS_TCB_PRE_CRYPTO_STATS_OFFSET,
                    TABLE_SIZE_512_BITS)
    nop.e
    nop
