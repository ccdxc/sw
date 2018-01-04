/*
 *      Implements the reading of packet descriptor
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct phv_                 p;
struct tx_table_s2_t0_d     d;
struct tx_table_s2_t0_k     k;

%%
    .param      tls_dec_rx_serq_process
    .align

tls_dec_pkt_descriptor_process:
    phvwr   p.s2_s3_t0_phv_idesc_aol0_addr, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    phvwr   p.s2_s3_t0_phv_idesc_aol0_offset, d.{u.tls_read_pkt_descr_aol_d.O0}.wx 
    phvwr   p.s2_s3_t0_phv_idesc_aol0_len, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

    /*
     * Check if this is AES-CCM decrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    bbeq        k.tls_global_phv_do_pre_ccm_dec, 1, tls_dec_pkt_descriptor_ccm_process
    nop

    phvwr   p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    add     r2, r0, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    add     r1, r0, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    phvwr   p.idesc_O0, r1.wx
    
    add     r3, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    subi    r3, r3, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr   p.idesc_L0, r3.wx

    /* Setup DMA command to write the AAD */
    add     r3, r2, r1

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r3, s4_s6_t0_phv_aad_seq_num,
                                s4_s6_t0_phv_aad_length)

    /* Setup barco command authentication tag address */
    add     r1, d.{u.tls_read_pkt_descr_aol_d.A0}.dx, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    add     r1, r1, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    sub     r1, r1, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr   p.barco_desc_auth_tag_addr, r1.dx

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_serq_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                                 TABLE_SIZE_512_BITS)

tls_pkt_descriptor_process_done:
    nop.e
    nop


tls_dec_pkt_descriptor_ccm_process:

    /*
     * The pre-decrypt packet has AAD + data-payload in the ipage, we need to setup
     * the CCM-header(which includes AAD) + data-payload for decrypt request with barco.
     */
    phvwr   p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    sub     r1, d.{u.tls_read_pkt_descr_aol_d.O0}.wx, (TLS_AES_CCM_HEADER_SIZE - NTLS_AAD_SIZE)
    phvwr   p.idesc_O0, r1.wx
    
    add     r3, d.{u.tls_read_pkt_descr_aol_d.L0}.wx, (TLS_AES_CCM_HEADER_SIZE - NTLS_AAD_SIZE)
    subi    r3, r3, TLS_AES_CCM_AUTH_TAG_SIZE
    phvwr   p.idesc_L0, r3.wx

    /* Setup DMA command to write the AAD */
    add     r3, r1, d.{u.tls_read_pkt_descr_aol_d.A0}.dx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r3, ccm_header_with_aad_B_0_flags,
                                ccm_header_with_aad_B_1_zero_pad)

    /* Setup barco command authentication tag address */
    add     r1, d.{u.tls_read_pkt_descr_aol_d.A0}.dx, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    add     r1, r1, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    sub     r1, r1, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr   p.barco_desc_auth_tag_addr, r1.dx

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_serq_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                                 TABLE_SIZE_512_BITS)
    nop.e
    nop
	
