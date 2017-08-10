/*****************************************************************************
 *  Stage: Push NVME status to NVME VF's CQ. TODO: How to trigger the MSI-X
 *         interrupt ?
 *****************************************************************************/


#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_vf_cq_ctx_k k;
struct nvme_vf_cq_ctx_qpush_d d;
struct phv_ p;

%%
nvme_vf_cq_entry_push_start:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // NVME VF completion queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, NVME_VF_CQ_ENTRY_SIZE)

   // DMA write of pvm_status.cspec ... pvm_status.status to NVME VF CQ entry
   DMA_PHV2MEM_SETUP(pvm_status_cspec, pvm_status_status, r7, 
                     dma_cmd0_phv_start, dma_cmd0_phv_end, dma_cmd0_cmdtype, 
                     dma_cmd0_addr, dma_cmd0_host_addr)
   
   // Push the entry
   QUEUE_PUSH_SAVE_PNDX(d.p_ndx, d.num_entries, p.nvme_init_kivec0_p_ndx)
 
   // DMA NVME backend CQ p_ndx to p_ndx_db (doorbell) register
   // TODO: Check of this is by scheduler bit or doorbell write
   DMA_PHV2MEM_SETUP(nvme_init_kivec0_p_ndx, nvme_init_kivec0_p_ndx, d.p_ndx_db, 
                     dma_cmd1_phv_start, dma_cmd1_phv_end, dma_cmd1_cmdtype, 
                     dma_cmd1_addr, dma_cmd1_host_addr)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd1_cmdeop, p4_txdma_intr_dma_cmd_ptr)
exit:
   nop.e
   nop
