/*
 * 	Implements the reading of BSQ to pick up the completed decryption barco request
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct tx_table_s1_t0_k k       ;
struct phv_ p	;
struct tx_table_s1_t0_tls_rx_bsq_d d	;
	
%%
	.param		tls_read_desc_process
	
tls_rx_bsq_enc_process:	
	/* Release the descriptor from the tls cb decrypt queue */
	/*
	   DEQ_DESC(*dtlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   dtlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
	seq		c1, d.qhead, r0
	bcf		[c1], tls_rx_bsq_enc_process_done
	nop
table_read_desc:	
	add		r3, r0, d.qhead
    CAPRI_NEXT_IDX0_READ(TABLE_LOCK_DIS, tls_read_desc_process, r3, TABLE_SIZE_512_BITS)


tls_rx_bsq_enc_process_done:
	nop.e
	nop.e
