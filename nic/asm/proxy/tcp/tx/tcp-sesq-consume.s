/*
 * 	Doorbell write to clear the sched bit for the SESQ having
 *      finished the consumption processing.
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
/* d is the data returned by lookup result */
struct d_struct {
        SESQ_cidx                        : 16;
};

/* SERQ consumer index */
struct k_struct {
	sesq_ci				: RING_INDEX_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_sesq_consume_process_start:
	/* SESQ_cidx got incremented due to the auto-inc read address used */
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_GLOBALQ)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, SESQ_QID, 0, d.SESQ_cidx)
	memwr.d		r4, r3
	nop.e
	nop
