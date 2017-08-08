/*****************************************************************************
 * r2n_hbuf_post.p4: This models the the roce2nvme functionality in the P4+ 
 *                   pipeline of Capri for processing Rx buffer posts from the 
 *                   host side
 *****************************************************************************/

#include "common/dummy.p4"

#define tx_table_s0_t0 r2n_hq_ctx
#define tx_table_s1_t0 r2n_hq_entry
#define tx_table_s2_t0 r2n_rq_tbl_addr
#define tx_table_s3_t0 r2n_rq_ctx

#define tx_table_s0_t0_action qcheck
#define tx_table_s1_t0_action qpop
#define tx_table_s2_t0_action load
#define tx_table_s3_t0_action rq_push
#define tx_table_s4_t0_action dummy
#define tx_table_s5_t0_action dummy
#define tx_table_s6_t0_action dummy
#define tx_table_s7_t0_action dummy

#include "../common-p4+/common_txdma.p4"
#include "common/capri.h"
#include "common/target.h"
#include "common/r2n_common.p4"


/*****************************************************************************
 *  Stage: Check ROCE HQ context. If busy bit set, yield control. Else set the
 *         busy bit, save HQ context to PHV and load the HQ entry.
 *****************************************************************************/

action qcheck(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
              base_addr, num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(rhq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.idx, idx);
  modify_field(nvme_tgt_kivec0.state, state);
  modify_field(nvme_tgt_kivec0.c_ndx, c_ndx);
  modify_field(nvme_tgt_kivec1.base_addr, base_addr);
  modify_field(nvme_tgt_kivec0.num_entries, num_entries);
  modify_field(nvme_tgt_kivec0.paired_q_idx, paired_q_idx);

  // If busy state was set or queue is empty => yield contol
  if (QUEUE_CANT_POP(rhq_ctx)) {
    exit();
  } else {
    // Can pop => process the HQ context by locking it and 
    // loading the address for next stage
    QUEUE_LOCK_AND_LOAD(rhq_ctx, common_te0_phv, R2N_RX_BUF_POST_ENTRY_SIZE,
                        R2N_RX_BUF_POST_ENTRY_SIZE, r2n_cq_entry_pop)
  }
}

/*****************************************************************************
 *  Stage: Save ROCE HQ entry to PHV Increment consumer index in ROCE HQ context
 *         to pop the entry. Parse the HQ entry completion handle to determine 
 *         actions for next stage.
 *****************************************************************************/

action qpop(cmd_handle, local_addr, data_size) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(rbuf_post.cmd_handle, cmd_handle);
  modify_field(rbuf_post.local_addr, local_addr);
  modify_field(rbuf_post.data_size, data_size);

  // Store ROCE buffer header address for offset computation
  modify_field(r2n.rbuf_hdr_addr, cmd_handle);

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec1.rbuf_hdr_addr, cmd_handle);

  // Pop the queue entry
  QUEUE_POP(rhq_ctx, R2N_HQ_CTX_TABLE_BASE)

  // Move the next (skip) stage without loading a table. This is done to 
  // line up the stages for a push operation.
  CAPRI_LOAD_NO_TABLE(common_te0_phv, r2n_rq_tbl_addr_load)
}

/*****************************************************************************
 *  Stage: Load the ROCE RQ context table for the next stage to push the Rx 
 *         buffer
 *****************************************************************************/

action load() {
  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Set the key to load the ROCE Receive queue context
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, R2N_RQ_CTX_TABLE_BASE, 
                       nvme_tgt_kivec0.paired_q_idx,
                       Q_CTX_SIZE, Q_CTX_SIZE, r2n_rq_entry_push)
}
