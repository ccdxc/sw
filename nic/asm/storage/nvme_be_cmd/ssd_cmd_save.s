/*****************************************************************************
 *  Stage: Perform weighted round robin between the 3 priority queues and pick
 *         a queue to pop the command from. Load that queue's address for the
 *         next stage. If no command can be popped, exit the pipeline.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_cmds_k k;
struct ssd_cmds_save_d d;
struct phv_ p;

%%
   .param	ssd_sq_entry_push_start
ssd_cmd_save_start:
   // Find first clear bit in the bitmap and set it
   add		r1, r0, d.bitmap
   ffcv		r2, d.bitmap, r0
   addi		r7, r0, 1
   sllv		r7, r7, r2

   // Write the bitmap back to the table and set the command index in PHV
   tblor	d.bitmap, r7

   // Modify the NVME command to include the generated command index in
   // the CID of Dword0
   phvwr	p.nvme_be_cmd_p_nvme_cmd_w0, r7

   // Calculate the address to which the command has to be written to in the
   // SSD command queue. Input and Output will be stored in GPR r7.
   SSD_CMD_ENTRY_ADDR(k.nvme_tgt_kivec0_ssd_handle, r7)

   // DMA nvme_be_cmd_p_src_queue_id .. nvme_be_cmd_p_nvme_cmd_hi to 
   // saved commands. 
   DMA_PHV2MEM_SETUP(nvme_be_cmd_p_src_queue_id, nvme_be_cmd_p_nvme_cmd_hi, r7, 
                     dma_cmd0_phv_start, dma_cmd0_phv_end, dma_cmd0_cmdtype, 
                     dma_cmd0_addr, dma_cmd0_host_addr)
   
   // Overwrite the original NVME command id at the calculated index
   addi		r7, r0, SSD_CMD_CID_OFFSET
   DMA_PHV2MEM_SETUP(nvme_be_cmd_p_nvme_cmd_cid, nvme_be_cmd_p_nvme_cmd_cid, r7,
                     dma_cmd1_phv_start, dma_cmd1_phv_end, dma_cmd1_cmdtype, 
                     dma_cmd1_addr, dma_cmd1_host_addr)

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_SQ_CTX_TABLE_BASE, k.nvme_tgt_kivec0_ssd_handle, 
                        Q_CTX_SIZE, Q_CTX_SIZE, ssd_sq_entry_push_start)
