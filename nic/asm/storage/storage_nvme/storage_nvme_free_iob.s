/*****************************************************************************
 *  free_iob : Free IO buffer from free list by producing an item to the
 *             to the iob_ring. Ring being full is an error condition which
 *             should never be hit.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s4_tbl0_k k;
struct s4_tbl0_free_iob_d d;
struct phv_ p;

%%

storage_nvme_free_iob_start:
   // Check queue full condition based on working producer index and exit.
   // If IOB ring is full, dont free it. This error condition should never be hit
   // but for a bug.  TODO: Push error handling
   QUEUE_FULL(d.wp_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the IOB ring. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.wp_ndx, d.entry_size)

   // DMA the I/O buffer address to be freed
   DMA_PHV2MEM_SETUP_ADDR34(io_ctx_iob_addr, io_ctx_iob_addr, 
                            r7, dma_p2m_9)

   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.wp_ndx, d.num_entries)

   // Delay slot to calculate the address for the PDMA (since reading p_ndx 
   // straight after writing takes a one cycle stall)
   add		r7, NVME_KIVEC_IOB_RING4_BASE_ADDR, RING_STATE_P_NDX_OFFSET

   // Store the updated wp_ndx in PHV
   phvwr	p.qpush_pndx_data_p_ndx, d.wp_ndx

   // DMA the wp_ndx to p_ndx via fenced DMA. This ensures that the p_ndx
   // is written in the queue state only after the I/O buffer is actually
   // added back to the free list.
   DMA_PHV2MEM_SETUP_ADDR34(qpush_pndx_data_p_ndx, qpush_pndx_data_p_ndx, 
                            r7, dma_p2m_10)
   DMA_PHV2MEM_FENCE(dma_p2m_10)
   
   // Setup the start and end DMA pointers 
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_10_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
  
tbl_load:
   // Load no tables and exit the pipeline
   LOAD_NO_TABLES
