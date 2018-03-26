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

   // Check if next doorbell is to be enabled and branch
   bbeq		d.next_db_en, 0, check_intr
   phvwr	p.{storage_kivec5_status_dma_en...storage_kivec5_stop_chain_on_error}, \
   	        d.{status_dma_en...stop_chain_on_error} // delay slot

   // Ring the sequencer doorbell based on addr/data provided in the descriptor
   SEQUENCER_DOORBELL_RING(dma_p2m_11)

   // Done ringing doorbell, don't fire the interrupt in this path
   b		status_dma_setup
   nop

check_intr:
   // Check if interrupt is enabled and branch
   bbeq		d.intr_en, 0, status_dma_setup
   nop

   // Raise interrupt based on addr/data provided in descriptor
   PCI_RAISE_INTERRUPT(dma_p2m_11)

status_dma_setup:

   // Set up further status xfer if applicable
   bbeq         d.status_dma_en, 0, tbl_load
   nop
   
   // Set up the status DMA:
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.status_hbm_addr, 
                     d.status_len[13:0], 0, 0, dma_m2m_1)
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.status_host_addr, 
                     d.status_len[13:0], 0, 0, dma_m2m_2)
   DMA_MEM2MEM_FENCE(dma_m2m_2)

tbl_load:

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_hbm_addr, STORAGE_TBL_LOAD_SIZE_64_BITS,
                              storage_tx_seq_xts_status_handler_start)

