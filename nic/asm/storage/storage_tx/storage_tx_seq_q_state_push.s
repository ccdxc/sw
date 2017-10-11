/*****************************************************************************
 *  seq_q_state_push: Push to a sequencer destination queue by issuing the
 *                    mem2mem DMA commands and incrementing the p_ndx via
 *                    ringing the doorbell. Assumes that data to be pushed has
 *                    its source in DMA cmd 1 and destination in DMA cmd 2.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_q_state_push_d d;
struct phv_ p;

%%

storage_tx_seq_q_state_push_start:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_m2m_2)
   
   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Ring the doorbell for the recipient of the push.
   QUEUE_PUSH_DOORBELL_RING(dma_p2m_3)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_3_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
