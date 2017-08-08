/*****************************************************************************
 *  Stage: Punt the entire NVME command handling by pushing it to PVM error 
 *         queue in case of error 
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct pvm_errq_ctx_k k;
struct pvm_errq_ctx_errq_push_d d;
struct phv_ p;

%%

pvm_errq_entry_push_start:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // PVM error queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, PVM_ERRQ_ENTRY_SIZE)

   // DMA write of rcq_ent.comp_type .. rcq_ent.qp_index to PVM error queue
   DMA_PHV2MEM_SETUP(rcq_ent_comp_type, rcq_ent_qp_index, r7, 
                     dma_cmd0_phv_start, dma_cmd0_phv_end, dma_cmd0_cmdtype, 
                     dma_cmd0_addr, dma_cmd0_host_addr)
   
   // Push the entry
   QUEUE_PUSH_SAVE_PNDX(d.p_ndx, d.num_entries, p.nvme_tgt_kivec0_p_ndx)
 
   // DMA SSD's SQ p_ndx to p_ndx_db (doorbell) register
   DMA_PHV2MEM_SETUP(nvme_tgt_kivec0_p_ndx, nvme_tgt_kivec0_p_ndx, d.p_ndx_db, 
                     dma_cmd1_phv_start, dma_cmd1_phv_end, dma_cmd1_cmdtype, 
                     dma_cmd1_addr, dma_cmd1_host_addr)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd1_cmdeop, p4_txdma_intr_dma_cmd_ptr)
exit:
   nop.e
   nop
