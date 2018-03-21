/*****************************************************************************
 *  cleanup_iob: Cleanup the I/O buffer by issuing DMA commands to reset the
 *               various sequencer doorbells.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl0_k k;
struct s2_tbl0_cleanup_iob_d d;
struct phv_ p;

%%
   .param storage_nvme_cleanup_io_ctx_start

storage_nvme_cleanup_iob_start:


   // Form the fields needed to be cleaned up in the doorbell q_state
   // in the PHV. p_ndx needs to be set to 1, c_ndx and w_ndx need to
   // be set to 0 (which is the default value of the PHV.
   phvwr	p.doorbell_cleanup_q_state_p_ndx, 1

   // Issue PHV2MEM DMA commands to cleanup the queue states of various doorbells

   // First save the p_ndx offset of first sequencer doorbell in the I/O buffer 
   // into GPR r6. Then for each sequencer add the relative offsets.
   add		r6, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_SEQ_DB_OFFSET
   add		r6, r6, DOORBELL_CLEANUP_Q_STATE_OFFSET

   // Sequencer for Barco XTS encryption doorbell
   add		r7, r6, IO_BUF_SEQ_BARCO_XTS_ENC_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_1)

   // Sequencer for Barco XTS decryption doorbell
   add		r7, r6, IO_BUF_SEQ_BARCO_XTS_DEC_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_2)

   // Sequencer for compression request doorbell
   add		r7, r6, IO_BUF_SEQ_COMP_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_3)

   // Sequencer for decompression request doorbell
   add		r7, r6, IO_BUF_SEQ_DECOMP_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_4)

   // Sequencer for integrity tag generation doorbell
   add		r7, r6, IO_BUF_SEQ_INT_TAG_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_5)

   // Sequencer for dedup tag generation doorbell
   add		r7, r6, IO_BUF_SEQ_DEDUP_TAG_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_6)

   // Sequencer for R2N WQE xfer doorbell
   add		r7, r6, IO_BUF_SEQ_R2N_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_7)

   // Sequencer for PDMA doorbell
   add		r7, r6, IO_BUF_SEQ_PDMA_DB_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_8)

   // Load the IO context for the next stage to modify oper status with table locking
   add		r7, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_IO_CTX_OFFSET
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_cleanup_io_ctx_start)
