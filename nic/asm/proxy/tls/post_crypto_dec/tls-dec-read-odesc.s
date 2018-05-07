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
    .param      tls_dec_queue_sesq_process
    .param      tls_dec_post_crypto_stats_process
    .param      tls_dec_queue_l7q_process
    .param      tls_dec_gc_setup

tls_dec_post_read_odesc:
    CAPRI_SET_DEBUG_STAGE4_7(p.stats_debug_stage4_7_thread, CAPRI_MPU_STAGE_6, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID
    smeqb       c5, k.{to_s6_debug_dol_sbit0_ebit6...to_s6_debug_dol_sbit7_ebit7}, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO

    sne         c1, k.tls_global_phv_l7_proxy_en, r0
    sne         c2, k.tls_global_phv_l7_proxy_type_span, r0
    
    phvwr.c1    p.l7_desc_A0, d.u.tls_read_odesc_d.A0

    /*
     * Trim off the AAD from the output
     *  - AES-CCM uses 2 16-byte header blocks to specify AAD.
     */
    seq         c3, k.to_s6_do_post_ccm_dec, 1
    addi.!c3    r2, r0, NTLS_AAD_SIZE
    addi.c3     r2, r0, TLS_AES_CCM_HEADER_SIZE
    add         r1, d.{u.tls_read_odesc_d.O0}.wx, r2
    phvwrpair   p.odesc_A0, d.u.tls_read_odesc_d.A0,    \
                p.odesc_O0, r1.wx
    phvwr.c1    p.l7_desc_O0, r1.wx
    
    sub         r1, d.{u.tls_read_odesc_d.L0}.wx, r2
    phvwr       p.odesc_L0, r1.wx
    phvwr.c1    p.l7_desc_L0, r1.wx

    /* Skip queue to SESQ for L7 IPS case */
    bcf         [c1 & !c2], tls_dec_queue_l7q
    nop

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_queue_sesq_process,
                           k.tls_global_phv_qstate_addr,
                       	   TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

    bcf         [c5], tls_dec_post_read_odesc_skip_gc
    nop

    CAPRI_NEXT_TABLE_READ_i(2, TABLE_LOCK_DIS, tls_dec_gc_setup,
                        CAPRI_SEM_TLS_RNMDR_IDX_INC_ADDR, TABLE_SIZE_32_BITS)

tls_dec_post_read_odesc_skip_gc:

	CAPRI_NEXT_TABLE_READ_OFFSET(3, TABLE_LOCK_DIS, tls_dec_post_crypto_stats_process,
	                    k.tls_global_phv_qstate_addr,
	                    TLS_TCB_POST_CRYPTO_STATS_OFFSET, TABLE_SIZE_512_BITS)

    /* Skip queue to L7Q if L7 proxy is not enabled */
    bcf         [!c1], tls_dec_post_read_odesc_done
    nop

tls_dec_queue_l7q:

    CAPRI_NEXT_TABLE_READ_OFFSET(1, 
                                 TABLE_LOCK_EN, 
                                 tls_dec_queue_l7q_process,
                                 k.tls_global_phv_qstate_addr,
                       	         TLS_TCB_CRYPT_OFFSET,
                                 TABLE_SIZE_512_BITS
                                 )

tls_dec_post_read_odesc_done:
    nop.e
    nop
    
