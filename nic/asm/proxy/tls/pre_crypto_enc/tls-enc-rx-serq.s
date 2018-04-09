/*
 *      Implements the receipt of tls encrypt request from SERQ 
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
        

struct tx_table_s2_t0_k k       ;
struct phv_ p   ;
struct tx_table_s2_t0_tls_rx_serq_d d   ;
        
%%
    .param      tls_enc_serq_consume_process
        
tls_enc_rx_serq_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /*
     * The 'salt' field is in the 2nd 64-bit block of the TLSCB, which
     * we're doing table-read here, so we'll populate phv for salt here.
     */
    seq         c1, k.to_s2_do_pre_ccm_enc, 1
    phvwr.!c1   p.crypto_iv_salt, d.salt
    phvwr.c1    p.ccm_header_with_aad_B_0_nonce_salt, d.salt
    CAPRI_OPERAND_DEBUG(d.salt)

    addi            r5, r0, TLS_PHV_DMA_COMMANDS_START
    phvwr           p.p4_txdma_intr_dma_cmd_ptr, r5

    /* TODO: Verify queue full condition */

    add             r5, r0, d.recq_pi
    sll             r4, r5, CAPRI_BSQ_RING_SLOT_SIZE_SHFT
    add             r4, r4, d.{recq_base}.wx

    phvwr           p.bsq_slot_desc, k.to_s2_idesc

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r4, bsq_slot_desc, bsq_slot_desc)

    tblmincri       d.recq_pi, CAPRI_BSQ_RING_SLOTS_SHIFT ,1


table_read_serq_consume:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_serq_consume_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_OFFSET,
                                 TABLE_SIZE_512_BITS)
        
tls_rx_serq_process_enc_done:
        nop.e
        nop


