/*
 * 	Implements the reading of BRQ to pick up the completed barco request
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"


/* d is the data returned by lookup result */
struct d_struct {
	TLS_SHARED_STATE
};
/* k is the key fed in for the lookup */
struct k_struct {
	fid				: 32 ;
	odesc                           : ADDRESS_WIDTH ;


	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;
	pending_queue_brq		 : 1		 ;
	enc_flow                         : 1		 ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	.param		tls_read_desc_process
	
tls_rx_brq_process_start:
	phvwr		p.pending_rx_serq, k.pending_rx_serq
	phvwr		p.pending_rx_brq, k.pending_rx_brq
	phvwr		p.enc_flow, k.enc_flow
	phvwr		p.odesc, k.odesc

	/*   if (IS_ENC_FLOW) { */
	sne		c1, r0, k.enc_flow
	bcf		[c1], tls_rx_brq_enc_process
	nop
tls_rx_brq_enc_process:
	/* Release the descriptor from the tls cb encrypt queue */
	/*
	   DEQ_DESC(*etlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   etlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
	seq		c1, d.qhead, r0
	bcf		[c1], tls_rx_brq_process_done
	nop
	b		table_read_desc
	nop
tls_rx_brq_dec_process:	
	/* Release the descriptor from the tls cb decrypt queue */
	/*
	   DEQ_DESC(*dtlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   dtlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
	seq		c1, d.qhead, r0
	bcf		[c1], tls_rx_brq_process_done
	nop
table_read_desc:	
	add		r2, r0, d.qhead
	phvwri		p.pending_rx_brq, 1
	phvwr		p.fid, k.fid
	phvwr		p.odesc, k.odesc
	TLS_READ_ADDR(r2, TABLE_TYPE_RAW, tls_read_desc_process)


tls_rx_brq_process_done:
	nop.e
	nop.e
