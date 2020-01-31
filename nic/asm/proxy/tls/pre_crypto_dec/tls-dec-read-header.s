/*
 *      Implements the reading of TLS header from tcp data stream
 *  Stage 3, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s3_t0_k k;
struct phv_             p;
struct tx_table_s3_t0_d d;

#define D d.u.tls_read_tls_header_d
        
%%
	.param      TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE
	.param      TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE
	.param      tls_dec_read_barco_pi_process
        
tls_dec_read_header_process:
    phvwr       p.tls_global_phv_tls_hdr_type, d.u.tls_read_tls_header_d.tls_hdr_type
    phvwr       p.tls_global_phv_tls_hdr_version_major, d.u.tls_read_tls_header_d.tls_hdr_version_major
    phvwr       p.tls_global_phv_tls_hdr_version_minor, d.u.tls_read_tls_header_d.tls_hdr_version_minor
    phvwr       p.tls_global_phv_tls_hdr_len, d.u.tls_read_tls_header_d.tls_hdr_len
    /* Check if this is a TLS handshake packet */

    addi        r1, r0, NTLS_RECORD_HANDSHAKE
    /* FIXME: To be removed once the payload is generated right at DoL */
    smeqb       c1, k.tls_global_phv_debug_dol, TLS_DDOL_FAKE_HANDSHAKE_MSG, TLS_DDOL_FAKE_HANDSHAKE_MSG
    addi.c1     r1, r0, 0x01

    seq         c1, D.tls_hdr_type, r1
    phvwri.c1   p.tls_global_phv_write_arq, 1

    /* Remember input A,O,L for write-arq stage later */
    phvwr.c1    p.s5_s6_t1_s2s_arq_opage, k.s2_s3_t0_phv_idesc_aol0_addr
    phvwr.c1    p.to_s6_next_tls_hdr_offset, k.s2_s3_t0_phv_idesc_aol0_offset
    phvwr.c1    p.to_s6_cur_tls_data_len, k.s2_s3_t0_phv_idesc_aol0_len      

    /*
     * Check if this is AES-CCM decrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    bbeq        k.tls_global_phv_do_pre_ccm_dec, 1, tls_dec_read_header_ccm_process
    nop
	
    phvwr       p.crypto_iv_explicit_iv, d.u.tls_read_tls_header_d.tls_iv

    sub.c1      r1, k.s2_s3_t0_phv_idesc_aol0_len, k.tls_global_phv_next_tls_hdr_offset
    phvwr.c1    p.to_s6_cur_tls_data_len, r1

    add.!c1     r6, k.s2_s3_t0_phv_idesc_aol0_len, r0
    subi.!c1    r2, r6, (NTLS_NONCE_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE + NTLS_TLS_HEADER_SIZE)
    setcf       c2, [!c0]
    slt.!c1     c2, r1, r2
    sub.c2      r2, r2, r1
    phvwr.c2    p.tls_global_phv_next_tls_hdr_offset, r2
    phvwr.!c1   p.to_s6_cur_tls_data_len, d.u.tls_read_tls_header_d.tls_hdr_len

    /* Setup AAD using the incoming TLS record information */
    phvwr       p.s4_s6_t0_phv_aad_type, d.u.tls_read_tls_header_d.tls_hdr_type
    phvwr       p.s4_s6_t0_phv_aad_version_major, d.u.tls_read_tls_header_d.tls_hdr_version_major
    phvwr       p.s4_s6_t0_phv_aad_version_minor, d.u.tls_read_tls_header_d.tls_hdr_version_minor

    /* Adjust length to TLSCompressed.length */
    add         r1, r0, d.u.tls_read_tls_header_d.tls_hdr_len
    subi        r1, r1, (NTLS_NONCE_SIZE + TLS_AES_GCM_AUTH_TAG_SIZE)
    phvwr       p.s4_s6_t0_phv_aad_length, r1

    /*
     * Launch a locked table-read to the Barco GCM1 PI address in HBM.
     */
    addui       r5, r0, hiword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    addi        r5, r5, loword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_dec_read_barco_pi_process, r5, TABLE_SIZE_256_BITS)

tls_read_desc_process_done:
    nop.e
    nop.e

tls_dec_read_header_ccm_process:
    /* Setup AAD using the incoming TLS record information */
    phvwr       p.ccm_header_with_aad_B_1_aad_type, d.u.tls_read_tls_header_d.tls_hdr_type
    phvwr       p.ccm_header_with_aad_B_1_aad_version_major, d.u.tls_read_tls_header_d.tls_hdr_version_major
    phvwr       p.ccm_header_with_aad_B_1_aad_version_minor, d.u.tls_read_tls_header_d.tls_hdr_version_minor
    /* Adjust length to TLSCompressed.length */
    add         r1, r0, d.u.tls_read_tls_header_d.tls_hdr_len
    subi        r1, r1, (NTLS_NONCE_SIZE + TLS_AES_CCM_AUTH_TAG_SIZE)
    phvwr       p.ccm_header_with_aad_B_1_aad_length, r1
    phvwr       p.ccm_header_with_aad_B_0_length, r1

    phvwr       p.ccm_header_with_aad_B_0_nonce_explicit_iv, d.u.tls_read_tls_header_d.tls_iv

    sub.c1      r1, k.s2_s3_t0_phv_idesc_aol0_len, k.tls_global_phv_next_tls_hdr_offset
    phvwr.c1    p.to_s6_cur_tls_data_len, r1

    add.!c1     r6, k.s2_s3_t0_phv_idesc_aol0_len, r0
    subi.!c1    r2, r6, (NTLS_NONCE_SIZE + TLS_AES_CCM_AUTH_TAG_SIZE + NTLS_TLS_HEADER_SIZE)
    setcf       c2, [!c0]
    slt.!c1     c2, r1, r2
    sub.c2      r2, r2, r1
    phvwr.c2    p.tls_global_phv_next_tls_hdr_offset, r2
    phvwr.!c1   p.to_s6_cur_tls_data_len, d.u.tls_read_tls_header_d.tls_hdr_len

    /*
     * Launch a locked table-read to the Barco MPP1 PI address in HBM.
     */
    addui       r5, r0, hiword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)
    addi        r5, r5, loword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_dec_read_barco_pi_process, r5, TABLE_SIZE_256_BITS)

tls_read_desc_ccm_process_done:
    nop.e
    nop.e

