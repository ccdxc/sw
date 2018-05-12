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
   .param storage_seq_barco_chain_action

storage_seq_xts_status_handler:

   sne          c1, d.err, r0
   bcf          [c1], xts_error
   CLEAR_TABLE0 // delay slot

possible_barco_push:

   // if Barco ring push is applicable, execute table lock read
   // to get the current ring pindex. Note that this must be done
   // in the same stage as storage_seq_barco_entry_handler()
   // which is stage 2.
   bbeq		SEQ_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH, 0, all_dma_complete
   nop

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM_CONT(SEQ_KIVEC4_BARCO_PNDX_SHADOW_ADDR,
                                     SEQ_KIVEC4_BARCO_PNDX_SIZE,
                                     storage_seq_barco_chain_action)
                                
all_dma_complete:

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP_e(dma_p2m_0_dma_cmd_pad,
                   dma_p2m_21_dma_cmd_eop,
                   p4_txdma_intr_dma_cmd_ptr)

xts_error:

   // if next_db_en and !stop_chain_on_error then ring_db
   seq          c1, SEQ_KIVEC5_NEXT_DB_EN, 1
   bbeq.c1      SEQ_KIVEC5_STOP_CHAIN_ON_ERROR, 0, possible_barco_push
   nop

   // cancel any barco push prep
   DMA_CMD_CANCEL(dma_m2m_19)
   DMA_CMD_CANCEL(dma_m2m_20)
   DMA_CMD_CANCEL(dma_p2m_21)
   
   // else if intr_en then complete any status DMA and 
   // override doorbell to raising an interrupt
   bbeq         SEQ_KIVEC5_INTR_EN, 0, all_dma_complete
   nop

   PCI_SET_INTERRUPT_ADDR_DMA(SEQ_KIVEC5_INTR_ADDR,
                              dma_p2m_21)
   b            all_dma_complete
   nop

