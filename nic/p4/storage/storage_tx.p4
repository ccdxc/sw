

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
#define tx_table_s1_t0_action5	seq_pdma_entry_handler
#define tx_table_s1_t0_action6	seq_r2n_entry_handler
#define tx_table_s1_t0_action7	seq_barco_entry_handler

#define tx_table_s2_t0_action	q_state_push
#define tx_table_s2_t0_action1	seq_q_state_push
#define tx_table_s2_t0_action2	pci_q_state_push
#define tx_table_s2_t0_action3	seq_barco_ring_push
#define tx_table_s2_t0_action4	nvme_be_wqe_prep
#define tx_table_s2_t0_action5	nvme_be_cmd_handler
#define tx_table_s2_t0_action6	nvme_be_wqe_handler

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
@pragma pa_header_union ingress to_stage_2
metadata storage_kivec2_t storage_kivec2;

// NVME command (occupies full flit, so keep it first) 
@pragma dont_trim
metadata nvme_cmd_t nvme_cmd;

// PVM command metadata (immediately follows NVME command as phv2mem DMA of 
// both these are done together)
@pragma dont_trim
metadata pvm_cmd_trailer_t pvm_cmd_trailer;

// R2N work queue entry 
@pragma dont_trim
metadata r2n_wqe_t r2n_wqe;

// SSD's consumer index
@pragma dont_trim
metadata ssd_ci_t ssd_ci;

// TODO: Remove this if NCC does not add pads to DMA regions with pragma
@pragma dont_trim
metadata storage_pad0_t storage_pad0;

// NVME backend status 
@pragma dont_trim
metadata nvme_be_sta_hdr_t nvme_be_sta_hdr;

// NVME status metadata (immediately follows NVME backend status header as 
// phv2mem DMA of both these are done together)
@pragma dont_trim
metadata nvme_sta_t nvme_sta;

// Push/Pop doorbells
@pragma dont_trim
metadata storage_doorbell_data_t qpush_doorbell_data;
@pragma dont_trim
metadata storage_doorbell_data_t qpop_doorbell_data;
@pragma dont_trim
metadata storage_doorbell_data_t seq_doorbell_data;

// Push/interrupt data across PCI bus
@pragma dont_trim
metadata storage_pci_data_t pci_push_data;
@pragma dont_trim
metadata storage_pci_data_t pci_intr_data;

#if 0
// TODO: Remove this when NCC supports pragma for aligning this at 16 byte boundary
@pragma dont_trim
metadata storage_pad1_t storage_pad1;
#endif
  
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

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_4;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_4
metadata dma_cmd_mem2mem_t dma_m2m_4;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_5;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_5
metadata dma_cmd_mem2mem_t dma_m2m_5;

/*****************************************************************************
 * Storage Tx PHV layout END 
 *****************************************************************************/


// Scratch metadatas to get d-vector generated correctly

@pragma scratch_metadata
metadata q_state_t q_state_scratch;

@pragma scratch_metadata
metadata pri_q_state_t pri_q_state_scratch;

@pragma scratch_metadata
metadata pci_q_state_t pci_q_state_scratch;

@pragma scratch_metadata
metadata barco_xts_ring_t barco_xts_ring_scratch;

@pragma scratch_metadata
metadata nvme_be_cmd_hdr_t nvme_be_cmd_hdr;

@pragma scratch_metadata
metadata storage_doorbell_addr_t doorbell_addr;

@pragma scratch_metadata
metadata storage_doorbell_addr_t pci_intr_addr;

@pragma scratch_metadata
metadata storage_kivec0_t storage_kivec0_scratch;

@pragma scratch_metadata
metadata storage_kivec1_t storage_kivec1_scratch;

@pragma scratch_metadata
metadata storage_kivec2_t storage_kivec2_scratch;

@pragma scratch_metadata
metadata ssd_cmds_t ssd_cmds;

@pragma scratch_metadata
metadata r2n_wqe_t r2n_wqe_scratch;

@pragma scratch_metadata
metadata pvm_sta_trailer_t pvm_sta_trailer;

@pragma scratch_metadata
metadata seq_pdma_entry_t seq_pdma_entry;

@pragma scratch_metadata
metadata seq_r2n_entry_t seq_r2n_entry;

@pragma scratch_metadata
metadata seq_barco_entry_t seq_barco_entry;

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
                   ssd_bm_addr, ssd_q_num, ssd_q_size, ssd_ci_addr, pad) {

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
    modify_field(storage_kivec1.ssd_ci_addr, ssd_ci_addr);

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
  NVME_CMD_COPY(nvme_cmd)

  // Initialize the remaining fields of the PVM command in the PHV
  modify_field(pvm_cmd_trailer.num_prps, 0);
  modify_field(pvm_cmd_trailer.tickreg, 0);

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
    modify_field(pvm_cmd_trailer.num_prps, NVME_MAX_XTRA_PRPS);
  }

  // H/W assist and PRPs not capped to Max
  if ((storage_kivec0.prp_assist == 1) and
      (NVME_MAX_XTRA_PRPS > PRP_SIZE(dptr2) >> 3)) {
    modify_field(pvm_cmd_trailer.num_prps, ((PRP_SIZE(dptr2)) >> 3));
  }

  // Setup the DMA command to push the NVME command entry
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(pvm_cmd_trailer.opc),
                           PHV_FIELD_OFFSET(pvm_cmd_trailer.tickreg),
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

action pvm_cq_handler(cspec, rsvd, sq_head, sq_id, cid, phase, status,
                      dst_lif, dst_qtype, dst_qid, dst_qaddr) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Carry forward NVME status information to be sent to driver in the PHV 
  NVME_STATUS_COPY(nvme_sta)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(pvm_sta_trailer.dst_lif, dst_lif);
  modify_field(pvm_sta_trailer.dst_qtype, dst_qtype);
  modify_field(pvm_sta_trailer.dst_qid, dst_qid);
  modify_field(pvm_sta_trailer.dst_qaddr, dst_qaddr);

  // Overwrite the K+I vector for the push operation to derive the correct 
  // destination queue
  modify_field(storage_kivec0.dst_lif, dst_lif);
  modify_field(storage_kivec0.dst_qtype, dst_qtype);
  modify_field(storage_kivec0.dst_qid, dst_qid);
  modify_field(storage_kivec0.dst_qaddr, dst_qaddr);

  // Setup the DMA command to push the NVME status entry
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(nvme_sta.cspec),
                           PHV_FIELD_OFFSET(nvme_sta.status),
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
                    ssd_bm_addr, ssd_q_num, ssd_q_size, ssd_ci_addr, pad) {

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
 *  pri_q_state_pop : Check the queue state and see if there's anything to be 
 *                    popped from any priority rings. If so increment the 
 *                    working index for that ring and load the queue entry.
 *****************************************************************************/

action pri_q_state_pop(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                       total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                       p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                       w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                       hi_weight, med_weight, lo_weight, hi_running, 
                       med_running, lo_running, num_running, max_cmds,
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
                        total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                        p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                        w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                        hi_weight, med_weight, lo_weight, hi_running, 
                        med_running, lo_running, num_running, max_cmds,
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
                        total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                        p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                        w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                        hi_weight, med_weight, lo_weight, hi_running, 
                        med_running, lo_running, num_running, max_cmds,
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
                        total_rings, host_rings, pid, p_ndx_hi, c_ndx_hi, 
                        p_ndx_med, c_ndx_med, p_ndx_lo, c_ndx_lo, w_ndx_hi,
                        w_ndx_med, w_ndx_lo, num_entries, base_addr, entry_size,
                        hi_weight, med_weight, lo_weight, hi_running, 
                        med_running, lo_running, num_running, max_cmds,
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

action r2n_sq_handler(handle, data_size, opcode, status, db_enable, db_lif,
                      db_qtype, db_qid, db_index) {

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
  STORAGE_KIVEC2_USE(storage_kivec2_scratch, storage_kivec2)

  // Carry forward state information to be saved with R2N WQE in PHV
  NVME_BE_CMD_HDR_COPY(r2n_wqe)
  modify_field(r2n_wqe.pri_qaddr, storage_kivec0.dst_qaddr);

  // Store fields needed in the K+I vector
  modify_field(storage_kivec0.ssd_handle, ssd_handle);
  modify_field(storage_kivec0.io_priority, io_priority);

  // Setup the DMA command to push the modified R2N WQE to NVME backend
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(r2n_wqe.handle),
                           PHV_FIELD_OFFSET(r2n_wqe.pri_qaddr),
                           0, 0, 0, 0)

  // Load the NVME backkend SQ context for the next stage to push the command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        storage_kivec0.dst_qaddr + 
                        nvme_be_cmd_hdr.ssd_handle * storage_kivec2.ssd_q_size,
                        Q_STATE_SIZE, pri_q_state_push_start)
}

/*****************************************************************************
 *  nvme_be_sq_handler: Read the NVME backend priority submission queue entry.
 *                      Load the actual NVME command for the next stage.
 *****************************************************************************/

action nvme_be_sq_handler(handle, data_size, opcode, status, db_enable, 
                          db_lif, db_qtype, db_qid, db_index, src_queue_id, 
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
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, handle + NVME_BE_NVME_CMD_OFFSET,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, nvme_be_cmd_handler_start)
}

/*****************************************************************************
 *  nvme_be_cmd_handler: Save the NVME command to PHV. Load the address of the
 *                       the priority queue state again to increment the 
 *                       running counters in the next stage.
 *****************************************************************************/

action nvme_be_cmd_handler(opc, fuse, rsvd0, psdt, cid, nsid, rsvd2, rsvd3,
                           mptr, dptr1, dptr2, slba, nlb, rsvd12, prinfo,
                           fua, lr, dsm, rsvd13, dw14, dw15) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Carry forward NVME status information in the PHV 
  NVME_CMD_COPY(nvme_cmd)

  // Store the original value of NVME command id in R2N WQE in PHV. This will
  // be saved to HBM so that this command id can be restored when processing
  // the NVME status.
  modify_field(r2n_wqe.nvme_cmd_cid, cid);

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
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_2,
                           0,
                           PHV_FIELD_OFFSET(r2n_wqe.handle),
                           PHV_FIELD_OFFSET(r2n_wqe.pri_qaddr),
                           0, 0, 0, 0)

  // Load the SSD SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                        Q_STATE_SIZE, pci_q_state_push_start)
}

/*****************************************************************************
 *  nvme_be_cq_handler: Save the NVME status into PHV. Load the saved R2N WQE
 *                      for the command that was sent to the SSD
 *****************************************************************************/

action nvme_be_cq_handler(cspec, rsvd, sq_head, sq_id, cid, phase, status) { 

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Carry forward NVME status information in the PHV 
  NVME_STATUS_COPY(nvme_sta)

  // Set the state information for the NVME backend status header
  // TODO: FIXME
  modify_field(nvme_be_sta_hdr.time_us, 0);
  modify_field(nvme_be_sta_hdr.be_status, 0);
  modify_field(nvme_be_sta_hdr.is_q0, 0);
  modify_field(nvme_be_sta_hdr.rsvd, 0);

  // Store the SSD's c_ndx value for DMA to the NVME backend SQ
  modify_field(ssd_ci.c_ndx, sq_head);

  // Derive the save command index from the NVME status
  modify_field(storage_kivec0_scratch.cmd_index, (0xFF & cid));

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Setup the DMA command to push the sq_head to the c_ndx of the SSD
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_2, 
                           storage_kivec1.ssd_ci_addr,
                           PHV_FIELD_OFFSET(ssd_ci.c_ndx),
                           PHV_FIELD_OFFSET(ssd_ci.c_ndx),
                           0, 0, 0, 0)

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


action nvme_be_wqe_handler(handle, data_size, opcode, status, db_enable, 
                           db_lif, db_qtype, db_qid, db_index, src_queue_id, 
                           ssd_handle, io_priority, is_read, r2n_buf_handle, 
                           is_local, nvme_cmd_cid, pri_qaddr) {

  // For D vector generation (type inference). No need to translate this to ASM.
  R2N_WQE_FULL_COPY(r2n_wqe_scratch)

  // Restore the fields in the NVME backend status to saved values
  modify_field(nvme_be_sta_hdr.r2n_buf_handle, r2n_wqe_scratch.r2n_buf_handle);
  modify_field(nvme_sta.cid, r2n_wqe_scratch.nvme_cmd_cid);

  // Store fields needed in the K+I vector
  modify_field(storage_kivec0.ssd_handle, ssd_handle);
  modify_field(storage_kivec0.io_priority, io_priority);

  // Rewrite the destination as needed
  if (r2n_wqe_scratch.db_enable == 1) {
    // TODO: In ASM calculate from LIF/Type/Qid etc.
    modify_field(doorbell_addr.addr, 0);
    modify_field(seq_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             0, 0, 0, 0)
  }

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
                           PHV_FIELD_OFFSET(nvme_be_sta.status_phase),
                           0, 0, 0, 0)

  // Load the R2N CQ context for the next stage to push the completion
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                        Q_STATE_SIZE, q_state_push_start)
}

/*****************************************************************************
 *  seq_pdma_entry_handler: Handle the PDMA entry in sequencer. Issue the DMA
 *                          command to copy data and ring the next doorbell 
 *                          with a fence bit to guarantee ordering.
 *****************************************************************************/

action seq_pdma_entry_handler(next_db_addr, next_db_data, src_addr, dst_addr,
                              data_size, src_lif_override, src_lif,
                              dst_lif_override, dst_lif) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_pdma_entry.next_db_addr, next_db_addr);
  modify_field(seq_pdma_entry.next_db_data, next_db_data);
  modify_field(seq_pdma_entry.src_addr, src_addr);
  modify_field(seq_pdma_entry.dst_addr, dst_addr);
  modify_field(seq_pdma_entry.data_size, data_size);
  modify_field(seq_pdma_entry.src_lif_override, src_lif_override);
  modify_field(seq_pdma_entry.src_lif, src_lif);
  modify_field(seq_pdma_entry.dst_lif_override, dst_lif_override);
  modify_field(seq_pdma_entry.dst_lif, dst_lif);

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Setup the DMA command to move the data from source to destination address
  // In ASM, set the host, fence bits etc correctly
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, seq_pdma_entry.src_addr, 0,
                           seq_pdma_entry.dst_addr, 0, seq_pdma_entry.data_size,
                           0, 0, 0)

  // Copy the doorbell data to PHV
  modify_field(qpush_doorbell_data.data, seq_pdma_entry.next_db_data);

  // Setup the doorbell to be rung based on a fence with the previous mem2mem DMA
  modify_field(doorbell_addr.addr, seq_pdma_entry.next_db_addr);
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                           0,
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           0, 0, 0, 0)

  // Exit the pipeline here 
}

/*****************************************************************************
 *  seq_r2n_entry_handler: Handle the R2N WQE entry in sequencer. Form the DMA
 *                         command to copy the R2N WQE as part of the push 
 *                         operation in the next stage.
 *****************************************************************************/

action seq_r2n_entry_handler(r2n_wqe_addr, r2n_wqe_size, dst_lif, dst_qtype,
                             dst_qid, dst_qaddr) {

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_r2n_entry.r2n_wqe_addr, r2n_wqe_addr);
  modify_field(seq_r2n_entry.r2n_wqe_size, r2n_wqe_size);
  modify_field(seq_r2n_entry.dst_lif, dst_lif);
  modify_field(seq_r2n_entry.dst_qtype, dst_qtype);
  modify_field(seq_r2n_entry.dst_qid, dst_qid);
  modify_field(seq_r2n_entry.dst_qaddr, dst_qaddr);

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Overwrite the K+I vector for the push operation to derive the correct 
  // destination queue
  modify_field(storage_kivec0.dst_lif, dst_lif);
  modify_field(storage_kivec0.dst_qtype, dst_qtype);
  modify_field(storage_kivec0.dst_qid, dst_qid);
  modify_field(storage_kivec0.dst_qaddr, dst_qaddr);

  // Setup the DMA command to move the data from source to destination address
  // In ASM, set the host, fence bits etc correctly
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, seq_r2n_entry.r2n_wqe_addr, 0,
                           0, 0, seq_r2n_entry.r2n_wqe_size, 0, 0, 0)

  // Load the Serivce SQ context for the next stage to push the NVME command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                        Q_STATE_SIZE, seq_q_state_push_start)
}

/*****************************************************************************
 *  seq_q_state_push: Push to a sequencer destination queue by issuing the 
 *                    mem2mem DMA commands and incrementing the p_ndx via 
 *                    ringing the doorbell. Assumes that data to be pushed has
 *                    its source in DMA cmd 1 and destination in DMA cmd 2.
 *****************************************************************************/

action seq_q_state_push(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                        total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                        num_entries, base_addr, entry_size, next_pc, dst_qaddr,
                        dst_lif, dst_qtype, dst_qid, vf_id, sq_id,
                        ssd_bm_addr, ssd_q_num, ssd_q_size, ssd_ci_addr, pad) {

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
    DMA_COMMAND_PHV2MEM_FILL(dma_m2m_2, 
                             q_state_scratch.base_addr +
                             (q_state_scratch.p_ndx * q_state_scratch.entry_size),
                             0, 0,
                             0, 0, 0, 0)

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

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             0, 0, 0, 0)

    // Exit the pipeline here 
  }
}

/*****************************************************************************
 *  seq_barco_entry_handler: Handle the Barco XTS entry in sequencer. Form the
 *                           DMA command to copy the Barco XTS descriptor as 
 *                           part of the push operation in the next stage.
 *****************************************************************************/

action seq_barco_entry_handler(xts_desc_addr, xts_desc_size, xts_pndx_size,
                               xts_pndx_addr, xts_ring_addr) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(seq_barco_entry.xts_desc_addr, xts_desc_addr);
  modify_field(seq_barco_entry.xts_desc_size, xts_desc_size);
  modify_field(seq_barco_entry.xts_pndx_size, xts_pndx_size);
  modify_field(seq_barco_entry.xts_pndx_addr, xts_pndx_addr);
  modify_field(seq_barco_entry.xts_ring_addr, xts_ring_addr);

  // Update the K+I vector with the barco descriptor size to be used 
  // when calculating the offset for the push operation 
  modify_field(storage_kivec1.xts_desc_size, seq_barco_entry.xts_desc_size);
  modify_field(storage_kivec1.ssd_ci_addr, seq_barco_entry.xts_ring_addr);

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Setup the DMA command to move the data from source to destination address
  // In ASM, set the host, fence bits etc correctly
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, seq_barco_entry.xts_desc_addr, 0,
                           0, 0, seq_barco_entry.xts_desc_size, 0, 0, 0)

  // Setup the doorbell to be rung based on a fence with the previous mem2mem 
  // DMA. Form the doorbell DMA command in this stage as opposed the push 
  // stage (as is the norm) to avoid carrying the doorbell address in K+I
  // vector.
  modify_field(doorbell_addr.addr, seq_barco_entry.xts_pndx_addr);
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                           0,
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                           0, 0, 0, 0)

  // Load the Barco ring for the next stage to push the Barco XTS descriptor
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        seq_barco_entry.xts_pndx_addr,
                        seq_barco_entry.xts_pndx_size, 
                        seq_barco_ring_push_start)
}

/*****************************************************************************
 *  seq_barco_ring_push: Push to Barco XTS ring by issuing the mem2mem DMA 
 *                       commands and incrementing the p_ndx via ringing the 
 *                       doorbell. Assumes that data to be pushed has its 
 *                       source in DMA cmd 1 and destination in DMA cmd 2.
 *****************************************************************************/

action seq_barco_ring_push(p_ndx) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(barco_xts_ring_scratch.p_ndx, p_ndx);

  // Copy the doorbell data to PHV
  modify_field(qpush_doorbell_data.data, barco_xts_ring_scratch.p_ndx);

  // Modify the DMA command 2 to fill the destination address based on p_ndx 
  // NOTE: This API in P4 land will not work, but in ASM we can selectively
  // overwrite the fields
  DMA_COMMAND_PHV2MEM_FILL(dma_m2m_2, 
                           storage_kivec1.ssd_ci_addr +
                           (barco_xts_ring_scratch.p_ndx * 
                            storage_kivec1.xts_desc_size),
                           0, 0,
                           0, 0, 0, 0)


  // Doorbell has already been setup

  // Exit the pipeline here 
}
