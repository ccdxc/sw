/*****************************************************************************
 *  seq_barco_entry_handler: Handle the Barco XTS entry in sequencer. Form the
 *                           DMA command to copy the Barco XTS descriptor as
 *                           part of the push operation in the next stage.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_barco_entry_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_barco_ring_pndx_read_start

storage_tx_seq_barco_entry_handler_start:

   // Update the K+I vector with the barco descriptor size to be used
   // when calculating the offset for the push operation.
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.storage_kivec4_barco_ring_addr, d.barco_ring_addr, \
                p.{storage_kivec4_barco_pndx_shadow_addr...storage_kivec4_barco_ring_size}, \
                d.{barco_pndx_shadow_addr...barco_ring_size}
  
   // Save the descriptor size in bytes in r7
   sll      r7, 1, d.barco_desc_size

   // Setup the source of the mem2mem DMA into DMA cmd 1.
   // For now, not using any override LIF parameters.
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.barco_desc_addr, r7,
                     r0, r0, dma_m2m_1)

   // Setup the destination of the mem2mem DMA into DMA cmd 2 (just fill
   // the size). For now, not using any override LIF parameters.
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r0, r7,
                              r0, r0, dma_m2m_2)

   // Copy the data for the doorbell into the PHV and setup a DMA command
   // to ring it. Form the doorbell DMA command in this stage as opposed 
   // the push stage (as is the norm) to avoid carrying the doorbell address 
   // in K+I vector.
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_3)

   bbeq     d.barco_batch_mode, 1, barco_batch_mode
   nop
   
   // Set the fence bit for the doorbell 
   DMA_PHV2MEM_FENCE(dma_p2m_3)


   // Advance to a common stage for executing table lock read to get the
   // Barco ring pindex.
   LOAD_TABLE_NO_LKUP_PC_IMM_e(0, storage_tx_seq_barco_ring_pndx_read_start)

barco_batch_mode:

   // in batch mode, the caller supplies the Barco pndx value with which
   // we can immediately use to set up the mem2mem destination
   QUEUE_PUSH_ADDR(d.barco_ring_addr, d.barco_batch_pndx, d.barco_desc_size)
   
   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_m2m_2)
   
   bbeq     d.barco_batch_last, 1, barco_batch_last
   add      r6, d.barco_batch_pndx, 1   // delay slot
   
   // not the last entry of the batch so don't ring barco doorbell
   DMA_PHV2MEM_FENCE(dma_m2m_2)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_m2m_2_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES

   
barco_batch_last:

   // Need to word swap before writing back as the p_ndx is little endian
   phvwr    p.barco_doorbell_data_p_ndx, r6.wx
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_3_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES
   
