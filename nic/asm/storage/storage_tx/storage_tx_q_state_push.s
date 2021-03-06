/*****************************************************************************
 *  q_state_push: Push to a queue by issuing the DMA commands and incrementing
 *                the p_ndx via ringing the doorbell. Assumes that data to be
 *                pushed is in DMA command 1.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_q_state_push_d d;
struct phv_ p;

%%

storage_tx_q_state_push_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.wp_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.wp_ndx, d.entry_size)

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_p2m_1)
   
   // DMA entries #3, #4 are used for ringing additional doorbells, 
   // writes for fixing up pointers etc. (default NOP)

   // Push the entry to the queue (this increments wp_ndx and writes to table)
   QUEUE_PUSH(d.wp_ndx, d.num_entries)

   // Ring the doorbell for the recipient of the push.
   QUEUE_PUSH_DOORBELL_UPDATE_RING(dma_p2m_5, d.wp_ndx)


   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_5_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
