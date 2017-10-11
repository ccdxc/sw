/*****************************************************************************
 *  seq_pdma_entry_handler: Handle the PDMA entry in sequencer. Issue the DMA
 *                          command to copy data and ring the next doorbell
 *                          with a fence bit to guarantee ordering.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_pdma_entry_handler_d d;
struct phv_ p;

%%
storage_tx_seq_pdma_entry_handler_start:

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_UPDATE

   // Setup the source of the mem2mem DMA into DMA cmd 1
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.src_addr, d.data_size,
                     d.src_lif_override, d.src_lif_override, dma_m2m_1)

   // Setup the destination of the mem2mem DMA into DMA cmd 2
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.dst_addr, d.data_size,
                     d.dst_lif_override, d.dst_lif_override, dma_m2m_2)

   // Copy the data for the doorbell into the PHV and setup a DMA command
   // to ring it
   phvwr	p.qpush_doorbell_data_data, d.next_db_data
   DMA_PHV2MEM_SETUP(qpush_doorbell_data_data, qpush_doorbell_data_data,
                     d.next_db_addr, dma_p2m_3)

   // Set the fence bit for the doorbell 
   DMA_PHV2MEM_FENCE(dma_p2m_3)

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_3_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   LOAD_NO_TABLES
