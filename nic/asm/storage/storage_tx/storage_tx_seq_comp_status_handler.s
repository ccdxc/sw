/*****************************************************************************
 *  seq_comp_status_handler: Store the compression status in K+I vector. Load
 *                           SGL address for next stage to do the PDMA.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_comp_status_handler_d d;
struct phv_ p;

/*
 * Registers usage
 */
#define r_comp_data_len             r1  // compression output data length
#define r_total_len                 r2  // above length plus padding
#define r_pad_len                   r3  // padding length
#define r_pad_boundary              r4  // user specified padding boundary
#define r_status                    r5  // comp status

%%
   .param storage_tx_seq_barco_chain_action_start
   .param storage_tx_seq_comp_sgl_handler_start

storage_tx_seq_comp_status_handler_start:
   
   // bit 15: valid bit, bits 14-12: error bits
   add          r_status, d.status, r0
   smeqh        c1, r_status, 0xf000, 0x8000
   bcf          [!c1], comp_error_handle
   
   // AOL padding makes sense only when next_db_en is true, with the
   // assumption that the next db handler would act on the AOL.
   //
   // Note: output_data_len contains compressed data length plus header length.
   seq		c3, STORAGE_KIVEC5_DATA_LEN_FROM_DESC, 1        // delay slot
   cmov         r_comp_data_len, c3, STORAGE_KIVEC5_DATA_LEN, d.output_data_len
   bbeq         STORAGE_KIVEC5_AOL_PAD_XFER_EN, 0, sgl_xfer_handle
   phvwr	p.storage_kivec5_data_len, r_comp_data_len      // delay slot
   
   // AOL padding: calculate:
   // r_total_len = ((r_comp_data_len + r_pad_boundary - 1) / r_pad_boundary) * r_pad_boundary
   // r_pad_len = r_total_len - r_comp_data_len
   //
   // TODO: can AOL L0/L1/L2 specify any byte size, 
   //       or do they have to conform to some block size?
   sll          r_pad_boundary, 1, STORAGE_KIVEC5_PAD_LEN_SHIFT
   add          r_total_len, r_comp_data_len, r_pad_boundary
   sub          r_total_len, r_total_len, 1
   srl          r_total_len, r_total_len, STORAGE_KIVEC5_PAD_LEN_SHIFT
   sll          r_total_len, r_total_len, STORAGE_KIVEC5_PAD_LEN_SHIFT
   sub          r_pad_len, r_total_len, r_comp_data_len
   
   phvwrpair    p.data_len_len, r_comp_data_len.wx, \
                p.pad_len_len, r_pad_len.wx
   phvwr        p.total_len_len, r_total_len.wx

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
   
comp_error_handle:

   // TODO: if copy_src_dst_on_error wss set, copy header (presumably containing
   // an error header) plus source data to destination.
   //
   
   // cancel any AOL DMA
   bbeq         STORAGE_KIVEC5_AOL_PAD_XFER_EN, 0, stop_chain_check
   seq          c1, STORAGE_KIVEC5_NEXT_DB_EN, 1 // delay slot
   
   DMA_CMD_CANCEL(dma_p2m_3)
   DMA_CMD_CANCEL(dma_p2m_4)
   DMA_CMD_CANCEL(dma_p2m_5)

stop_chain_check:
   
   // if next_db_en and !stop_chain_on_error then ring_db
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
      
sgl_xfer_handle:
   
   // If SGL xfer is not applicable, end the DMA with the
   // previously set up status DMA and doorbell/intr setup.
   bbeq         STORAGE_KIVEC5_SGL_XFER_EN, 0, barco_push_check
   nop
   
   // Set the table and program address 
   LOAD_TABLE1_FOR_ADDR_PC_IMM_e(STORAGE_KIVEC2_SGL_OUT_AOL_ADDR, 
                                 STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                 storage_tx_seq_comp_sgl_handler_start)
