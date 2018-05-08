/*****************************************************************************
 *  seq_xts_status_desc_handler: Handle the XTS status descriptor entry in the
 *                               sequencer.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_xts_status_desc_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_xts_status_handler_start

storage_tx_seq_xts_status_desc_handler_start:

   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt

   // Note: if a compression error arises and status_dma_en is set,
   // interrupts (if set) will also be raised
   PCI_SET_INTERRUPT_DATA()
   
   // Check if next doorbell is to be enabled
   bbeq		d.next_db_en, 0, intr_check
   phvwr 	p.{storage_kivec5_status_dma_en...storage_kivec5_stop_chain_on_error}, \
   	        d.{status_dma_en...stop_chain_on_error} // delay slot

   // if doorbell is actually a Barco push action, handle accordingly
   bbeq		d.next_db_action_barco_push, 0, next_db_ring
   sll          r7, 1, d.barco_desc_size        // delay slot
                
   // Setup the source of the mem2mem DMA into DMA cmd 1.
   // Note: next_db_data doubles as barco_desc_addr in this case
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.next_db_data, r7,
                     r0, r0, dma_m2m_9)

   // Setup the destination of the mem2mem DMA into DMA cmd 2 (just fill
   // the size).
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r0, r7,
                              r0, r0, dma_m2m_10)

   // Copy the data for the doorbell into the PHV and setup a DMA command
   // to ring it. Form the doorbell DMA command in this stage as opposed 
   // the push stage (as is the norm) to avoid carrying the doorbell address 
   // in K+I vector.
   DMA_PHV2MEM_SETUP_ADDR34(barco_doorbell_data_p_ndx, barco_doorbell_data_p_ndx,
                            d.barco_pndx_addr, dma_p2m_11)
   DMA_PHV2MEM_FENCE(dma_p2m_11)
   
   // Note that d.next_db_addr in this case is really d.barco_ring_addr
   // phvwrpair limits destination p[] to 64 bits per.
   phvwrpair	p.storage_kivec4_barco_ring_addr, d.next_db_addr[33:0], \
                p.{storage_kivec4_barco_pndx_shadow_addr...storage_kivec4_barco_ring_size}, \
                d.{barco_pndx_shadow_addr...barco_ring_size}
   b            status_dma_setup
   phvwri       p.storage_kivec4_barco_num_descs, 1     // delay slot
                
next_db_ring:

   // Ring the sequencer doorbell based on addr/data provided in the descriptor
   SEQUENCER_DOORBELL_RING(dma_p2m_11)

status_dma_setup:

   // Set up further status xfer if applicable
   bbeq         d.status_dma_en, 0, tbl_load
   add          r3, r0, d.status_len    // delay slot
   
   // Set up the status DMA:
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.status_hbm_addr, 
                     r3, 0, 0, dma_m2m_0)
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.status_host_addr, 
                     r3, 0, 0, dma_m2m_1)

tbl_load:

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_hbm_addr, STORAGE_TBL_LOAD_SIZE_64_BITS,
                              storage_tx_seq_xts_status_handler_start)

intr_check:
   // Check if interrupt is enabled
   bbeq		d.intr_en, 0, status_dma_setup
   nop

   // Raise interrupt based on addr/data provided in descriptor
   PCI_SET_INTERRUPT_ADDR_DMA(d.intr_addr, dma_p2m_11)
   b            status_dma_setup
   nop


