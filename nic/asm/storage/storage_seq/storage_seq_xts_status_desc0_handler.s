/*****************************************************************************
 *  seq_xts_status_desc0_handler: Handle the XTS status descriptor entry in the
 *                                sequencer.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_xts_status_desc0_handler_d d;
struct phv_ p;

%%
   .param storage_seq_xts_status_handler

storage_tx_seq_xts_status_desc0_handler:

   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt

   // Note: if a compression error arises and status_dma_en is set,
   // interrupts (if set) will also be raised
   PCI_SET_INTERRUPT_DATA()
   
   // Check if next doorbell is to be enabled
   bbeq		d.next_db_en, 0, intr_check
   phvwrpair	p.seq_kivec5xts_intr_addr, d.intr_addr, \
        	p.{seq_kivec5xts_status_dma_en...seq_kivec5xts_next_db_action_barco_push}, \
   	        d.{status_dma_en...next_db_action_barco_push} // delay slot

   // If doorbell is actually a Barco push action, handle accordingly.
   // Note that d.next_db_data in this case is really d.barco_desc_addr
   bbeq		d.next_db_action_barco_push, 0, next_db_ring
   phvwr	p.seq_kivec7xts_comp_desc_addr, d.next_db_data // delay slot
                
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_21)
   DMA_PHV2MEM_FENCE(dma_p2m_21)
   
   // Note that d.next_db_addr in this case is really d.barco_ring_addr
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.seq_kivec4_barco_ring_addr, d.next_db_addr[33:0], \
                p.{seq_kivec4_barco_pndx_shadow_addr...seq_kivec4_barco_num_descs}, \
                d.{barco_pndx_shadow_addr...barco_num_descs}
   phvwr        p.seq_kivec4_barco_desc_addr, d.next_db_data
                
possible_status_dma:

   // Set up further status xfer if applicable
   bbeq         d.status_dma_en, 0, tbl_load
   add          r3, r0, d.status_len    // delay slot
   
   // Set up the status DMA:
   add          r4, d.status_addr0, d.status_offset0
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, r4,
                            r3, dma_m2m_0)
   DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.status_addr1, 
                            r3, dma_m2m_1)

tbl_load:

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_addr0, STORAGE_TBL_LOAD_SIZE_64_BITS,
                              storage_seq_xts_status_handler)

intr_check:
   // Check if interrupt is enabled
   bbeq		d.intr_en, 0, possible_status_dma
   nop

   // Raise interrupt based on addr/data provided in descriptor
   PCI_SET_INTERRUPT_ADDR_DMA(d.intr_addr, dma_p2m_21)
   b            possible_status_dma
   nop


next_db_ring:

   // Ring the sequencer doorbell based on addr/data provided in the descriptor
   SEQUENCER_DOORBELL_RING(dma_p2m_21)
   b            possible_status_dma
   nop

