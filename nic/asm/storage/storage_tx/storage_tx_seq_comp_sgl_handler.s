/*****************************************************************************
 *  seq_comp_sgl_handler: Parse the destination SGL and DMA the status,
 *                        data (if status was success) and set the interrupt.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl1_k k;
struct s3_tbl1_seq_comp_sgl_handler_d d;
struct phv_ p;

%%

storage_tx_seq_comp_sgl_handler_start:

   // Store the data length into r6 accouting for the fact that 0 => 64K
   add          r6, STORAGE_KIVEC5_DATA_LEN, r0

   // Setup the compression data buffer DMA based on flat source buffer 
   // and destination SGL.The macro processes one SGL entry and branches
   // if xfer is complete at any point.
   // Notes: These GPRs are used for input/output to/from this macro
   //  1. r6 stores the running count of data remaining to be xfered
   //  2. r7 stores the offeset of the source data buffer from where
   //     the current xfer is to be done.
   //
   // Note that when next_db_action_barco_push is set, dma_m2m_2/3/4
   // are already taken which limits the number of SGLs we can process here.
   bbeq         STORAGE_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH, 1, limited_sgl_case
   add		r7, STORAGE_KIVEC3_DATA_ADDR, r0        // delay slot
      
   COMP_SGL_DMA(dma_m2m_3, dma_m2m_4, d.addr0, d.len0, complete_dma)
   COMP_SGL_DMA(dma_m2m_5, dma_m2m_6, d.addr1, d.len1, complete_dma)
   COMP_SGL_DMA(dma_m2m_7, dma_m2m_8, d.addr2, d.len2, complete_dma)
   COMP_SGL_DMA(dma_m2m_9, dma_m2m_10, d.addr3, d.len3, complete_dma)

xfer_len_check:

   // Catch any driver errors here for debugging, i.e., driver did not 
   // provision the SGL correctly relative to comp output data length.
   bne          r6, r0, pdma_xfer_error
   nop
      
complete_dma:
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Exit the pipeline here
   LOAD_NO_TABLES

limited_sgl_case:
   // Only has enough dma_m2m for one, two, or three transfers
   // because an AOL pad action or SGL pad action have occupied
   // some of the DMA descriptors. So, try the best we could here.
   bbeq         STORAGE_KIVEC5_AOL_PAD_EN, 1, limited_one_sgl_case
   nop
   bbeq         STORAGE_KIVEC5_STATUS_DMA_EN, 0, limited_three_sgl_case
   nop
   COMP_SGL_DMA(dma_m2m_5, dma_m2m_6, d.addr0, d.len0, complete_dma)
   COMP_SGL_DMA(dma_m2m_7, dma_m2m_8, d.addr1, d.len1, complete_dma)
   b            xfer_len_check
   nop

limited_one_sgl_case:
   // Only has enough dma_m2m for one transfer, but if status DMA
   // was not required, we can do two
   bbeq         STORAGE_KIVEC5_STATUS_DMA_EN, 0, limited_two_sgl_case
   nop
   COMP_SGL_DMA(dma_m2m_6, dma_m2m_7, d.addr0, d.len0, complete_dma)
   b            xfer_len_check
   nop

limited_two_sgl_case:
   // Only has enough dma_m2m for two transfers (status DMA not involved)
   COMP_SGL_DMA(dma_m2m_0, dma_m2m_1, d.addr0, d.len0, complete_dma)
   COMP_SGL_DMA(dma_m2m_6, dma_m2m_7, d.addr1, d.len1, complete_dma)
   b            xfer_len_check
   nop

limited_three_sgl_case:
   // Only has enough dma_m2m for three transfers (status DMA not involved)
   COMP_SGL_DMA(dma_m2m_0, dma_m2m_1, d.addr0, d.len0, complete_dma)
   COMP_SGL_DMA(dma_m2m_5, dma_m2m_6, d.addr1, d.len1, complete_dma)
   COMP_SGL_DMA(dma_m2m_7, dma_m2m_8, d.addr2, d.len2, complete_dma)
   b            xfer_len_check
   nop

pdma_xfer_error:
   STORAGE_COMP_SGL_PDMA_XFER_ERROR_TRAP()
   b            complete_dma
   nop
   
