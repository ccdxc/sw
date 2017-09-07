/*
 * 	Doorbell write to clear the sched bit for the SESQ having
 *      finished the consumption processing.
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_sesq_consume_k k;
	
%%
        .align

tcp_tx_sesq_consume_stage2_start:
        CAPRI_CLEAR_TABLE_VALID(1)
        /* address will be in r4 */
        CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
        /* data will be in r3 */
        add             r3, k.to_s2_sesq_cidx, 1
        CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid, TCP_SCHED_RING_SESQ, r3)
        memwr.dx        r4, r3
        nop.e
        nop
