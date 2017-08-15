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
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_p2m_1)
   
   // Setup the lif, type, qid, pindex for the doorbell push, Output will
   // be stored in GPR r7.
   DOORBELL_DATA_SETUP(d.p_ndx, 0, STORAGE_KIVEC0_DST_QID, 0)
   DOORBELL_ADDR_SETUP(STORAGE_KIVEC0_DST_LIF, STORAGE_KIVEC0_DST_QTYPE, 
                       DOORBELL_UPDATE_P_NDX_INCR)
 
   // DMA the doorbell addr/data to increment the p_ndx and ring the doorbell
   DMA_PHV2MEM_SETUP(qpush_doorbell_data_index, qpush_doorbell_data_pid, r7,
                     dma_p2m_2)

   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_type, dma_p2m_2_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
exit:
   nop.e
   nop
