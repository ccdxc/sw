/*
 * 	Implements the reading of SERQ descriptor
 *  Stage 1, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_             p;
struct tx_table_s1_t0_d d;
struct tx_table_s1_t0_k k;

%%
    .param          tls_dec_rx_serq_process
    .param          tls_dec_alloc_tnmdr_process
    .param          tls_dec_alloc_tnmpr_process
    .param          tls_dec_serq_consume_process


tls_dec_read_serq_entry_process:
    phvwr       p.to_s2_idesc, d.u.read_serq_entry_d.idesc[31:0]
    phvwrpair   p.to_s5_idesc, d.u.read_serq_entry_d.idesc[31:0], \
                p.to_s6_idesc, d.u.read_serq_entry_d.idesc[31:0]

    phvwr       p.bsq_slot_desc, d.u.read_serq_entry_d.idesc[31:0]

tls_dec_serq_consume:
    /* Skip allocating the descriptor and the page when we are bypassing Barco offload */
    smeqb   c1, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    bcf     [c1], tls_dec_pkt_descriptor_process
    nop

table_read_TNMDR_ALLOC_IDX:
    addi        r3, r0, TNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tls_dec_alloc_tnmdr_process,
                          r3, TABLE_SIZE_16_BITS)

table_read_TNMPR_ALLOC_IDX:
    addi 	r3, r0, TNMPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tls_dec_alloc_tnmpr_process,
                          r3, TABLE_SIZE_16_BITS)

tls_dec_pkt_descriptor_process:
    add         r5, r0, d.{u.read_serq_entry_d.A0}
    add         r6, r0, d.{u.read_serq_entry_d.O0}
    
    phvwrpair   p.s1_s2_t0_phv_idesc_aol0_addr, r5.dx,   \
                p.s1_s2_t0_phv_idesc_aol0_offset, r6.wx
    phvwr       p.s1_s2_t0_phv_idesc_aol0_len, d.{u.read_serq_entry_d.L0}.wx

    /*
     * Check if this is AES-CCM decrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    bbeq        k.tls_global_phv_do_pre_ccm_dec, 1, tls_dec_pkt_descriptor_ccm_process
    nop

    phvwrpair   p.idesc_A0, r5,    \
                p.idesc_O0, r6
    
    add         r3, r0, d.{u.read_serq_entry_d.L0}.wx
    subi        r3, r3, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr       p.idesc_L0, r3.wx

    /* Setup DMA command to write the AAD */
    add         r3, r5.dx, r6.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_aad_dma_cmd, r3, s4_s6_t0_phv_aad_seq_num,
                                s4_s6_t0_phv_aad_length)

    /* Setup barco command authentication tag address */
    add         r1, d.{u.read_serq_entry_d.A0}.dx, d.{u.read_serq_entry_d.O0}.wx
    add         r1, r1, d.{u.read_serq_entry_d.L0}.wx
    sub         r1, r1, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr       p.barco_desc_auth_tag_addr, r1.dx

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_serq_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_CONFIG,
                                 TABLE_SIZE_512_BITS)

tls_pkt_descriptor_process_done:
    nop.e
    nop


tls_dec_pkt_descriptor_ccm_process:

    /*
     * The pre-decrypt packet has AAD + data-payload in the ipage, we need to setup
     * the CCM-header(which includes AAD) + data-payload for decrypt request with barco.
     */
    sub         r1, d.{u.read_serq_entry_d.O0}.wx, (TLS_AES_CCM_HEADER_SIZE - NTLS_AAD_SIZE)
    phvwrpair   p.idesc_A0, d.u.read_serq_entry_d.A0,    \
                p.idesc_O0, r1.wx
    
    add         r3, d.{u.read_serq_entry_d.L0}.wx, (TLS_AES_CCM_HEADER_SIZE - NTLS_AAD_SIZE)
    subi        r3, r3, TLS_AES_CCM_AUTH_TAG_SIZE
    phvwr       p.idesc_L0, r3.wx

    /* Setup DMA command to write the AAD */
    add         r3, r1, d.{u.read_serq_entry_d.A0}.dx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_aad_dma_cmd, r3, ccm_header_with_aad_B_0_flags,
                                ccm_header_with_aad_B_1_zero_pad)

    /* Setup barco command authentication tag address */
    add         r1, d.{u.read_serq_entry_d.A0}.dx, d.{u.read_serq_entry_d.O0}.wx
    add         r1, r1, d.{u.read_serq_entry_d.L0}.wx
    sub         r1, r1, TLS_AES_GCM_AUTH_TAG_SIZE
    phvwr       p.barco_desc_auth_tag_addr, r1.dx

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_serq_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_CONFIG,
                                 TABLE_SIZE_512_BITS)
    nop.e
    nop
