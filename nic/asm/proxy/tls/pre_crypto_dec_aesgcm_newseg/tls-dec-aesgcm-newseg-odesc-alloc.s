/*
 *	Implements the tnmdr ring desc of the TxDMA P4+ tls  pipeline
 *  Stage 5, Table 1
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.odesc_alloc_d.##field}
#define K(field)    k.{##field}

struct tx_table_s5_t1_k k;
struct phv_             p;
struct tx_table_s5_t1_d d;

%%
	
	    .align
tls_dec_aesgcm_read_tnmdr_odesc_entry:

        CAPRI_CLEAR_TABLE1_VALID

	    phvwr		p.to_s7_odesc, D(desc)
        
	    nop.e
	    nop

