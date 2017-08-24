/*****************************************************************************
 *  q_state_pop : Check the queue state and see if there's anything to be
 *                popped. If so increment the working index and load the
 *                queue entry.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
struct s0_tbl_q_state_pop_d d;
struct phv_ p;

%%

storage_tx_q_state_pop_start:
   // If queue is empty, exit
   QUEUE_EMPTY(d.p_ndx, d.c_ndx, exit)

   // Pop the entry from the queue. Note: The working consumer index is updated
   // in the pop operation to ensure that two consumers don't pop the same entry.
   // The update of the consumer index happens via DMA write to c_ndx only after 
   // the popped entry has been fully consumed in subsequent stages. Also, the
   // w_ndx to be used is saved in GPR r6 for use as the tblmincr alters the
   // d-vector.
   add		r6, r0, d.w_ndx
   QUEUE_POP(d.w_ndx, d.num_entries)

   // Store fields needed in the K+I vector into the PHV
   phvwr	p.storage_kivec0_w_ndx, d.w_ndx
   phvwr	p.storage_kivec0_dst_qaddr, d.dst_qaddr
   phvwr	p.storage_kivec0_dst_lif, d.dst_lif
   phvwr	p.storage_kivec0_dst_qtype, d.dst_qtype
   phvwr	p.storage_kivec0_dst_qid, d.dst_qid
   // TODO: FIXME, derive these from the K+I for stage 0
   phvwr	p.storage_kivec0_is_q0, 0
   phvwr	p.storage_kivec0_src_qaddr, 0
   
   // Initialize the vf_id and sq_id fields in the PHV
   phvwr	p.pvm_cmd_vf_id, d.vf_id
   phvwr	p.pvm_cmd_sq_id, d.sq_id
   
   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   LOAD_TABLE_FOR_INDEX(d.base_addr, r6, d.entry_size, 6,
                        d.next_pc)
exit:
   nop.e
   nop
