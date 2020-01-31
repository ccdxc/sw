/*
 *	Implements the read tnmpr alloc idx of the TxDMA P4+ tls  pipeline
 * Stage 2, Table 2
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s2_t2_k k;
struct phv_             p;
struct tx_table_s2_t2_d d;

        
%%
	
        .param          tls_dec_tpage_alloc_process
        .param          TNMPR_TABLE_BASE
	    .align

tls_dec_alloc_tnmpr_process:

        CAPRI_CLEAR_TABLE2_VALID

	    phvwr		p.s3_t2_s2s_tnmpr_pidx, d.{u.read_tnmpr_pidx_d.tnmpr_pidx}.wx

table_read_TNMDR_PAGE:
	    addui		r3, r0, hiword(TNMPR_TABLE_BASE)
	    addi		r3, r3, loword(TNMPR_TABLE_BASE)
	    CAPRI_NEXT_TABLE_READ_INDEX(2, d.{u.read_tnmpr_pidx_d.tnmpr_pidx}.wx, TABLE_LOCK_DIS,
                        tls_dec_tpage_alloc_process,
	                    r3, TNMPR_TABLE_ENTRY_SIZE_SHFT,
	                    TABLE_SIZE_512_BITS)
	    nop.e
	    nop

