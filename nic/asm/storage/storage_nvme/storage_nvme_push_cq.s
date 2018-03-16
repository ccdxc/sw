/*****************************************************************************
 *  push_cq : Push the NVME status after setting the phase bit to the CQ.
 *            Alter the phase bit in the CQ context in event wrapping around
 *            the CQ.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s6_tbl0_k k;
struct s6_tbl0_push_cq_d d;
struct phv_ p;

%%

storage_nvme_push_cq_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA the NVME status entry to the CQ ring buffer
   DMA_PHV2MEM_SETUP_ADDR64(nvme_sta_cspec, nvme_sta_status, 
                            r7, dma_p2m_13)

   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // If new p_ndx has wrapped around, flip the phase bit
   // TODO: Remove nop after checking if d.p_ndx seen here will be the new value
   nop
   seq		c1, d.p_ndx, 0
   tblmincri.c1	d.phase, 2, 1

   // Check if interrupt is enabled and branch
   seq		c1, d.intr_en, 1
   bcf		![c1], skip_intr
   nop

   // Raise the interrupt with a DMA update
   PCI_RAISE_INTERRUPT(dma_p2m_14)


skip_intr:
   // Setup the start and end DMA pointers and branch to table load
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_15_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   // Exit pipeline
   LOAD_NO_TABLES
