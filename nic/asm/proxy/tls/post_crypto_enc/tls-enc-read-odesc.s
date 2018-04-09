/* 
 *  Read odesc to setup the DMA request
 *  Stage 6, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s6_t0_d     d;
struct tx_table_s6_t0_k     k;
struct phv_                 p;

%%
    .param      tls_enc_post_crypto_stats_process
    .param	    tls_enc_queue_sesq_process
    .param      tls_enc_read_aad_process

tls_enc_post_read_odesc:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_6, CAPRI_MPU_TABLE_0)

    /*
     * If its the post-encrypt of AES-CBC-HMAC-SHA2, the output page is
     * already setup in previous stage (post-hmac). Adjust the odesc AOLs accordingly.
     *
     * Note: The branch delay slots for the two 'bbeq' instructions below are used to
     * execute common instructions instead of nop.
     */
    bbeq        k.tls_global_phv_post_cbc_enc, 1, tls_enc_post_read_odesc_do_cbc
    phvwr       p.odesc_A0, d.u.tls_read_odesc_d.A0
    bbeq        k.to_s6_do_post_ccm_enc, 1, tls_enc_post_read_odesc_do_ccm
    phvwr       p.odesc_O0, d.u.tls_read_odesc_d.O0
        

    /* Account for 16 bytes of additional authentication tag */
    addi        r2, r0, TLS_AES_GCM_AUTH_TAG_SIZE
    add         r1, d.{u.tls_read_odesc_d.L0}.wx, r2
    phvwr       p.odesc_L0, r1.wx

    /* Compute TLS header address and setup DMA command */
    add         r1, r0, d.{u.tls_read_odesc_d.A0}.dx
    add         r1, r1, d.{u.tls_read_odesc_d.O0}.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r1, tls_hdr_tls_hdr_type, tls_hdr_tls_iv)    

    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, tls_enc_read_aad_process,
                           r1, TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_queue_sesq_process,
                                 k.tls_global_phv_qstate_addr,
                                 TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

	CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS, tls_enc_post_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr,
	                    TLS_TCB_POST_CRYPTO_STATS_OFFSET, TABLE_SIZE_512_BITS)


tls_enc_post_read_odesc_done:
    nop.e
    nop.e

tls_enc_post_read_odesc_do_ccm:
    /* Account for 16 bytes of additional authentication tag */
    addi        r2, r0, TLS_AES_CCM_AUTH_TAG_SIZE
    add         r1, d.{u.tls_read_odesc_d.L0}.wx, r2

    /*
     * For the AES-CCM barco request, we would have used 2 16-byte blocks
     * of header. For the actual output packet, we need to use only 13-bytes of
     * AAD. Adjust the L0 accordingly.
     */
    sub         r1, r1, (TLS_AES_CCM_HEADER_SIZE - NTLS_AAD_SIZE)

    /*
     * At the O0 of the odesc, there's 2 16-byte blocks of CCM-header, followed by
     * ciphertext. For output packet we'll move the offset to have 13-bytes of AAD
     * followed by ciphertext.
     * We will also need to write the AAD content (5-byte TLS-header + 8-byte seq-num)
     * at the new O0, which we'll setup a DMA for.
     */
    add         r3, d.{u.tls_read_odesc_d.O0}.wx, (TLS_AES_CCM_HEADER_SIZE - NTLS_AAD_SIZE)
    phvwrpair   p.odesc_O0, r3.wx, p.odesc_L0, r1.wx
    add         r2, r3, d.{u.tls_read_odesc_d.A0}.dx
	
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r2, tls_hdr_tls_hdr_type, tls_hdr_tls_iv)    

    /*
     * We will set the next table-read from the AAD offset inside the CCM header we currently
     * have at O0 to obtain the AAD fields.
     * (d.{u.tls_read_odesc_d.A0}.dx + d.{u.tls_read_odesc_d.O0}.wx +  TLS_AES_CCM_HEADER_AAD_OFFSET)
     */
    subi         r1, r2, 1
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, tls_enc_read_aad_process,
                          r1, TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_queue_sesq_process,
                                 k.tls_global_phv_qstate_addr,
                                 TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

	CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS, tls_enc_post_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr,
	                    TLS_TCB_POST_CRYPTO_STATS_OFFSET, TABLE_SIZE_512_BITS)

    nop.e
    nop
        
tls_enc_post_read_odesc_do_cbc:

    /*
     * The TLS-header (5 bytes) + Random-IV (16 bytes) is already present in the
     * opage setup in the previous post-mac pipeline. So we'll just reset the
     * odesc offset to include that for the final packet, and fix up the L0 accordingly.
     * We don't strictly need to read the AAD from the header into PHV, as the opage has
     * everything setup already in the AES-CBC case, but we'll trigger a table-read anyway,
     * to have the same flow as non AES-CBC post-encrypt pipeline.
     */
    sub         r3, d.{u.tls_read_odesc_d.O0}.wx, (TLS_HDR_SIZE + TLS_AES_CBC_RANDOM_IV_SIZE)

    add         r4, d.{u.tls_read_odesc_d.L0}.wx, (TLS_HDR_SIZE + TLS_AES_CBC_RANDOM_IV_SIZE)
    phvwrpair   p.odesc_O0, r3.wx, p.odesc_L0, r4.wx

    add         r5, d.{u.tls_read_odesc_d.A0}.dx, r3
        
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, tls_enc_read_aad_process,
                          r5, TABLE_SIZE_512_BITS)

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_queue_sesq_process,
                                 k.tls_global_phv_qstate_addr,
                                 TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

	CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_DIS, tls_enc_post_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr,
	                    TLS_TCB_POST_CRYPTO_STATS_OFFSET, TABLE_SIZE_512_BITS)

    nop.e
    nop
    
