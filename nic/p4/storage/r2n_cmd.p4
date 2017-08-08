/*****************************************************************************
 * r2n_cmd.p4: This models the the roce2nvme functionality in the P4+ pipeline
 *             of Capri for processing NVME commands from ROCE layer
 *****************************************************************************/

#include "common/dummy.p4"

#define tx_table_s0_t0 r2n_cq_ctx
#define tx_table_s1_t0 r2n_cq_entry
#define tx_table_s2_t0 r2n_nvme_cmd
#define tx_table_s3_t0 nvme_be_push_sq_ctx
#define tx_table_s4_t0 pvm_errq_ctx
#define tx_table_s5_t0 r2n_cq_rx_buf
#define tx_table_s6_t0 r2n_rq_ctx

#define tx_table_s0_t0_action qcheck
#define tx_table_s1_t0_action qpop
#define tx_table_s2_t0_action handle
#define tx_table_s3_t0_action sq_push
#define tx_table_s4_t0_action errq_push
#define tx_table_s5_t0_action get
#define tx_table_s6_t0_action rq_push
#define tx_table_s7_t0_action dummy

#include "../common-p4+/common_txdma.p4"
#include "common/capri.h"
#include "common/target.h"
#include "common/r2n_common.p4"


/*****************************************************************************
 *  Stage: Check ROCE CQ context. If busy bit set, yield control. Else set the
 *         busy bit, save CQ context to PHV and load the CQ entry.
 *****************************************************************************/

action qcheck(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, base_addr,
              num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(rcq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.idx, idx);              
  modify_field(nvme_tgt_kivec0.state, state);          
  modify_field(nvme_tgt_kivec0.c_ndx, c_ndx);
  modify_field(nvme_tgt_kivec1.base_addr, base_addr);
  modify_field(nvme_tgt_kivec0.num_entries, num_entries);
  modify_field(nvme_tgt_kivec0.paired_q_idx, paired_q_idx);

  // If busy state was set or queue is empty => yield contol
  if (QUEUE_CANT_POP(rcq_ctx)) {
    exit();
  } else {
    // Can pop => process the CQ context by locking it and loading the address 
    // for next stage
    QUEUE_LOCK_AND_LOAD(rcq_ctx, common_te0_phv, R2N_CQ_ENTRY_SIZE,
                        R2N_CQ_ENTRY_SIZE, r2n_cq_entry_pop)
  }
}

/*****************************************************************************
 *  Stage: Save ROCE CQ entry to PHV Increment consumer index in ROCE CQ context
 *         to pop the entry. Parse the CQ entry completion handle to determine 
 *         actions for next stage.
 *****************************************************************************/

action qpop(comp_type, rsvd, cmd_handle, status, qp_index) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(rcq_ent.comp_type, comp_type);
  modify_field(rcq_ent.rsvd, rsvd);
  modify_field(rcq_ent.cmd_handle, cmd_handle);
  modify_field(rcq_ent.status, status);
  modify_field(rcq_ent.qp_index, qp_index);

  // Store ROCE buffer header address for offset computation
  modify_field(r2n.rbuf_hdr_addr, cmd_handle);

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec1.rbuf_hdr_addr, cmd_handle);
  modify_field(nvme_tgt_kivec2.qp_index, qp_index);

  // Pop the queue entry
  QUEUE_POP(rcq_ctx, R2N_CQ_CTX_TABLE_BASE)

  // Handle new NVME command
  if (comp_type == R2N_COMP_TYPE_NEW_BUFFER) {
    // Set key to load the ROCE NVME command header in next stage.
    // Command is at an offset into the ROCE Rx buffer.
    CAPRI_LOAD_TABLE_OFFSET(common_te0_phv, r2n.rbuf_hdr_addr, 
                            R2N_RX_BUF_ENTRY_BE_CMD_OFFSET,
                            R2N_RX_BUF_ENTRY_BE_CMD_SIZE,
                            r2n_nvme_cmd_handle)


    // Set the PHV bit to handle new ROCE NVME command
    modify_field(r2n.new_cmd, 1);
  }

  // Handle status xfer completion
  if ((comp_type == R2N_COMP_TYPE_SEND_REQ) and 
      (cmd_handle != 0)) {
    // Set key to load the ROCE RQ buffer post address to read in next stage
    CAPRI_LOAD_TABLE_OFFSET(common_te0_phv, r2n.rbuf_hdr_addr, 
                            R2N_RX_BUF_ENTRY_BUF_POST_OFFSET,
                            R2N_RX_BUF_ENTRY_BUF_POST_SIZE,
                            r2n_cq_rx_buf_get)

    // Set the PHV bit to post ROCE buffer back
    modify_field(r2n.status_xfer, 1);
  }
}

/*****************************************************************************
 *  Stage: Copy the ROCE NVME command header into PHV which contains the SSD
 *         and priority queue to send it to
 *****************************************************************************/

action handle(src_queue_id, ssd_handle, io_priority, is_read, cmd_handle) {

  // For D vector generation (type inference). No need to translate this to ASM.
  COPY_NVME_CMD1(nvme_be_cmd)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_tgt_kivec0.io_priority, io_priority);
  modify_field(nvme_tgt_kivec0.is_read, is_read);

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)
  NVME_TGT_KIVEC2_USE(storage_scratch, nvme_tgt_kivec2)

  // Setup the various descriptors in the R2N buffer for passing the status
  // and read data back. For now do this only in the ASM.
#if 0
  // Copy the handle to send back to initiator
  memwr(nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_BE_STA_CMD_HANDLE_OFFSET,
        cmd_handle);

  // Save the queue pair in the command
  memwr(nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_BE_CMD_SRC_Q_ID_OFFSET,
        nvme_tgt_kivec2.qp_index);

  // Update the data pointers
  memwr(nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_BE_CMD_CMD_HANDLE_OFFSET,
        nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_DATA_OFFSET);
  if (is_read == 1) {
    memwr(nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_WRITE_REQ_LADDR_OFFSET,
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_DATA_OFFSET);
  }

  // Save the ROCE buffer address in the NVME command
  memwr(nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_BE_CMD_SGL_ADDR_OFFSET,
        nvme_tgt_kivec1.rbuf_hdr_addr);
#endif

  // Set key to load the NVME backend queue context in the last stage (which is 
  // derived based on the SSD handle and I/O priority)
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, NVME_BE_SQ_CTX_TABLE_BASE,
                       ssd_handle, Q_CTX_SIZE,
                       Q_CTX_SIZE, nvme_be_sq_entry_push)
}

/*****************************************************************************
 *  Stage: Push NVME command from ROCE to the selected priority queue in the
 *         NVME backend's SQ
 *****************************************************************************/

action sq_push(idx, state, c_ndx_lo, c_ndx_med, c_ndx_hi, p_ndx_lo, 
               p_ndx_med, p_ndx_hi, p_ndx_lo_db, p_ndx_med_db, 
               p_ndx_hi_db, base_addr, num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_QUEUE_CTX_COPY(nsq_ctx)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Intialize DMA commands so that fields are present in the PHV
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
  CAPRI_PHV2MEM_DMA_INIT(dma_cmd1)
  CAPRI_MEM2MEM_DMA_INIT(dma_cmd6)
  CAPRI_MEM2MEM_DMA_INIT(dma_cmd7)

  // Initialize to good condition
  modify_field(r2n.send_pvm_error, 0);

  // Check for queue full condition (by priority) before pushing
  if (((nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_LO) and
       QUEUE_CANT_PUSH_LO(nsq_ctx)) or 
      ((nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_MED) and
       QUEUE_CANT_PUSH_MED(nsq_ctx)) or
      ((nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_HI) and
       QUEUE_CANT_PUSH_HI(nsq_ctx))) {

    // Mark the error state to send error status back to the initiator
    modify_field(r2n.send_pvm_error, 1);
  }

  // Push to the right priority queue
  // Low priority queue
  if ((r2n.send_pvm_error != 1) and
      (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_LO)) {

#if 0
    // Copy ROCE NVME command to NVME backend SQ memory
    // In ASM: DMA write of the fields from PHV (which are contiguous)
    dmawr(nsq_ctx.base_addr + 
          (nsq_ctx.p_ndx_lo * NVME_BE_SQ_ENTRY_SIZE),
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_BE_CMD_OFFSET .. 
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET);
#endif

    // Push the entry to the queue
    QUEUE_PUSH_LO(nsq_ctx)

    // Exit the pipeline here after scheduling bit for the next pipeline
    //modify_field(scheduler.sched_bit, SCHED_BIT_NVME_BE_CMD);
  }

  // Medium priority queue
  if ((r2n.send_pvm_error != 1) and
      (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_MED)) {

#if 0
    // Copy ROCE NVME command to NVME backend SQ memory
    // In ASM: DMA write of the fields from PHV (which are contiguous)
    dmawr(nsq_ctx.base_addr + 
          (nsq_ctx.p_ndx_med * NVME_BE_SQ_ENTRY_SIZE),
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_BE_CMD_OFFSET .. 
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET);
#endif

    // Push the entry to the queue
    QUEUE_PUSH_MED(nsq_ctx)

    // Exit the pipeline here after scheduling bit for the next pipeline
    //modify_field(scheduler.sched_bit, SCHED_BIT_NVME_BE_CMD);
  }

  // High priority queue
  if ((r2n.send_pvm_error != 1) and
      (nvme_tgt_kivec0.io_priority == NVME_BE_PRIORITY_HI)) {

#if 0
    // Copy ROCE NVME command to NVME backend SQ memory
    // In ASM: DMA write of the fields from PHV (which are contiguous)
    dmawr(nsq_ctx.base_addr + 
          (nsq_ctx.p_ndx_hi * NVME_BE_SQ_ENTRY_SIZE),
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_BE_CMD_OFFSET .. 
          nvme_tgt_kivec1.rbuf_hdr_addr + R2N_RX_BUF_ENTRY_WRITE_REQ_OFFSET);
#endif

    // Push the entry to the queue
    QUEUE_PUSH_HI(nsq_ctx)

    // Exit the pipeline here after scheduling bit for the next pipeline
    //modify_field(scheduler.sched_bit, SCHED_BIT_NVME_BE_CMD);
  }

  // Error in push => punt to PVM error queue
  if (r2n.send_pvm_error == 1) {
    // Set key to load the NVME backend queue context in the last stage (which 
    // is derived based on the SSD handle and I/O priority)
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, PVM_ERRQ_CTX_TABLE_BASE,
                         PVM_ERRQ_DEFAULT_IDX, Q_CTX_SIZE, Q_CTX_SIZE,
                         pvm_errq_entry_push)
  }
}

/**********************************************************
 *  Stage: Get the ROCE Rx buf to post 
 **********************************************************/

action get(cmd_handle, local_addr, data_size) {

  // Save the ROCE Rx buf post entry to PHV
  modify_field(rbuf_post.cmd_handle, cmd_handle);
  modify_field(rbuf_post.local_addr, local_addr);
  modify_field(rbuf_post.data_size, data_size);

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Set key to load the ROCE Receive queue context
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, R2N_RQ_CTX_TABLE_BASE,
                       nvme_tgt_kivec0.paired_q_idx, Q_CTX_SIZE,
                       Q_CTX_SIZE, r2n_rq_entry_push)
}

/*****************************************************************************
 *  Stage: Punt the entire NVME command handling by pushing it to PVM error 
 *         queue in case of error 
 *****************************************************************************/

action errq_push(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, base_addr,
                 num_entries, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  QUEUE_CTX_COPY(peq_ctx)

  // Store to scratch metadata to get the I part of the K vector
  NVME_TGT_KIVEC0_USE(storage_scratch, nvme_tgt_kivec0)
  NVME_TGT_KIVEC1_USE(storage_scratch, nvme_tgt_kivec1)

  // Check for queue full condition before pushing
  if (QUEUE_CANT_PUSH(peq_ctx)) {

    // Exit pipeline here without error handling for now. This event of ROCE RQ
    // being full is so rare that it may not require special handling.
    exit();

  } else {

#if 0
    // Copy ROCE NVME command to NVME backend SQ memory. In ASM, DMA write 
    // of the fields from PHV (which are contiguous).
    dmawr(peq_ctx.base_addr + (peq_ctx.p_ndx * PVM_ERRQ_ENTRY_SIZE),
          rcq_ent.comp_type ..
          rcq_ent.qp_index);
#endif

    // Push the entry to the queue
    QUEUE_PUSH(peq_ctx)
  }
}

#if 0
control process_r2n_cmd {
  // Process ROCE CQ context to check to see if busy bit is set
  process_r2n_cq_ctx_check();

  // Save the ROCE CQ entry to PHV and pop it
  process_r2n_cq_entry_pop();

  // If new command, call control flow to post to NVME backend
  if (r2n.new_cmd == 1) {
    // Read the NVME command header and determine SSD, priority
    process_r2n_nvme_cmd_handle();

    // Push the NVME command to the NVME backend
    process_nvme_be_sq_entry_push();
  }

  // Push error status that was set before recirc
  if ((r2n.new_cmd == 1) and
      (r2n.send_pvm_error == 1)) {
    // Push the command to PVM error queue
    process_pvm_errq_entry_push();
  }

  // If status xfer complete, call control flow to return buffer to ROCE
  if (r2n.status_xfer == 1) {
    // Get the ROCE Rx buf to post
    process_r2n_cq_rx_buf_get();

    // Push the CQ's Rx buffer back to ROCE. Align this to stage #4 
    // (same as buffer post from host queue processing).
    process_r2n_rq_entry_push();
  }

  // Exit the pipeline here
}
#endif

