/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * Implements the reading of the input packet descriptor.
 * This is needed to derive the AAD and the output length from the 
 * length of the input plain-text
 * Stage 2, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"        


struct phv_             p;
struct tx_table_s2_t3_k k;
struct tx_table_s2_t3_d d;

%%


tls_mac_pkt_descriptor_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_3)
    CAPRI_CLEAR_TABLE3_VALID

    /* Setup idesc */
    phvwr       p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    add         r2, r0, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    subi        r2, r2, NTLS_AAD_SIZE
    phvwr       p.idesc_O0, r2.wx
    add         r3, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    addi        r3, r3, NTLS_AAD_SIZE
    phvwr       p.idesc_L0, r3.wx

    /* Setup PHV2MEM DMA for AAD */
    addi	r5, r0, TLS_PHV_DMA_COMMANDS_START
    add		r6, r5, r0
    phvwr	p.p4_txdma_intr_dma_cmd_ptr, r6
	
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    add         r1, r1, r2
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r1,
                                s2_s5_t0_phv_aad_seq_num, s2_s5_t0_phv_aad_length)

    phvwr       p.s2_s5_t0_phv_aad_length, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

    add         r4, r3, r1
    phvwr       p.barco_desc_auth_tag_addr, r4.dx
    CAPRI_OPERAND_DEBUG(r4)
    CAPRI_OPERAND_DEBUG(r4.dx)

    nop.e
    nop

