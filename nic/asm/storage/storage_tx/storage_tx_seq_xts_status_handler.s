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

   seq          c1, d.err, r0
   bcf          [c1], all_dma_complete
   
   // XTS error case, if next_db_en and !stop_chain_on_error then ring_db
   seq          c1, STORAGE_KIVEC5_NEXT_DB_EN, 1        // delay slot
   bbeq.c1      STORAGE_KIVEC5_STOP_CHAIN_ON_ERROR, 0, all_dma_complete
   nop

   // else if intr_en then complete any status DMA and 
   // override doorbell to raising an interrupt
   bbeq         STORAGE_KIVEC5_INTR_EN, 0, exit
   nop
   PCI_SET_INTERRUPT_ADDR_DMA(STORAGE_KIVEC5_INTR_ADDR, dma_p2m_11)

all_dma_complete:

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

exit:
   LOAD_NO_TABLES
   
