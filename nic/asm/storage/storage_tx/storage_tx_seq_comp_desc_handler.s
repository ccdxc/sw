/*****************************************************************************
 *  seq_comp_desc_handler: Handle the compression descriptor entry in the
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
struct s1_tbl_seq_comp_desc_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_comp_status_handler_start

storage_tx_seq_comp_desc_handler_start:

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_UPDATE

   // Store the various parts of the descriptor in the K+I vectors for later use
   phvwr	p.storage_kivec4_sgl_addr, d.sgl_addr
   phvwr	p.storage_kivec4_data_addr, d.data_addr
   phvwr	p.storage_kivec5_status_addr, d.status_addr
   phvwr	p.storage_kivec5_status_len, d.status_len
   phvwr	p.storage_kivec5_status_dma_en, d.status_dma_en
   phvwr	p.storage_kivec6_intr_addr, d.intr_addr
   phvwr	p.storage_kivec6_intr_data, d.intr_data
  
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(d.status_addr, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                             storage_tx_seq_comp_status_handler_start)

