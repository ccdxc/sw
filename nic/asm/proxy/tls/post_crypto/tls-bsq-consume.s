/*
 * 	Doorbell write to clear the sched bit for the BSQ having
 *      finished the consumption processing.
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

        
struct tx_table_s3_t0_k k                  ;
struct phv_ p	;
struct tx_table_s3_t0_tls_bsq_consume_d d;
	
%%
	.param      tls_queue_sesq_process
        
tls_bsq_consume_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
	add		r1, k.tls_global_phv_fid, r0
	/* data will be in r3 */
    add     r3, d.{pi_1}, r0
	CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_BSQ, r3)

	memwr.dx  	 r4, r3
table_read_QUEUE_SESQ:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_queue_sesq_process,
                           k.tls_global_phv_qstate_addr,
                       	   TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
	nop
