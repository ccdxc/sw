/*****************************************************************************
 *  Stage: Check SSD CQ context. If busy bit set, yield control. Else
 *         set the busy bit, save SQ context to PHV and load CQ entry.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_cq_ctx_k k;
struct ssd_cq_ctx_qcheck_d d;
struct phv_ p;

%%

ssd_cq_ctx_check_start:
   // Save the entire d vector to PHV
   phvwr	p.{scq_ctx_idx...scq_ctx_rsvd}, d.{idx...rsvd}                                           
   // If queue context is locked (state is busy), exit
   QUEUE_CTX_LOCKED(d.state, exit)

   // If queue is empty, exit
   QUEUE_EMPTY(d.p_ndx, d.c_ndx, exit)

   // Lock the queue context
   QUEUE_CTX_LOCK(d.state)

   // Set the table and program address 
   LOAD_TABLE_CBASE_IDX(d.base_addr, d.c_ndx, SSD_CQ_ENTRY_SIZE, 
                        SSD_CQ_ENTRY_SIZE, ssd_cq_entry_pop)

exit:
   nop.e
   nop
