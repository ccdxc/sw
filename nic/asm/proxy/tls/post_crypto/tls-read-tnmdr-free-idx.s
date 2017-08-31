/*
 *	Implements the read tnmdr free idx of the TxDMA P4+ tls  pipeline
 *  Also writes the freed object into the object ring slot
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

                
struct tx_table_s3_t1_k k                  ;
struct phv_ p	;
struct tx_table_s3_t1_read_tnmdr_free_pi_d d ;

%%
	
        .param          TNMDR_TABLE_BASE
	    .align
tls_free_tnmdr:

        CAPRI_CLEAR_TABLE1_VALID

	    addi		r3, r0, TNMDR_TABLE_BASE

        add         r4, d.tnmdr_free_pi, r0
        sll         r4, r4, TNMDR_TABLE_ENTRY_SIZE_SHFT
        add         r3, r3, r4
        memwr.wx    r3, k.{to_s3_idesc}.wx

        /* TBD memwr A.CI */
	    nop.e
	    nop

