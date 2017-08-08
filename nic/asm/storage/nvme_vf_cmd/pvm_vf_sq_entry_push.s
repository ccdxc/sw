/*****************************************************************************
 *  Stage: Push NVME command to PVM VF's SQ. TODO: How to trigger MSI-X 
 *         interrupt ?
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_qpush_d d;
struct phv_ p;

%%

pvm_vf_sq_entry_push_start:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // NVME backend completion queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, PVM_VF_SQ_ENTRY_SIZE)

   // DMA write of pvm_cmd.opc ... pvm_cmd.dw15 to PVM VF SQ entry
   DMA_PHV2MEM_SETUP(pvm_cmd_opc, pvm_cmd_tickreg, r7, 
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
