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
struct s1_tbl_seq_comp_status_desc_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_comp_status_handler_start

storage_tx_seq_comp_status_desc_handler_start:

   // Store the various parts of the descriptor in the K+I vectors for later use
   phvwrpair	p.storage_kivec4_sgl_addr, d.sgl_addr, \
   	        p.storage_kivec4_data_addr, d.data_addr
   phvwr	p.storage_kivec4_data_len, d.data_len
  
   phvwrpair	p.storage_kivec5_status_addr, d.status_addr, \
   	        p.storage_kivec5_status_len, d.status_len
   phvwrpair	p.storage_kivec5_status_dma_en, d.status_dma_en, \
   	        p.storage_kivec5_data_len_from_desc, d.data_len_from_desc
   
   // Order of evaluation of next doorbell and interrupts
   // 1. If next_db_en is set => ring the next doorbell and don't raise interrupt
   // 2. If next_db_en is not set and intr_en is set => raise interrupt
   // 2. If next_db_en is not set and intr_en is not set => do nothing more

   // Check if next doorbell is to be enabled and branch
   seq		c1, d.next_db_en, 1
   bcf		![c1], check_intr
   nop

   // Ring the sequencer doorbell based on addr/data provided in the descriptor
   SEQUENCER_DOORBELL_RING(dma_p2m_11)

   // Done ringing doorbell, don't fire the interrupt in this path
   b		tbl_load

check_intr:
   // Check if interrupt is enabled and branch
   seq		c1, d.intr_en, 1
   bcf		![c1], tbl_load
   nop

   // Raise interrupt based on addr/data provided in descriptor
   PCI_RAISE_INTERRUPT(dma_p2m_11)

tbl_load:
   // Setup the start and end DMA pointers
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.status_addr, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_tx_seq_comp_status_handler_start)

