/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 *	Implements the read tnmpr alloc idx of the TxDMA P4+ tls  pipeline
 *  Stage 2, Table 2
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s2_t2_k k                  ;
struct phv_ p	;
struct tx_table_s2_t2_tls_read_tnmpr_d d ;

        
%%
	
        .param          tls_mac_tpage_alloc_process
        .param          TNMPR_TABLE_BASE
	    .align

tls_mac_alloc_tnmpr_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_2)
        CAPRI_CLEAR_TABLE2_VALID

table_read_TNMDR_PAGE:
	addi		r3, r0, TNMPR_TABLE_BASE
	CAPRI_NEXT_TABLE_READ_INDEX(2, d.{tnmpr_pidx}.wx, TABLE_LOCK_EN,
                                    tls_mac_tpage_alloc_process,
	                            r3, TNMPR_TABLE_ENTRY_SIZE_SHFT,
	                            TABLE_SIZE_512_BITS)
        nop.e
        nop

