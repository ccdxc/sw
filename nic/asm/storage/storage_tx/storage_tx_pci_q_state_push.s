/*****************************************************************************
 *  pci_q_state_push: Push to a queue across the PCI bus (e.g. NVME driver,
 *                    SSD, PVM) by issuing the DMA commands to write the p_ndx
 *                    and trigger the MSI-X interrupt.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_pci_q_state_push_d d;
struct phv_ p;

%%

storage_tx_pci_q_state_push_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_p2m_1)
   
   // DMA entries #3, #4 are used for ringing additional doorbells, 
   // writes for fixing up pointers etc. (default NOP)
   
   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Push the p_ndx by doing a DMA update
   PCI_QUEUE_PUSH_DATA_UPDATE(dma_p2m_5)

   // Check if interrupt is enabled and branch
   seq		c1, d.intr_en, 1
   bcf		[c1], send_intr

   // Setup the start and end DMA pointers and branch to table load
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_5_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   b		tbl_load

send_intr:
   // Raise the interrupt with a DMA update
   PCI_RAISE_INTERRUPT(dma_p2m_6)

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_6_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
