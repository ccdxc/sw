/*
 *	Implements the read tnmpr free idx of the TxDMA P4+ tls  pipeline
 *  Also writes the freed object into the object ring slot
 *  Stage 4, Table 2
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

                
struct tx_table_s4_t2_k k                  ;
struct phv_ p	;
struct tx_table_s4_t2_read_rnmpr_free_pi_d d ;

%%
	
        .param          RNMPR_TABLE_BASE
	    .align
tls_enc_free_rnmpr:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s7_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_2)
        CAPRI_CLEAR_TABLE2_VALID

	    addui		r3, r0, hiword(RNMPR_TABLE_BASE)
	    addi		r3, r0, loword(RNMPR_TABLE_BASE)

        add         r4, d.rnmpr_free_pi, r0
        sll         r4, r4, RNMPR_TABLE_ENTRY_SIZE_SHFT
        add         r3, r3, r4
        memwr.wx    r3, k.{to_s4_ipage}.wx

        /* TBD memwr A.CI */
        phvwri      p.to_s7_rnmpr_free, 1
	    nop.e
	    nop

