/*
 *	Implements the read tnmdr alloc idx of the TxDMA P4+ tls  pipeline
 *  Stage 4, Table 1
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_tnmdr_pidx_odesc_d.##field}
#define K(field)    k.{##field}
                
struct tx_table_s4_t1_k k;
struct phv_             p;
struct tx_table_s4_t1_d d;


%%
	
        .param          tls_dec_aesgcm_read_tnmdr_odesc_entry
        .param          TNMDPR_BIG_TABLE_BASE
	    .align
tls_dec_aesgcm_read_tnmdr_odesc:

        CAPRI_CLEAR_TABLE1_VALID

table_read_TNMDR_DESC:
	    addui		r3, r0, hiword(TNMDPR_BIG_TABLE_BASE)
	    addi		r3, r3, loword(TNMDPR_BIG_TABLE_BASE)
	    CAPRI_NEXT_TABLE_READ_INDEX(1, D(tnmdr_pidx).wx, TABLE_LOCK_EN,
                               tls_dec_aesgcm_read_tnmdr_odesc_entry,
  	                           r3, TNMDR_TABLE_ENTRY_SIZE_SHFT,
	                           TABLE_SIZE_512_BITS)
	    nop.e
	    nop

