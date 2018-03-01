/*
 * 	Implements the dequeue of the descriptor queue head and point to next descriptor.
 *  Stage 4, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	

struct tx_table_s4_t3_k k       ;
struct phv_ p	;
struct tx_table_s4_t3_tls_update_desc_q_d d	;
	
%%
	
tls_enc_update_desc_q_process:	
    CAPRI_CLEAR_TABLE3_VALID
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_3)

    /*
     * If the head we dequeued was the last entry, we'll set the tail to NULL too, as the
     * pre-crypto stage which enqueues the descriptors to this queue checks the tail.
     */
    tblwr     d.qhead, k.to_s4_next_idesc
    seq       c1, d.qhead, r0
    tblwr.c1  d.qtail, r0

    nop.e
    nop
