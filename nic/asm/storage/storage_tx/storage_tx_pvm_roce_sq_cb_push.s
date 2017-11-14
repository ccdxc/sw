/*****************************************************************************
 *  pvm_roce_sq_cb_push: Push a ROCE SQ WQE by issuing the DMA commands to
 *                       write the ROCE SQ WQE and incrementing the p_ndx via
 *                       ringing the doorbell. Assumes SQ WQE to be pushed is
 *                       in DMA command 1.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl_k k;
struct s5_tbl_pvm_roce_sq_cb_push_d d;
struct phv_ p;

%%

storage_tx_pvm_roce_sq_cb_push_start:
  
   // Save the original p_ndx into GPR r6 to use for calculating the addresses as 
   // the tblmincr alters the d-vector.
   add		r6, r0, d.p_ndx
   seq		c1, STORAGE_KIVEC0_IS_READ, 1
   bcf		![c1], check_push_status
   nop

   // Check queue full condition for pushing both read data and status 
   // and exit if full. TODO: Push error handling
   QUEUE_FULL2(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)
   
   // Push the entry to the queue with an increment of 2 for both read data
   // and status (this writes to table)
   QUEUE_PUSH_INCR(d.p_ndx, d.num_entries, 2)

   // Calculate the address to which the write WQE descriptor to be pushed 
   // has to be written to in the destination ROCE SQ. Output will be stored 
   // in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, r6, d.entry_size)

   // DMA command address update to store the destination address
   DMA_ADDR_UPDATE(r7, dma_p2m_4)
   // HACK: This is because RDMA base address is only 32 bits and we are 
   //       passing a host based queue here
   // TODO: Fix this and remove hack after we resolve this with RDMA folks
   phvwr	p.dma_m2m_4_dma_cmd_host_addr, 1
   
   // Increment the p_ndx stored in GPR r6 so that the status WQE descriptor
   // will occupy the next slot
   addi		r6, r0, 1
 
   // Jump to pushing status without checking queue full or updating the table
   // as it has already been done
   
check_push_status:
   // Check queue full condition for pushing only status and exit if full. 
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Push the entry to the queue with an increment of 1 only for status
   // (this writes to table)
   QUEUE_PUSH_INCR(d.p_ndx, d.num_entries, 1)

push_status:

   // Calculate the address to which the write WQE descriptor to be pushed 
   // has to be written to in the destination ROCE SQ. Output will be stored 
   // in GPR r7. Store the value calculated earlier in GPR r6
   QUEUE_PUSH_ADDR(d.base_addr, r6, d.entry_size)

   // DMA command address update to store the destination address
   DMA_ADDR_UPDATE(r7, dma_p2m_6)
   // HACK: This is because RDMA base address is only 32 bits and we are 
   //       passing a host based queue here
   // TODO: Fix this and remove hack after we resolve this with RDMA folks
   phvwr	p.dma_m2m_6_dma_cmd_host_addr, 1
   
   // Ring the doorbell for the recipient of the push.
   ROCE_QUEUE_PUSH_DOORBELL_RING(dma_p2m_7)

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_7_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
