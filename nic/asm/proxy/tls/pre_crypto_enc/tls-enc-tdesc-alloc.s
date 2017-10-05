/*
 *	Implements the tnmdr ring desc of the TxDMA P4+ tls  pipeline
 *  Stage 3, Table 1
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

struct phv_ p;
struct d_struct d;

%%
	
        .param          TNMDR_TABLE_BASE
	    .align
tls_enc_tdesc_alloc_process:
        CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_1)
        CAPRI_CLEAR_TABLE1_VALID

	    phvwr		p.to_s5_odesc, d.odesc
   	    phvwr		p.to_s4_odesc, d.odesc
        phvwri      p.to_s6_tnmdr_alloc, 1
        
	    nop.e
	    nop

