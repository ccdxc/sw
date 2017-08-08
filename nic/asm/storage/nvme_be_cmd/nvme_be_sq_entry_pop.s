/*****************************************************************************
 *  Stage: Save second part of the NVME command in SQ entry to PHV. Increment
 *         consumer index in NVME backend SQ context to pop the entry. Load the
 *         address of SSD's list of outstanding commands for the next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_sq_entry2_k k;
struct nvme_be_sq_entry2_qpop_d d;
struct phv_ p;

%%
   .param	ssd_cmd_save_start
nvme_be_sq_entry_pop_start:
   // Save the entire d vector to PHV
   phvwr	p.{nvme_be_cmd_p_nvme_cmd_w0...nvme_be_cmd_p_nvme_cmd_hi},\
		d.{nvme_cmd_w0...nvme_cmd_hi}

   // Save the command id to the K+I vector
   phvwr	p.nvme_tgt_kivec0_cmd_index, d.nvme_cmd_w0
  
   // Pop the SQ entry from the high priority queue (if set)
   PRI_QUEUE_POP(k.nvme_tgt_kivec0_io_priority, NVME_BE_PRIORITY_HI,
                 k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
                 k.nvme_tgt_kivec0_idx, 
                 NVME_BE_SQ_CTX_TABLE_BASE, PRI_Q_CTX_STATE_OFFSET, 
                 PRI_Q_CTX_C_NDX_HI_OFFSET, pop_med)

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_CMDS_TABLE_BASE, k.nvme_tgt_kivec0_ssd_handle,
                        SSD_CMDS_ENTRY_SIZE, SSD_CMDS_ENTRY_SIZE,
                        ssd_cmd_save_start)

pop_med:
   // Pop the SQ entry from the medium priority queue (if set)
   PRI_QUEUE_POP(k.nvme_tgt_kivec0_io_priority, NVME_BE_PRIORITY_MED,
                 k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
                 k.nvme_tgt_kivec0_idx, 
                 NVME_BE_SQ_CTX_TABLE_BASE, PRI_Q_CTX_STATE_OFFSET, 
                 PRI_Q_CTX_C_NDX_MED_OFFSET, pop_lo)

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_CMDS_TABLE_BASE, k.nvme_tgt_kivec0_ssd_handle,
                        SSD_CMDS_ENTRY_SIZE, SSD_CMDS_ENTRY_SIZE,
                        ssd_cmd_save_start)

pop_lo:
   // Pop the SQ entry from the low priority queue (if set)
   PRI_QUEUE_POP(k.nvme_tgt_kivec0_io_priority, NVME_BE_PRIORITY_HI,
                 k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
                 k.nvme_tgt_kivec0_idx, 
                 NVME_BE_SQ_CTX_TABLE_BASE, PRI_Q_CTX_STATE_OFFSET, 
                 PRI_Q_CTX_C_NDX_HI_OFFSET, exit)

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_CMDS_TABLE_BASE, k.nvme_tgt_kivec0_ssd_handle,
                        SSD_CMDS_ENTRY_SIZE, SSD_CMDS_ENTRY_SIZE,
                        ssd_cmd_save_start)

exit:
   nop.e
   nop
