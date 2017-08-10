/*****************************************************************************
 * nvme_be_comp.p4: This models the the NVME backend functionality in the P4+ 
 *                  pipeline of Capri for processing NVME completions and 
 *                  submitting them back to ROCE2NVME
 *****************************************************************************/

#include "common/dummy.p4"

#define tx_table_s0_t0 ssd_cq_ctx
#define tx_table_s1_t0 ssd_cq_entry
#define tx_table_s2_t0 ssd_saved_cmd
#define tx_table_s3_t0 ssd_info
#define tx_table_s4_t0 ssd_saved_cmd_stage
#define tx_table_s5_t0 ssd_saved_cmd_tbl_addr
#define tx_table_s6_t0 ssd_cmds
#define tx_table_s7_t0 nvme_be_cq_ctx

#define tx_table_s0_t0_action qcheck
#define tx_table_s1_t0_action qpop
#define tx_table_s2_t0_action handle
#define tx_table_s3_t0_action update_weights
#define tx_table_s4_t0_action skip
#define tx_table_s5_t0_action load
#define tx_table_s6_t0_action release
#define tx_table_s7_t0_action qpush

#include "../common-p4+/common_txdma.p4"
#include "common/capri.h"
#include "common/target.h"


/*****************************************************************************
 *  Stage: Check SSD CQ context. If busy bit set, yield control. Else 
 *         set the busy bit, save SQ context to PHV and load CQ entry.
 *****************************************************************************/

action qcheck(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
              base_addr, num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(scq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.idx, idx);
  modify_field(nvme_tgt_kivec0.state, state);
  modify_field(nvme_tgt_kivec0.c_ndx, c_ndx);
  modify_field(nvme_tgt_kivec1.base_addr, base_addr);
  modify_field(nvme_tgt_kivec0.num_entries, num_entries);
  modify_field(nvme_tgt_kivec0.paired_q_idx, paired_q_idx);

  // If busy state was set or queue is empty => yield control
  if (QUEUE_CANT_POP(scq_ctx)) {
    exit();

  // Process the CQ context by locking it and loading the address for next stage
  } else {
    QUEUE_LOCK_AND_LOAD(scq_ctx, common_te0_phv, SSD_CQ_ENTRY_SIZE,
                        SSD_CQ_ENTRY_SIZE, ssd_cq_entry_pop)
  }
}


/*****************************************************************************
 *  Stage: Save the NVME status entry in CQ to PHV. Increment consumer index in 
 *         CQ context to pop the entry. Load the address of the command index
 *         that was allocated in the SSD saved command list for the next stage.
 *****************************************************************************/

action qpop(nvme_sta_lo, nvme_sta_cid, nvme_sta_w7) {

  // Carry forward state information in the PHV
  COPY_NVME_STATUS(nvme_be_resp_p)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Derive the save command index from the NVME status
  modify_field(nvme_tgt_kivec0.cmd_index, (0xFF & nvme_sta_cid));

  // Pop the entry from the SSD CQ
  QUEUE_POP(scq_ctx, SSD_CQ_CTX_TABLE_BASE)

  // Load the address of the saved NVME backend command for the next stage.
  // TODO: Verify if idx of SSD CQ is the same as idx of NVME backend SQ
  //       Else this logic won't work
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, 
                       SSD_CMDS_TABLE_BASE +
                       (nvme_tgt_kivec0.idx * SSD_CMDS_ENTRY_SIZE) +
                       SSD_CMDS_HEADER_SIZE,
                       (0xFF & nvme_sta_cid), NVME_BE_SQ_ENTRY_SIZE,
                       R2N_NVME_HDR_SIZE, ssd_saved_cmd_handle)
}


/*****************************************************************************
 *  Stage: Read and process the NVME backend command that was saved before 
 *         sending to SSD. Form NVME backend status based on this. Load the 
 *         address of the SSD info table to update the running counters in the
 *         next stage.
 *****************************************************************************/

action handle(src_queue_id, ssd_handle, io_priority, is_read, cmd_handle) {

  // For D vector generation (type inference). No need to translate this to ASM.
  COPY_NVME_CMD1(nvme_be_cmd)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Set the fields in the response. TODO: Fill time_us
  modify_field(nvme_be_resp_p.time_us, 0);
  modify_field(nvme_be_resp_p.be_status, 0);
  modify_field(nvme_be_resp_p.rsvd, 0);
  modify_field(nvme_be_resp_p.cmd_handle, cmd_handle);
  if (src_queue_id == 0) {
    modify_field(nvme_be_resp_p.is_q0, 1);
  } else {
    modify_field(nvme_be_resp_p.is_q0, 0);
  }

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.io_priority, io_priority);

  // Load the address of the SSD info for the next stage
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, SSD_INFO_TABLE_BASE, 
                       nvme_tgt_kivec0.idx, SSD_INFO_ENTRY_SIZE, 
                       SSD_INFO_ENTRY_SIZE, ssd_info_update)
}


/*****************************************************************************
 *  Stage: Update the SSD info table with the running counters based on 
 *         I/O command priority. Load the address of the bitmap of the 
 *         saved commands to the SSD for next stage to release.
 *****************************************************************************/

action update_weights(lo_weight, med_weight, hi_weight, lo_running, 
                      med_running, hi_running, num_running, max_cmds) {

  // For D vector generation (type inference). No need to translate this to ASM.
  COPY_SSD_INFO(ssd_info)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Pop the queue entry based on priority queue from where it was dequeued
  if (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_HI) {
    modify_field(ssd_info.hi_running,
                 ssd_info.hi_running - 1);
    modify_field(ssd_info.num_running,
                 ssd_info.num_running - 1);
  }
  if (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_MED) {
    modify_field(ssd_info.med_running,
                 ssd_info.med_running - 1);
    modify_field(ssd_info.num_running,
                 ssd_info.num_running - 1);
  }
  if (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_LO) {
    modify_field(ssd_info.lo_running,
                 ssd_info.lo_running - 1);
    modify_field(ssd_info.num_running,
                 ssd_info.num_running - 1);
  }

  // Move the next (skip) stage without loading a table. This is done to 
  // line up the stages.
  CAPRI_LOAD_NO_TABLE(common_te0_phv, ssd_saved_cmd_skip_stage)
}


/*****************************************************************************
 *  Stage: Move the next (skip) stage without loading a table. This is done to 
 *         line up the stages.
 *****************************************************************************/

action skip() {
  CAPRI_LOAD_NO_TABLE(common_te0_phv, ssd_saved_cmd_tbl_addr_load)
}


/*****************************************************************************
 *  Stage: Load the SSD saved command table for the next stage to release it
 *****************************************************************************/

action load() {

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Load the address of the bitmap of the saved NVME backend command for 
  // the next stage
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, SSD_CMDS_TABLE_BASE, 
                       nvme_tgt_kivec0.idx, SSD_CMDS_ENTRY_SIZE, 
                       SSD_CMDS_HEADER_SIZE, ssd_saved_cmd_release)
}


/*****************************************************************************
 *  Stage: Release the memory which contained the saved NVME command that was
 *         sent to the SSD by freeing the bit in the bitmap. Load the address
 *         of the NVME backend's CQ context for the next stage.
 *****************************************************************************/

action release(bitmap) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(ssd_cmds.bitmap, bitmap);
  
  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Reset the cmd_index bit in the bitmap to account for it as free
  modify_field(ssd_cmds.bitmap, 
               bitmap & (~(0x1 << nvme_be.cmd_index)));

  // Load the address of the SSD's SQ for the next stage 
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, NVME_BE_CQ_CTX_TABLE_BASE, 
                       nvme_tgt_kivec0.paired_q_idx, Q_CTX_SIZE, Q_CTX_SIZE, 
                       nvme_be_cq_entry_push)
}


/*****************************************************************************
 *  Stage: Push NVME backend status to Roce2Nvme over the NVME backend CQ
 *****************************************************************************/

action qpush(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, base_addr,
             num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(ncq_ctx)

  // Check for queue full condition before pushing
  if (QUEUE_CANT_PUSH(ncq_ctx)) {

    // Exit pipeline here without error handling for now. This event of SSD SQ
    // being full should never happen as software will program the max_cmds to
    // be popped to a value less than the SSD's SQ size.
    exit();

  } else {

#if 0
    // Copy NVME status to SSD SQ memory. In ASM, DMA write 
    // of the fields from PHV (which are contiguous).
    dmawr(ncq_ctx.base_addr + 
          (ncq_ctx.p_ndx * NVME_BE_CQ_ENTRY_SIZE),
          nvme_be_resp_p.time_us ..
          nvme_be_resp_p.nvme_status_w7);
#endif

    // Push the entry to the queue 
    QUEUE_PUSH(ncq_ctx)

    // Initialize the fields so that it will appear in the auto generated PHV
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd1)

    // Exit the pipeline here after scheduling bit for the next pipeline
    //modify_field(scheduler.sched_bit, SCHED_BIT_R2N_COMP);
  }
}
