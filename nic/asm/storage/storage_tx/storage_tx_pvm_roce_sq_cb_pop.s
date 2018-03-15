/*****************************************************************************
 *  pvm_roce_sq_cb_pop : Check the queue state and see if there's anything to
 *                       be reclaimed by comparing cindex against the
 *                       (roce_msn % num_entries). If so increment the working
 *                       index and load the queue entry.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
// Use q_state_push d-vector as the stage 0 d-vector has space for action-pc
struct s5_tbl_pvm_roce_sq_cb_push_d d;
struct phv_ p;

%%

storage_tx_pvm_roce_sq_cb_pop_start:
   // If queue is empty, exit
   add          r2, d.roce_msn, r0
   mincr        r2, d.num_entries, r0
   seq		c1, d.w_ndx, r2
   bcf		[c1], clear_doorbell
   
   // Pop the entry from the queue. Note: The working consumer index is updated
   // in the pop operation to ensure that 2 consumers don't pop the same entry.
   // The update of the consumer index happens via DMA write to c_ndx only after 
   // the popped entry has been fully consumed in subsequent stages. Also, the
   // w_ndx to be used is saved in GPR r6 for use later as the tblmincr alters 
   // the d-vector.
   add		r6, r0, d.w_ndx     // delay slot
   QUEUE_POP(d.w_ndx, d.num_entries)

   // Store fields needed in the K+I vector into the PHV
   phvwr	p.storage_kivec0_w_ndx, d.w_ndx
   phvwr	p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr}, \
                d.{rrq_lif...rrq_qaddr}
   
   phvwrpair	p.storage_kivec1_src_lif, STAGE0_KIVEC_LIF, \
        	p.storage_kivec1_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair    p.storage_kivec1_src_qid, STAGE0_KIVEC_QID, \
   	        p.storage_kivec1_src_qaddr, STAGE0_KIVEC_QADDR
   
   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   LOAD_TABLE_FOR_INDEX(d.base_addr, r6, d.entry_size, d.entry_size[2:0],
                        d.next_pc)

clear_doorbell:
   QUEUE_EMPTY(d.c_ndx, d.w_ndx, drop_n_exit)

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_CLEAR_RESET

   // Setup the start and end DMA pointers to the doorbell pop
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   b 		exit
   nop

drop_n_exit:
   phvwr	p.p4_intr_global_drop, 1

exit:
   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES
