/*
 *	Implements the read tnmdr alloc idx of the TxDMA P4+ tls  pipeline
 *  Stage 2, Table 1
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

                
struct tx_table_s2_t1_k k                  ;
struct phv_ p	;
struct tx_table_s2_t1_read_tnmdr_d d ;

%%
	
        .param          tls_enc_tdesc_alloc_process
        .param          TNMDR_TABLE_BASE
	    .align
tls_enc_alloc_tnmdr_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_1)
        CAPRI_CLEAR_TABLE1_VALID

	    phvwr		p.s3_t1_s2s_tnmdr_pidx, d.{tnmdr_pidx}.wx

table_read_TNMDR_DESC:
	    addui		r3, r0, hiword(TNMDR_TABLE_BASE)
	    addi		r3, r0, loword(TNMDR_TABLE_BASE)
	    CAPRI_NEXT_TABLE_READ_INDEX(1, d.{tnmdr_pidx}.wx, TABLE_LOCK_EN,
                               tls_enc_tdesc_alloc_process,
  	                           r3, TNMDR_TABLE_ENTRY_SIZE_SHFT,
	                           TABLE_SIZE_512_BITS)
	    nop.e
	    nop

