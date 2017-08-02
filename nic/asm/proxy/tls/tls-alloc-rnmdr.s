/*
 *	Implements the desc alloc stage of the RxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	odesc				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				 : 32 ;
	RNMDR_alloc_idx			 : RING_INDEX_WIDTH ;
	write_serq			 : 1		   ;
	desc				 : ADDRESS_WIDTH	   ;

	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;
	pending_queue_brq		 : 1		 ;
	enc_flow                         : 1		 ;

	desc_aol0_addr                   : 30 ;
	desc_aol0_offset                 : 16 ;
	desc_aol0_len                    : 16 ;

};

struct p_struct p;
struct k_struct k;
struct d_struct d;

%%
	
tls_alloc_rnmdr_process_start:
	phvwr		p.pending_rx_serq, k.pending_rx_serq
	phvwr		p.pending_rx_brq, k.pending_rx_brq
	phvwr		p.enc_flow, k.enc_flow
	
	phvwr		p.odesc, d.odesc

	seq		c1, k.pending_queue_brq, r0
	sne		c2, k.pending_rx_brq, r0

	bcf		[c1|c2], tls_alloc_rnmdr_process_done
	nop

table_read_alloc_rnmpr:
	TLS_READ_IDX(RNMPR_ALLOC_IDX, TABLE_TYPE_RAW, tls_alloc_rnmpr_process)

tls_alloc_rnmdr_process_done:
	nop.e
	nop
