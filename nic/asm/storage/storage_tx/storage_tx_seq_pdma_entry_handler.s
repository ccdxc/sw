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

   // Setup the source of the mem2mem DMA into DMA cmd 1
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.src_addr, d.data_size[13:0],
                     d.src_lif_override, d.src_lif, dma_m2m_1)

   // Setup the destination of the mem2mem DMA into DMA cmd 2
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.dst_addr, d.data_size[13:0],
                     d.dst_lif_override, d.dst_lif, dma_m2m_2)

   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt
   // 2. If next_db_en is not set and intr_en is not set => do nothing more

   // Check if next doorbell is to be enabled and branch
   seq		c1, d.next_db_en, 1
   bcf		![c1], check_intr
   nop

   // Ring the sequencer doorbell based on addr/data provided in the descriptor
   SEQUENCER_DOORBELL_RING(dma_p2m_3)

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_3_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Done ringing doorbell, don't fire the interrupt in this path
   b		tbl_load

check_intr:
   // Check if interrupt is enabled and branch
   seq		c1, d.intr_en, 1
   bcf		![c1], pdma_only
   nop

   // Raise interrupt update based on addr/data provided in descriptor
   PCI_RAISE_INTERRUPT(dma_p2m_3)

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_3_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Done firing the interrupt, exit
   b		tbl_load
   nop

pdma_only:
   // Setup the start and end DMA pointers to just the PDMA portions
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_2_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
