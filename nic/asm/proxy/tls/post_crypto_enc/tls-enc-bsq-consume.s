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

    /* TODO: This stage is not needed anymore, cleanup */

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_post_read_odesc,
                       	  k.to_s5_odesc, TABLE_SIZE_512_BITS)
tls_enc_bsq_consume_process_done:
	nop.e
	nop
