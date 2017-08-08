/*****************************************************************************
 *  Stage: Perform weighted round robin between the 3 priority queues and pick
 *         a queue to pop the command from. Load that queue's address for the
 *         next stage. If no command can be popped, exit the pipeline.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_sq_ctx_k k;
struct ssd_sq_ctx_qpush_d d;
struct phv_ p;

%%
ssd_sq_entry_push_start:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // SSD command queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, SSD_SQ_ENTRY_SIZE)

   // DMA write of nvme_be_cmd.nvme_cmd_w0 .. nvme_be_cmd.nvme_cmd_hi to 
   // SSD's SQ 
   DMA_PHV2MEM_SETUP(nvme_be_cmd_p_nvme_cmd_w0, nvme_be_cmd_p_nvme_cmd_hi, r7, 
                     dma_cmd2_phv_start, dma_cmd2_phv_end, dma_cmd2_cmdtype, 
                     dma_cmd2_addr, dma_cmd2_host_addr)
   
   // Push the entry
   QUEUE_PUSH_SAVE_PNDX(d.p_ndx, d.num_entries, p.nvme_tgt_kivec0_p_ndx)
 
   // DMA SSD's SQ p_ndx to p_ndx_db (doorbell) register
   DMA_PHV2MEM_SETUP(nvme_tgt_kivec0_p_ndx, nvme_tgt_kivec0_p_ndx, d.p_ndx_db,
                     dma_cmd3_phv_start, dma_cmd3_phv_end, dma_cmd3_cmdtype, 
                     dma_cmd3_addr, dma_cmd3_host_addr)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd3_cmdeop, p4_txdma_intr_dma_cmd_ptr)
exit:
   nop.e
   nop
