/*****************************************************************************
 *  pop_sq : Increment the consumer index to pop the entry and load
 *           the queue entry for the next stage to process. If there is 
 *           nothing to be popped, clear the doorbell (as needed).
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl0_k k;
// Use pop_sq d-vector as the stage 0 d-vector has space for action-pc
struct s2_tbl0_pop_sq_d d;
struct phv_ p;

%%
   .param storage_nvme_handle_cmd_start

storage_nvme_pop_sq_start:

   // Store fields needed in the K+I vector into the PHV
   phvwr	p.{nvme_kivec_arm_dst6_arm_lif...nvme_kivec_arm_dst6_arm_qaddr}, \
                d.{arm_lif...arm_qaddr}
   phvwr	p.{nvme_kivec_arm_dst7_arm_lif...nvme_kivec_arm_dst7_arm_qaddr}, \
                d.{arm_lif...arm_qaddr}

   // Is oper_status is a non starter
   seq		c1, NVME_KIVEC_GLOBAL_OPER_STATUS, IO_CTX_OPER_STATUS_NON_STARTER
   // Is queue empty
   seq		c2, d.p_ndx, d.w_ndx

   // Oper_status is good and queue not empty => process SQ entry
   bcf		![c1 & c2], process_sq_entry
   
   // Has doorbell been cleared (delay slot)
   seq		c3, d.c_ndx, d.w_ndx 

   // Not of c1,c3 for use in branch instructions
   sne		c4, NVME_KIVEC_GLOBAL_OPER_STATUS, IO_CTX_OPER_STATUS_NON_STARTER
   sne		c5, d.c_ndx, d.w_ndx

   // if c1 && c5			clear_doorbell; load no tables
   // if c4 && c2 && c5			clear_doorbell; setup arm xfer of unused allocated iob 
   // if c4 && c2 && c3			setup arm xfer of unused allocated iob 
   // Catchall				drop PHV; load no tables

   bcf		[c1 & c5], clear_doorbell_n_exit
   nop
   bcf		[c4 & c2 & c5], clear_doorbell_n_iob_xfer
   nop
   bcf		[c4 & c2 & c3], iob_xfer
   nop

   // Set PHV to drop
   phvwr	p.p4_intr_global_drop, 1

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES

clear_doorbell_n_exit:
   // Update the queue doorbell to clear the scheduler bit
   NVME_QUEUE_POP_DOORBELL_CLEAR

   // Setup the start and end DMA pointers to the doorbell pop
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES

clear_doorbell_n_iob_xfer:
   // Update the queue doorbell to clear the scheduler bit
   NVME_QUEUE_POP_DOORBELL_CLEAR

iob_xfer:
   // TODO: FIXME. Don't setup DMA. Load tables for skipping 
   //              until stage 7 for pushing IOB to ARM
   // Setup the start and end DMA pointers to the doorbell pop
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES

process_sq_entry:
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
   
   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   LOAD_TABLE_FOR_INDEX_PARAM(d.base_addr, r6, d.entry_size, d.entry_size[2:0],
                              storage_nvme_handle_cmd_start)

