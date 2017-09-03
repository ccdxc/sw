

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0		s0_tbl
#define tx_table_s1_t0		s1_tbl
#define tx_table_s2_t0		s2_tbl
#define tx_table_s3_t0		s3_tbl
#define tx_table_s4_t0		s4_tbl

#define tx_table_s0_t0_action	q_state_pop
#define tx_table_s0_t0_action1	pri_q_state_pop

#define tx_table_s1_t0_action	nvme_sq_handler
#define tx_table_s1_t0_action1	pvm_cq_handler
#define tx_table_s1_t0_action2	r2n_sq_handler
#define tx_table_s1_t0_action3	nvme_be_sq_handler
#define tx_table_s1_t0_action4	nvme_be_cq_handler

#define tx_table_s2_t0_action	q_state_push
#define tx_table_s2_t0_action1	nvme_be_wqe_prep
#define tx_table_s2_t0_action2	nvme_be_cmd_handler
#define tx_table_s2_t0_action3	nvme_be_wqe_handler

#define tx_table_s3_t0_action	pri_q_state_push
#define tx_table_s3_t0_action1	pri_q_state_incr
#define tx_table_s3_t0_action2	pri_q_state_decr

#define tx_table_s4_t0_action	nvme_be_wqe_save
#define tx_table_s4_t0_action1	nvme_be_wqe_release

// tx_table_s5_t0_action is actually a q_state_push for NVME backend
// But this is already defined in tx_table_s2_t0_action

#include "../common-p4+/common_txdma.p4"

#include "common/storage_p4_defines.h"


/*****************************************************************************
 * Storage Tx PHV layout BEGIN 
 * Will be processed by NCC in this order 
 *****************************************************************************/

// Global and stage to stage K+I vectors
@pragma pa_header_union ingress common_t0_s2s
metadata storage_kivec0_t storage_kivec0;
@pragma pa_header_union ingress common_global
metadata storage_kivec1_t storage_kivec1;

// NVME backend command (occupies full flit, so keep it first) 
@pragma dont_trim
metadata nvme_be_cmd_t nvme_be_cmd;

// PVM metadata
@pragma dont_trim
metadata pvm_cmd_t pvm_cmd;
@pragma dont_trim
metadata pvm_status_t pvm_status;

// Push/Pop doorbells
@pragma dont_trim
metadata storage_doorbell_data_t qpush_doorbell_data;
@pragma dont_trim
metadata storage_doorbell_data_t qpop_doorbell_data;

// R2N work queue entry 
@pragma dont_trim
metadata r2n_wqe_t r2n_wqe;

// NVME backend status 
@pragma dont_trim
metadata nvme_be_sta_hdr_t nvme_be_sta_hdr;
@pragma dont_trim
metadata nvme_be_sta_t nvme_be_sta;

// Manually computed pad to align DMA commands to 16 byte boundary
// TODO: Remove this when NCC supports pragma for this
@pragma dont_trim
metadata storage_pad_t storage_pad;
  
// DMA commands metadata
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_0;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_0
metadata dma_cmd_mem2mem_t dma_m2m_0;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_1;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_1
metadata dma_cmd_mem2mem_t dma_m2m_1;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_2;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_2
metadata dma_cmd_mem2mem_t dma_m2m_2;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_3;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_3
metadata dma_cmd_mem2mem_t dma_m2m_3;

/*****************************************************************************
 * Storage Tx PHV layout END 
 *****************************************************************************/


// Scratch metadatas to get d-vector generated correctly

@pragma scratch_metadata
metadata q_state_t q_state_scratch;

@pragma scratch_metadata
metadata pri_q_state_t pri_q_state_scratch;

@pragma scratch_metadata
metadata nvme_be_cmd_hdr_t nvme_be_cmd_hdr;

@pragma scratch_metadata
metadata storage_doorbell_addr_t doorbell_addr;

@pragma scratch_metadata
metadata storage_kivec0_t storage_kivec0_scratch;

@pragma scratch_metadata
metadata storage_kivec1_t storage_kivec1_scratch;

@pragma scratch_metadata
metadata ssd_cmds_t ssd_cmds;

@pragma scratch_metadata
metadata r2n_wqe_t r2n_wqe_scratch;

/*****************************************************************************
 * exit: Exit action handler needs to be stubbed out for NCC 
 *****************************************************************************/

action exit() {
}

/*****************************************************************************
 *  q_state_pop : Check the queue state and see if there's anything to be 
 *                popped. If so increment the working index and load the
 *                queue entry.
 *****************************************************************************/

action q_state_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                   total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                   num_entries, base_addr, entry_size, next_pc, dst_qaddr,
                   dst_lif, dst_qtype, dst_qid, vf_id, sq_id,
                   ssd_bm_addr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  Q_STATE_COPY_STAGE0(q_state_scratch)

  // If queue is empty exit
  if (QUEUE_EMPTY(q_state_scratch)) {
    exit();
  } else {
    // Increment the working consumer index. In ASM this should be a table write.
    QUEUE_POP(q_state_scratch)
   
    // Store fields needed in the K+I vector
    modify_field(storage_kivec0.w_ndx, w_ndx);
    modify_field(storage_kivec0.dst_qaddr, dst_qaddr);
    modify_field(storage_kivec0.dst_lif, dst_lif);
    modify_field(storage_kivec0.dst_qtype, dst_qtype);
    modify_field(storage_kivec0.dst_qid, dst_qid);
    modify_field(storage_kivec0.ssd_bm_addr, ssd_bm_addr);

    // In ASM, derive these from the K+I for stage 0
    modify_field(storage_kivec1.src_qaddr, 0);
    modify_field(storage_kivec1.src_lif, 0);
    modify_field(storage_kivec1.src_qtype, 0);
    modify_field(storage_kivec1.src_qid, 0);
    modify_field(storage_kivec0.is_q0, 0);

    // Initialize the vf_id and sq_id fields in the PHV
    modify_field(pvm_cmd.vf_id, vf_id);
    modify_field(pvm_cmd.sq_id, sq_id);

    // Load the table and program for processing the queue entry in the next stage
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, q_state_scratch.base_addr,
                         q_state_scratch.w_ndx, q_state_scratch.entry_size,
                         q_state_scratch.entry_size, q_state_scratch.next_pc)
  }
}

/*****************************************************************************
 *  nvme_sq_handler: Save the NVME command in SQ entry to PHV. DMA the 
 *                   working consumer index to the consumer index in the 
 *                   queue state. Check to see if we can do PRP assist and 
 *                   load the address for the next stage based on that.
 *****************************************************************************/

action nvme_sq_handler(opc, fuse, rsvd0, psdt, cid, nsid, rsvd2, rsvd3,
                       mptr, dptr1, dptr2, slba, nlb, rsvd12, prinfo,
                       fua, lr, dsm, rsvd13, dw14, dw15) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Carry forward NVME command information to be sent to PVM in the PHV 
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

  // Initialize the remaining fields of the PVM command in the PHV
  modify_field(pvm_cmd.num_prps, 0);
  modify_field(pvm_cmd.tickreg, 0);

  // Initialize set PRP assist flag to false (unless check logic overrides this)
  modify_field(storage_kivec0.prp_assist, 0);

  // NVMe H/W Assist determination (AND of all these conditions)
  // 1. q_id > 0 (not admin queue) AND (read OR write command)
  // 2. PRP list indicated in psdt bit
  // 3. LBA size is less than the max assist size
  // 4. LBA size is greater than the sum of the PRP sizes
  //
  // If doing assist, calculate the number of PRPs to be loaded
  if (((storage_kivec0.is_q0 == 0) and
       ((opc == NVME_READ_CMD_OPCODE) or
        (opc == NVME_WRITE_CMD_OPCODE))) and
      (psdt == 0) and
      (LB_SIZE(nlb + 1) <= MAX_ASSIST_SIZE) and
      (LB_SIZE(nlb + 1) > (PRP_SIZE(dptr1) + PRP_SIZE(dptr2)))) {
    modify_field(storage_kivec0.prp_assist, 1);
  }

  // H/W assist and PRPs capped to Max
  if ((storage_kivec0.prp_assist == 1) and
      (NVME_MAX_XTRA_PRPS <= PRP_SIZE(dptr2) >> 3)) {
    modify_field(pvm_cmd.num_prps, NVME_MAX_XTRA_PRPS);
  }

  // H/W assist and PRPs not capped to Max
  if ((storage_kivec0.prp_assist == 1) and
      (NVME_MAX_XTRA_PRPS > PRP_SIZE(dptr2) >> 3)) {
    modify_field(pvm_cmd.num_prps, ((PRP_SIZE(dptr2)) >> 3));
  }

  // Setup the DMA command to push the NVME command entry
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(pvm_cmd.opc),
                           PHV_FIELD_OFFSET(pvm_cmd.tickreg),
                           0, 0, 0, 0)

#if 0
  // TODO: Figure out the PRP assist DMA
  if (storage_kivec0.prp_assist == 1)
  }
#endif

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                        Q_STATE_SIZE, q_state_push_start)
}

/*****************************************************************************
 *  pvm_cq_handler: Save the NVME command in SQ entry to PHV. DMA the 
 *                  working consumer index to the consumer index in the 
 *                  queue state. Check to see if we can do PRP assist and 
 *                  load the address for the next stage based on that.
 *****************************************************************************/

action pvm_cq_handler(cspec, rsvd0, sq_head, sq_id, cid, phase, status,
                      dst_qaddr) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Carry forward NVME status information to be sent to driver in the PHV 
  modify_field(pvm_status.cspec, cspec);
  modify_field(pvm_status.rsvd0, rsvd0);
  modify_field(pvm_status.sq_head, sq_head);
  modify_field(pvm_status.sq_id, sq_id);
  modify_field(pvm_status.cid, cid);
  modify_field(pvm_status.phase, phase);
  modify_field(pvm_status.status, status);
  modify_field(pvm_status.dst_qaddr, dst_qaddr);

  // Setup the DMA command to push the NVME status entry
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(pvm_status.cspec),
                           PHV_FIELD_OFFSET(pvm_status.status),
                           0, 0, 0, 0)

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr, 
                        Q_STATE_SIZE, q_state_push_start)
}

/*****************************************************************************
 *  q_state_push: Push to a queue by issuing the DMA commands and incrementing
 *                the p_ndx via ringing the doorbell. Assumes that data to be
 *                pushed is in DMA command 1.
 *****************************************************************************/

action q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                    total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                    num_entries, base_addr, entry_size, next_pc, dst_qaddr,
                    dst_lif, dst_qtype, dst_qid, vf_id, sq_id,
                    ssd_bm_addr, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  Q_STATE_COPY(q_state_scratch)

  // Check for queue full condition before pushing
  if (QUEUE_FULL(q_state_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  } else {

    // Modify the DMA command 1 to fill the source address based on p_ndx 
    // NOTE: This API in P4 land will not work, but in ASM we can selectively
    // overwrite the fields
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                             q_state_scratch.base_addr +
                             (q_state_scratch.p_ndx * q_state_scratch.entry_size),
                             0, 0,
                             0, 0, 0, 0)

#if 0
    // TODO: Figure out the PRP assist DMA
    if (storage_kivec0.prp_assist == 1)
    }
#endif

    // Push the entry to the queue.  
    QUEUE_PUSH(q_state_scratch)

    // Form the doorbell and setup the DMA command to push the entry by
    // incrementing p_ndx
    modify_field(doorbell_addr.addr,
                 STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype, 
                                          storage_kivec0.dst_lif,
                                          DOORBELL_SCHED_WR_RESET, 
                                          DOORBELL_UPDATE_P_NDX_INCR));
    modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_2, 
                             0,
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             0, 0, 0, 0)

    // Exit the pipeline here 
  }
}


/*****************************************************************************
 *  pri_q_state_pop : Check the queue state and see if there's anything to be 
 *                    popped from any priority rings. If so increment the 
 *                    working index for that ring and load the queue entry.
 *****************************************************************************/

action pri_q_state_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                       total_rings, host_rings, pid, p_ndx_lo, c_ndx_lo, 
                       p_ndx_med, c_ndx_med, p_ndx_hi, c_ndx_hi, w_ndx_lo,
                       w_ndx_med, w_ndx_hi, num_entries, base_addr, entry_size,
                       lo_weight, med_weight, hi_weight, lo_running, 
                       med_running, hi_running, num_running, max_cmds,
                       next_pc, dst_qaddr, dst_lif, dst_qtype, dst_qid, 
                       ssd_bm_addr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_Q_STATE_COPY_STAGE0(pri_q_state_scratch)

  // Store fields needed in the K+I vector
  modify_field(storage_kivec0.dst_qaddr, dst_qaddr);
  modify_field(storage_kivec0.dst_lif, dst_lif);
  modify_field(storage_kivec0.dst_qtype, dst_qtype);
  modify_field(storage_kivec0.dst_qid, dst_qid);
  modify_field(storage_kivec0.ssd_bm_addr, ssd_bm_addr);

  // In ASM, derive these from the K+I for stage 0
  modify_field(storage_kivec0.is_q0, 0);
  modify_field(storage_kivec1.src_lif, 0);
  modify_field(storage_kivec1.src_qtype, 0);
  modify_field(storage_kivec1.src_qid, 0);
  modify_field(storage_kivec1.src_qaddr, 0);

  // If high priority queue can be serviced, go with it
  if (PRI_QUEUE_CAN_POP_HI(pri_q_state_scratch)) {
    // Service the high priority queue
    SERVICE_PRI_QUEUE(storage_kivec0, pri_q_state_scratch, common_te0_phv,
                      pri_q_state_scratch.w_ndx_hi, NVME_BE_PRIORITY_HI)

    // In ASM exit the stage here
  }

  // If medium priority queue can be serviced, go with it
  if (PRI_QUEUE_CAN_POP_MED(pri_q_state_scratch)) {
    // Service the high priority queue
    SERVICE_PRI_QUEUE(storage_kivec0, pri_q_state_scratch, common_te0_phv,
                      pri_q_state_scratch.w_ndx_med, NVME_BE_PRIORITY_MED)

    // In ASM exit the stage here
  }

  // If low priority queue can be serviced, go with it
  if (PRI_QUEUE_CAN_POP_LO(pri_q_state_scratch)) {
    // Service the low priority queue
    SERVICE_PRI_QUEUE(storage_kivec0, pri_q_state_scratch, common_te0_phv,
                      pri_q_state_scratch.w_ndx_lo, NVME_BE_PRIORITY_LO)

    // In ASM exit the stage here
  }
}

/*****************************************************************************
 *  pri_q_state_incr: Increment the running counters in the priority queue. 
 *                    Load the table to save the R2N WQE in the next stage.
 *****************************************************************************/

action pri_q_state_incr(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx_lo, c_ndx_lo, 
                        p_ndx_med, c_ndx_med, p_ndx_hi, c_ndx_hi, w_ndx_lo,
                        w_ndx_med, w_ndx_hi, num_entries, base_addr, entry_size,
                        lo_weight, med_weight, hi_weight, lo_running, 
                        med_running, hi_running, num_running, max_cmds,
                        next_pc, dst_qaddr, dst_lif, dst_qtype, dst_qid, 
                        ssd_bm_addr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_Q_STATE_COPY(pri_q_state_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Update the running counters based on priority
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_HI) {
    modify_field(pri_q_state_scratch.hi_running,
                 pri_q_state_scratch.hi_running + 1);
    modify_field(pri_q_state_scratch.num_running,
                 pri_q_state_scratch.num_running + 1);
  }
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_MED) {
    modify_field(pri_q_state_scratch.med_running,
                 pri_q_state_scratch.med_running + 1);
    modify_field(pri_q_state_scratch.num_running,
                 pri_q_state_scratch.num_running + 1);
  }
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_LO) {
    modify_field(pri_q_state_scratch.lo_running,
                 pri_q_state_scratch.lo_running + 1);
    modify_field(pri_q_state_scratch.num_running,
                 pri_q_state_scratch.num_running + 1);
  }

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.ssd_bm_addr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, nvme_be_wqe_save_start)
}

/*****************************************************************************
 *  pri_q_state_decr: Decrement the running counters in the priority queue. 
 *                    Load the table to release the bitmap in the next stage.
 *****************************************************************************/

action pri_q_state_decr(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx_lo, c_ndx_lo, 
                        p_ndx_med, c_ndx_med, p_ndx_hi, c_ndx_hi, w_ndx_lo,
                        w_ndx_med, w_ndx_hi, num_entries, base_addr, entry_size,
                        lo_weight, med_weight, hi_weight, lo_running, 
                        med_running, hi_running, num_running, max_cmds,
                        next_pc, dst_qaddr, dst_lif, dst_qtype, dst_qid, 
                        ssd_bm_addr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_Q_STATE_COPY(pri_q_state_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Update the running counters based on priority
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_HI) {
    modify_field(pri_q_state_scratch.hi_running,
                 pri_q_state_scratch.hi_running - 1);
    modify_field(pri_q_state_scratch.num_running,
                 pri_q_state_scratch.num_running - 1);
  }
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_MED) {
    modify_field(pri_q_state_scratch.med_running,
                 pri_q_state_scratch.med_running - 1);
    modify_field(pri_q_state_scratch.num_running,
                 pri_q_state_scratch.num_running - 1);
  }
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_LO) {
    modify_field(pri_q_state_scratch.lo_running,
                 pri_q_state_scratch.lo_running - 1);
    modify_field(pri_q_state_scratch.num_running,
                 pri_q_state_scratch.num_running - 1);
  }

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.ssd_bm_addr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, nvme_be_wqe_release_start)
}

/*****************************************************************************
 *  pri_q_state_push: Push to a queue by issuing the DMA commands and 
 *                    incrementing the priority p_ndx via ringing the doorbell.
 *                    Assumes that data to be pushed is in DMA command 1.
 *****************************************************************************/

action pri_q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx_lo, c_ndx_lo, 
                        p_ndx_med, c_ndx_med, p_ndx_hi, c_ndx_hi, w_ndx_lo,
                        w_ndx_med, w_ndx_hi, num_entries, base_addr, entry_size,
                        lo_weight, med_weight, hi_weight, lo_running, 
                        med_running, hi_running, num_running, max_cmds,
                        next_pc, dst_qaddr, dst_lif, dst_qtype, dst_qid, 
                        ssd_bm_addr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_Q_STATE_COPY(pri_q_state_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Check for queue full condition before pushing
  if (PRI_QUEUE_FULL(pri_q_state_scratch, storage_kivec0_scratch.io_priority)) {

    // Exit pipeline here without error handling for now. 
    // TODO: FIXME to push to PVM error queue
    exit();
  }


  // Queue push based on priority
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_HI) {
    PRI_QUEUE_PUSH(pri_q_state_scratch, pri_q_state_scratch.p_ndx_hi, 
                   NVME_BE_PRIORITY_HI)
  }
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_MED) {
    PRI_QUEUE_PUSH(pri_q_state_scratch, pri_q_state_scratch.p_ndx_med, 
                   NVME_BE_PRIORITY_MED)
  }
  if (storage_kivec0.io_priority == NVME_BE_PRIORITY_LO) {
    PRI_QUEUE_PUSH(pri_q_state_scratch, pri_q_state_scratch.p_ndx_lo, 
                   NVME_BE_PRIORITY_LO)
  }

  // Update the push doorbell
  QUEUE_PUSH_DOORBELL_UPDATE

  // Exit the pipeline here 
}

/*****************************************************************************
 *  r2n_sq_handler: Read the R2N WQE posted by local PVM to get the pointer to 
 *                  the NVME backend command. Call the next stage to read the 
 *                  NVME backend command to determine the SSD queue and 
 *                  priority ring to post to.
 *****************************************************************************/

action r2n_sq_handler(handle, data_size, opcode, status) {

  // Carry forward state information to be saved with R2N WQE in PHV
  R2N_WQE_BASE_COPY(r2n_wqe)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, handle,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, nvme_be_wqe_prep_start)
}

/*****************************************************************************
 *  nvme_be_wqe_prep: From the NVME backend command, determine which SSD and 
 *                    priority queue to send it to.
 *****************************************************************************/

action nvme_be_wqe_prep(src_queue_id, ssd_handle, io_priority, is_read,
                        r2n_buf_handle, is_local) {

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_BE_CMD_HDR_COPY(nvme_be_cmd_hdr)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Carry forward state information to be saved with R2N WQE in PHV
  NVME_BE_CMD_HDR_COPY(r2n_wqe)
  modify_field(r2n_wqe.pri_qaddr, storage_kivec0.dst_qaddr);

  // Setup the DMA command to push the modified R2N WQE to NVME backend
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(r2n_wqe.handle),
                           PHV_FIELD_OFFSET(r2n_wqe.pri_qaddr),
                           0, 0, 0, 0)

  // Load the NVME backkend SQ context for the next stage to push the command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr, 
                        Q_STATE_SIZE, pri_q_state_push_start)
}

/*****************************************************************************
 *  nvme_be_sq_handler: Read the NVME backend priority submission queue entry.
 *                      Load the actual NVME command for the next stage.
 *****************************************************************************/

action nvme_be_sq_handler(handle, data_size, opcode, status, src_queue_id,
                          ssd_handle, io_priority, is_read, r2n_buf_handle,
                          is_local, nvme_cmd_cid, pri_qaddr) {

  // Carry forward state information to be saved with R2N WQE in PHV
  R2N_WQE_FULL_COPY(r2n_wqe)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, handle,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, nvme_be_cmd_handler_start)
}

/*****************************************************************************
 *  nvme_be_cmd_handler: Save the NVME command to PHV. Load the address of the
 *                       the priority queue state again to increment the 
 *                       running counters in the next stage.
 *****************************************************************************/

action nvme_be_cmd_handler(nvme_cmd_w0, nvme_cmd_cid, nvme_cmd_hi) {

  // Save the NVME command to PHV
  modify_field(nvme_be_cmd.nvme_cmd_w0, nvme_cmd_w0);
  modify_field(nvme_be_cmd.nvme_cmd_cid, nvme_cmd_cid);
  modify_field(nvme_be_cmd.nvme_cmd_hi, nvme_cmd_hi);

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Store the original value of NVME command id in R2N WQE in PHV. This will
  // be saved to HBM so that this command id can be restored when processing
  // the NVME status.
  modify_field(r2n_wqe.nvme_cmd_cid, nvme_cmd_cid);

  // Load the priority queue state table for incrementing the running 
  // counters in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec1.src_qaddr, Q_STATE_SIZE,
                        pri_q_state_incr_start)
}

/*****************************************************************************
 *  nvme_be_wqe_save: Save the R2N WQE pointing to the NVME command to be sent 
 *                    to the SSD into memory. Load the address of the SSD's 
 *                    SQ context for the next stage to push the command.
 *****************************************************************************/

action nvme_be_wqe_save(bitmap) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(ssd_cmds.bitmap, bitmap);
  
  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Get the first free bit in the bitmap and use it as the cmd_index.
#if 0
  // In ASM, do these
  // 1. Find first clear bit in the bitmap and set it
  // 2. Write the bitmap back to the table and set the command index in PHV
  add          r1, r0, d.bitmap
  ffcv         r2, d.bitmap, r0
  addi         r7, r0, 1
  sllv         r7, r7, r2
  tblor        d.bitmap, r7

  // Set the cmd_index in the CID of Dword0 of the NVME command to be sent
  // to the SSD
  modify_field(nvme_be_cmd.nvme_cmd_cid, r2);
#endif

  // Setup the DMA command to push the NVME status entry
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(nvme_be_cmd.nvme_cmd_w0),
                           PHV_FIELD_OFFSET(nvme_be_cmd.nvme_cmd_hi),
                           0, 0, 0, 0)

  // Save the NVME backend command to memory at the calculated index
  // In ASM: get the address from storage_kivec0.ssd_bm_addr +  SSD_CMDS_ENTRY_OFFSET(r2),
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3,
                           0,
                           PHV_FIELD_OFFSET(r2n_wqe.handle),
                           PHV_FIELD_OFFSET(r2n_wqe.pri_qaddr),
                           0, 0, 0, 0)

  // Load the SSD SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                        Q_STATE_SIZE, q_state_push_start)
}

/*****************************************************************************
 *  nvme_be_cq_handler: Save the NVME status into PHV. Load the saved R2N WQE
 *                      for the command that was sent to the SSD
 *****************************************************************************/

action nvme_be_cq_handler(nvme_sta_lo, nvme_sta_cid, nvme_sta_w7) {

  // Carry forward NVME status information in the PHV 
  modify_field(nvme_be_sta.nvme_sta_lo, nvme_sta_lo);
  modify_field(nvme_be_sta.nvme_sta_cid, nvme_sta_cid);
  modify_field(nvme_be_sta.nvme_sta_w7, nvme_sta_w7);

  // Set the state information for the NVME backend status header
  // TODO: FIXME
  modify_field(nvme_be_sta_hdr.time_us, 0);
  modify_field(nvme_be_sta_hdr.be_status, 0);
  modify_field(nvme_be_sta_hdr.is_q0, 0);
  modify_field(nvme_be_sta_hdr.rsvd, 0);

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Derive the save command index from the NVME status
  modify_field(storage_kivec0_scratch.cmd_index, (0xFF & nvme_sta_cid));

  // Load the PVM VF SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv,
                        storage_kivec0.ssd_bm_addr +  
                        SSD_CMDS_ENTRY_OFFSET(storage_kivec0_scratch.cmd_index),
                        SSD_CMDS_ENTRY_SIZE, nvme_be_wqe_handler_start)
}

/*****************************************************************************
 *  nvme_be_wqe_handler: Read the saved R2N WQE to determine which priority 
 *                       queue's running counters to update
 *****************************************************************************/

action nvme_be_wqe_handler(handle, data_size, opcode, status, src_queue_id,
                           ssd_handle, io_priority, is_read, r2n_buf_handle,
                           is_local, nvme_cmd_cid, pri_qaddr) {

  // For D vector generation (type inference). No need to translate this to ASM.
  R2N_WQE_FULL_COPY(r2n_wqe_scratch)

  // Restore the fields in the NVME backend status to saved values
  modify_field(nvme_be_sta_hdr.r2n_buf_handle, r2n_wqe_scratch.r2n_buf_handle);
  modify_field(nvme_be_sta.nvme_sta_cid, r2n_wqe_scratch.nvme_cmd_cid);

  // Load the priority queue state of the NVME backend to clear the running
  // counters
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, r2n_wqe_scratch.pri_qaddr, Q_STATE_SIZE,
                        pri_q_state_decr_start)
}

/*****************************************************************************
 *  nvme_be_wqe_release: Save NVME command to be sent to the SSD into memory. 
 *                    Load the address of the SSD's SQ context for the next 
 *                    stage to push the command.
 *****************************************************************************/

action nvme_be_wqe_release(bitmap) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(ssd_cmds.bitmap, bitmap);
  
  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Get the first free bit in the bitmap and use it as the cmd_index.
#if 0
  // In ASM, // Clear the bit corresponding to the cmd_index in the table
  add          r5, r0, k.storage_kivec0_cmd_index
  sllv         r4, 1, r5
  xor          r4, r4, -1
  tbland       d.bitmap, r4
#endif

  // Setup the DMA command to push the NVME backend status entry
  // that was formed in the previous stages
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(nvme_be_sta_hdr.time_us),
                           PHV_FIELD_OFFSET(nvme_be_sta.nvme_sta_w7),
                           0, 0, 0, 0)

  // Load the SSD SQ context for the next stage to push the NVME command
  // TODO: FIXME. This has multiple destinations (PVM/ROCE). Need to 
  //       qualify dest_addr
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                        Q_STATE_SIZE, q_state_push_start)
}
