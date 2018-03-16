/*****************************************************************************
 * storage_tx.p4: Storage_tx P4+ program that implements PVM initiator and
 *                PVM target
 *****************************************************************************/

#include "common/storage_tx_p4_hdr.h"

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0		s0_tbl
#define tx_table_s1_t0		s1_tbl
#define tx_table_s2_t0		s2_tbl
#define tx_table_s3_t0		s3_tbl
#define tx_table_s4_t0		s4_tbl
#define tx_table_s5_t0		s5_tbl

#define tx_table_s0_t0_action	q_state_pop
#define tx_table_s0_t0_action1	pri_q_state_pop
#define tx_table_s0_t0_action2	roce_cq_cb_pop
#define tx_table_s0_t0_action3	pvm_roce_sq_cb_pop

#define tx_table_s1_t0_action	nvme_sq_handler
#define tx_table_s1_t0_action1	pvm_cq_handler
#define tx_table_s1_t0_action2	r2n_sq_handler
#define tx_table_s1_t0_action3	nvme_be_sq_handler
#define tx_table_s1_t0_action4	nvme_be_cq_handler
#define tx_table_s1_t0_action5	seq_pdma_entry_handler
#define tx_table_s1_t0_action6	seq_r2n_entry_handler
#define tx_table_s1_t0_action7	seq_barco_entry_handler
#define tx_table_s1_t0_action8	roce_cq_handler
#define tx_table_s1_t0_action9	pvm_roce_sq_wqe_process
#define tx_table_s1_t0_action10	seq_comp_status_desc_handler

#define tx_table_s2_t0_action	q_state_push
#define tx_table_s2_t0_action1	seq_q_state_push
#define tx_table_s2_t0_action2	pci_q_state_push
#define tx_table_s2_t0_action3	seq_barco_ring_push
#define tx_table_s2_t0_action4	nvme_be_wqe_prep
#define tx_table_s2_t0_action5	nvme_be_cmd_handler
#define tx_table_s2_t0_action6	nvme_be_wqe_handler
#define tx_table_s2_t0_action7	roce_sq_xlate
#define tx_table_s2_t0_action8	roce_rq_push
#define tx_table_s2_t0_action9	seq_pvm_roce_sq_cb_push
#define tx_table_s2_t0_action10	seq_comp_status_handler

#define tx_table_s3_t0_action	pri_q_state_push
#define tx_table_s3_t0_action1	pri_q_state_incr
#define tx_table_s3_t0_action2	pri_q_state_decr
#define tx_table_s3_t0_action3	roce_r2n_wqe_prep
#define tx_table_s3_t0_action4	pvm_roce_sq_cb_update
#define tx_table_s3_t0_action5	seq_comp_sgl_handler

#define tx_table_s4_t0_action	nvme_be_wqe_save
#define tx_table_s4_t0_action1	nvme_be_wqe_release

#define tx_table_s5_t0_action	pvm_roce_sq_cb_push
#define tx_table_s5_t0_action1	roce_cq_cb_push
// tx_table_s5_t0_action* actually correlates to other push
// operations like q_state_push (NVME backend) etc. But this
// is already defined in tx_table_s2_t0_action.

#include "../common-p4+/common_txdma.p4"

#include "storage_queues.p4"
#include "storage_seq.p4"


/*****************************************************************************
 * Storage Tx PHV layout BEGIN 
 * Will be processed by NCC in this order 
 *****************************************************************************/

// Global and stage to stage K+I vectors
@pragma pa_header_union ingress common_t0_s2s
metadata storage_kivec0_t storage_kivec0;
@pragma pa_header_union ingress common_t0_s2s
metadata storage_kivec4_t storage_kivec4;
@pragma pa_header_union ingress common_global
metadata storage_kivec1_t storage_kivec1;
@pragma pa_header_union ingress common_global
metadata storage_kivec5_t storage_kivec5;
@pragma pa_header_union ingress to_stage_2
metadata storage_kivec2_t storage_kivec2;
@pragma pa_header_union ingress to_stage_3
metadata storage_kivec3_t storage_kivec3;

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

// Keep the WQEs/commands that occupy full flit aligned at flit boundaries

// NVME command (occupies full flit)
@pragma dont_trim
metadata nvme_cmd_t nvme_cmd;

// PVM command metadata (immediately follows NVME command as phv2mem DMA of 
// both these are done together)
@pragma dont_trim
metadata pvm_cmd_trailer_t pvm_cmd_trailer;

// R2N work queue entry 
@pragma dont_trim
metadata r2n_wqe_t r2n_wqe;

// NVME backend status 
@pragma dont_trim
metadata nvme_be_sta_hdr_t nvme_be_sta_hdr;

// NVME status metadata (immediately follows NVME backend status header as 
// phv2mem DMA of both these are done together)
@pragma dont_trim
metadata nvme_sta_t nvme_sta;

// SSD's consumer index
@pragma dont_trim
metadata ssd_ci_t ssd_ci;

// Barco ring doorbell data
@pragma dont_trim
metadata barco_ring_t barco_doorbell_data;

// R2N data buffer address that needs to be fixed up for passing to ROCE
// for RDMA Write command
@pragma dont_trim
metadata storage_capri_addr_t r2n_data_buff_addr;

// DMA commands metadata
@pragma dont_trim
@pragma pa_align 128
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

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_6;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_6
metadata dma_cmd_mem2mem_t dma_m2m_6;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_7;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_7
metadata dma_cmd_mem2mem_t dma_m2m_7;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_8;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_8
metadata dma_cmd_mem2mem_t dma_m2m_8;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_9;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_9
metadata dma_cmd_mem2mem_t dma_m2m_9;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_10;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_10
metadata dma_cmd_mem2mem_t dma_m2m_10;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_11;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_11
metadata dma_cmd_mem2mem_t dma_m2m_11;

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
metadata barco_ring_t barco_ring_scratch;

@pragma scratch_metadata
metadata roce_cq_cb_t roce_cq_cb_scratch;

@pragma scratch_metadata
metadata pvm_roce_sq_cb_t pvm_roce_sq_cb_scratch;

@pragma scratch_metadata
metadata roce_rq_cb_t roce_rq_cb_scratch;

@pragma scratch_metadata
metadata nvme_be_cmd_hdr_t nvme_be_cmd_hdr_scratch;

@pragma scratch_metadata
metadata storage_capri_addr_t doorbell_addr_scratch;

@pragma scratch_metadata
metadata storage_capri_addr_t pci_intr_addr_scratch;

@pragma scratch_metadata
metadata storage_kivec0_t storage_kivec0_scratch;

@pragma scratch_metadata
metadata storage_kivec1_t storage_kivec1_scratch;

@pragma scratch_metadata
metadata storage_kivec2_t storage_kivec2_scratch;

@pragma scratch_metadata
metadata storage_kivec3_t storage_kivec3_scratch;

@pragma scratch_metadata
metadata storage_kivec4_t storage_kivec4_scratch;

@pragma scratch_metadata
metadata storage_kivec5_t storage_kivec5_scratch;

@pragma scratch_metadata
metadata ssd_cmds_t ssd_cmds_scratch;

@pragma scratch_metadata
metadata r2n_wqe_t r2n_wqe_scratch;

@pragma scratch_metadata
metadata pvm_sta_trailer_t pvm_sta_trailer_scratch;

@pragma scratch_metadata
metadata seq_pdma_entry_t seq_pdma_entry_scratch;

@pragma scratch_metadata
metadata seq_r2n_entry_t seq_r2n_entry_scratch;

@pragma scratch_metadata
metadata seq_barco_entry_t seq_barco_entry_scratch;

@pragma scratch_metadata
metadata roce_cq_wqe_t roce_cq_wqe_scratch;

@pragma scratch_metadata
metadata roce_sq_wqe_t roce_sq_wqe_scratch;

@pragma scratch_metadata
metadata roce_rq_wqe_t roce_rq_wqe_scratch;

@pragma scratch_metadata
metadata roce_sq_xlate_entry_t roce_sq_xlate_entry_scratch;

@pragma scratch_metadata
metadata seq_comp_status_desc_t seq_comp_status_desc_scratch;

@pragma scratch_metadata
metadata seq_comp_status_t seq_comp_status_scratch;

@pragma scratch_metadata
metadata seq_comp_sgl_t seq_comp_sgl_scratch;

/*****************************************************************************
 * Storage Tx PVM initiator BEGIN
 *****************************************************************************/

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
                        Q_STATE_SIZE, pci_q_state_push_start)
}

/*****************************************************************************
 *  pvm_cq_handler: Save the NVME command in SQ entry to PHV. DMA the 
 *                  working consumer index to the consumer index in the 
 *                  queue state. Check to see if we can do PRP assist and 
 *                  load the address for the next stage based on that.
 *****************************************************************************/

@pragma little_endian sq_head
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
  modify_field(pvm_sta_trailer_scratch.dst_lif, dst_lif);
  modify_field(pvm_sta_trailer_scratch.dst_qtype, dst_qtype);
  modify_field(pvm_sta_trailer_scratch.dst_qid, dst_qid);
  modify_field(pvm_sta_trailer_scratch.dst_qaddr, dst_qaddr);

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
                        Q_STATE_SIZE, pci_q_state_push_start)
}

/*****************************************************************************
 * Storage Tx PVM initiator END
 *****************************************************************************/



/*****************************************************************************
 * Storage Tx PVM target BEGIN
 *****************************************************************************/


/*****************************************************************************
 *  r2n_sq_handler: Read the R2N WQE posted by local PVM to get the pointer to 
 *                  the NVME backend command. Call the next stage to read the 
 *                  NVME backend command to determine the SSD queue and 
 *                  priority ring to post to.
 *****************************************************************************/

action r2n_sq_handler(handle, data_size, opcode, status, db_enable, db_lif,
                      db_qtype, db_qid, db_index, is_remote, dst_lif, dst_qtype,
                      dst_qid, dst_qaddr) {

  // For D vector generation (type inference). No need to translate this to ASM.
  R2N_WQE_BASE_COPY(r2n_wqe_scratch)

  // Carry forward state information to be saved with R2N WQE in PHV
  R2N_WQE_BASE_COPY(r2n_wqe)

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // If opcode is set to process WQE
  if (r2n_wqe_scratch.opcode == R2N_OPCODE_PROCESS_WQE) {
    // Load the PVM VF SQ context for the next stage to push the NVME command
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, handle,
                          STORAGE_DEFAULT_TBL_LOAD_SIZE, nvme_be_wqe_prep_start)
  }

  // If opcode is set to post buffer
  if (r2n_wqe_scratch.opcode == R2N_OPCODE_BUF_POST) {

    // Overwrite the destinatrion queue information in the K+I vector
    modify_field(storage_kivec0.dst_lif, r2n_wqe_scratch.dst_lif);
    modify_field(storage_kivec0.dst_qtype, r2n_wqe_scratch.dst_qtype);
    modify_field(storage_kivec0.dst_qid, r2n_wqe_scratch.dst_qid);
    modify_field(storage_kivec0.dst_qaddr, r2n_wqe_scratch.dst_qaddr);

    // DMA the buf post descritor with a mem2mem DMA. Update source here
    // and destination in the push stage. 
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, 
                             r2n_wqe_scratch.handle - R2N_BUF_NVME_BE_CMD_OFFSET,
                             0, 0, 0, ROCE_RQ_WQE_SIZE, 0, 0, 0) 

    // Load the ROCE RQ CB to push the WQE
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          Q_STATE_SIZE, roce_rq_push_start)
  }
}

/*****************************************************************************
 *  nvme_be_wqe_prep: From the NVME backend command, determine which SSD and 
 *                    priority queue to send it to.
 *****************************************************************************/

action nvme_be_wqe_prep(src_queue_id, ssd_handle, io_priority, is_read,
                        r2n_buf_handle) {

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_BE_CMD_HDR_COPY(nvme_be_cmd_hdr_scratch)

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
                           PHV_FIELD_OFFSET(r2n_wqe.pad),
                           0, 0, 0, 0)

  // Load the NVME backkend SQ context for the next stage to push the command
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        storage_kivec0.dst_qaddr + 
                        nvme_be_cmd_hdr_scratch.ssd_handle * storage_kivec2.ssd_q_size,
                        Q_STATE_SIZE, pri_q_state_push_start)
}

/*****************************************************************************
 *  nvme_be_sq_handler: Read the NVME backend priority submission queue entry.
 *                      Load the actual NVME command for the next stage.
 *****************************************************************************/

action nvme_be_sq_handler(handle, data_size, opcode, status, db_enable, db_lif, 
                          db_qtype, db_qid, db_index, is_remote, dst_lif, 
                          dst_qtype, dst_qid, dst_qaddr, src_queue_id,
                          ssd_handle, io_priority, is_read, r2n_buf_handle, 
                          nvme_cmd_cid, pri_qaddr, pad) {

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
  modify_field(ssd_cmds_scratch.bitmap, bitmap);
  
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
                           PHV_FIELD_OFFSET(r2n_wqe.pad),
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
  modify_field(nvme_be_sta_hdr.be_rsvd, 0);

  // Store the SSD's c_ndx value for DMA to the NVME backend SQ
  modify_field(ssd_ci.c_ndx, sq_head);

  // Derive the save command index from the NVME status
  modify_field(storage_kivec0_scratch.cmd_index, (0xFF & cid));

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // Setup the DMA command to push the sq_head to the c_ndx of the SSD
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_2, 
                           storage_kivec1.device_addr,
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


action nvme_be_wqe_handler(handle, data_size, opcode, status, db_enable, db_lif, 
                           db_qtype, db_qid, db_index, is_remote, dst_lif, 
                           dst_qtype, dst_qid, dst_qaddr, src_queue_id,
                           ssd_handle, io_priority, is_read, r2n_buf_handle, 
                           nvme_cmd_cid, pri_qaddr, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  R2N_WQE_FULL_COPY(r2n_wqe_scratch)

  // Restore the fields in the NVME backend status to saved values
  modify_field(nvme_be_sta_hdr.r2n_buf_handle, r2n_wqe_scratch.r2n_buf_handle);
  modify_field(nvme_sta.cid, r2n_wqe_scratch.nvme_cmd_cid);

  // Store fields needed in the K+I vector
  modify_field(storage_kivec0.ssd_handle, ssd_handle);
  modify_field(storage_kivec0.io_priority, io_priority);

  // Store if command is a read command
  if (r2n_wqe_scratch.is_read == 1) {
    // Set the read bit in the K+I vector
    modify_field(storage_kivec0.is_read, 1);
  }

  // Rewrite the destination as needed
  if (r2n_wqe_scratch.db_enable == 1) {
    // TODO: In ASM calculate from LIF/Type/Qid etc.
    modify_field(doorbell_addr_scratch.addr, 0);
    modify_field(seq_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                             0,
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             PHV_FIELD_OFFSET(seq_doorbell_data.data),
                             0, 0, 0, 0)
  }

  // if is_remote is set => send write data (if any) and status over ROCE SQ; 
  // else => send status alone to local R2N CQ
  if (r2n_wqe_scratch.is_remote == 1) {
    // Set the remote bit in the K+I vector
    modify_field(storage_kivec0.is_remote, 1);

    // Overwrite the K+I vector for the push operation to derive the correct 
    // destination queue
    modify_field(storage_kivec0.dst_lif, dst_lif);
    modify_field(storage_kivec0.dst_qtype, dst_qtype);
    modify_field(storage_kivec0.dst_qid, dst_qid);
    modify_field(storage_kivec0.dst_qaddr, dst_qaddr);

    // Setup the DMA command to push the NVME backend status entry
    // that was formed in the previous stages to the status offset in the
    // R2N buffer.
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                             r2n_wqe_scratch.handle - R2N_BUF_NVME_BE_CMD_OFFSET + 
                             R2N_BUF_STATUS_BUF_OFFSET,
                             PHV_FIELD_OFFSET(nvme_be_sta_hdr.time_us),
                             PHV_FIELD_OFFSET(nvme_be_sta.status_phase),
                             0, 0, 0, 0)

    // Setup the DMA command to push the write request pointer
    // In ASM, set the host, fence bits etc correctly
    // NOTE: In ASM, do this only if is_read is set to 1. Can't have nested ifs in P4
    //       so not implementing that check here.
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_3, dma_m2m_4, 
                             r2n_wqe_scratch.handle - R2N_BUF_NVME_BE_CMD_OFFSET + 
                             R2N_BUF_WRITE_REQ_OFFSET,
                             0, 0, 0, ROCE_SQ_WQE_SIZE,
                             0, 0, 0)

    // Setup the DMA command to push the write request pointer
    // In ASM, set the host, fence bits etc correctly
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_5, dma_m2m_6, 
                             r2n_wqe_scratch.handle - R2N_BUF_NVME_BE_CMD_OFFSET + 
                             R2N_BUF_STATUS_REQ_OFFSET,
                             0, 0, 0, ROCE_SQ_WQE_SIZE,
                             0, 1, 0)

  } else {
    // Setup the DMA command to push the NVME backend status entry
    // that was formed in the previous stages. The destination address
    // will be filled in the push stage.
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                             0,
                             PHV_FIELD_OFFSET(nvme_be_sta_hdr.time_us),
                             PHV_FIELD_OFFSET(nvme_be_sta.status_phase),
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
  modify_field(ssd_cmds_scratch.bitmap, bitmap);
  
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


  // if is_remote is set => send write data (if any) and status over ROCE SQ; 
  // else => send status alone to local R2N CQ
  if (storage_kivec0.is_remote == 1) {
    // Load the PVM ROCE SQ CB context for the next stage to push the completion
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          Q_STATE_SIZE, pvm_roce_sq_cb_push_start)
  } else {
    // Load the R2N CQ context for the next stage to push the completion
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          Q_STATE_SIZE, pci_q_state_push_start)
  }
}

/*****************************************************************************
 *  roce_cq_handler: Handle the ROCE CQ entry by looking at the operation type
 *                   to determine one of the cases and process it accordingly:
 *                   1. New command received in the send buffer => handle  it
 *                   2. RDMA write operation succeded => post the buffer back
 *****************************************************************************/

action roce_cq_handler(wrid_msn, op_type, status, rsvd0, qp, rsvd1) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(roce_cq_wqe_scratch.wrid_msn, wrid_msn);
  modify_field(roce_cq_wqe_scratch.op_type, op_type);
  modify_field(roce_cq_wqe_scratch.status, status);
  modify_field(roce_cq_wqe_scratch.qp, qp);
  modify_field(roce_cq_wqe_scratch.rsvd1, rsvd1);

  // Store the qp in the K+I vector
  modify_field(storage_kivec0.dst_qid, qp);

  // New command received in the send buffer => handle  it
  if (roce_cq_wqe_scratch.op_type == ROCE_OP_TYPE_SEND_RCVD) {
    // Form the R2N WQE to send to R2N queue
    modify_field(r2n_wqe.handle, wrid_msn);
    modify_field(r2n_wqe.is_remote, 1);

    // In ASM: 1. Fix the pointers and PRPs in the R2N buffer
    //         2. Fix the opcode, size etc in R2N WQE
    //         3. Fix the data pointer in the write request 

    // Set the roce_cq_new_cmd bit in the K+I vector
    modify_field(storage_kivec1.roce_cq_new_cmd, 1);

    // Load the ROCE SQ xlate table for the queue id to figure out the PVM's 
    // ROCE SQ details. The base address to this table is programmed into the
    // ROCE CQ CB, which in turn is populated into storage_kivec0.dst_qaddr
    // in stage 0 during the pop of the ROCE CQ.
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, storage_kivec0.dst_qaddr,
                         roce_cq_wqe_scratch.qp + 1, 
                         STORAGE_DEFAULT_TBL_LOAD_SIZE, 
                         STORAGE_DEFAULT_TBL_LOAD_SIZE, roce_sq_xlate_start)
  }

  // RDMA write operation succeded => post the buffer back
  if (((roce_cq_wqe_scratch.op_type == ROCE_OP_TYPE_WRITE) or
       (roce_cq_wqe_scratch.op_type == ROCE_OP_TYPE_WRITE_IMM)) and
      (roce_cq_wqe_scratch.status == ROCE_CQ_STATUS_SUCCESS)) {

    // TODO: In ASM store the lower 32 bits as MSN into the K+I vector
    //modify_field(storage_kivec3.roce_msn, (roce_cq_wqe_scratch.wrid_msn & 0xFFFFFFFF));

    // Load the ROCE SQ xlate table for the queue id to figure out the PVM's 
    // ROCE SQ details. The base address to this table is programmed into the
    // ROCE CQ CB, which in turn is populated into storage_kivec0.dst_qaddr
    // in stage 0 during the pop of the ROCE CQ.
    CAPRI_LOAD_TABLE_IDX(common_te0_phv, storage_kivec0.dst_qaddr,
                         roce_cq_wqe_scratch.qp + 1, 
                         STORAGE_DEFAULT_TBL_LOAD_SIZE, 
                         STORAGE_DEFAULT_TBL_LOAD_SIZE, roce_sq_xlate_start)
  }
}

/*****************************************************************************
 *  roce_sq_xlate: Read the ROCE SQ xlate entry and figure out the correct
 *                 PVM SQ for this ROCE CQ/SQ queue pair.
 *****************************************************************************/

action roce_sq_xlate(next_pc, dst_lif, dst_qtype, dst_qid, dst_qaddr, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  ROCE_XLATE_ENTRY_COPY(roce_sq_xlate_entry_scratch)

  // New command from ROCE => fully form the R2N WQE and push it
  if (storage_kivec1.roce_cq_new_cmd == 1) {
    // Add the response queue details
    modify_field(r2n_wqe.dst_lif, roce_sq_xlate_entry_scratch.dst_lif);
    modify_field(r2n_wqe.dst_qtype, roce_sq_xlate_entry_scratch.dst_qtype);
    modify_field(r2n_wqe.dst_qid, roce_sq_xlate_entry_scratch.dst_qid);
    modify_field(r2n_wqe.dst_qaddr, roce_sq_xlate_entry_scratch.dst_qaddr);
    
    // Load ROCE SQ xlate entry 0 (stored in storage_kivec0.dst_qaddr) which 
    // contains the PVM's R2N SQ information for sending the R2N WQE. 
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          STORAGE_DEFAULT_TBL_LOAD_SIZE, 
                          roce_r2n_wqe_prep_start)

  // Status transfer done => fully form the R2N WQE and push it
  } else {
    // Overwrite the destinatrion queue information in the K+I vector
    modify_field(storage_kivec0.dst_lif, roce_sq_xlate_entry_scratch.dst_lif);
    modify_field(storage_kivec0.dst_qtype, roce_sq_xlate_entry_scratch.dst_qtype);
    modify_field(storage_kivec0.dst_qid, roce_sq_xlate_entry_scratch.dst_qid);
    modify_field(storage_kivec0.dst_qaddr, roce_sq_xlate_entry_scratch.dst_qaddr);

    // Load the queue state information of PVM's ROCE SQ to store the MSN and ring
    // the doorbell
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, roce_sq_xlate_entry_scratch.dst_qaddr,
                          Q_STATE_SIZE, pvm_roce_sq_cb_update_start)
  }
}

/*****************************************************************************
 *  roce_r2n_wqe_prep: Read the ROCE SQ xlate entry 0 and figure out the 
 *                     destination R2N lif/type/queue to send the R2N WQE to.
 *****************************************************************************/

action roce_r2n_wqe_prep(next_pc, dst_lif, dst_qtype, dst_qid, dst_qaddr, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // For D vector generation (type inference). No need to translate this to ASM.
  ROCE_XLATE_ENTRY_COPY(roce_sq_xlate_entry_scratch)

  // Overwrite the destinatrion queue information in the K+I vector
  modify_field(storage_kivec0.dst_lif, roce_sq_xlate_entry_scratch.dst_lif);
  modify_field(storage_kivec0.dst_qtype, roce_sq_xlate_entry_scratch.dst_qtype);
  modify_field(storage_kivec0.dst_qid, roce_sq_xlate_entry_scratch.dst_qid);
  modify_field(storage_kivec0.dst_qaddr, roce_sq_xlate_entry_scratch.dst_qaddr);

  // Setup the DMA command to push the R2N WQE to PVM's R2N SQ
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1, 
                           0,
                           PHV_FIELD_OFFSET(r2n_wqe.handle),
                           PHV_FIELD_OFFSET(r2n_wqe.dst_qaddr),
                           0, 0, 0, 0)

  // Load the queue state information to push the R2N WQE to PVM's R2N 
  // queue for ROCE
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, roce_sq_xlate_entry_scratch.dst_qaddr,
                        Q_STATE_SIZE, q_state_push_start)

}

/*****************************************************************************
 *  pvm_roce_sq_cb_update: Update the PVM's ROCE SQ with the MSN from this 
 *                         completion queue. Then ring its doorbell for it to 
 *                         reclaim the entries for further use.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action pvm_roce_sq_cb_update(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                             total_rings, host_rings, pid, p_ndx, c_ndx, base_addr,
                             page_size, entry_size, num_entries, rsvd0, roce_msn, 
                             w_ndx, next_pc, rrq_lif, rrq_qtype,
                             rrq_qid, rrq_qaddr, rsq_lif, rsq_qtype, rsq_qid, pad) {


  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)
  STORAGE_KIVEC3_USE(storage_kivec3_scratch, storage_kivec3)

  // For D vector generation (type inference). No need to translate this to ASM.
  PVM_ROCE_SQ_CB_COPY(pvm_roce_sq_cb_scratch)

  // Overwrite the MSN field in PVM's ROCE SQ CB with that obtained from the 
  // CQ entry
  // In ASM table write of the MSN field.
  modify_field(pvm_roce_sq_cb_scratch.roce_msn, storage_kivec3.roce_msn);

  // Form the doorbell and setup the DMA command to push the entry by
  // incrementing p_ndx
  modify_field(doorbell_addr_scratch.addr,
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


/*****************************************************************************
 *  pvm_roce_sq_wqe_process: Process the ROCE SQ WQE as pointed to by the 
 *                           PVM's ROCE SQ and load the R2N buffer from the
 *                           WRID in the WQE if the operation type was 
 *                           RDMA_SEND. Then post the buffer back to ROCE RQ.
 *****************************************************************************/

action pvm_roce_sq_wqe_process(wrid, op_type, complete_notify, fence,
                           solicited_event, inline_data_vld, num_sges, rsvd2,
                           op_data) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  STORAGE_KIVEC0_USE(storage_kivec0_scratch, storage_kivec0)
  STORAGE_KIVEC1_USE(storage_kivec1_scratch, storage_kivec1)

  // Form the doorbell and setup the DMA command to pop the entry by writing 
  // w_ndx to c_ndx
  QUEUE_POP_DOORBELL_UPDATE

  // For D vector generation (type inference). No need to translate this to ASM.
  modify_field(roce_sq_wqe_scratch.wrid, wrid);
  modify_field(roce_sq_wqe_scratch.op_type, op_type);
  modify_field(roce_sq_wqe_scratch.complete_notify, complete_notify);
  modify_field(roce_sq_wqe_scratch.fence, fence);
  modify_field(roce_sq_wqe_scratch.solicited_event, solicited_event);
  modify_field(roce_sq_wqe_scratch.inline_data_vld, inline_data_vld);
  modify_field(roce_sq_wqe_scratch.num_sges, num_sges);
  modify_field(roce_sq_wqe_scratch.rsvd2, rsvd2);
  modify_field(roce_sq_wqe_scratch.op_data, op_data);

  if ((roce_sq_wqe_scratch.op_type == ROCE_OP_TYPE_SEND) or
      (roce_sq_wqe_scratch.op_type == ROCE_OP_TYPE_SEND_INV) or
      (roce_sq_wqe_scratch.op_type == ROCE_OP_TYPE_SEND_IMM) or
      (roce_sq_wqe_scratch.op_type == ROCE_OP_TYPE_SEND_INV_IMM)) {

    // DMA the buf post descritor with a mem2mem DMA. Update source here
    // and destination in the push stage. 
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, 
                             roce_sq_wqe_scratch.wrid - R2N_BUF_NVME_BE_CMD_OFFSET,
                             0, 0, 0, ROCE_RQ_WQE_SIZE, 0, 0, 0) 


    // Load the ROCE RQ CB to push the WQE
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, storage_kivec0.dst_qaddr,
                          Q_STATE_SIZE, roce_rq_push_start)
  }
}



/*****************************************************************************
 * Storage Tx PVM target BEGIN
 *****************************************************************************/

