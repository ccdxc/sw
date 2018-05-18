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
   .param storage_nvme_send_cmd_free_iob_start

storage_nvme_pop_sq_start:

   // Is oper_status is a non starter
   seq		c1, NVME_KIVEC_GLOBAL_OPER_STATUS, IO_CTX_OPER_STATUS_NON_STARTER
   // Has doorbell been cleared 
   seq		c2, d.c_ndx, d.w_ndx 

   // If oper_status == NON_STARTER && doorbell has been cleared
   bcf		[c1 & c2], drop_phv
   nop

   // If oper_status == NON_STARTER && doorbell has not been cleared
   bcf		[c1], clear_doorbell_n_exit
   nop

   // Does oper_status indicate IOB was not allocated ?  => drop PHV without 
   // clearing doorbell and let subsequent scheduling instance check for IOB availability
   seq		c3, NVME_KIVEC_GLOBAL_OPER_STATUS, IO_CTX_OPER_STATUS_NO_FREE_IOB
   bcf		[c3], drop_phv
   nop

   // Is queue empty (some other thread processed this entry) => free the IOB without
   // clearing doorbell and let subsequent scheduling instance handle this
   seq		c4, d.p_ndx, d.w_ndx
   bcf		[c4], iob_free
   nop

   // Continue to process the SQ entry
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
   phvwr	p.{nvme_kivec_arm_dst6_arm_lif...nvme_kivec_arm_dst6_arm_qtype}, \
                d.{arm_lif...arm_qtype}
   phvwr	p.{nvme_kivec_arm_dst7_arm_lif...nvme_kivec_arm_dst7_arm_qtype}, \
                d.{arm_lif...arm_qtype}
   add		r1, d.arm_base_qid, ARM_QID_OFFSET_SQ
   add		r2, d.arm_base_qaddr, ARM_QID_OFFSET_SQ, ARM_QSTATE_ENTRY_SIZE_LOG2
   phvwrpair	p.nvme_kivec_arm_dst6_arm_qid, r1, p.nvme_kivec_arm_dst6_arm_qaddr, r2
   phvwrpair	p.nvme_kivec_arm_dst7_arm_qid, r1, p.nvme_kivec_arm_dst7_arm_qaddr, r2
   phvwr	p.nvme_kivec_t0_s2s_w_ndx, d.w_ndx
   
   // Set table 1 to be valid. The address, PC, table load size is populated by the 
   // save_iob_addr program.
   SET_TABLE1

   // Set table 0 and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   // This MUST be the last table config in this path.
   LOAD_TABLE_FOR_INDEX_PARAM(d.base_addr, r6, d.entry_size, d.entry_size[2:0],
                              storage_nvme_handle_cmd_start)

drop_phv:
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

clear_doorbell_n_iob_free:
   // Update the queue doorbell to clear the scheduler bit
   NVME_QUEUE_POP_DOORBELL_CLEAR

iob_free:
   // Store fields needed in the K+I vector into the PHV
   phvwr	p.{nvme_kivec_t0_s2s_dst_lif...nvme_kivec_t0_s2s_dst_qtype}, \
                d.{arm_lif...arm_qtype}
   phvwr	p.{nvme_kivec_t0_s2s_dst_lif...nvme_kivec_t0_s2s_dst_qtype}, \
                d.{arm_lif...arm_qtype}
   add		r1, d.arm_base_qid, ARM_QID_OFFSET_CMD_FREE_IOB_Q
   add		r2, d.arm_base_qaddr, ARM_QID_OFFSET_CMD_FREE_IOB_Q, ARM_QSTATE_ENTRY_SIZE_LOG2
   phvwrpair	p.nvme_kivec_t0_s2s_dst_qid, r1, p.nvme_kivec_t0_s2s_dst_qaddr, r2
   phvwrpair	p.nvme_kivec_t0_s2s_dst_qid, r1, p.nvme_kivec_t0_s2s_dst_qaddr, r2
   phvwr	p.nvme_kivec_t0_s2s_w_ndx, d.w_ndx

   // Table 1 was set in the previous stage, clear it
   CLEAR_TABLE1

   // Set table 0 and program address for the next stage to read the ARM queue
   // state entry to send a posted free of the IOB
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r2, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_send_cmd_free_iob_start)



