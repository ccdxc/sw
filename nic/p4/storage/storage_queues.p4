

#include "common/storage_p4_defines.h"


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
@pragma little_endian p_ndx c_ndx
action q_state_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                   total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                   num_entries, base_addr, entry_size, next_pc,
                   dst_lif, dst_qtype, dst_qid,  dst_qaddr, vf_id, sq_id, ssd_bm_addr, 
                   ssd_q_num, ssd_q_size, ssd_ci_addr, pad) {

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
    modify_field(storage_kivec0.dst_lif, dst_lif);
    modify_field(storage_kivec0.dst_qtype, dst_qtype);
    modify_field(storage_kivec0.dst_qid, dst_qid);
    modify_field(storage_kivec0.dst_qaddr, dst_qaddr);
    modify_field(storage_kivec0.ssd_bm_addr, ssd_bm_addr);
    modify_field(storage_kivec1.device_addr, ssd_ci_addr);

    // In ASM, derive these from the K+I for stage 0
    modify_field(storage_kivec1.src_qaddr, 0);
    modify_field(storage_kivec1.src_lif, 0);
    modify_field(storage_kivec1.src_qtype, 0);
    modify_field(storage_kivec1.src_qid, 0);
    modify_field(storage_kivec0.is_q0, 0);

    // Initialize the vf_id and sq_id fields in the PHV
    modify_field(pvm_cmd_trailer.vf_id, vf_id);
    modify_field(pvm_cmd_trailer.sq_id, sq_id);

    // Load the table and program for processing the queue entry in the next stage
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, q_state_scratch.base_addr,
                         q_state_scratch.w_ndx, q_state_scratch.entry_size,
                         q_state_scratch.entry_size, q_state_scratch.next_pc)
  }
}

/*****************************************************************************
 *  q_state_push: Push to a queue by issuing the DMA commands and incrementing
 *                the p_ndx via ringing the doorbell. Assumes that data to be
 *                pushed is in DMA command 1.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                    total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                    num_entries, base_addr, entry_size, next_pc, 
                    dst_lif, dst_qtype, dst_qid, dst_qaddr, vf_id, sq_id, ssd_bm_addr, 
                    ssd_q_num, ssd_q_size, ssd_ci_addr, pad) {

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
                                          DOORBELL_SCHED_WR_SET, 
                                          DOORBELL_UPDATE_NONE));
    modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_4, 
                             0,
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             0, 0, 0, 0)

    // Exit the pipeline here 
  }
}


/*****************************************************************************
 *  pci_q_state_push: Push to a queue across the PCI bus (e.g. NVME driver, 
 *                    SSD, PVM) by issuing the DMA commands to write the p_ndx
 *                    and trigger the MSI-X interrupt.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action pci_q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                        num_entries, base_addr, entry_size, push_addr,
                        intr_addr, intr_data, intr_en, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  PCI_Q_STATE_COPY(pci_q_state_scratch)

  // Check for queue full condition before pushing
  if (QUEUE_FULL(pci_q_state_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  }

  // Modify the DMA command 1 to fill the source address based on p_ndx 
  // NOTE: This API in P4 land will not work, but in ASM we can selectively
  // overwrite the fields
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           pci_q_state_scratch.base_addr +
                           (pci_q_state_scratch.p_ndx * pci_q_state_scratch.entry_size),
                           0, 0,
                           0, 0, 0, 0)

  // Push the entry to the queue.  
  QUEUE_PUSH(pci_q_state_scratch)

  // Setup the DMA command to push the entry by incrementing p_ndx
  modify_field(doorbell_addr.addr, pci_q_state_scratch.push_addr);
  modify_field(pci_push_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_4, 
                           0,
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           0, 0, 0, 0)

  // Raise the MSIx interrupt if enabled
  if (pci_q_state_scratch.intr_en == 1) {
    modify_field(pci_intr_addr.addr, pci_q_state_scratch.intr_addr);
    modify_field(pci_intr_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_5, 
                             0,
                             PHV_FIELD_OFFSET(intr_data.data),
                             PHV_FIELD_OFFSET(intr_data.data),
                             0, 0, 0, 0)
  }

  // Exit the pipeline here 
}

/*****************************************************************************
 *  pvm_roce_sq_cb_pop : Check the queue state and see if there's anything to 
 *                       be reclaimed by comparing cindex against the 
 *                       (roce_msn % num_entries). If so increment the working 
 *                       index and load the queue entry.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action pvm_roce_sq_cb_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                          total_rings, host_rings, pid, p_ndx, c_ndx, base_addr, 
                          page_size, entry_size, num_entries, rsvd0, roce_msn, 
                          w_ndx, next_pc, rrq_lif, rrq_qtype,
                          rrq_qid, rrq_qaddr, rsq_lif, rsq_qtype, rsq_qid, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PVM_ROCE_SQ_CB_COPY_STAGE0(pvm_roce_sq_cb_scratch)

  // If dequeued upto roce_msn, exit the pipeline. Note that roce_msn is a
  // running sequence number across the lifetime of this queue. So modulo 
  // against the number of entries in the queue needs to be done before 
  // checking against the c_ndx.
  if (pvm_roce_sq_cb_scratch.c_ndx == 
      (pvm_roce_sq_cb_scratch.roce_msn & ((1 << pvm_roce_sq_cb_scratch.num_entries) - 1))) {
    exit();
  } else {
    // Increment the working consumer index. In ASM this should be a table write.
    QUEUE_POP(pvm_roce_sq_cb_scratch)
   
    // Store fields needed in the K+I vector
    // Note: The ROCE LIF actually points to ROCE RQ which is where the R2N buffer 
    //       is to be posted when PVM's view of the ROCE SQ is popped
    modify_field(storage_kivec0.w_ndx, w_ndx);
    modify_field(storage_kivec0.dst_lif, rrq_lif);
    modify_field(storage_kivec0.dst_qtype, rrq_qtype);
    modify_field(storage_kivec0.dst_qid, rrq_qid);
    modify_field(storage_kivec0.dst_qaddr, rrq_qaddr);

    // In ASM, derive these from the K+I for stage 0
    modify_field(storage_kivec1.src_qaddr, 0);
    modify_field(storage_kivec1.src_lif, 0);
    modify_field(storage_kivec1.src_qtype, 0);
    modify_field(storage_kivec1.src_qid, 0);
    modify_field(storage_kivec0.is_q0, 0);

    // Load the table and program for processing the queue entry in the next stage
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, 
                         pvm_roce_sq_cb_scratch.base_addr,
                         pvm_roce_sq_cb_scratch.w_ndx, 
                         pvm_roce_sq_cb_scratch.entry_size,
                         pvm_roce_sq_cb_scratch.entry_size, 
                         pvm_roce_sq_cb_scratch.next_pc)
  }
}

/*****************************************************************************
 *  roce_cq_cb_pop : Check the ROCE CQ QPCB and see if there's anything to
 *                   be popped. If so increment the working index and load
 *                   the queue entry.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action roce_cq_cb_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                      total_rings, host_rings, pid, p_ndx, c_ndx, base_addr, 
                      page_size, entry_size, num_entries, rsvd0, cq_id, eq_id,
                      rsvd1, w_ndx, next_pc, xlate_addr, rcq_lif, rcq_qtype,
                      rcq_qid, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  ROCE_CQ_CB_COPY_STAGE0(roce_cq_cb_scratch)

  // If queue is empty exit
  if (QUEUE_EMPTY(roce_cq_cb_scratch)) {
    exit();
  } else {
    // Increment the working consumer index. In ASM this should be a table write.
    QUEUE_POP(roce_cq_cb_scratch)
   
    // Store fields needed in the K+I vector. Copy the xlate table's address into 
    // the dst_qaddr. This will be modified after use  in the next stage.
    modify_field(storage_kivec0.w_ndx, w_ndx);
    modify_field(storage_kivec0.dst_qaddr, xlate_addr);

    // In ASM, derive these from the K+I for stage 0
    modify_field(storage_kivec1.src_qaddr, 0);
    modify_field(storage_kivec1.src_lif, 0);
    modify_field(storage_kivec1.src_qtype, 0);
    modify_field(storage_kivec1.src_qid, 0);
    modify_field(storage_kivec0.is_q0, 0);

    // Load the table and program for processing the queue entry in the next stage
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, roce_cq_cb_scratch.base_addr,
                         roce_cq_cb_scratch.w_ndx, roce_cq_cb_scratch.entry_size,
                         roce_cq_cb_scratch.entry_size, roce_cq_cb_scratch.next_pc)
  }
}

/*****************************************************************************
 *  roce_cq_cb_push : Dummy push function to generate the D vector.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action roce_cq_cb_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                       total_rings, host_rings, pid, p_ndx, c_ndx, base_addr, 
                       page_size, entry_size, num_entries, rsvd0, cq_id, eq_id,
                       rsvd1, w_ndx, next_pc, xlate_addr, rcq_lif, rcq_qtype,
                       rcq_qid, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  ROCE_CQ_CB_COPY(roce_cq_cb_scratch)
}

/*****************************************************************************
 *  pvm_roce_sq_cb_push: Push a ROCE SQ WQE by issuing the DMA commands to 
 *                       write the ROCE SQ WQE and incrementing the p_ndx via
 *                       ringing the doorbell. Assumes SQ WQE to be pushed is 
 *                       in DMA command 1. 
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action pvm_roce_sq_cb_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                           total_rings, host_rings, pid, p_ndx, c_ndx, base_addr, 
                           page_size, entry_size, num_entries, rsvd0, roce_msn, 
                           w_ndx, next_pc, rrq_lif, rrq_qtype,
                           rrq_qid, rrq_qaddr, rsq_lif, rsq_qtype, rsq_qid, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  PVM_ROCE_SQ_CB_COPY(pvm_roce_sq_cb_scratch)

  // Check for queue full condition before pushing both read data and status
  if ((storage_kivec0.is_read == 1) and QUEUE_FULL2(pvm_roce_sq_cb_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();
  }

  // Check for queue full condition before pushing only status
  if ((storage_kivec0.is_read == 0) and QUEUE_FULL(pvm_roce_sq_cb_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();
  }

  // If read data needs to be sent, do that
  if (storage_kivec0.is_read == 1) {
    // Update destination in the mem2mem DMA without touching the source. This wont 
    // work in P4 as there is no update API for mem2mem DMA. In ASM, there are two APIs.
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_3, dma_m2m_4, 0, 0, 
                             pvm_roce_sq_cb_scratch.base_addr +
                             (pvm_roce_sq_cb_scratch.p_ndx * pvm_roce_sq_cb_scratch.entry_size),
                             0, ROCE_SQ_WQE_SIZE, 0, 0, 0)


    // Push the entry to the queue. In ASM tblwr of pndx. 
    QUEUE_PUSH(pvm_roce_sq_cb_scratch)
  }
  
  // Update destination in the mem2mem DMA without touching the source. This wont 
  // work in P4 as there is no update API for mem2mem DMA. In ASM, there are two APIs.
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_5, dma_m2m_6, 0, 0, 
                           pvm_roce_sq_cb_scratch.base_addr +
                           (pvm_roce_sq_cb_scratch.p_ndx * pvm_roce_sq_cb_scratch.entry_size),
                           0, ROCE_SQ_WQE_SIZE, 0, 0, 0)


  // Push the entry to the queue. In ASM tblwr of pndx. 
  QUEUE_PUSH(pvm_roce_sq_cb_scratch)
  
  // Form the doorbell and setup the DMA command to push the entry by
  // incrementing p_ndx
  modify_field(doorbell_addr.addr,
               STORAGE_DOORBELL_ADDRESS(pvm_roce_sq_cb_scratch.rsq_qtype, 
                                        pvm_roce_sq_cb_scratch.rsq_lif,
                                        DOORBELL_SCHED_WR_SET, 
                                        DOORBELL_UPDATE_P_NDX));
  modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_7, 
                           0,
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           0, 0, 0, 0)

}

/*****************************************************************************
 *  roce_rq_push: Push a ROCE RQ WQE by issuing the DMA commands to write
 *                the ROCE RQ WQE and incrementing the p_ndx via ringing the 
 *                doorbell. Assumes RQ WQE to be pushed is in DMA command 1.
 *                This is used to post the R2N buffer to ROCE RQ.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action roce_rq_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                    total_rings, host_rings, pid, p_ndx, c_ndx, extra_rings,
                    base_addr, rsvd0, pmtu, page_size, entry_size, num_entries,
                    pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  ROCE_RQ_CB_COPY(roce_rq_cb_scratch)

  // Check for queue full condition before pushing
  if (QUEUE_FULL(roce_rq_cb_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  } else {

    // Update destination in the mem2mem DMA without touching the source. This wont 
    // work in P4 as there is no update API for mem2mem DMA. In ASM, there are two APIs.
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, 0, 0, 
                             roce_rq_cb_scratch.base_addr +
                             (roce_rq_cb_scratch.p_ndx * roce_rq_cb_scratch.entry_size),
                             0, ROCE_RQ_WQE_SIZE, 0, 0, 0)


    // Push the entry to the queue. In ASM tblwr of pndx. 
    QUEUE_PUSH(roce_rq_cb_scratch)

    // Form the doorbell and setup the DMA command to push the entry by
    // incrementing p_ndx
    modify_field(doorbell_addr.addr,
                 STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype, 
                                          storage_kivec0.dst_lif,
                                          DOORBELL_SCHED_WR_SET, 
                                          DOORBELL_UPDATE_P_NDX));
    modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_4, 
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

@pragma little_endian p_ndx_hi c_ndx_hi p_ndx_med c_ndx_med p_ndx_lo c_ndx_lo
action pri_q_state_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                       total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                       p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                       w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                       hi_weight, med_weight, lo_weight, hi_running, 
                       med_running, lo_running, num_running, max_cmds,
                       next_pc, dst_lif, dst_qtype, dst_qid, dst_qaddr, 
                       ssd_bm_addr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  PRI_Q_STATE_COPY_STAGE0(pri_q_state_scratch)

  // Store fields needed in the K+I vector
  modify_field(storage_kivec0.dst_lif, dst_lif);
  modify_field(storage_kivec0.dst_qtype, dst_qtype);
  modify_field(storage_kivec0.dst_qid, dst_qid);
  modify_field(storage_kivec0.dst_qaddr, dst_qaddr);
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

@pragma little_endian p_ndx_hi c_ndx_hi p_ndx_med c_ndx_med p_ndx_lo c_ndx_lo
action pri_q_state_incr(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                        p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                        w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                        hi_weight, med_weight, lo_weight, hi_running, 
                        med_running, lo_running, num_running, max_cmds,
                        next_pc, dst_lif, dst_qtype, dst_qid, dst_qaddr, 
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

@pragma little_endian p_ndx_hi c_ndx_hi p_ndx_med c_ndx_med p_ndx_lo c_ndx_lo
action pri_q_state_decr(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                        p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                        w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                        hi_weight, med_weight, lo_weight, hi_running, 
                        med_running, lo_running, num_running, max_cmds,
                        next_pc, dst_lif, dst_qtype, dst_qid, dst_qaddr, 
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

@pragma little_endian p_ndx_hi c_ndx_hi p_ndx_med c_ndx_med p_ndx_lo c_ndx_lo
action pri_q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                        p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                        w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                        hi_weight, med_weight, lo_weight, hi_running, 
                        med_running, lo_running, num_running, max_cmds,
                        next_pc, dst_lif, dst_qtype, dst_qid, dst_qaddr, 
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
 *  seq_q_state_push: Push to a sequencer destination queue by issuing the 
 *                    mem2mem DMA commands and incrementing the p_ndx via 
 *                    ringing the doorbell. Assumes that data to be pushed has
 *                    its source in DMA cmd 1 and destination in DMA cmd 2.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx 
action seq_q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                        num_entries, base_addr, entry_size, next_pc, 
                        dst_lif, dst_qtype, dst_qid, dst_qaddr, vf_id, sq_id, ssd_bm_addr, 
                        ssd_q_num, ssd_q_size, ssd_ci_addr, pad) {

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

    // Modify the DMA command 2 to fill the destination address based on p_ndx 
    // NOTE: This API in P4 land will not work, but in ASM we can selectively
    // overwrite the fields
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, 
                             q_state_scratch.base_addr +
                             (q_state_scratch.p_ndx * q_state_scratch.entry_size),
                             0, 0, 0, 0, 0, 0, 0)

    // Push the entry to the queue.  
    QUEUE_PUSH(q_state_scratch)

    // Form the doorbell and setup the DMA command to push the entry by
    // incrementing p_ndx
    modify_field(doorbell_addr.addr,
                 STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype, 
                                          storage_kivec0.dst_lif,
                                          DOORBELL_SCHED_WR_SET, 
                                          DOORBELL_UPDATE_NONE));
    modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             0, 0, 0, 0)

    // Exit the pipeline here 
  }
}

/*****************************************************************************
 *  seq_pvm_roce_sq_cb_push: Push to a ROCE SQ (using PVM's SQ CB context) by 
 *                           issuing the mem2mem DMA commands and incrementing 
 *                           the p_ndx via ringing the doorbell. Assumes that 
 *                           data to be pushed has its source in DMA cmd 1 
 *                           and destination in DMA cmd 2.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx 
action seq_pvm_roce_sq_cb_push(pc_offset, rsvd, cosA, cosB, cos_sel, 
                               eval_last, total_rings, host_rings, pid, 
                               p_ndx, c_ndx, base_addr, page_size, entry_size, 
                               num_entries, rsvd0, roce_msn, w_ndx, next_pc, 
                               rrq_lif, rrq_qtype, rrq_qid, rrq_qaddr, rsq_lif, 
                               rsq_qtype, rsq_qid, pad) {


  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  PVM_ROCE_SQ_CB_COPY(pvm_roce_sq_cb_scratch)

  // Check for queue full condition before pushing
  if (QUEUE_FULL(pvm_roce_sq_cb_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  } else {

    // Modify the DMA command 2 to fill the destination address based on p_ndx 
    // NOTE: This API in P4 land will not work, but in ASM we can selectively
    // overwrite the fields
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, 
                             pvm_roce_sq_cb_scratch.base_addr +
                             (pvm_roce_sq_cb_scratch.p_ndx * q_state_scratch.entry_size),
                             0, 0, 0, 0, 0, 0, 0)

    // Push the entry to the queue.  
    QUEUE_PUSH(pvm_roce_sq_cb_scratch)

    // Form the doorbell and setup the DMA command to push the entry by
    // incrementing p_ndx
    modify_field(doorbell_addr.addr,
                 STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype, 
                                          storage_kivec0.dst_lif,
                                          DOORBELL_SCHED_WR_SET, 
                                          DOORBELL_UPDATE_NONE));
    modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             0, 0, 0, 0)

    // Exit the pipeline here 
  }
}
