/******************************************************************************
 * r2n_comp.p4: This models the the roce2nvme functionality in the P4+ pipeline
 *              of Capri for processing completions from the NVME backend
 ******************************************************************************/

#include "common/dummy.p4"

#define tx_table_s0_t0 nvme_be_cq_ctx
#define tx_table_s1_t0 nvme_be_cq_entry
#define tx_table_s2_t0 nvme_be_r2n_sq
#define tx_table_s3_t0 r2n_sq_ctx

#define tx_table_s0_t0_action qcheck
#define tx_table_s1_t0_action qpop
#define tx_table_s2_t0_action derive
#define tx_table_s3_t0_action qpush
#define tx_table_s4_t0_action dummy
#define tx_table_s5_t0_action dummy
#define tx_table_s6_t0_action dummy
#define tx_table_s7_t0_action dummy

#include "../common-p4+/common_txdma.p4"
#include "common/capri.h"
#include "common/target.h"
#include "common/r2n_common.p4"


/*****************************************************************************
 *  Stage: Check NVME backend CQ context. If busy bit set, yield control. Else
 *         set the busy bit, save CQ context to PHV and load the CQ entry.
 *****************************************************************************/

action qcheck(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, base_addr,
              num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(ncq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.idx, idx);              
  modify_field(nvme_tgt_kivec0.state, state);          
  modify_field(nvme_tgt_kivec0.c_ndx, c_ndx);
  modify_field(nvme_tgt_kivec1.base_addr, base_addr);
  modify_field(nvme_tgt_kivec0.num_entries, num_entries);
  modify_field(nvme_tgt_kivec0.paired_q_idx, paired_q_idx);

  // If busy state was set or queue is empty => yield contol
  if (QUEUE_CANT_POP(ncq_ctx)) {
    exit();
  }

  // Can pop => process the CQ context by locking it and loading 
  // the address for next stage
  QUEUE_LOCK_AND_LOAD(ncq_ctx, common_te0_phv, NVME_BE_CQ_ENTRY_SIZE,
                      NVME_BE_CQ_ENTRY_SIZE, nvme_be_cq_entry_pop)
}

/*****************************************************************************
 *  Stage: Save NVME backend CQ entry to PHV. Increment consumer index in NVME 
 *         backend CQ context to pop the entry. Load the ROCE NVME command to
 *         figure out actions for next stage.
 *****************************************************************************/

action qpop(time_us, be_status, is_q0, rsvd, cmd_handle, nvme_status) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(nvme_be_resp.time_us, time_us);
  modify_field(nvme_be_resp.be_status, be_status);
  modify_field(nvme_be_resp.is_q0, is_q0);
  modify_field(nvme_be_resp.rsvd, rsvd);
  modify_field(nvme_be_resp.cmd_handle, cmd_handle);
  modify_field(nvme_be_resp.nvme_status, nvme_status);

  // Save ROCE buffer header address in a special place for offset computation
  modify_field(r2n.rbuf_hdr_addr, cmd_handle);

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec1.rbuf_hdr_addr, cmd_handle);

#if 0
    // Form error status in ROCE Rx buffer status. In ASM, memory write of
    // the fixed fields with immediate values.
    // TODO: Check if this is better done with a table write.
    memwr(r2n.rbuf_hdr_addr + R2N_RX_BUF_BE_STA_STATUS_OFFSET,
          NVME_BE_STATUS_GOOD);
#endif

  // Pop the queue entry
  QUEUE_POP(ncq_ctx, NVME_BE_CQ_CTX_TABLE_BASE)

  // Write the ROCE NVME command pointer to address to read in next stage.
  // Command is at an offset into the ROCE buffer.
  CAPRI_LOAD_TABLE_OFFSET(common_te0_phv, r2n.rbuf_hdr_addr, 
                          R2N_RX_BUF_ENTRY_BE_CMD_OFFSET,
                          R2N_RX_BUF_ENTRY_BE_CMD_HDR_SIZE,
                          nvme_be_r2n_sq_derive)
}

/*****************************************************************************
 *  Stage: Derive the R2N SQ from the saved R2N command
 *****************************************************************************/

action derive(src_queue_id, ssd_handle, io_priority, is_read, cmd_handle) {

  // For D vector generation (type inference). No need to translate this to ASM.
  COPY_NVME_CMD1(nvme_be_cmd)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.is_read, is_read);

  // Save key to load the ROCE SQ to return read data (if any) and status 
  // back to initiator
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, R2N_SQ_CTX_TABLE_BASE,
                       src_queue_id,
                       Q_CTX_SIZE, Q_CTX_SIZE, r2n_sq_entry_push)
}

/*****************************************************************************
 *  Stage: Push RDMA write (if any for read data) and status to initiator over 
 *         ROCE SQ. This may require 2 producer indices for atomic operation.
 *****************************************************************************/

action qpush(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
             base_addr, num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(rsq_ctx)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Intialize DMA commands so that fields are present in the PHV
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
  CAPRI_MEM2MEM_DMA_INIT(dma_cmd4)
  CAPRI_MEM2MEM_DMA_INIT(dma_cmd5)
  CAPRI_MEM2MEM_DMA_INIT(dma_cmd6)
  CAPRI_MEM2MEM_DMA_INIT(dma_cmd7)

  // Check for queue full condition before pushing. If read data needs
  // to be sent ask for 2 producer indices to be available.
  if (((nvme_tgt_kivec0.is_read == 1) and
       QUEUE_CANT_PUSH2(rsq_ctx)) or
       QUEUE_CANT_PUSH(rsq_ctx)) {

    // Exit pipeline here without error handling for now. This event of ROCE RQ
    // being full is so rare that it may not require special handling.
    exit();

  } 

  // Check if read data needs to be sent back
  if (nvme_tgt_kivec0.is_read == 1) {
#if 0
    // Copy Read data (via RDMA write) to ROCE SQ memory. In ASM it is a 
    // Mem2Mem DMA write.
    dmawr(base_addr + (p_ndx * R2N_SQ_ENTRY_SIZE),
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET ..
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET +
          R2N_RX_BUF_ENTRY_WRITE_REQ_SIZE)
#endif
    QUEUE_PUSH(rsq_ctx)
  }

#if 0
  // Copy NVME backend status to ROCE SQ memory. In ASM it is a 
  // Mem2Mem DMA write.
  dmawr(base_addr + (p_ndx * R2N_SQ_ENTRY_SIZE),
        nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_STATUS_REQ_OFFSET ..
        nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_STATUS_REQ_OFFSET +
        R2N_RX_BUF_ENTRY_STATUS_REQ_SIZE)
#endif

  // Push the entry to the queue
  QUEUE_PUSH(rsq_ctx)
}
