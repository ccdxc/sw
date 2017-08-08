/*****************************************************************************
 * nvme_be_cmd.p4: This models the the NVME backend functionality in the P4+ 
 *                 pipeline of Capri for processing NVME commands and 
 *                 submitting them to the SSD
 *****************************************************************************/

#include "common/dummy.p4"

#define tx_table_s0_t0 nvme_be_sq_ctx
#define tx_table_s1_t0 nvme_be_sq_stage
#define tx_table_s2_t0 nvme_be_sq_ssd_tbl_addr
#define tx_table_s3_t0 nvme_be_ssd_info
#define tx_table_s4_t0 nvme_be_sq_entry1
#define tx_table_s5_t0 nvme_be_sq_entry2
#define tx_table_s6_t0 ssd_cmds
#define tx_table_s7_t0 ssd_sq_ctx

#define tx_table_s0_t0_action qcheck
#define tx_table_s1_t0_action skip
#define tx_table_s2_t0_action load
#define tx_table_s3_t0_action wrr
#define tx_table_s4_t0_action copy
#define tx_table_s5_t0_action qpop
#define tx_table_s6_t0_action save
#define tx_table_s7_t0_action qpush

#include "../common-p4+/common_txdma.p4"
#include "common/capri.h"
#include "common/target.h"


/*****************************************************************************
 *  Stage: Check NVME backend SQ context. If busy bit set, yield control. Else 
 *         set the busy bit, save SQ context to PHV and load SSD info table.
 *****************************************************************************/

action qcheck(idx, state, c_ndx_lo, c_ndx_med, c_ndx_hi, p_ndx_lo,
             p_ndx_med, p_ndx_hi, p_ndx_lo_db, p_ndx_med_db, 
             p_ndx_hi_db, base_addr, num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_QUEUE_CTX_COPY(nsq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.idx, idx);
  modify_field(nvme_tgt_kivec0.state, state);
  modify_field(nvme_tgt_kivec1.base_addr, base_addr);
  modify_field(nvme_tgt_kivec0.num_entries, num_entries);
  modify_field(nvme_tgt_kivec0.paired_q_idx, paired_q_idx);
  modify_field(nvme_tgt_kivec3.c_ndx_lo, c_ndx_lo);
  modify_field(nvme_tgt_kivec3.c_ndx_med, c_ndx_med);
  modify_field(nvme_tgt_kivec3.c_ndx_hi, c_ndx_hi);
  modify_field(nvme_tgt_kivec3.p_ndx_lo, p_ndx_lo);
  modify_field(nvme_tgt_kivec3.p_ndx_med, p_ndx_med);
  modify_field(nvme_tgt_kivec3.p_ndx_hi, p_ndx_hi);

  // If busy state was set or queue is empty for all priorities => yield control
  if (QUEUE_CANT_POP_LO(nsq_ctx) and 
      QUEUE_CANT_POP_MED(nsq_ctx) and
      QUEUE_CANT_POP_HI(nsq_ctx)) {
    exit();
  }

  // Lock the queue 
  _QUEUE_LOCK(nsq_ctx)

  // Move the next (skip) stage without loading a table. This is done to 
  // line up the stages.
  CAPRI_LOAD_NO_TABLE(common_te0_phv, nvme_be_sq_skip_stage)
}


/*****************************************************************************
 *  Stage: Move the next (skip) stage without loading a table. This is done to 
 *         line up the stages.
 *****************************************************************************/

action skip() {
  CAPRI_LOAD_NO_TABLE(common_te0_phv, nvme_be_sq_ssd_tbl_addr_load)
}


/*****************************************************************************
 *  Stage: Load the SSD info table based on the NVME backend SQ idx for 
 *         the next stage to perform WWR
 *****************************************************************************/

action load() {
  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Load the SSD info table in the next stage to perform I/O prioritization
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, SSD_INFO_TABLE_BASE, nvme_tgt_kivec0.idx,
                       SSD_INFO_ENTRY_SIZE, SSD_INFO_ENTRY_SIZE, 
                       nvme_be_sq_pri_wrr)
}


/*****************************************************************************
 *  Stage: Perform weighted round robin between the 3 priority queues and pick
 *         a queue to pop the command from. Load that queue's address for the
 *         next stage. If no command can be popped, exit the pipeline.
 *****************************************************************************/
action wrr(lo_weight, med_weight, hi_weight, lo_running, 
           med_running, hi_running, num_running, max_cmds) {

  // Save the SSD information into PHV
  COPY_SSD_INFO(ssd_info)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)
  NVME_TGT_KIVEC3_USE(storage_scratch, nvme_tgt_kivec3)

  // Reset got_cmd as it will be conditionally filled when servicing queues
  modify_field(nvme_be.got_cmd, 0);

  // Check and service the high priority queue
  SERVICE_NVME_BE_SSD_QUEUE(nvme_be, ssd_info, 
                            nsq_ctx, hi_running, hi_weight, 
                            QUEUE_CANT_POP_HI, QUEUE_LOCK_AND_LOAD_HI,
                            nvme_be_sq_entry_copy)

  // Write the correct c_ndx to K+I vector 
  modify_field(nvme_tgt_kivec0.c_ndx, nvme_tgt_kivec3.c_ndx_hi);

  // Check and service the medium priority queue
  SERVICE_NVME_BE_SSD_QUEUE(nvme_be, ssd_info, 
                            nsq_ctx, med_running, med_weight, 
                            QUEUE_CANT_POP_MED, QUEUE_LOCK_AND_LOAD_MED,
                            nvme_be_sq_entry_copy)

  // Write the correct c_ndx to K+I vector 
  modify_field(nvme_tgt_kivec0.c_ndx, nvme_tgt_kivec3.c_ndx_med);

  // Check and service the low priority queue
  SERVICE_NVME_BE_SSD_QUEUE(nvme_be, ssd_info, 
                            nsq_ctx, lo_running, lo_weight, 
                            QUEUE_CANT_POP_LO, QUEUE_LOCK_AND_LOAD_LO,
                            nvme_be_sq_entry_copy)

  // Write the correct c_ndx to K+I vector 
  modify_field(nvme_tgt_kivec0.c_ndx, nvme_tgt_kivec3.c_ndx_lo);

  // If command to be processed was not obtained, exit the pipeline
  if (nvme_be.got_cmd == 0) {
    exit();

  // Else increment running counter of commands and proceed to process it
  } else {
    modify_field(ssd_info.num_running, num_running + 1);
  }
}


/*****************************************************************************
 *  Stage: Copy the first part of the NVME command into PHV. Load the 
 *         address of the second part of the NVME command in the SQ entry for 
 *         the next stage.
 *****************************************************************************/

action copy(src_queue_id, ssd_handle, io_priority, is_read, cmd_handle) {
  // Carry forward state information in the PHV
  COPY_NVME_CMD1(nvme_be_cmd_p)

  // Set key to load the second part of NVME command in next stage.
  CAPRI_LOAD_TABLE_OFFSET(common_te0_phv, common_te0_phv.table_pc, 
                          R2N_NVME_CMD_OFFSET, R2N_NVME_CMD_SIZE,
                          nvme_be_sq_entry_pop)
}


/*****************************************************************************
 *  Stage: Save second part of the NVME command in SQ entry to PHV. Increment 
 *         consumer index in NVME backend SQ context to pop the entry. Load the 
 *         address of SSD's list of outstanding commands for the next stage.
 *****************************************************************************/

action qpop(nvme_cmd_w0, nvme_cmd_cid, nvme_cmd_hi) {
  // Carry forward state information in the PHV
  COPY_NVME_CMD2(nvme_be_cmd_p)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.cmd_index, 0);
#if 0
  // In ASM, the right side argument is as follows 
  modify_field(nvme_tgt_kivec0.cmd_index, nvme_be_cmd_p.nvme_cmd_cid);
#endif

  // Pop the queue entry based on priority queue from where it was dequeued
  if (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_HI) {
    QUEUE_POP_HI(nsq_ctx, NVME_BE_SQ_CTX_TABLE_BASE)
  }
  if (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_MED) {
    QUEUE_POP_MED(nsq_ctx, NVME_BE_SQ_CTX_TABLE_BASE)
  }
  if (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_LO) {
    QUEUE_POP_LO(nsq_ctx, NVME_BE_SQ_CTX_TABLE_BASE)
  }

  // Load address of the SSD's list of outstanding commands for the next stage 
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, SSD_CMDS_TABLE_BASE,
                       nvme_tgt_kivec0.ssd_handle,
                       SSD_CMDS_ENTRY_SIZE, SSD_CMDS_ENTRY_SIZE, ssd_cmd_save)
}


/*****************************************************************************
 *  Stage: Save NVME command to be sent to the SSD into memory. Load the
 *         address of the SSD's SQ context for the next stage.
 *****************************************************************************/

action save(bitmap) {

  // Carry forward state information in the PHV
  modify_field(ssd_cmds.bitmap, bitmap);
  
  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Get the first free bit in the bitmap and use it as the cmd_index.
  // Writing this in P4 messes up the K+I vector size, so putting this in #if 0
#if 0
  // In ASM: 1. Compute ffs on bitmap and store in GPR r1 (r0 = 0)
  //         2. Table write of r1 into cmd_index 
  ffcvx(r1, d.bitmap, r0);
  tblor(d.bitmap, r1);

  // Set the cmd_index bit in the bitmap to account for it as taken
  modify_field(ssd_cmds.bitmap, bitmap | (0x1 << r1);

  // Set the cmd_index in the CID of Dword0 of the NVME command to be sent
  // to the SSD
  modify_field(nvme_be_cmd_p.nvme_cmd_cid, r1);

  // Save the NVME backend command to memory at the calculated index
  // In ASM, DMA write of the fields from PHV (which are contiguous).
  dmawr(SSD_CMDS_TABLE_BASE +
        (SSD_CMDS_ENTRY_SIZE * nvme_tgt_kivec0.ssd_handle) + 
        SSD_CMDS_HEADER_SIZE + 
        (nvme_tgt_kivec0.cmd_index * NVME_BE_SQ_ENTRY_SIZE),
        nvme_be_cmd_p.src_queue_id .. 
        nvme_be_cmd_p.nvme_cmd_hi);

  // Overwrite the original NVME command id at the calculated index
  // In ASM, DMA write of the fields from PHV (which are contiguous).
  dmawr(SSD_CMDS_TABLE_BASE +
        (SSD_CMDS_ENTRY_SIZE * nvme_tgt_kivec0.ssd_handle) + 
        SSD_CMDS_HEADER_SIZE + 
        (nvme_tgt_kivec0.cmd_index * NVME_BE_SQ_ENTRY_SIZE) +
        SSD_CMD_CID_OFFSET,
        nvme_tgt_kivec0.cmd_index .. 
        nvme_tgt_kivec0.cmd_index);
#endif
 
  // Initialize the fields so that it will appear in the auto generated PHV
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd1)
  

  // Load the address of the SSD's SQ for the next stage 
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, SSD_SQ_CTX_TABLE_BASE,
                       nvme_tgt_kivec0.ssd_handle,
                       Q_CTX_SIZE, Q_CTX_SIZE, ssd_sq_entry_push)
}


/*****************************************************************************
 *  Stage: Push NVME command to SSD's SQ and ring the doorbell
 *****************************************************************************/

action qpush(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, base_addr,
             num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(ssq_ctx)

  // Initialize the fields so that it will appear in the auto generated PHV
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd2)
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd3)

  // Check for queue full condition before pushing
  if (QUEUE_CANT_PUSH(ssq_ctx)) {

    // Exit pipeline here without error handling for now. This event of SSD SQ
    // being full should never happen as software will program the max_cmds to
    // be popped to a value less than the SSD's SQ size.
    exit();

  } else {

#if 0
    // Copy NVME command to SSD SQ memory. In ASM, DMA write 
    // of the fields from PHV (which are contiguous).
    dmawr(ssq_ctx.base_addr + 
          (ssq_ctx.p_ndx * SSD_SQ_ENTRY_SIZE),
          nvme_be_cmd_p.nvme_cmd_lo .. 
          nvme_be_cmd_p.nvme_cmd_hi);
#endif


    // Push the entry to the queue (which also rings the doorbell)
    QUEUE_PUSH(ssq_ctx)

    // Exit the pipeline here 
  }
}
