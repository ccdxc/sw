/*
 * 	Doorbell write to clear the sched bit for the SERQ having
 *      finished the consumption processing.
 * Stage 2, Table 3
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
        
struct tx_table_s2_t3_k k;
struct phv_             p;
struct tx_table_s2_t3_d d;

#define D   d.u.tls_serq_consume_d
	
%%
	.param      tls_dec_bld_barco_req_process
        
tls_dec_serq_consume_process:
    CAPRI_CLEAR_TABLE3_VALID
	/* SERQ_cidx got incremented due to the auto-inc read address used */
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TLS)
	add		r1, k.tls_global_phv_fid, r0
	/* data will be in r3 */
    add     r5, d.{u.tls_serq_consume_d.pi_0}.hx, r0
	CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_SERQ, r5)

	memwr.dx  	 r4, r3

	nop.e
	nop
