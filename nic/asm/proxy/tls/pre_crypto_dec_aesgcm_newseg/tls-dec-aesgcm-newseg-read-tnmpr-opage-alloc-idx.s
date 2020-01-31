/*
 *	Implements the read tnmpr alloc idx of the TxDMA P4+ tls  pipeline
 *  Stage 4, Table 2
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_tnmpr_pidx_opage_d.##field}
#define K(field)    k.{##field}

struct tx_table_s4_t2_k k;
struct phv_             p;
struct tx_table_s4_t2_d d;

        
%%
	
        .param          tls_dec_aesgcm_read_tnmpr_opage_entry
        .param          TNMDPR_BIG_TABLE_BASE
	    .align

tls_dec_aesgcm_read_tnmpr_opage:

        CAPRI_CLEAR_TABLE2_VALID


table_read_TNMDR_PAGE:
	    addui		r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
	    addi		r3, r3, loword(TNMDPR_BIG_TABLE_BASE)
	    CAPRI_NEXT_TABLE_READ_INDEX(2, D(tnmpr_pidx).wx, TABLE_LOCK_EN,
                        tls_dec_aesgcm_read_tnmpr_opage_entry,
	                    r3, TNMPR_TABLE_ENTRY_SIZE_SHFT,
	                    TABLE_SIZE_512_BITS)
	    nop.e
	    nop

