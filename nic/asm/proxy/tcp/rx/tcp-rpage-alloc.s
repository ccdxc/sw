/*
 *	Implements the page allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	page				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	RNMPR_alloc_idx			: RING_INDEX_WIDTH ;
	write_serq                      : 1		   ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
%%
	
flow_rpage_alloc_process_start:
	phvwr.e		p.page, d.page
	nop
