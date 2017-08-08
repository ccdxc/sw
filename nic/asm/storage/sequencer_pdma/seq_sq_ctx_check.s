/*****************************************************************************
 *  Stage: Check Sequencer SQ context. If busy bit set, yield control. Else
 *         set the busy bit, save SQ context to PHV and load the SQ entry.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct seq_sq_ctx_k k;
struct seq_sq_ctx_qcheck_d d;
struct phv_ p;

%%

seq_sq_ctx_check_start:
   // Save the entire d vector to PHV
   phvwr	p.{seq_sq_ctx_idx...seq_sq_ctx_rsvd}, d.{idx...rsvd}                                           
   // If queue context is locked (state is busy), exit
   QUEUE_CTX_LOCKED(d.state, exit)

   // If queue is empty, exit
   QUEUE_EMPTY(d.p_ndx, d.c_ndx, exit)

   // Lock the queue context
   QUEUE_CTX_LOCK(d.state)

   // Set the table and program address 
   LOAD_TABLE_CBASE_IDX(d.base_addr, d.c_ndx, SEQ_SQ_ENTRY_SIZE, 
                        SEQ_SQ_ENTRY_SIZE, seq_sq_entry_pop)
exit:
   nop.e
   nop
