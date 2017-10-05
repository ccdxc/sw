/*
 *	Implements the tnmdr ring page read of the TxDMA P4+ tls  pipeline
 *  Stage 3, Table 2
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
	opage			 : DESC_ADDRESS_WIDTH ;
    pad              : 448                ;
};

/* Readonly Parsed packet header info for the current packet */

struct phv_ p;
struct d_struct d;

%%
	
	    .align
tls_enc_tpage_alloc_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_2)
        CAPRI_CLEAR_TABLE2_VALID

	    phvwr		p.to_s5_opage, d.opage
        phvwri      p.to_s6_tnmpr_alloc, 1
	    nop.e
	    nop

