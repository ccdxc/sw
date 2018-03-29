/*****************************************************************************
 *  seq_xts_status_handler:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_xts_status_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_barco_chain_action_start

storage_tx_seq_xts_status_handler_start:

   sne          c1, d.err, r0
   bcf          [c1], xts_error_handle
   nop   

barco_push_check:

   // if Barco ring push is applicable, execute table lock read
   // to get the current ring pindex. Note that this must be done
   // in the same stage as storage_tx_seq_barco_entry_handler_start()
   // which is stage 2.
   bbeq		STORAGE_KIVEC5_NEXT_DB_BARCO_PUSH, 0, all_dma_complete
   nop

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(STORAGE_KIVEC4_BARCO_PNDX_ADDR,
                                STORAGE_KIVEC4_BARCO_PNDX_SIZE,
                                storage_tx_seq_barco_chain_action_start)
                                
all_dma_complete:

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

exit:
   LOAD_NO_TABLES

   
xts_error_handle:

   // if next_db_en and !stop_chain_on_error then ring_db
   seq          c1, STORAGE_KIVEC5_NEXT_DB_EN, 1
   bbeq.c1      STORAGE_KIVEC5_STOP_CHAIN_ON_ERROR, 0, barco_push_check
   nop

   // cancel any barco push prep
   DMA_CMD_CANCEL(dma_m2m_9)
   DMA_CMD_CANCEL(dma_m2m_10)
   DMA_CMD_CANCEL(dma_p2m_11)
   
   // else if intr_en then complete any status DMA and 
   // override doorbell to raising an interrupt
   bbeq         STORAGE_KIVEC5_INTR_EN, 0, exit
   nop

   PCI_SET_INTERRUPT_ADDR_DMA(STORAGE_KIVEC5_INTR_ADDR, dma_p2m_11)
   b            all_dma_complete
   nop

