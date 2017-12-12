/*****************************************************************************
 *  roce_rq_push: Push a ROCE RQ WQE by issuing the DMA commands to write
 *                the ROCE RQ WQE and incrementing the p_ndx via ringing the
 *                doorbell. Assumes RQ WQE to be pushed is in DMA command 1.
 *                This is used to post the R2N buffer to ROCE RQ.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_roce_rq_push_d d;
struct phv_ p;

%%

storage_tx_roce_rq_push_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // HACK: This is because RDMA base address is only 32 bits and we are 
   //       passing a host based queue here
   // TODO: Fix this and remove hack after we resolve this with RDMA folks
   addi		r7, r0, 1
   sll		r7, r7, 63
   add		r7, r7, d.base_addr

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(r7, d.p_ndx, d.entry_size)

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_m2m_2)
   
   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Ring the doorbell for the recipient of the push.
   QUEUE_PUSH_DOORBELL_RING(dma_p2m_4)


   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_4_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
