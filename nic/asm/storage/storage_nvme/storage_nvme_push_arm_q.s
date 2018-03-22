/*****************************************************************************
 *  push_arm_q: Push the IO buffer to the ARM in all cases. ARM looks at
 *              the oper_status flag in the IO context to determine if the
 *              IO buffer is punted or processed in the datapath. When
 *               Case 1: When IO buffer is punted, ARM handles it entirely
 *                       and frees it after processing
 *               Case 2: When IO buffer is processed in the datapath, ARM
 *                       runs a timer and monitors completion of the IO
 *                         and frees it
 *               In all cases, IO buffer is allocated by P4+ and freed by ARM
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s7_tbl0_k k;
struct s7_tbl0_push_arm_q_d d;
struct phv_ p;

%%

storage_nvme_push_arm_q_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA the I/O context from PHV to the ARM queue
   DMA_PHV2MEM_SETUP_ADDR34(io_ctx_iob_addr, io_ctx_nvme_sq_qaddr, 
                            r7, dma_p2m_12)

   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Check if interrupt is enabled and branch
   seq		c1, d.intr_en, 1
   bcf		[c1], send_intr
   nop

   // Setup the start and end DMA pointers and branch to table load
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_14_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   b		tbl_load
   nop

send_intr:
   // Raise the interrupt with a DMA update
   PCI_RAISE_INTERRUPT(dma_p2m_13)

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_15_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
  
tbl_load:
   // Load no tables and exit the pipeline
   LOAD_NO_TABLES
