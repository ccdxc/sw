/*
 *	Implements the tnmdr ring page read of the TxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
	opage			 : ADDRESS_WIDTH ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	qstate_addr			: ADDRESS_WIDTH ;
};

struct phv_ p;
struct k_struct k;
struct d_struct d;

%%
	
	.align
tls_tpage_alloc_start:

        CAPRI_CLEAR_TABLE1_VALID

	#phvwr		p.opage, d.opage

	nop.e
	nop

