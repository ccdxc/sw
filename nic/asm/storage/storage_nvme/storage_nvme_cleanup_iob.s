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
   // in the PHV. p_ndx, c_ndx and w_ndx need to be set to 0 (which 
   // is the default value of the PHV).

   // Issue PHV2MEM DMA commands to cleanup the queue states of various doorbells
   // TODO: Enable the ones under if 0 as and when they are setup for use

#if 0
   // Sequencer for Barco XTS encryption doorbell
   add		r7, d.xts_enc, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_1)

   // Sequencer for Barco XTS decryption doorbell
   add		r7, d.xts_dec, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_2)

   // Sequencer for compression request doorbell
   add		r7, d.comp, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_3)

   // Sequencer for decompression request doorbell
   add		r7, d.decomp, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_4)

   // Sequencer for integrity tag generation doorbell
   add		r7, d.int_tag, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_5)

   // Sequencer for dedup tag generation doorbell
   add		r7, d.dedup_tag, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_6)
#endif

   // Sequencer for R2N WQE xfer doorbell
   add		r7, d.r2n, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_7)

#if 0
   // Sequencer for PDMA doorbell
   add		r7, d.pdma, DOORBELL_CLEANUP_Q_STATE_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(doorbell_cleanup_q_state_p_ndx,
                            doorbell_cleanup_q_state_w_ndx, 
                            r7, dma_p2m_8)
#endif

   // Load the IO context for the next stage to modify oper status with table locking
   add		r7, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_IO_CTX_OFFSET
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_cleanup_io_ctx_start)
