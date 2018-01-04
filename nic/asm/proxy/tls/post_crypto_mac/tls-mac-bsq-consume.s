/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Doorbell write to clear the sched bit for the BSQ having
 *      finished the consumption processing.
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s2_t0_k k                  ;
struct phv_ p	;
struct tx_table_s2_t0_tls_bsq_consume_d d;
	
%%
	.param      tls_mac_bld_barco_enc_req_process
        
tls_mac_bsq_consume_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add		r1, k.tls_global_phv_fid, r0

    /*
     * data will be in r3
     *
     * We'd have incremented CI for the BSQ-2PASS ring in stage 0, we'll ring the doorbell
     * with that value and let the scheduler re-evaluate if ci != pi. We can optimize to
     * not ring the doorbell if we can do the ci != pi check ourselves (in stage-0)
     */
    add         r5, d.{ci_2}.hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_BSQ_2PASS, r5)

    memwr.dx  	r4, r3

    /* For now, if we have a Barco Op error, bail out right here */
    sne         c1, r0, k.tls_global_phv_barco_op_failed
    bcf         [c1], tls_mac_bsq_consume_process_done
    nop
table_read_QUEUE_ENC_BRQ:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_mac_bld_barco_enc_req_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)
tls_mac_bsq_consume_process_done:
	nop.e
	nop
