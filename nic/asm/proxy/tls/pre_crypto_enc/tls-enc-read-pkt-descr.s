/*
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


tls_enc_pkt_descriptor_process:
    CAPRI_CLEAR_TABLE3_VALID

    /*
     * Check if this is CCM encrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    bbeq        k.to_s2_do_pre_ccm_enc, 1, tls_enc_pkt_descriptor_ccm_process
    nop
        
    /* Setup idesc */
    phvwr       p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    add         r2, r0, d.{u.tls_read_pkt_descr_aol_d.O0}.wx
    subi        r2, r2, NTLS_AAD_SIZE
    phvwr       p.idesc_O0, r2.wx
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    addi        r1, r1, NTLS_AAD_SIZE
    phvwr       p.idesc_L0, r1.wx

    /* Setup PHV2MEM for AAD */
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    add         r1, r1, r2
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r1,
                                s2_s5_t0_phv_aad_seq_num, s2_s5_t0_phv_aad_length)

    /* Setup odesc length */
    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    addi        r1, r1, (NTLS_AAD_SIZE)
    phvwr       p.odesc_L0, r1.wx

    phvwr       p.s2_s5_t0_phv_aad_length, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

    nop.e
    nop

tls_enc_pkt_descriptor_ccm_process:
    phvwr       p.idesc_A0, d.u.tls_read_pkt_descr_aol_d.A0
    add         r2, r0, d.{u.tls_read_pkt_descr_aol_d.O0}.wx

    /*
     * For AES-CCM, we'll setup the input data-page as follows:
     *
     *  -  2 16-byte Blocks of Header (B_0 and B_1), FOLLOWED BY
     *  -  N 16-byte Blocks of input data-packet (plaintext), where N
     *     is length(data-packet) / 16 + Last-block with any zero
     *     padding if length(data-packet) % 16 != 0
     *
     * The 2 16-byte Blocks of Header are as below:
     *   B_0: 1-byte Flags + 12-byte Nonce + 3-Byte data-length
     *   B_1: 2-byte AAD-size + 13-byte AAD (8-byte Seq-num + 5-byte
     *                                       TLS-hdr) + 1-byte zero-pad
     */ 
    subi        r2, r2, TLS_AES_CCM_HEADER_SIZE
    phvwr       p.idesc_O0, r2.wx

    add         r1, r0, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    addi        r1, r1, TLS_AES_CCM_HEADER_SIZE
    phvwr       p.idesc_L0, r1.wx

    /* Setup odesc length */
    phvwr       p.odesc_L0, r1.wx
    phvwr       p.s2_s5_t0_phv_aad_length, d.{u.tls_read_pkt_descr_aol_d.L0}.wx


    add         r2, r2, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r2,
                                ccm_header_with_aad_B_0_flags, ccm_header_with_aad_B_1_zero_pad)

    phvwr       p.ccm_header_with_aad_B_0_length, d.{u.tls_read_pkt_descr_aol_d.L0}.wx
    phvwr       p.ccm_header_with_aad_B_1_aad_length, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

    nop.e
    nop
