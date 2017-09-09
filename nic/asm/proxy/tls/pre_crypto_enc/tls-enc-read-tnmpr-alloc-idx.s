/*
 *	Implements the read tnmpr alloc idx of the TxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s2_t2_k k                  ;
struct phv_ p	;
struct tx_table_s2_t2_read_tnmpr_d d ;

        
%%
	
        .param          tls_enc_tpage_alloc_process
        .param          TNMPR_TABLE_BASE
	    .align

tls_enc_alloc_tnmpr_process:

        CAPRI_CLEAR_TABLE2_VALID

	    phvwr		p.s3_t2_s2s_tnmpr_pidx, d.tnmpr_pidx

table_read_TNMDR_PAGE:
	    addi		r3, r0, TNMPR_TABLE_BASE
	    CAPRI_NEXT_TABLE2_READ(d.tnmpr_pidx, TABLE_LOCK_EN,
                            tls_enc_tpage_alloc_process,
	                    r3, TNMPR_TABLE_ENTRY_SIZE_SHFT,
	                    0, TABLE_SIZE_512_BITS)
	    nop.e
	    nop

