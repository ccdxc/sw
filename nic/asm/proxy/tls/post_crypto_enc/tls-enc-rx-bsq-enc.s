/*
 * 	Implements the reading of BSQ to pick up the completed decryption barco request
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
    .param  tls_enc_read_recq
	
tls_enc_rx_bsq_enc_process:	
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s7_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_0)
    phvwr           p.to_s6_gc_base, d.gc_base
    /* Release the descriptor from the tls cb decrypt queue */
	/*
	   DEQ_DESC(*dtlsp, dec, idesc);
	   RING_FREE(idesc, RNMDR);
	   dtlsp->dec_una.desc = HEAD_DESC(*dtlsp, dec);
	*/
    bbeq        k.tls_global_phv_flags_post_cbc_enc, 1, tls_enc_rx_bsq_enc_process_do_cbc
    nop

    /*
     * We read the descriptor at the head of the queue we've maintained in the qstate
     * CB as the barco response will be in order of our requests (Note: We will be
     * adding an additional verification of the CI from barco response using opaque
     * tag subsequently).
     * The head will be dequeued and pointed to the next-entry in subsequent stage
     * after we read the 'next-desc' field in the descriptor memory with a table
     * read below.
     */

    /* TODO: This could be moved to Stage 0 where we allocate the CI and
        setup the read of the recq slot
    */
    add             r3, r0, d.recq_ci
    sll             r3, r3, ASIC_BSQ_RING_SLOT_SIZE_SHFT
    add             r3, r3, d.{recq_base}.wx
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_enc_read_recq, r3, TABLE_SIZE_512_BITS)
    tblmincri.f     d.recq_ci, ASIC_BSQ_RING_SLOTS_SHIFT, 1
	nop.e
	nop

tls_enc_rx_bsq_enc_process_do_cbc:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tls_enc_read_recq)

	nop.e
	nop



