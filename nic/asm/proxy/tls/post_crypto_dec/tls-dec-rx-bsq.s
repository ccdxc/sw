/*
 * 	Implements the reading of procesed head entry to pick up the completed decryption barco request
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct tx_table_s2_t0_k k       ;
struct phv_ p	;
struct tx_table_s2_t0_tls_rx_bsq_d d	;
	
%%
	.param		tls_dec_read_recq

tls_dec_rx_bsq_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.stats_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_0)
	/* Release the descriptor from the tls cb decrypt queue */
    /*
	   DEQ_DESC(*dtlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   dtlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
    phvwri  p.stats_dec_completions, 1
    phvwr   p.to_s6_gc_base, d.gc_base

    /* Queue empty */
    seq     c1, d.recq_pi, d.recq_ci
	bcf		[c1], tls_rx_bsq_dec_process_done
	nop

    add             r3, r0, d.recq_ci
    tblmincri.f     d.recq_ci, ASIC_BSQ_RING_SLOTS_SHIFT, 1
    sll             r3, r3, ASIC_BSQ_RING_SLOT_SIZE_SHFT
    add             r3, r3, d.{recq_base}.wx
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_recq, r3, TABLE_SIZE_512_BITS)

tls_rx_bsq_dec_process_done:
	nop.e
	nop
