/*
 *	Implements the read tnmpr alloc idx of the TxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
	tnmpr_pidx			 : RING_INDEX_WIDTH ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				 : 32 ;
};

struct phv_ p;
struct k_struct k;
struct d_struct d;

%%
	
        .param          tls_tpage_alloc_start
        .param          TNMPR_TABLE_BASE
	.align
tls_alloc_tnmdr_start:

        CAPRI_CLEAR_TABLE0_VALID

	#phvwr		p.s3_t1_s2s_tnmpr_pidx, d.tnmdr_pidx

table_read_TNMDR_PAGE:
	addi		r3, r0, TNMPR_TABLE_BASE
	CAPRI_NEXT_TABLE0_READ(d.tnmpr_pidx, TABLE_LOCK_EN,
                            tls_tpage_alloc_start,
	                    r3, TNMPR_TABLE_ENTRY_SIZE_SHFT,
	                    0, TABLE_SIZE_512_BITS)
	nop.e
	nop

