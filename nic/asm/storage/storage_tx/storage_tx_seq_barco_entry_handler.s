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
   .param storage_tx_seq_barco_ring_push_start

storage_tx_seq_barco_entry_handler_start:

   // Update the K+I vector with the barco descriptor size to be used
   // when calculating the offset for the push operation
   phvwrpair	p.storage_kivec1_xts_desc_size, d.xts_desc_size[15:0], \
                p.storage_kivec1_device_addr, d.xts_ring_addr
  
   // Save the descriptor size in bytes in r7
   sll      r7, 1, d.xts_desc_size

   // Setup the source of the mem2mem DMA into DMA cmd 1.
   // For now, not using any override LIF parameters.
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.xts_desc_addr, r7,
                     r0, r0, dma_m2m_1)

   // Setup the destination of the mem2mem DMA into DMA cmd 2 (just fill
   // the size). For now, not using any override LIF parameters.
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r0, r7,
                              r0, r0, dma_m2m_2)

   // Copy the data for the doorbell into the PHV and setup a DMA command
   // to ring it. Form the doorbell DMA command in this stage as opposed 
   // the push stage (as is the norm) to avoid carrying the doorbell address 
   // in K+I vector.
   DMA_PHV2MEM_SETUP_ADDR34(xts_doorbell_data_p_ndx, xts_doorbell_data_p_ndx,
                            d.xts_pndx_addr, dma_p2m_3)

   // Set the fence bit for the doorbell 
   DMA_PHV2MEM_FENCE(dma_p2m_3)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.xts_pndx_addr, d.xts_pndx_size[2:0],
                                storage_tx_seq_barco_ring_push_start)

