/*
 *      Implements the reading of BSQ to pick up barco completion
 *  Stage 0, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
        

struct phv_ p   ;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

        .param          tls_enc_rx_bsq_enc_dummy_process
	    .param          TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE
	    .param          TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE
        .param          tls_enc_free_barco_ci
        
tls_enc_post_crypto_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s7_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)

    /*  Verify that we do have space in the SESQ, if not
        defer processing of the post-Barco scheduler event
    */
    add         r1, r0, d.u.read_tls_stg0_d.sw_sesq_pi
    mincr       r1, ASIC_SESQ_RING_SLOTS_SHIFT, 1
    seq         c1, r1, d.u.read_tls_stg0_d.sw_sesq_ci /* SESQ Ring Full */
    bcf         [c1], tls_enc_post_crypto_process_defer
    nop

    /* Record Queue empty check - this should really never happen */
    seq     c1, d.u.read_tls_stg0_d.recq_pi, d.u.read_tls_stg0_d.recq_ci
    bcf     [c1], tls_enc_post_crypto_process_defer /* Record queue empty */
    nop

    /* Allocate the SESQ PI for this segment to TCP-TX */
    phvwr       p.tls_global_phv_sesq_pi, d.u.read_tls_stg0_d.sw_sesq_pi
    tblmincri   d.u.read_tls_stg0_d.sw_sesq_pi, ASIC_SESQ_RING_SLOTS_SHIFT, 1

    /* Increment CI in stage 0 */
    tbladd.f    d.{u.read_tls_stg0_d.ci_1}.hx, 1


    phvwr       p.tls_global_phv_debug_dol, d.u.read_tls_stg0_d.debug_dol

    /*
     * Check if this is post-encrypt stage of AES-CBC-HMAC-SHA2 (MAC-then-encrypt) or CCM
     * (barco-command[31:24], endian-swapped).
     */
    add         r3, d.u.read_tls_stg0_d.barco_command[7:0], r0
    indexb      r2, r3, [0x73, 0x74, 0x05], 0
    seq.s       c1, 0, r2
    phvwri.c1   p.to_s6_do_post_ccm_enc, 1
    slt.s       c2, 0, r2
    phvwri.c2   p.tls_global_phv_flags_post_cbc_enc, 1

    phvwrpair   p.tls_global_phv_qstate_addr,               \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}[31:0], \
                p.tls_global_phv_fid, k.p4_txdma_intr_qid[15:0]

    /*
     * Check if pi == ci, in which case we need to ring the BSQ doorbell later.
     */
    seq         c4, d.{u.read_tls_stg0_d.ci_1}.hx, d.{u.read_tls_stg0_d.pi_1}.hx
    bcf         [!c4], table_read_rx_bsq_enc
    nop

    /* address will be in r4 */
    addi         r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TLS)

    /*
     * data will be in r3
     *
     * We'd have incremented CI in stage 0, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */
    CAPRI_RING_DOORBELL_DATA_NOP(k.p4_txdma_intr_qid)

    memwr.dx     r4, r3

	
table_read_rx_bsq_enc: 
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_enc_rx_bsq_enc_dummy_process,
        k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}[31:0],
        TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)

    addui.!c1   r5, r0, hiword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    addi.!c1    r5, r5, loword(TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE)
    addui.c1    r5, r0, hiword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)
    addi.c1     r5, r5, loword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_EN, tls_enc_free_barco_ci, r5, TABLE_SIZE_256_BITS)
    nop

tls_enc_post_crypto_process_defer:
    phvwri.e        p.p4_intr_global_drop, 1
    nop
