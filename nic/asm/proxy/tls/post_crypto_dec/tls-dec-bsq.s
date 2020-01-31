/*
 * 	Implements the reading of BSQ to pick up barco completion
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
	

struct phv_ p	;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

   	.param		tls_dec_rx_bsq_dec_dummy_process
    .param      TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE
	.param      TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE
    .param      tls_dec_free_barco_ci
	
tls_dec_post_crypto_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.stats_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)

    /*  Verify that we do have space in the SESQ, if not
        defer processing of the post-Barco scheduler event
    */
    add         r1, r0, d.u.read_tls_stg0_d.sw_sesq_pi
    mincr       r1, CAPRI_SESQ_RING_SLOTS_SHIFT, 1
    seq         c1, r1, d.u.read_tls_stg0_d.sw_sesq_ci /* SESQ Ring Full */
    bcf         [c1], tls_dec_post_crypto_process_defer
    nop

    /* Record Queue empty check - this should really never happen */
    seq     c1, d.u.read_tls_stg0_d.recq_pi, d.u.read_tls_stg0_d.recq_ci
    bcf     [c1], tls_dec_post_crypto_process_defer /* Record queue empty */
    nop

    /* Allocate the SESQ PI for this segment to TCP-TX */
    phvwr       p.tls_global_phv_sesq_pi, d.u.read_tls_stg0_d.sw_sesq_pi
    tblmincri   d.u.read_tls_stg0_d.sw_sesq_pi, CAPRI_SESQ_RING_SLOTS_SHIFT, 1

    /* Increment CI in stage 0 */
    tbladd.f    d.{u.read_tls_stg0_d.ci_1}.hx, 1

    phvwr       p.tls_global_phv_debug_dol, d.u.read_tls_stg0_d.debug_dol

    /*
     * Check if this is AES-CCM decrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    seq         c1, d.u.read_tls_stg0_d.barco_command[7:0], 0x05
    phvwri.c1   p.to_s6_do_post_ccm_dec, 1


    phvwrpair   p.tls_global_phv_qstate_addr,                       \
                k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}[31:0], \
                p.tls_global_phv_fid, k.p4_txdma_intr_qid[15:0]

    /*
     * Check if pi == ci, in which case we need to ring the BSQ doorbell later.
     */
    seq         c4, d.{u.read_tls_stg0_d.ci_1}.hx, d.{u.read_tls_stg0_d.pi_1}.hx
    bcf         [!c4], tls_dec_post_crypto_skip_bsq_dbell
    nop

    /* address will be in r4 */
    addi         r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add	         r1, k.p4_txdma_intr_qid[15:0], r0

    /*
     * data will be in r3
     *
     * We'd have incremented CI in stage 0, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */
    add          r5, d.{u.read_tls_stg0_d.ci_1}.hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_BSQ, r5)

    memwr.dx     r4, r3

tls_dec_post_crypto_skip_bsq_dbell:

    sne         c2, d.u.read_tls_stg0_d.l7_proxy_type, L7_PROXY_TYPE_NONE
    phvwri.c2   p.tls_global_phv_flags_l7_proxy_en, 1
    seq         c2, d.u.read_tls_stg0_d.l7_proxy_type, L7_PROXY_TYPE_SPAN
    phvwri.c2   p.tls_global_phv_flags_l7_proxy_type_span, 1

table_read_rx_bsq_dec: 
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS, tls_dec_rx_bsq_dec_dummy_process,
                                 k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}[31:0],
                                 TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
    addui.!c1   r5, r0, hiword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    addi.!c1    r5, r5, loword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    addui.c1    r5, r0, hiword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)
    addi.c1     r5, r5, loword(TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_EN, tls_dec_free_barco_ci, r5, TABLE_SIZE_256_BITS)
    nop

tls_dec_post_crypto_process_defer:
    nop.e
    nop
