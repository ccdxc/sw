/*****************************************************************************
 * nvme_vf_cmd.p4: This models the the NVME VF functionality in the P4+ 
 *                 pipeline of Capri for processing NVME commands NVME driver
 *                 and submitting them to the PVM VF's SQ
 *****************************************************************************/


#include "common/dummy.p4"

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
 *  Stage: Check NVME VF SQ context. If busy bit set, yield control. Else 
 *         set the busy bit, save SQ context to PHV and load the SQ entry.
 *****************************************************************************/

action qcheck(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
              base_addr, num_entries, vf_id, sq_id, cq_id, 
              sq_ctx_idx, cq_ctx_idx, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  VF_QUEUE_CTX_COPY(nvme_vf_sq_ctx)

  // For correctness sake, pass the information into the K+I vector
  modify_field(nvme_init_kivec0.idx, idx);
  modify_field(nvme_init_kivec0.state, state);
  modify_field(nvme_init_kivec0.c_ndx, c_ndx);
  modify_field(nvme_init_kivec1.base_addr, base_addr);
  modify_field(nvme_init_kivec0.num_entries, num_entries);
  modify_field(nvme_init_kivec0.sq_id, sq_id);
  modify_field(nvme_init_kivec0.paired_q_idx, paired_q_idx);

  // Also save to PVM command metadata into PHV.
  modify_field(pvm_cmd.vf_id, vf_id);
  modify_field(pvm_cmd.sq_id, sq_id);
  modify_field(pvm_cmd.sq_ctx_idx, sq_ctx_idx);
  modify_field(pvm_cmd.cq_ctx_idx, cq_ctx_idx);

  // If busy state was set or queue is empty => yield control
  if (QUEUE_CANT_POP(nvme_vf_sq_ctx)) {
    exit();

  // If not, process the CQ context by locking it and loading the address 
  // of CQ entry to pop for next stage
  } else {
    QUEUE_LOCK_AND_LOAD(nvme_vf_sq_ctx, common_te0_phv, NVME_VF_SQ_ENTRY_SIZE,
                        NVME_VF_SQ_ENTRY_SIZE, nvme_vf_sq_entry_pop)
  }
}


/*****************************************************************************
 *  Stage: Save the NVME command in SQ entry to PHV. Increment consumer index
 *         in NVME VF's SQ context to pop the entry. Check to see if we can do
 *         PRP assist and load the address for the next stage based on that.
 *****************************************************************************/

action qpop(opc, fuse, rsvd0, psdt, cid, nsid, rsvd2, rsvd3,
            mptr, dptr1, dptr2, slba, nlb, rsvd12, prinfo,
            fua, lr, dsm, rsvd13, dw14, dw15) {

  // Carry forward state information in the PHV
  modify_field(pvm_cmd.opc, opc);
  modify_field(pvm_cmd.fuse, fuse);
  modify_field(pvm_cmd.rsvd0, rsvd0);
  modify_field(pvm_cmd.psdt, psdt);
  modify_field(pvm_cmd.cid, cid);
  modify_field(pvm_cmd.nsid, nsid);
  modify_field(pvm_cmd.rsvd2, rsvd2);
  modify_field(pvm_cmd.rsvd3, rsvd3);
  modify_field(pvm_cmd.mptr, mptr);
  modify_field(pvm_cmd.dptr1, dptr1);
  modify_field(pvm_cmd.dptr2, dptr2);
  modify_field(pvm_cmd.slba, slba);
  modify_field(pvm_cmd.nlb, nlb);
  modify_field(pvm_cmd.rsvd12, rsvd12);
  modify_field(pvm_cmd.prinfo, prinfo);
  modify_field(pvm_cmd.fua, fua);
  modify_field(pvm_cmd.lr, lr);
  modify_field(pvm_cmd.dsm, dsm);
  modify_field(pvm_cmd.rsvd13, rsvd13);
  modify_field(pvm_cmd.dw14, dw14);
  modify_field(pvm_cmd.dw15, dw15);

  // Store to scratch metadata to get the I part of the K vector
  NVME_INIT_KIVEC0_USE(storage_scratch, nvme_init_kivec0)
  NVME_INIT_KIVEC1_USE(storage_scratch, nvme_init_kivec1)

  // Pop the SQ entry
  QUEUE_POP(nvme_init_kivec0, NVME_VF_SQ_CTX_TABLE_BASE)

  // Initialize the remaining fields in the PVM command 
  modify_field(pvm_cmd.num_prps, 0);
  modify_field(pvm_cmd.rsvd, 0);
  modify_field(pvm_cmd.tickreg, 0);

  // Initialize set PRP assist flag to false (unless check logic overrides this)
  modify_field(nvme_init_kivec1.prp_assist, 0);

  // NVMe H/W Assist determination (AND of all these conditions)
  // 1. q_id > 0 (not admin queue) AND (read OR write command)
  // 2. PRP list indicated in psdt bit
  // 3. LBA size is less than the max assist size
  // 4. LBA size is greater than the sum of the PRP sizes
  //
  // If doing assist, calculate the number of PRPs to be loaded
  if (((nvme_init_kivec0.sq_id > 0) and
       ((opc == NVME_READ_CMD_OPCODE) or
        (opc == NVME_WRITE_CMD_OPCODE))) and
      (psdt == 0) and
      (LB_SIZE(nlb + 1) <= MAX_ASSIST_SIZE) and
      (LB_SIZE(nlb + 1) > (PRP_SIZE(dptr1) + PRP_SIZE(dptr2)))) {
    modify_field(nvme_init_kivec1.prp_assist, 1);
  }

  // H/W assist and PRPs capped to Max
  if ((nvme_init_kivec1.prp_assist == 1) and
      (NVME_MAX_XTRA_PRPS <= PRP_SIZE(dptr2) >> 3)) {
    modify_field(pvm_cmd.num_prps, NVME_MAX_XTRA_PRPS);
  }

  // H/W assist and PRPs not capped to Max
  if ((nvme_init_kivec1.prp_assist == 1) and
      (NVME_MAX_XTRA_PRPS > PRP_SIZE(dptr2) >> 3)) {
    modify_field(pvm_cmd.num_prps, ((PRP_SIZE(dptr2)) >> 3));
  }

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_IDX(common_te0_phv, PVM_VF_SQ_CTX_TABLE_BASE, 
                       nvme_init_kivec0.paired_q_idx,
                       Q_CTX_SIZE, Q_CTX_SIZE, pvm_vf_sq_entry_push)
}

/*****************************************************************************
 *  Stage: Push NVME command to PVM VF's SQ. TODO: When mapping to ASM, no
 *         need to ring the doorbell as it will be interrupt mapped ?
 *****************************************************************************/

action qpush(idx, state, c_ndx, p_ndx, p_ndx_db, c_ndx_db, 
              base_addr, num_entries, vf_id, sq_id, cq_id, 
              sq_ctx_idx, cq_ctx_idx, paired_q_idx, rsvd) {

  // For D vector generation (type inference). No need to translate this to ASM.
  VF_QUEUE_CTX_COPY(pvm_vf_sq_ctx)

  // Check for queue full condition before pushing
  if (QUEUE_CANT_PUSH(pvm_vf_sq_ctx)) {

    // Exit pipeline here without error handling for now. This event of 
    // PVM VF's SQ being full should never happen as software will program 
    // the queue depth of PVM VF to be more than the sum of queue depths of
    // NVME VFs mapped to it.
    exit();

  } else {

#if 0
    // Copy PVM NVME command to PVM VF SQ memory. In ASM, DMA write 
    // of the fields from PHV (which are contiguous).
    dmawr(pvm_vf_sq_ctx.base_addr + 
          (pvm_vf_sq_ctx.p_ndx * PVM_VF_SQ_ENTRY_SIZE),
          pvm_cmd.opc .. 
          pvm_cmd.tickreg);
    if ((nvme_init_kivec1.prp_assist == 1) {
      dmawr(pvm_vf_sq_ctx.base_addr + 
            (pvm_vf_sq_ctx.p_ndx * PVM_VF_SQ_ENTRY_SIZE) +
            offsetof(pvm_cmd.prp_list_lo), 
            pvm_cmd.dptr2 ..
            pvm_cmd.dptr2 + (pvm_cmd.num_prps*SIZEOF_EACH_PRP));
    }
#endif

    // Push the entry to the queue.  TODO: When mapping to ASM, no need to ring
    // the doorbell as done by the macro (because it is be interrupt mapped ?)
    QUEUE_PUSH(pvm_vf_sq_ctx)

    // Initialize the fields so that it will appear in the auto generated PHV
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd0)
    CAPRI_PHV2MEM_DMA_INIT(dma_cmd1)

    // Exit the pipeline here 
  }
}

