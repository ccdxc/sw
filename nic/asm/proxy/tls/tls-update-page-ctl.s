/*
 *	Implements the desc alloc stage of the RxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	flags				 : 12 ;
	size				 : 4  ;
	refcnt				 : 8  ;
	rsvd				 : 8  ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	page_addr                        : ADDRESS_WIDTH ;

	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;
	enc_flow                         : 1		 ;
	
};
struct p_struct p;
struct k_struct k;
struct d_struct d;

%%
	
tls_update_page_ctl_process_start:
	phvwr		p.pending_rx_serq, k.pending_rx_serq
	phvwr		p.pending_rx_brq, k.pending_rx_brq
	phvwr		p.enc_flow, k.enc_flow
	
	tbladd		d.refcnt, 1

table_read_alloc_rnmdr:
	TLS_READ_IDX(RNMDR_ALLOC_IDX, TABLE_TYPE_RAW, tls_alloc_rnmdr_process)

tls_update_page_ctl_process_done:
	nop.e
	nop
