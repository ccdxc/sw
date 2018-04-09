/*
 * 	Doorbell write to clear the sched bit for the BSQ having
 *      finished the consumption processing.
 *  Stage 5, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s5_t0_k k                  ;
struct phv_ p	;
struct tx_table_s5_t0_tls_bsq_consume_d d;
	
%%
	.param      tls_enc_post_read_odesc
        
tls_enc_bsq_consume_process:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_5, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Check if we need to ring the BSQ doorbell for CI update (we do only if ci == pi).
     */
    seq          c4, k.tls_global_phv_pending_rx_bsq, 1
    b.!c4        tls_enc_check_barco_status
    nop
	
    /* address will be in r4 */
    addi         r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add	         r1, k.tls_global_phv_fid, r0

    /*
     * data will be in r3
     *
     * We'd have incremented CI in stage 0, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */
    add          r5, d.{ci_1}.hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_BSQ, r5)

    memwr.dx     r4, r3

tls_enc_check_barco_status:
	
    /* For now, if we have a Barco Op error, bail out right here */
    sne          c1, r0, k.tls_global_phv_barco_op_failed
    bcf          [c1], tls_enc_bsq_consume_process_done
    nop
table_read_QUEUE_SESQ:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_post_read_odesc,
                       	  k.to_s5_odesc, TABLE_SIZE_512_BITS)
tls_enc_bsq_consume_process_done:
	nop.e
	nop
