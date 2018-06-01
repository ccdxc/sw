/*****************************************************************************
 *  seq_xts_status_handler:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_xts_status_handler_d d;
struct phv_ p;

/*
 * Registers usage:
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
 
%%
   .param storage_seq_barco_ring_pndx_read
   .param storage_seq_xts_comp_len_update
   .param storage_seq_xts_sgl_pdma_xfer

storage_seq_xts_status_handler:

   seq          c4, d.err, r0
   bcf          [!c4], xts_error
   CLEAR_TABLE0 // delay slot

    // Launch read of compression header from the decrypted data buffer
    // (if applicable) so next-in-chain descriptor and SGLs
    // can be updated accordingly.
    bbeq        SEQ_KIVEC5XTS_COMP_LEN_UPDATE_EN, 0, possible_sgl_pdma_xfer
    nop
    LOAD_TABLE2_FOR_ADDR_PC_IMM(SEQ_KIVEC2XTS_DECR_BUF_ADDR,
                                STORAGE_TBL_LOAD_SIZE_64_BITS,
                                storage_seq_xts_comp_len_update)

possible_sgl_pdma_xfer:

    // PDMA decrypted data to user buffers specified in SGL
    bbeq        SEQ_KIVEC5XTS_SGL_PDMA_EN, 0, possible_barco_push
    phvwr       p.seq_kivec3xts_decr_buf_addr, SEQ_KIVEC2XTS_DECR_BUF_ADDR // delay slot
    
    // PDMA compressed data to user buffers specified in SGL
    LOAD_TABLE1_FOR_ADDR_PC_IMM(SEQ_KIVEC2XTS_SGL_PDMA_DST_ADDR, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_xts_sgl_pdma_xfer)
possible_barco_push:

   // if Barco ring push is applicable, execute table lock read
   // to get the current ring pindex. Note that this must be done
   // in the same stage as storage_seq_barco_entry_handler()
   // which is stage 3.
   bbeq		SEQ_KIVEC5XTS_NEXT_DB_ACTION_BARCO_PUSH, 0, all_dma_complete
   nop

   LOAD_TABLE_NO_LKUP_PC_IMM(0, storage_seq_barco_ring_pndx_read)
   
all_dma_complete:

   // Setup the start and end DMA pointers
   DMA_PTR_SETUP_e(dma_p2m_0_dma_cmd_pad,
                   dma_p2m_21_dma_cmd_eop,
                   p4_txdma_intr_dma_cmd_ptr)

xts_error:

   // if next_db_en and !stop_chain_on_error then ring_db
   seq          c5, SEQ_KIVEC5XTS_NEXT_DB_EN, 1
   bbeq.c5      SEQ_KIVEC5XTS_STOP_CHAIN_ON_ERROR, 0, possible_barco_push
   nop

   // cancel any barco push prep
   DMA_CMD_CANCEL(dma_p2m_21)
   
   // else if intr_en then complete any status DMA and 
   // override doorbell to raising an interrupt
   bbeq         SEQ_KIVEC5XTS_INTR_EN, 0, all_dma_complete
   nop

   PCI_SET_INTERRUPT_ADDR_DMA(SEQ_KIVEC5XTS_INTR_ADDR,
                              dma_p2m_21)
   b            all_dma_complete
   nop

