/*****************************************************************************
 * pvm_vf_comp.p4: This models the the PVM VF functionality in the P4+ 
 *                 pipeline of Capri for processing NVME statuses from PVM and 
 *                 submitting them to the NVME VF's CQ
 *****************************************************************************/

#include "common/dummy.p4"

#define tx_table_s0_t0 pvm_vf_cq_ctx
#define tx_table_s1_t0 pvm_vf_cq_entry
#define tx_table_s2_t0 nvme_vf_cq_ctx

#define tx_table_s0_t0_action qcheck
#define tx_table_s1_t0_action qpop
#define tx_table_s2_t0_action qpush
#define tx_table_s3_t0_action dummy
#define tx_table_s4_t0_action dummy
#define tx_table_s5_t0_action dummy
#define tx_table_s6_t0_action dummy
#define tx_table_s7_t0_action dummy

#include "../common-p4+/common_txdma.p4"
#include "common/capri.h"
#include "common/initiator.h"


/*****************************************************************************
 *  Stage: Check PVM VF CQ context. If busy bit set, yield control. Else 
 *         set the busy bit, save CQ context to PHV and load CQ entry.
 *****************************************************************************/

action qcheck(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
              base_addr, num_entries, vf_id, sq_id, cq_id, 
              sq_ctx_idx, cq_ctx_idx, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  VF_QUEUE_CTX_COPY(pvm_vf_cq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_init_kivec0.idx, idx);
  modify_field(nvme_init_kivec0.state, state);
  modify_field(nvme_init_kivec0.c_ndx, c_ndx);
  modify_field(nvme_init_kivec1.base_addr, base_addr);
  modify_field(nvme_init_kivec0.num_entries, num_entries);
  modify_field(nvme_init_kivec0.sq_id, sq_id);
  modify_field(nvme_init_kivec0.paired_q_idx, paired_q_idx);

  // If busy state was set or queue is empty => yield control
  if (QUEUE_CANT_POP(pvm_vf_cq_ctx)) {
    exit();

  // If not, process the CQ context by locking it and loading the address 
  // of CQ entry to pop for next stage
  } else {
    QUEUE_LOCK_AND_LOAD(pvm_vf_cq_ctx, common_te0_phv, PVM_VF_CQ_ENTRY_SIZE,
                        PVM_VF_CQ_ENTRY_SIZE, pvm_vf_cq_entry_pop)
  }
}

/*****************************************************************************
 *  Stage: Save the PVM status in CQ entry to PHV. Increment consumer index
 *         in PVM VF's CQ context to pop the entry. Check to see if we can do
 *         PRP assist and load the address for the next stage based on that.
 *****************************************************************************/

action qpop(cspec, rsvd0, sq_head, sq_id, cid, phase, status,
            vf_id, cq_ctx_idx, rsvd) {

  // Carry forward state information in the PHV
  modify_field(pvm_status.cspec, cspec);
  modify_field(pvm_status.rsvd0, rsvd0);
  modify_field(pvm_status.sq_head, sq_head);
  modify_field(pvm_status.sq_id, sq_id);
  modify_field(pvm_status.cid, cid);
  modify_field(pvm_status.phase, phase);
  modify_field(pvm_status.status, status);
  modify_field(pvm_status.vf_id, vf_id);
  modify_field(pvm_status.cq_ctx_idx, cq_ctx_idx);
  modify_field(pvm_status.rsvd, rsvd);

  // Store to scratch metadata to get the I part of the K vector
  NVME_INIT_KIVEC0_USE(storage_scratch, nvme_init_kivec0)
  NVME_INIT_KIVEC1_USE(storage_scratch, nvme_init_kivec1)

  // Pop the CQ entry
  QUEUE_POP(nvme_init_kivec0, PVM_VF_CQ_CTX_TABLE_BASE)

  // Load the NVME VF CQ context for the next stage to push the NVME status
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, NVME_VF_CQ_CTX_TABLE_BASE, 
                       cq_ctx_idx,
                       Q_CTX_SIZE, Q_CTX_SIZE, nvme_vf_cq_entry_push)
}

/*****************************************************************************
 *  Stage: Push NVME status to NVME VF's CQ. TODO: When mapping to ASM, no
 *         need to ring the doorbell as it will be interrupt mapped ?
 *****************************************************************************/

action qpush(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
              base_addr, num_entries, vf_id, sq_id, cq_id, 
              sq_ctx_idx, cq_ctx_idx, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  VF_QUEUE_CTX_COPY(nvme_vf_cq_ctx)

  // Check for queue full condition before pushing
  if (QUEUE_CANT_PUSH(nvme_vf_cq_ctx)) {

    // Exit pipeline here without error handling for now. 
    // TODO: Error handling in case of shared CQ's where the CQ could be full.
    exit();

  } else {

#if 0
    // Copy PVM NVME status to NVME VF CQ memory. In ASM, DMA write 
    // of the fields from PHV (which are contiguous).
    dmawr(nvme_vf_cq_ctx.base_addr + 
          (nvme_vf_cq_ctx.p_ndx * NVME_VF_CQ_ENTRY_SIZE),
          pvm_status.cspec .. 
          pvm_status.status);
#endif

    // Push the entry to the queue.  TODO: When mapping to ASM, no need to ring
    // the doorbell as done by the macro (because it is be interrupt mapped ?)
    QUEUE_PUSH(nvme_vf_cq_ctx)

    // Initialize the fields so that it will appear in the auto generated PHV
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd1)

    // Exit the pipeline here 
  }
}
