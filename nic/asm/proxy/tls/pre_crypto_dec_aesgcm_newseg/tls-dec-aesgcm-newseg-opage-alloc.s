/*
 *	Implements the tnmdr ring page read of the TxDMA P4+ tls  pipeline
 *  Stage 5, Table 2
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.opage_alloc_d.##field}
#define K(field)    k.{##field}

/* Readonly Parsed packet header info for the current packet */
struct tx_table_s5_t2_k k;
struct phv_             p;
struct tx_table_s5_t2_d d;

%%
	
	    .align
tls_dec_aesgcm_read_tnmpr_opage_entry:

        CAPRI_CLEAR_TABLE2_VALID

	    phvwr		p.to_s7_opage, D(desc)
	    nop.e
	    nop

