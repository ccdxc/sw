/*****************************************************************************
 *  Stage: Push RDMA write (if any for read data) and status to initiator over
 *         ROCE SQ. This may require 2 producer indices for atomic operation.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_sq_ctx_k k;
struct r2n_sq_ctx_qpush_d d;
struct phv_ p;

%%

r2n_sq_entry_push_start:
   // Check if it is a read command, if not branch to push status
   seq		c1, k.nvme_tgt_kivec0_is_read, 1
   bcf		[!c1], push_status

   // Push read data via RDMA write
   // Check queue full condition two times (one for each p_ndx increment) and
   // exit if either of them fails. For the 2nd increment, store the value of
   // d.p_ndx into register r7 as it is to be used as input.
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)
   add		r7, r0, d.p_ndx
   mincr	r7, d.num_entries, 1
   QUEUE_FULL(r7, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // R2N submission queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, R2N_SQ_ENTRY_SIZE)

   // Push both data and status entries by incrementing p_ndx by 2
   QUEUE_PUSH_INCR_SAVE_PNDX(d.p_ndx, d.num_entries, 2, p.nvme_tgt_kivec0_p_ndx)

   // DMA data
   // 1. Derive source address
   addi		r1, r0, R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET
   add		r1, r1, k.nvme_tgt_kivec1_rbuf_hdr_addr
 
   // 2. Setup source for Mem2Mem DMA
   DMA_MEM2MEM_SETUP(r1, R2N_RX_BUF_ENTRY_WRITE_REQ_SIZE,
                     CAPRI_DMA_M2M_TYPE_SRC, dma_cmd4_cmdtype,
                     dma_cmd4_addr, dma_cmd4_data_size,
                     dma_cmd4_mem2mem_type, dma_cmd4_host_addr)

   // 3. Setup destination for Mem2Mem DMA whose address has been stored in r7
   DMA_MEM2MEM_SETUP(r7, R2N_RX_BUF_ENTRY_WRITE_REQ_SIZE,
                     CAPRI_DMA_M2M_TYPE_SRC, dma_cmd5_cmdtype,
                     dma_cmd5_addr, dma_cmd5_data_size,
                     dma_cmd5_mem2mem_type, dma_cmd5_host_addr)

   // Move the address saved in r7 by R2N_SQ_ENTRY_SIZE to get the
   // position to DMA the status
   QUEUE_PUSH_ADDR_OFFSET(r7, R2N_SQ_ENTRY_SIZE)

   // Setup the start and end DMA pointers for data + status
   DMA_PTR_SETUP(dma_cmd4_rsvd, dma_cmd0_cmdeop, p4_txdma_intr_dma_cmd_ptr)

   // Jump to DMA status (skipping steps to push status)
   b		dma_status

push_status:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // R2N submission queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, R2N_SQ_ENTRY_SIZE)

   // Push the entry
   QUEUE_PUSH_SAVE_PNDX(d.p_ndx, d.num_entries, p.nvme_tgt_kivec0_p_ndx)

   // Setup the start and end DMA pointers for status only
   DMA_PTR_SETUP(dma_cmd6_rsvd, dma_cmd0_cmdeop, p4_txdma_intr_dma_cmd_ptr)

dma_status:
   // DMA status
   // 1. Derive source address
   addi		r1, r0, R2N_RX_BUF_ENTRY_STATUS_REQ_OFFSET
   add		r1, r1, k.nvme_tgt_kivec1_rbuf_hdr_addr
 
   // 2. Setup source for Mem2Mem DMA
   DMA_MEM2MEM_SETUP(r1, R2N_RX_BUF_ENTRY_STATUS_REQ_SIZE,
                     CAPRI_DMA_M2M_TYPE_SRC, dma_cmd6_cmdtype,
                     dma_cmd6_addr, dma_cmd6_data_size,
                     dma_cmd6_mem2mem_type, dma_cmd6_host_addr)

   // 3. Setup destination for Mem2Mem DMA whose address has been stored in r7
   DMA_MEM2MEM_SETUP(r7, R2N_RX_BUF_ENTRY_STATUS_REQ_SIZE,
                     CAPRI_DMA_M2M_TYPE_SRC, dma_cmd7_cmdtype,
                     dma_cmd7_addr, dma_cmd7_data_size,
                     dma_cmd7_mem2mem_type, dma_cmd7_host_addr)

   // DMA R2N SQ p_ndx to p_ndx_db (doorbell) register
   // TODO: Check of this is by scheduler bit or doorbell write
   DMA_PHV2MEM_SETUP(nvme_tgt_kivec0_p_ndx, nvme_tgt_kivec0_p_ndx, d.p_ndx_db, 
                     dma_cmd0_phv_start, dma_cmd0_phv_end, dma_cmd0_cmdtype, 
                     dma_cmd0_addr, dma_cmd0_host_addr)
   
exit:
   nop.e
   nop
