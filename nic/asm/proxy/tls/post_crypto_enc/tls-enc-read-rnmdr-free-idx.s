/*
 *	Implements the read tnmdr free idx of the TxDMA P4+ tls  pipeline
 *  Also writes the freed object into the object ring slot
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

                
struct tx_table_s3_t1_k k                  ;
struct phv_ p	;
struct tx_table_s3_t1_read_rnmdr_free_pi_d d ;

%%
	
        .param          RNMDR_TABLE_BASE
	    .align
tls_enc_free_rnmdr:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s7_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_1)
        CAPRI_CLEAR_TABLE1_VALID

	    addui		r3, r0, hiword(RNMDR_TABLE_BASE)
	    addi		r3, r0, loword(RNMDR_TABLE_BASE)

        add         r4, d.rnmdr_free_pi, r0
        sll         r4, r4, RNMDR_TABLE_ENTRY_SIZE_SHFT
        add         r3, r3, r4
        memwr.wx    r3, k.{to_s3_idesc}.wx

        /* TBD memwr A.CI */
        phvwri      p.to_s7_rnmdr_free, 1
	    nop.e
	    nop

