/*
 *	Implements the tnmdr ring desc of the TxDMA P4+ tls  pipeline
 * Stage 3, Table 1
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
	odesc			 : DESC_ADDRESS_WIDTH ;
    pad              : 448               ;
};

/* Readonly Parsed packet header info for the current packet */
struct tx_table_s3_t1_k                 k;

struct phv_ p;
struct d_struct d;

%%
	
        .param          TNMDR_TABLE_BASE
	    .align
tls_dec_tdesc_alloc_process:

        CAPRI_CLEAR_TABLE1_VALID
	    phvwrpair   p.to_s5_odesc, d.odesc[31:0], \
	                p.to_s6_odesc, d.odesc[31:0]
        phvwri      p.to_s7_tnmdpr_alloc, 1
        
        add         r2, d.odesc, ASIC_NMDPR_PAGE_OFFSET
	    phvwrpair   p.to_s5_opage, r2,  \
	                p.to_s6_opage, r2

	    nop.e
	    nop

