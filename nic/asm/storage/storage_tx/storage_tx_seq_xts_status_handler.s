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

storage_tx_seq_xts_status_handler_start:

   // See if should exit chain on error
   seq          c1, d.err, r0
   seq.!c1      c1, STORAGE_KIVEC5_EXIT_CHAIN_ON_ERROR, 1
   seq.c1       c1, STORAGE_KIVEC5_NEXT_DB_EN, 1
   bcf          [c1], exit

   // Complete all DMA or only status DMA
   seq          c1, STORAGE_KIVEC5_INTR_EN, 1           // delay slot
   seq          c2, STORAGE_KIVEC5_NEXT_DB_EN, 1
   bcf          [c1 | c2], complete_all_dma
   seq          c3, STORAGE_KIVEC5_STATUS_DMA_EN, 0     // delay slot
   bcf          [c3], exit
   nop   
   
complete_status_dma:
    
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_m2m_2_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES

complete_all_dma:
                 
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
exit:
   LOAD_NO_TABLES
