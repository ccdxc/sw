/*
 *      Implements the reading of BSQ to pick up barco completion
 *  Stage 0, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
        

struct phv_ p   ;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

        .param          tls_enc_rx_bsq_enc_dummy_process
        
tls_enc_post_crypto_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s7_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)
    phvwr       p.to_s7_debug_dol, d.u.read_tls_stg0_d.debug_dol
    phvwr       p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow

    /*
     * Check if this is post-encrypt stage of AES-CBC-HMAC-SHA2 (MAC-then-encrypt) or CCM
     * (barco-command[31:24], endian-swapped).
     */
    add         r3, d.u.read_tls_stg0_d.barco_command[7:0], r0
    indexb      r2, r3, [0x73, 0x74, 0x05], 0
    seq.s       c1, 0, r2
    phvwri.c1   p.to_s6_do_post_ccm_enc, 1
    slt.s       c2, 0, r2
    phvwri.c2   p.tls_global_phv_post_cbc_enc, 1

    phvwrpair   p.tls_global_phv_fid, k.p4_txdma_intr_qid[15:0],  \
                p.tls_global_phv_qstate_addr,               \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}[31:0]

    add         r3, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

    /* Increment CI in stage 0 */
    tbladd      d.{u.read_tls_stg0_d.ci_1}.hx, 1

    /*
     * Check if pi == ci, in which case we need to ring the BSQ doorbell later.
     */
    seq         c4, d.{u.read_tls_stg0_d.ci_1}.hx, d.{u.read_tls_stg0_d.pi_1}.hx
    phvwri.c4   p.tls_global_phv_pending_rx_bsq, 1
	
table_read_rx_bsq_enc: 
        CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_enc_rx_bsq_enc_dummy_process,
                              r3, TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
