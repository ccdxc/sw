/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
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
struct tx_table_s2_t1_tls_read_tnmdr_d d ;

%%
	
        .param          tls_mac_tdesc_alloc_process
        .param          TNMDPR_BIG_TABLE_BASE
	    .align
tls_mac_alloc_tnmdr_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_1)
        CAPRI_CLEAR_TABLE1_VALID

table_read_TNMDR_DESC:
        addui		r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
        addi		r3, r3, loword(TNMDPR_BIG_TABLE_BASE)
        CAPRI_NEXT_TABLE_READ_INDEX(1, d.{tnmdr_pidx}.wx, TABLE_LOCK_EN,
                                    tls_mac_tdesc_alloc_process,
  	                            r3, TNMDR_TABLE_ENTRY_SIZE_SHFT,
	                            TABLE_SIZE_512_BITS)
	nop.e
	nop

