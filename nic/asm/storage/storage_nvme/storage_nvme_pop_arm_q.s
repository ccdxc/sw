/*****************************************************************************
 *  pop_arm_q: Pop the ARM queue to see if there is some activity queued by
 *             ARM or NVME command/status handling P4+ programs. This is a
 *             common pop function with next_pc determining the next action.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl0_k k;
// Use push_dst_seq_q d-vector as the stage 0 d-vector has space for action-pc
struct s7_tbl0_push_arm_q_d d;
struct phv_ p;

%%

storage_nvme_pop_r2n_sq_start:
   // If queue is empty, exit
   QUEUE_EMPTY(d.p_ndx, d.w_ndx, clear_doorbell)

   // Pop the entry from the queue. Note: The working consumer index is updated
   // in the pop operation to ensure that 2 consumers don't pop the same entry.
   // The update of the consumer index happens via DMA write to c_ndx only after 
   // the popped entry has been fully consumed in subsequent stages. Also, the
   // w_ndx to be used is saved in GPR r6 for use later as the tblmincr alters 
   // the d-vector.
   add		r6, r0, d.w_ndx
   QUEUE_POP(d.w_ndx, d.num_entries)

   // Store fields needed in the K+I vector into the PHV
   phvwr	p.nvme_kivec_t0_s2s_w_ndx, d.w_ndx
   
   phvwr	p.{nvme_kivec_t0_s2s_dst_lif...nvme_kivec_t0_s2s_dst_qaddr},	\
                d.{dst_lif...dst_qaddr}

   phvwr	p.nvme_kivec_iob_ring3_base_addr, d.iob_ring_base_addr
   phvwr	p.nvme_kivec_iob_ring4_base_addr, d.iob_ring_base_addr
                
   phvwrpair	p.nvme_kivec_global_src_lif, STAGE0_KIVEC_LIF,			\
        	p.nvme_kivec_global_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair	p.nvme_kivec_global_src_qid, STAGE0_KIVEC_QID,			\
   	        p.nvme_kivec_global_src_qaddr, STAGE0_KIVEC_QADDR

   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   LOAD_TABLE_FOR_INDEX(d.base_addr, r6, d.entry_size, d.entry_size[2:0],
                        d.next_pc)

clear_doorbell:
   QUEUE_EMPTY(d.c_ndx, d.w_ndx, drop_n_exit)

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_CLEAR

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

