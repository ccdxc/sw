/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * 	Doorbell write to clear the sched bit for the SERQ having
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
	

/* SERQ consumer index */
        
struct tx_table_s3_t0_k k                  ;
struct phv_ p	;
struct tx_table_s3_t0_d d;
	
%%
	.param      tls_mac_bld_barco_req_process
        
tls_mac_serq_consume_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /* SERQ_cidx got incremented due to the auto-inc read address used */
    /* address will be in r4 */
    addi    r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
    add		r1, k.tls_global_phv_fid, r0

    /*
     * data will be in r3
     *
     * We'd have incremented CI in stage 0, we'll ring the doorbell with that value and let
     * the scheduler re-evaluate if ci != pi. We can optimize to not ring the doorbell if
     * we can do the ci != pi check ourselves (in stage-0)
     */
    add     r5, d.{u.tls_serq_consume_d.ci_0}.hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_SERQ, r5)

    memwr.dx  	 r4, r3
table_read_BLD_BARCO_MAC_REQ:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_mac_bld_barco_req_process,
                           k.tls_global_phv_qstate_addr, TLS_TCB_OFFSET,
                           TABLE_SIZE_512_BITS)
    nop.e
    nop
