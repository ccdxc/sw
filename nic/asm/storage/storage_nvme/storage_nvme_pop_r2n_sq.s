/*****************************************************************************
 *  pop_r2n_sq: Pop the R2N SQ to dequeue the R2N WQE entry which contains a
 *              pointer to the status buffer.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl0_k k;
// Use push_dst_seq_q d-vector as the stage 0 d-vector has space for action-pc
struct s7_tbl0_push_dst_seq_q_d d;
struct phv_ p;

%%
   .param storage_nvme_handle_r2n_wqe_start

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
                
   phvwrpair	p.nvme_kivec_global_src_lif, STAGE0_KIVEC_LIF,			\
        	p.nvme_kivec_global_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair	p.nvme_kivec_global_src_qid, STAGE0_KIVEC_QID,			\
   	        p.nvme_kivec_global_src_qaddr, STAGE0_KIVEC_QADDR

   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   LOAD_TABLE_FOR_INDEX_PARAM(d.base_addr, r6, d.entry_size, d.entry_size[2:0],
                              storage_nvme_handle_r2n_wqe_start)

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

