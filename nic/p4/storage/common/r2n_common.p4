/*****************************************************************************
 * r2n_common.p4: This contains P4 table match/actions to implement common
 *                action handlers for Roce2NVME P4+ pipeline.  Per P4 semantics 
 *                the table names have to different if called more than once in 
 *                same control flow.
 *****************************************************************************/

#include "target.h"


/*****************************************************************************
 *  Stage: Push Rx buffer address to ROCE RQ
 *****************************************************************************/

action rq_push(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, base_addr,
               num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(rrq_ctx)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Check for queue full condition before pushing
  if (QUEUE_CANT_PUSH(rrq_ctx)) {

    // Exit pipeline here without error handling for now. This event of ROCE RQ
    // being full is so rare that it may not require special handling.
    exit();

  } else {

#if 0
    // Copy ROCE NVME command to NVME backend SQ memory. In ASM, DMA write 
    // of the fields from PHV (which are contiguous)
    dmawr(rrq_ctx_base_addr + (rrq_ctx_p_ndx * R2N_RX_BUF_POST_ENTRY_SIZE),
          rbuf_post.cmd_handle .. 
          rbuf_post.data_size);
#endif

    // Initialize the fields so that it will appear in the auto generated PHV
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd1)

    // Push the entry to the queue
    QUEUE_PUSH(rrq_ctx)

    // Exit the pipeline here after scheduling bit for the next pipeline
    //modify_field(scheduler.sched_bit, SCHED_BIT_ROCE);
  }
}
