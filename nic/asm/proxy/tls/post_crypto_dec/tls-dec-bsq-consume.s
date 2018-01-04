/*
 * 	Doorbell write to clear the sched bit for the BSQ having
 *      finished the consumption processing.
 *  Stage 3, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s3_t0_k k                  ;
struct phv_ p	;
struct tx_table_s3_t0_tls_bsq_consume_d d;
	
%%
	.param      tls_dec_post_read_odesc
        
tls_dec_bsq_consume_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add		r1, k.tls_global_phv_fid, r0

    /*
     * data will be in r3
     *
     * We'd have incremented CI in stage 0, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */	
    add     r2, d.{ci_1}.hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_BSQ, r2)

    memwr.dx  	 r4, r3
	
    /* For now, if we have a Barco Op error, bail out right here */
    sne     c1, r0, k.tls_global_phv_barco_op_failed
    bcf     [c1], tls_dec_bsq_consume_process_done
    nop

table_read_QUEUE_SESQ:
    add     r1, r0, k.to_s3_odesc
    addi    r1, r1, PKT_DESC_AOL_OFFSET

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_post_read_odesc,
                          r1, TABLE_SIZE_512_BITS)

tls_dec_bsq_consume_process_done:
	nop.e
	nop
