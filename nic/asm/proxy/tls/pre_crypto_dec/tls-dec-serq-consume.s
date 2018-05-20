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
        
tls_dec_serq_consume_process:
    CAPRI_CLEAR_TABLE3_VALID

    /* FIXME: Nothing to be done here, dbell moved to Stage 0 */

tls_dec_serq_consume_done:	
    nop.e
    nop
