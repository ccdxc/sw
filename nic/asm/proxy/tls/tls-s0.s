/*
 * 	Implements the stage0 of tls for pre or post barco
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "capri-macros.h"
	
/* d is the data returned by lookup result */
struct d_struct {
	CAPRI_QSTATE_HEADER_COMMON

	CAPRI_QSTATE_HEADER_RING(0)

	CAPRI_QSTATE_HEADER_RING(1)

	CAPRI_QSTATE_HEADER_RING(2)

	CAPRI_QSTATE_HEADER_RING(3)

	CAPRI_QSTATE_HEADER_RING(4)	

	TLS_SHARED_STATE
};

/* lif,qtype, qid */
struct k_struct {
	lif		: LIF_WIDTH ;
	qtype           : QTYPE_WIDTH ;
	qid             : QID_WIDTH   ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	.align
tls_s0_process_start:
#if 0
	.brbegin
	brpri		r7[4:0], [0,1,2,3,4]
	nop
	.brcase 0
	b tls_post_barco_process
	nop
	.brcase 1
	b tls_pre_barco_process
	nop
	.brcase 2
	b tls_timer_process
	nop
	.brcase 3
	b tls_pending_process
	nop
	.brcase 4
	b tls_arm_process
	nop
	.brcase 5
	nop
	nop.e
	.brend
tls_post_barco_process:
tls_pre_barco_process:
tls_timer_process:
tls_pending_process:
tls_arm_process:
#endif
	nop
	nop.e
