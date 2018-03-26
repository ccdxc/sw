/*****************************************************************************
 *  seq_comp_status_desc_handler: Handle the compression status descriptor entry in the
 *                         sequencer. This involves:
 *                          1. processing status to see if operation succeeded
 *                          2. breaking up the compressed data into the
 *                             destination SGL provided in the descriptor
 *                         In this stage, load the status entry for next stage
 *                         and save the other fields into I vector.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_comp_status_desc0_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_comp_status_handler_start

storage_tx_seq_comp_status_desc0_handler_start:

   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt
   //
   // Note: if a compression error arises and status_dma_en is set,
   // interrupts (if set) will also be raised
   PCI_SET_INTERRUPT_DATA()

   // Store the various parts of the descriptor in the K+I vectors for later use
   // Check if next doorbell is to be enabled and branch
   bbeq		d.next_db_en, 0, check_intr
   phvwrpair	p.storage_kivec5_intr_addr, d.intr_addr, \
                p.{storage_kivec5_status_dma_en...storage_kivec5_intr_en}, \
   	        d.{status_dma_en...intr_en}    // delay slot

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
   PCI_SET_INTERRUPT_ADDR_DMA(d.intr_addr, dma_p2m_11)

status_dma_setup:

   // Set up status xfer if applicable
   bbeq         d.status_dma_en, 0, tbl_load
   nop
   
   // Set up the status DMA:
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.status_hbm_addr, 
                     d.status_len[13:0], 0, 0, dma_m2m_1)
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, d.status_host_addr, 
                     d.status_len[13:0], 0, 0, dma_m2m_2)
   DMA_MEM2MEM_FENCE(dma_m2m_2)
   
tbl_load:
   // Setup the start and end DMA pointers
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_hbm_addr, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_tx_seq_comp_status_handler_start)
