

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0 s0_tbl
#define tx_table_s1_t0 s1_tbl
#define tx_table_s2_t0 s2_tbl
#define tx_table_s3_t0 s3_tbl


#define tx_table_s0_t0_action q_state_pop
#define tx_table_s1_t0_action nvme_sq_handler
#define tx_table_s2_t0_action q_state_push
#define tx_table_s3_t0_action pvm_cq_handler

#include "../common-p4+/common_txdma.p4"

#include "common/storage_p4_defines.h"


@pragma scratch_metadata
metadata q_state_t q_state_scratch;

@pragma pa_header_union ingress common_t0_s2s
metadata storage_kivec0_t storage_kivec0;

@pragma pa_header_union ingress common_global
metadata storage_kivec1_t storage_kivec1;

@pragma scratch_metadata
metadata storage_kivec0_t storage_kivec0_scratch;

@pragma scratch_metadata
metadata storage_kivec1_t storage_kivec1_scratch;

// PVM Command metadata
@pragma dont_trim
metadata pvm_cmd_t pvm_cmd;
@pragma dont_trim
metadata pvm_status_t pvm_status;

// Push doorbell
@pragma scratch_metadata
metadata storage_doorbell_addr_t doorbell_addr;

@pragma dont_trim
metadata storage_doorbell_data_t qpush_doorbell_data;
  
@pragma dont_trim
metadata storage_doorbell_data_t qpop_doorbell_data;

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


/* Exit action handler needs to be stubbed out for NCC */
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
                   dst_lif, dst_qtype, dst_qid, vf_id, sq_id, pad) {

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

    // In ASM, derive these from the K+I for stage 0
    modify_field(storage_kivec0.is_q0, 0);
    modify_field(storage_kivec1.src_qaddr, 0);
    modify_field(storage_kivec1.src_lif, 0);
    modify_field(storage_kivec1.src_qtype, 0);
    modify_field(storage_kivec1.src_qid, 0);

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
  modify_field(doorbell_addr.addr,
               STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype, 
                                        storage_kivec0.dst_lif,
                                        DOORBELL_SCHED_WR_RESET, 
                                        DOORBELL_UPDATE_C_NDX));
  modify_field(qpop_doorbell_data.data,
               STORAGE_DOORBELL_DATA(storage_kivec0.w_ndx, 0, 0, 0));

  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_0, 
                           0,
                           PHV_FIELD_OFFSET(qpop_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpop_doorbell_data.data),
                           0, 0, 0, 0)

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
  modify_field(doorbell_addr.addr,
               STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype, 
                                        storage_kivec0.dst_lif,
                                        DOORBELL_SCHED_WR_RESET, 
                                        DOORBELL_UPDATE_C_NDX));
  modify_field(qpop_doorbell_data.data,
               STORAGE_DOORBELL_DATA(storage_kivec0.w_ndx, 0, 0, 0));

  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_0, 
                           0,
                           PHV_FIELD_OFFSET(qpop_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpop_doorbell_data.data),
                           0, 0, 0, 0)
                           

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
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        0, // In ASM: Use d.dst_qaddr,
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
                    dst_lif, dst_qtype, dst_qid, vf_id, sq_id, pad) {

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
