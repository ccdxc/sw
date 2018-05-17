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
   .param storage_seq_barco_ring_pndx_read

storage_seq_barco_entry_handler:

   // Update the K+I vector with the barco descriptor size to be used
   // when calculating the offset for the push operation.
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.seq_kivec4_barco_ring_addr, d.barco_ring_addr, \
                p.{seq_kivec4_barco_pndx_shadow_addr...seq_kivec4_barco_ring_size}, \
                d.{barco_pndx_shadow_addr...barco_ring_size}
   bbeq         d.barco_batch_mode, 1, barco_batch_mode
   phvwrpair    p.seq_kivec4_barco_desc_addr, d.barco_desc_addr, \
                p.seq_kivec4_barco_num_descs, 1 // delay slot
   
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_21)
   DMA_PHV2MEM_FENCE(dma_p2m_21)
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad,
                 dma_p2m_21_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Advance to a common stage for executing table lock read to get the
   // Barco ring pindex.
   LOAD_TABLE_NO_LKUP_PC_IMM_e(0, storage_seq_barco_ring_pndx_read)

barco_batch_mode:

   // in batch mode, the caller supplies the Barco pndx value with which
   // we can immediately use to set up the mem2mem destination
   QUEUE_PUSH_ADDR(d.barco_ring_addr, d.barco_batch_pndx, d.barco_desc_size)
   
   // Setup the source of the mem2mem DMA into DMA cmd 1.
   // For now, not using any override LIF parameters.
   sll          r6, 1, d.barco_desc_size
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.barco_desc_addr,
                            r6, dma_m2m_16)
   DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r7,
                                     r6, dma_m2m_17)
   bbeq         d.barco_batch_last, 1, barco_batch_last
   add          r6, d.barco_batch_pndx, 1   // delay slot
   
   // not the last entry of the batch so don't ring barco doorbell
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad,
                 dma_p2m_17_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES

   
barco_batch_last:

   // Need to word swap before writing back as the p_ndx is little endian
   phvwr        p.barco_doorbell_data_p_ndx, r6.wx
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_21)
   DMA_PHV2MEM_FENCE(dma_p2m_21)
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad,
                 dma_p2m_21_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES
   
