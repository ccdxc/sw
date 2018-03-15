/*****************************************************************************
 * storage_nvme.p4: Storage_nvme P4+ program that implements NVME initiator 
 *****************************************************************************/

#include "common/storage_nvme_p4_hdr.h"

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0		s0_tbl0
#define tx_table_s1_t0		s1_tbl0
#define tx_table_s2_t0		s2_tbl0
#define tx_table_s3_t0		s3_tbl0
#define tx_table_s4_t0		s4_tbl0
#define tx_table_s5_t0		s5_tbl0
#define tx_table_s6_t0		s6_tbl0
#define tx_table_s7_t0		s7_tbl0
#define tx_table_s7_t1		s7_tbl1

#define tx_table_s0_t0_action	check_sq_state
#define tx_table_s0_t0_action1	pop_r2n_sq

#define tx_table_s1_t0_action	allocate_iob
#define tx_table_s1_t0_action1	handle_r2n_wqe

#define tx_table_s2_t0_action	pop_sq
#define tx_table_s2_t0_action1	process_be_status

#define tx_table_s3_t0_action	handle_cmd
#define tx_table_s3_t0_action1	process_io_ctx

#define tx_table_s4_t0_action	process_io_map
#define tx_table_s4_t0_action1	send_read_data

#define tx_table_s5_t0_action	handle_no_prp_list

#define tx_table_s6_t0_action	process_dst_seq

#define tx_table_s7_t0_action	push_arm_q
#define tx_table_s7_t1_action	push_dst_seq_q

#include "../common-p4+/common_txdma.p4"


/*****************************************************************************
 * Storage NVME PHV layout BEGIN 
 * Will be processed by NCC in this order 
 *****************************************************************************/

// Stage to stage K+I vectors
@pragma pa_header_union ingress common_t0_s2s
metadata nvme_kivec_s2s_t nvme_kivec_t0_s2s;
@pragma pa_header_union ingress common_t1_s2s
metadata nvme_kivec_s2s_t nvme_kivec_t1_s2s;

// Global K+I vector
@pragma pa_header_union ingress common_global
metadata nvme_kivec_global_t nvme_kivec_global;

// To stage K+I vectors
@pragma pa_header_union ingress to_stage_3
metadata nvme_kivec_sq_info_t nvme_kivec_sq_info;
@pragma pa_header_union ingress to_stage_5
metadata nvme_kivec_prp_base_t nvme_kivec_prp_base;
@pragma pa_header_union ingress to_stage_6
metadata nvme_kivec_arm_dst_t nvme_kivec_arm_dst6;
@pragma pa_header_union ingress to_stage_7
metadata nvme_kivec_arm_dst_t nvme_kivec_arm_dst7;

// Push/Pop doorbells
@pragma dont_trim
metadata storage_doorbell_data_t qpush_doorbell_data;
@pragma dont_trim
metadata storage_doorbell_data_t qpop_doorbell_data;

// Push/interrupt data across PCI bus
@pragma dont_trim
metadata storage_pci_data_t pci_push_data;
@pragma dont_trim
metadata storage_pci_data_t pci_intr_data;

// Keep the WQEs/commands that occupy full flit aligned at flit boundaries

// NVME command (occupies full flit)
@pragma dont_trim
@pragma pa_align 512
metadata nvme_cmd_t nvme_cmd;
@pragma dont_trim
@pragma pa_header_union ingress nvme_cmd
metadata nvme_sta_t nvme_sta;

// I/O context (immediately follows NVME command as phv2mem DMA of 
// both these are done together)
@pragma dont_trim
metadata io_ctx_entry_t io_ctx;
 
// Seq R2N work queue entry -- This points to ROCE (or) R2N WQE based on destination
@pragma dont_trim 
metadata seq_r2n_entry_t seq_r2n_wqe;

// NVME backend status 
@pragma dont_trim
metadata nvme_be_cmd_hdr_t nvme_be_cmd_hdr;
@pragma dont_trim
@pragma pa_header_union ingress nvme_be_cmd_hdr
metadata nvme_be_sta_hdr_t nvme_be_sta_hdr;


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

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_12;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_12
metadata dma_cmd_mem2mem_t dma_m2m_12;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_13;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_13
metadata dma_cmd_mem2mem_t dma_m2m_13;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_14;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_14
metadata dma_cmd_mem2mem_t dma_m2m_14;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_p2m_15;
@pragma dont_trim
@pragma pa_header_union ingress dma_p2m_15
metadata dma_cmd_mem2mem_t dma_m2m_15;

/*****************************************************************************
 * Storage Tx PHV layout END 
 *****************************************************************************/


// Scratch metadatas to get d-vector generated correctly

@pragma scratch_metadata
metadata nvme_sq_state_t nvme_sq_state_scratch;

@pragma scratch_metadata
metadata nvme_cq_state_t nvme_cq_state_scratch;

@pragma scratch_metadata
metadata arm_q_state_t arm_q_state_scratch;

@pragma scratch_metadata
metadata q_state_t q_state_scratch;

@pragma scratch_metadata
metadata ring_state_t iob_ring_state_scratch;

@pragma scratch_metadata
metadata nvme_cmd_t nvme_cmd_scratch;

@pragma scratch_metadata
metadata nvme_be_cmd_hdr_t nvme_be_cmd_hdr_scratch;

@pragma scratch_metadata
metadata storage_capri_addr_t doorbell_addr_scratch;

@pragma scratch_metadata
metadata storage_capri_addr_t pci_intr_addr_scratch;

@pragma scratch_metadata
metadata nvme_kivec_s2s_t nvme_kivec_t0_s2s_scratch;

@pragma scratch_metadata
metadata nvme_kivec_s2s_t nvme_kivec_t1_s2s_scratch;

@pragma scratch_metadata
metadata nvme_kivec_global_t nvme_kivec_global_scratch;

@pragma scratch_metadata
metadata nvme_kivec_sq_info_t nvme_kivec_sq_info_scratch;

@pragma scratch_metadata
metadata nvme_kivec_prp_base_t nvme_kivec_prp_base_scratch;

@pragma scratch_metadata
metadata nvme_kivec_arm_dst_t nvme_kivec_arm_dst_scratch;

@pragma scratch_metadata
metadata io_map_entry_t io_map_entry_scratch;

@pragma scratch_metadata
metadata nvme_prp_list_t prp_list_scratch;

@pragma scratch_metadata
metadata nvme_scratch_t nvme_scratch;

@pragma scratch_metadata
metadata seq_db_info_t seq_db_info_scratch;

@pragma scratch_metadata
metadata r2n_wqe_t r2n_wqe_scratch;

@pragma scratch_metadata
metadata io_ctx_entry_t io_ctx_scratch;


/*****************************************************************************
 * exit: Exit action handler needs to be stubbed out for NCC 
 *****************************************************************************/

action exit() {
}

/*****************************************************************************
 * Storage Tx NVME initiator command processing BEGIN
 *****************************************************************************/

/*****************************************************************************
 *  check_sq_state : Check the NVME queue state and see if there's 
 *                   anything to be popped. If yes, allocate the iob 
 *                   resource in the next stage. If not, set a flag to exit
 *                   the pipeline in the sq pop stage after clearing doorbell
 *                   (as needed).
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action check_sq_state(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                      total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                      num_entries, base_addr, entry_size, next_pc, vf_id, 
                      sq_id, cq_lif, cq_qtype, cq_qid, cq_qaddr,
                      arm_lif, arm_qtype, arm_qid, arm_qaddr,
                      io_map_base_addr, io_map_num_entries, iob_ring_base_addr,
                      pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_SQ_STATE_COPY_STAGE0(nvme_sq_state_scratch)

  // If queue is empty set the oper_status to non_starter so that IO buffer is
  // not allocated. Go through the pipeline and clear the doorbell only in
  // sq_pop stage to be consistent with table locking semantics. 
  if (QUEUE_EMPTY(nvme_sq_state_scratch)) {
    modify_field(nvme_kivec_global.oper_status, IO_CTX_OPER_STATUS_NON_STARTER);
  } 

  // In ASM, derive these from the K+I for stage 0
  modify_field(nvme_kivec_global.src_qaddr, 0);
  modify_field(nvme_kivec_global.src_lif, 0);
  modify_field(nvme_kivec_global.src_qtype, 0);
  modify_field(nvme_kivec_global.src_qid, 0);
  modify_field(io_ctx.nvme_sq_qaddr, 0);

  // Save information to K+I vector
  modify_field(nvme_kivec_t0_s2s.io_map_base_addr, io_map_base_addr);

  // Load the table and program for allocating IOB in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, nvme_sq_state_scratch.iob_ring_base_addr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, allocate_iob_start)
}

/*****************************************************************************
 *  allocate_iob : Allocate IO buffer from free list if the check SQ stage
 *                 has marked there is something to dequeue. This stage 
 *                 consumes an item off the iob_ring and stores the pointer
 *                 in the K+I vector part of the PHV. If the ring is empty,
 *                 error flag is set in the  K+I vector part of the PHV.
 *****************************************************************************/

action allocate_iob(p_ndx, c_ndx, w_ndx, num_entries, base_addr, entry_size, 
                    pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // For D vector generation (type inference). No need to translate this to ASM.
  RING_STATE_COPY(iob_ring_state_scratch)

  // If NVME SQ is empty or if IOB free ring is empty, dont allocate IOB
  if ((nvme_kivec_global_scratch.oper_status == IO_CTX_OPER_STATUS_NON_STARTER) or
      (QUEUE_EMPTY(iob_ring_state_scratch))) {

    modify_field(nvme_kivec_global.oper_status, IO_CTX_OPER_STATUS_NON_STARTER);

  } else {
    // Save the IOB address from the free list into K+I vector
    modify_field(nvme_kivec_t0_s2s.iob_addr, 
                 iob_ring_state_scratch.base_addr + 
                 (iob_ring_state_scratch.c_ndx * 
                  iob_ring_state_scratch.entry_size));

    // Save relevant fields for I/O context in the PHV
    modify_field(io_ctx.iob_addr,
                 iob_ring_state_scratch.base_addr + 
                 (iob_ring_state_scratch.c_ndx * 
                  iob_ring_state_scratch.entry_size));
    modify_field(io_ctx.oper_status, IO_CTX_OPER_STATUS_IN_USE);

    // Increment the consumer index. In ASM this should be a table write.
    QUEUE_POP(iob_ring_state_scratch)

  }

  // Load the table and program for processing the SQ entry in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, nvme_kivec_global.src_qaddr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, pop_sq_start)
}

/*****************************************************************************
 *  pop_sq : Increment the consumer index to pop the entry and load
 *           the queue entry for the next stage to process. If there is 
 *           nothing to be popped, clear the doorbell (as needed).
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action pop_sq(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
              total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
              num_entries, base_addr, entry_size, next_pc, vf_id, 
              sq_id, cq_lif, cq_qtype, cq_qid, cq_qaddr,
              arm_lif, arm_qtype, arm_qid, arm_qaddr,
              io_map_base_addr, io_map_num_entries, iob_ring_base_addr,
              pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_SQ_STATE_COPY(nvme_sq_state_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // Store fields in the K+I vector
  modify_field(nvme_kivec_arm_dst6.arm_lif, arm_lif);
  modify_field(nvme_kivec_arm_dst6.arm_qtype, arm_qtype);
  modify_field(nvme_kivec_arm_dst6.arm_qid, arm_qid);
  modify_field(nvme_kivec_arm_dst6.arm_qaddr, arm_qaddr);
  modify_field(nvme_kivec_arm_dst7.arm_lif, arm_lif);
  modify_field(nvme_kivec_arm_dst7.arm_qtype, arm_qtype);
  modify_field(nvme_kivec_arm_dst7.arm_qid, arm_qid);
  modify_field(nvme_kivec_arm_dst7.arm_qaddr, arm_qaddr);

  // If non starter (IOB is not allocated, so clear doorbell and exit)
  if (nvme_kivec_global_scratch.oper_status == IO_CTX_OPER_STATUS_NON_STARTER) {
    // Evaluate and clear the doorbell if p_ndx == c_ndx
    //QUEUE_POP_DOORBELL_UPDATE
    exit();
  }


  // If queue is empty, don't dequeue, but punt the IOB to be freed to ARM
  if (QUEUE_EMPTY(nvme_sq_state_scratch)) {
    // Don't dequeue
    
    // Evaluate and clear the doorbell if p_ndx == c_ndx
    //QUEUE_POP_DOORBELL_UPDATE

    // TODO: Go through stages and push IOB to ARM
  } else {

    // Increment the consumer index. In ASM this should be a table write.
    QUEUE_POP(nvme_sq_state_scratch)
   

    // Store fields in the K+I vector
    modify_field(nvme_kivec_t0_s2s.w_ndx, w_ndx);

    // Load the table and program for processing the queue entry in the next stage
    CAPRI_LOAD_TABLE_IDX(common_te0_phv,
                         nvme_sq_state_scratch.base_addr,
                         nvme_sq_state_scratch.w_ndx,
                         nvme_sq_state_scratch.entry_size,
                         nvme_sq_state_scratch.entry_size,
                         handle_cmd_start)
  }
}


/*****************************************************************************
 *  handle_cmd: Save the NVME command in SQ entry to PHV. Process the
 *              command to see if it is a Read/Write I/O which can
 *              be processed inline in the datapath. Setup the PRP
 *              assist descriptors for inline processing. For exception 
 *              cases, punt to ARM.
 *****************************************************************************/

action handle_cmd(opc, fuse, rsvd0, psdt, cid, nsid, rsvd2, rsvd3,
                  mptr, dptr1, dptr2, slba, nlb, rsvd12, prinfo,
                  fua, lr, dsm, rsvd13, dw14, dw15) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)
  NVME_KIVEC_SQ_INFO_USE(nvme_kivec_sq_info_scratch, nvme_kivec_sq_info)

  // Carry forward NVME command information to be sent to PVM in the PHV 
  NVME_CMD_COPY(nvme_cmd)

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_CMD_COPY(nvme_cmd_scratch)

  // Save relevant fields for I/O context, NVME cmd handling etc. in the PHV
  if (opc == NVME_READ_CMD_OPCODE) {
    modify_field(nvme_kivec_t0_s2s.is_read, 1);
    modify_field(nvme_be_cmd_hdr.is_read, 1);
    modify_field(io_ctx.is_read, 1);
  } else {
    modify_field(nvme_kivec_t0_s2s.is_read, 0);
    modify_field(nvme_be_cmd_hdr.is_read, 0);
    modify_field(io_ctx.is_read, 0);
  }
  // Store the NVME data length into PHV and into K+I vector. 
  modify_field(nvme_kivec_global.nvme_data_len, LB_SIZE(nvme_cmd_scratch.nlb + 1));
  modify_field(io_ctx.nvme_data_len, LB_SIZE(nvme_cmd_scratch.nlb + 1));

  // If Admin command (or) R/W command > 64K I/O, mark it to be punted to arm
  if ((nvme_kivec_sq_info.is_admin_q == 1) or
      ((opc != NVME_READ_CMD_OPCODE) and
       (opc != NVME_WRITE_CMD_OPCODE)) or
      (psdt != 0) or
      (LB_SIZE(nlb + 1) > MAX_ASSIST_SIZE)) {
    modify_field(nvme_kivec_t0_s2s.punt_to_arm, 1);
    modify_field(io_ctx.oper_status, IO_CTX_OPER_STATUS_PUNT);
  }

  // NVMe PRP list download determination (AND of all these conditions)
  // 1. q_id > 0 (not admin queue) AND (read OR write command)
  // 2. PRP list indicated in psdt bit
  // 3. LBA size is less than the max assist size
  // 4. LBA size is greater than the sum of the PRP sizes
  //
  // If need to download, use table read
  if (((nvme_kivec_sq_info.is_admin_q == 0) and
       ((opc == NVME_READ_CMD_OPCODE) or
        (opc == NVME_WRITE_CMD_OPCODE))) and
      (psdt == 0) and
      (LB_SIZE(nlb + 1) <= MAX_ASSIST_SIZE) and
      (LB_SIZE(nlb + 1) > (PRP_SIZE(dptr1) + PRP_SIZE(dptr2)))) {

    // Set the PRP assist flag and prp list pointer
    modify_field(nvme_kivec_t0_s2s.prp_assist, 1);

    // Set the PRP0 and PRP1 values into the K+I vector
    // PRP1 is used to download the PRP list
    modify_field(nvme_kivec_prp_base_scratch.prp0, nvme_cmd_scratch.dptr1);
    modify_field(nvme_kivec_prp_base_scratch.prp1, nvme_cmd_scratch.dptr2);
  }

  // No PRP list case. Optimize by placing PRP in the to_stage K+I vector
  // and directly doing the DMA for the write command
  if (((nvme_kivec_sq_info.is_admin_q == 0) and
       ((opc == NVME_READ_CMD_OPCODE) or
        (opc == NVME_WRITE_CMD_OPCODE))) and
      (psdt == 0) and
      (LB_SIZE(nlb + 1) <= MAX_ASSIST_SIZE) and
      (LB_SIZE(nlb + 1) <= (PRP_SIZE(dptr1) + PRP_SIZE(dptr2)))) {

    // Set the PRP0 and PRP1 values into the K+I vector
    modify_field(nvme_kivec_prp_base_scratch.prp0, nvme_cmd_scratch.dptr1);
    modify_field(nvme_kivec_prp_base_scratch.prp1, nvme_cmd_scratch.dptr2);
  }

  // Load the I/O mapping table to process the NVME command further
  // TODO: FIXME: API is incorrect, need a way to lookup NSID,VFID
  // TODO: FIXME: Need to validate against io_map_num_entries
  CAPRI_LOAD_TABLE_IDX(common_te0_phv,
                       nvme_kivec_t0_s2s.io_map_base_addr, 
                       nvme_cmd_scratch.nsid,
                       IO_MAP_ENTRY_SIZE_LOG2,
                       STORAGE_DEFAULT_TBL_LOAD_SIZE,
                       process_io_map_start)
}


/*****************************************************************************
 *  process_io_map: Load the I/O mapping table entry and determine the
 *                  following based on that:
 *                    1. PDMA to convert from PRP to flat buffer in HBM
 *                    2. XTS encryption/decryption
 *                    3. Destination queue to send the I/O
 *****************************************************************************/

action process_io_map(entry_addr, nsid, vf_id, dst_flags, 
                      r2n_lif, r2n_qtype, r2n_qid, r2n_qaddr,
                      roce_lif, roce_qtype, roce_qid, roce_qaddr, 
                      src_queue_id, ssd_handle, io_priority, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // For D vector generation (type inference). No need to translate this to ASM.
  IO_MAP_ENTRY_COPY(io_map_entry_scratch)

  // Save the NVME backend cmd information to PHV to write to the I/O buffer
  modify_field(nvme_be_cmd_hdr.src_queue_id, src_queue_id);
  modify_field(nvme_be_cmd_hdr.ssd_handle, ssd_handle);
  modify_field(nvme_be_cmd_hdr.io_priority, io_priority);

  // Copy the I/O map and the sequencer descriptors to I/O buffer
  DMA_COMMAND_MEM2MEM_FILL(dma_m2m_1, dma_m2m_2, entry_addr,
                           nvme_kivec_t0_s2s.iob_addr + IO_BUF_IO_MAP_DESC_OFFSET,
                           0, 0, IO_MAP_ENTRY_SIZE_BYTES, 0, 0, 0)

  // TODO: Adjust XTS and compression descriptors depending upon operation and dst_flags
  
  // Build Sequencer R2N WQE for remote target for Write command
  if ((nvme_kivec_t0_s2s.punt_to_arm == 0) and
      (nvme_kivec_t0_s2s.is_read  == 0) and
      ((dst_flags & IO_DST_REMOTE) == IO_DST_REMOTE)) {

    // Set address to point to ROCE SQ WQE
    modify_field(seq_r2n_wqe.r2n_wqe_addr,
                 nvme_kivec_t0_s2s.iob_addr + IO_BUF_WRITE_ROCE_SQ_WQE_OFFSET);
    modify_field(seq_r2n_wqe.r2n_wqe_size, IO_MAP_ROCE_SQ_WQE_SIZE);

    // Set destination LIF parameters to ROCE 
    modify_field(seq_r2n_wqe.dst_lif, roce_lif);
    modify_field(seq_r2n_wqe.dst_qtype, roce_qtype);
    modify_field(seq_r2n_wqe.dst_qid, roce_qid);
    modify_field(seq_r2n_wqe.dst_qaddr, roce_qaddr);

    // Save the is_remote flag in I/O context
    modify_field(io_ctx.is_remote, 1);
  }
     
  // Build Sequencer R2N WQE for remote target for Read command
  if ((nvme_kivec_t0_s2s.punt_to_arm == 0) and
      (nvme_kivec_t0_s2s.is_read  == 1) and
      ((dst_flags & IO_DST_REMOTE) == IO_DST_REMOTE)) {

    // Set address to point to ROCE SQ WQE
    modify_field(seq_r2n_wqe.r2n_wqe_addr,
                 nvme_kivec_t0_s2s.iob_addr + IO_BUF_READ_ROCE_SQ_WQE_OFFSET);
    modify_field(seq_r2n_wqe.r2n_wqe_size, IO_MAP_ROCE_SQ_WQE_SIZE);

    // Set destination LIF parameters to ROCE 
    modify_field(seq_r2n_wqe.dst_lif, roce_lif);
    modify_field(seq_r2n_wqe.dst_qtype, roce_qtype);
    modify_field(seq_r2n_wqe.dst_qid, roce_qid);
    modify_field(seq_r2n_wqe.dst_qaddr, roce_qaddr);

    // Save the is_remote flag in I/O context
    modify_field(io_ctx.is_remote, 1);
  }
     
  // Build Sequencer R2N WQE for local target for Write command
  if ((nvme_kivec_t0_s2s.punt_to_arm == 0) and
      (nvme_kivec_t0_s2s.is_read  == 0) and
      ((dst_flags & IO_DST_REMOTE) != IO_DST_REMOTE)) {

    // Set address to point to R2N WQE
    modify_field(seq_r2n_wqe.r2n_wqe_addr,
                 nvme_kivec_t0_s2s.iob_addr + IO_BUF_WRITE_R2N_WQE_OFFSET);
    modify_field(seq_r2n_wqe.r2n_wqe_size, IO_MAP_R2N_WQE_SIZE);

    // Set destination LIF parameters to local R2N SQ 
    modify_field(seq_r2n_wqe.dst_lif, r2n_lif);
    modify_field(seq_r2n_wqe.dst_qtype, r2n_qtype);
    modify_field(seq_r2n_wqe.dst_qid, r2n_qid);
    modify_field(seq_r2n_wqe.dst_qaddr, r2n_qaddr);

    // Save the is_remote flag in I/O context
    modify_field(io_ctx.is_remote, 0);
  }
     
  // Build Sequencer R2N WQE for local target for Read command
  if ((nvme_kivec_t0_s2s.punt_to_arm == 0) and
      (nvme_kivec_t0_s2s.is_read  == 1) and
      ((dst_flags & IO_DST_REMOTE) != IO_DST_REMOTE)) {

    // Set address to point to R2N WQE
    modify_field(seq_r2n_wqe.r2n_wqe_addr,
                 nvme_kivec_t0_s2s.iob_addr + IO_BUF_READ_R2N_WQE_OFFSET);
    modify_field(seq_r2n_wqe.r2n_wqe_size, IO_MAP_R2N_WQE_SIZE);

    // Set destination LIF parameters to local R2N SQ 
    modify_field(seq_r2n_wqe.dst_lif, r2n_lif);
    modify_field(seq_r2n_wqe.dst_qtype, r2n_qtype);
    modify_field(seq_r2n_wqe.dst_qid, r2n_qid);
    modify_field(seq_r2n_wqe.dst_qaddr, r2n_qaddr);

    // Save the is_remote flag in I/O context
    modify_field(io_ctx.is_remote, 0);
  }
     
  // Setup the DMA command to save the NVME command entry and I/O context
  // into the I/O buffer to be sent to target side
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_3, 
                           nvme_kivec_t0_s2s.iob_addr + IO_BUF_NVME_BE_CMD_OFFSET +
                           NVME_BE_NVME_CMD_OFFSET,
                           PHV_FIELD_OFFSET(nvme_cmd.opc),
                           PHV_FIELD_OFFSET(io_ctx.nvme_sq_qaddr),
                           0, 0, 0, 0)

  // Setup the DMA command to save the NVME backend command header
  // into the I/O buffer to be sent to target side
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_4, 
                           nvme_kivec_t0_s2s.iob_addr + IO_BUF_NVME_BE_CMD_OFFSET,
                           PHV_FIELD_OFFSET(nvme_be_cmd_hdr.src_queue_id),
                           PHV_FIELD_OFFSET(nvme_be_cmd_hdr.io_priority),
                           0, 0, 0, 0)


  // Load the table and program for downloading the data (if it is a write 
  // command and has no PRP list) in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        nvme_kivec_t0_s2s.io_map_base_addr + IO_BUF_PRP_LIST_OFFSET,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, handle_no_prp_list_start)

}

/*****************************************************************************
 *  handle_no_prp_list: Handle the case where there is no PRP list. If this is
 *                      a write command, download the data specified by 
 *                      prp0 and prp1 into the I/O buffer.
 *****************************************************************************/

action handle_no_prp_list(entry0, entry1, entry2, entry3, entry4, entry5, entry6,
                          entry7) {
  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)
  NVME_KIVEC_PRP_BASE_USE(nvme_kivec_prp_base_scratch, nvme_kivec_prp_base)

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_PRP_LIST_COPY(prp_list_scratch)

  // TODO: In ASM, table write of PRP0 and PRP1
#if 0
  tblwr		d.entry0, nvme_kivec_prp_base.prp0
  if (nvme_kivec_t0_s2s.prp_assist == 0) {
    tblwr	d.entry1, nvme_kivec_prp_base.prp1
  }
#endif


  // Setup the PRP download via PDMA if assist is set
  // TODO: FIXME: Size should be based on data len without blindly using
  //              PRP_ASSIST_MAX_LIST_SIZE
  if (nvme_kivec_t0_s2s.prp_assist == 1) {
    DMA_COMMAND_MEM2MEM_FILL(dma_m2m_5, dma_m2m_6, 
                             nvme_kivec_prp_base.prp1,
                             nvme_kivec_t0_s2s.iob_addr + 
                             IO_BUF_PRP_LIST_ENTRY_OFFSET(1),
                             0, 0, PRP_ASSIST_MAX_LIST_SIZE, 0, 0, 0)
  }


  // Initialize the data length transferred
  modify_field(nvme_scratch.data_len_xferred, 0);

  // Start the data xfer (upto 4k) if this is a write command. 
  // This Macro takes care of the write command check and the fact that
  // prp_assist maybe set to 0 and/or prp1 maybe set to 0
  NVME_DATA_XFER_FROM_HOST(nvme_kivec_t0_s2s, nvme_kivec_global, 
                           dma_m2m_7, dma_m2m_8,
                           nvme_kivec_prp_base.prp0, nvme_scratch)
  NVME_DATA_XFER_FROM_HOST(nvme_kivec_t0_s2s, nvme_kivec_global,
                           dma_m2m_9, dma_m2m_10,
                           nvme_kivec_prp_base.prp1, nvme_scratch)

  // Load the table for processing the destination (Sequencer R2N
  // descriptor) in the I/O buffer in the next stage. 
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        nvme_kivec_t0_s2s.io_map_base_addr + 
                        IO_BUF_SEQ_DB_OFFSET + IO_BUF_SEQ_R2N_DB_OFFSET,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, process_dst_seq_start)
}

/*****************************************************************************
 *  process_dst_seq: Process the destination sequencer doorbell in the I/O 
 *                   buffer to setup the sequencer push and doorbell
 *****************************************************************************/

action process_dst_seq(lif, qtype, qid, qaddr) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)
  NVME_KIVEC_ARM_DST_USE(nvme_kivec_arm_dst_scratch, nvme_kivec_arm_dst6)

  // For D vector generation (type inference). No need to translate this to ASM.
  SEQ_DB_INFO_COPY(seq_db_info_scratch)

  // Set destination LIF parameters to ROCE 
  modify_field(nvme_kivec_t0_s2s.dst_lif, lif);
  modify_field(nvme_kivec_t0_s2s.dst_qtype, qtype);
  modify_field(nvme_kivec_t0_s2s.dst_qid, qid);
  modify_field(nvme_kivec_t0_s2s.dst_qaddr, qaddr);

  // Setup DMA command to store the I/O context into the I/O buffer
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_11, 
                           nvme_kivec_t0_s2s.iob_addr + IO_BUF_IO_CTX_OFFSET,
                           PHV_FIELD_OFFSET(io_ctx.iob_addr),
                           PHV_FIELD_OFFSET(io_ctx.nvme_sq_qaddr),
                           0, 0, 0, 0)

  // Load the table and program for reading the destination sequencer queue
  // state in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, nvme_kivec_arm_dst6.arm_qaddr, 
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, push_arm_q_start)

  // If punt to arm is not set, load the sequencer destination queue state
  // for pushing the sequencer R2N WQE
  if (nvme_kivec_t0_s2s.punt_to_arm == 0) {
    // Copy over the Stage 2 Stage data from table 0 to table 1
    NVME_KIVEC_S2S_USE(nvme_kivec_t1_s2s, nvme_kivec_t0_s2s)

    // Load the table and program for reading the destination sequencer queue
    // state in the next stage
    CAPRI_LOAD_TABLE_ADDR(common_te1_phv, qaddr, STORAGE_DEFAULT_TBL_LOAD_SIZE, 
                          push_dst_seq_q_start)
  }
}

/*****************************************************************************
 *  push_arm_q: Push the IO buffer to the ARM in all cases. ARM looks at 
 *              the oper_status flag in the IO context to determine if the
 *              IO buffer is punted or processed in the datapath. When 
 *               Case 1: When IO buffer is punted, ARM handles it entirely
 *                       and frees it after processing
 *               Case 2: When IO buffer is processed in the datapath, ARM 
 *                       runs a timer and monitors completion of the IO
 *                         and frees it
 *               In all cases, IO buffer is allocated by P4+ and freed by ARM
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action push_arm_q(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                  total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                  num_entries, base_addr, entry_size, next_pc, vf_id, 
                  intr_addr, intr_data, intr_en, phase, pad) {


  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)
  NVME_KIVEC_ARM_DST_USE(nvme_kivec_arm_dst_scratch, nvme_kivec_arm_dst7)

  // For D vector generation (type inference). No need to translate this to ASM.
  ARM_Q_STATE_COPY(arm_q_state_scratch)

  // Check for queue full condition before pushing
  if (QUEUE_FULL(arm_q_state_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  } 

  // DMA the I/O context (which contains the I/O buffer address etc.) to ARM
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_14,
                           arm_q_state_scratch.base_addr +
                           (arm_q_state_scratch.p_ndx * arm_q_state_scratch.entry_size),
                           PHV_FIELD_OFFSET(io_ctx.iob_addr),
                           PHV_FIELD_OFFSET(io_ctx.nvme_sq_qaddr),
                           0, 0, 0, 0)

  // Push the entry to the queue.  
  QUEUE_PUSH(arm_q_state_scratch)

  // Raise the MSIx interrupt if enabled
  if (arm_q_state_scratch.intr_en == 1) {
    modify_field(pci_intr_addr_scratch.addr, arm_q_state_scratch.intr_addr);
    modify_field(pci_intr_data.data, arm_q_state_scratch.intr_data);
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_15, 
                             0,
                             PHV_FIELD_OFFSET(pci_intr_data.data),
                             PHV_FIELD_OFFSET(pci_intr_data.data),
                             0, 0, 0, 0)
  }
  // Fence the DMA command to ring the interrupt

  // Exit the pipeline here 
}

/*****************************************************************************
 *  push_dst_seq_q: Push the WQE to the next P4+ program in the chain (ROCE/R2N). 
 *                  This will be called only when the IO buffer is not being
 *                  punted to arm and needs to be handled in P4+.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action push_dst_seq_q(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                      total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                      num_entries, base_addr, entry_size, next_pc, dst_qaddr,
                      dst_lif, dst_qtype, dst_qid, pad) {


  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t1_s2s_scratch, nvme_kivec_t1_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)
  NVME_KIVEC_ARM_DST_USE(nvme_kivec_arm_dst_scratch, nvme_kivec_arm_dst7)

  // For D vector generation (type inference). No need to translate this to ASM.
  Q_STATE_COPY(q_state_scratch)

  // Check for queue full condition before pushing
  if (QUEUE_FULL(q_state_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  } else {

    // DMA the Sequencer R2N WQE to the R2N sequencer entry
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_12,
                             q_state_scratch.base_addr +
                             (q_state_scratch.p_ndx * q_state_scratch.entry_size),
                             PHV_FIELD_OFFSET(seq_r2n_wqe.r2n_wqe_addr),
                             PHV_FIELD_OFFSET(seq_r2n_wqe.dst_qaddr),
                             0, 0, 0, 0)

    // Push the entry to the queue.  
    QUEUE_PUSH(q_state_scratch)

    // Form the doorbell and setup the DMA command to push the entry by
    // incrementing p_ndx
    modify_field(doorbell_addr_scratch.addr,
                 STORAGE_DOORBELL_ADDRESS(nvme_kivec_t1_s2s.dst_qtype, 
                                          nvme_kivec_t1_s2s.dst_lif,
                                          DOORBELL_SCHED_WR_SET, 
                                          DOORBELL_UPDATE_NONE));
    modify_field(qpush_doorbell_data.data, STORAGE_DOORBELL_DATA(0, 0, 0, 0));

    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_13, 
                             0,
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             PHV_FIELD_OFFSET(qpush_doorbell_data.data),
                             0, 0, 0, 0)

    // Fence the DMA command to ring the doorbell

    // Exit the pipeline here 
  }
}

/*****************************************************************************
 * Storage Tx NVME initiator command processing END
 *****************************************************************************/


/*****************************************************************************
 * Storage Tx NVME initiator status processing BEGIN
 *****************************************************************************/

/*****************************************************************************
 *  pop_r2n_sq: Pop the R2N SQ to dequeue the R2N WQE entry which contains a 
 *             pointer to the status buffer.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action pop_r2n_sq(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                  total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                  num_entries, base_addr, entry_size, next_pc, dst_qaddr,
                  dst_lif, dst_qtype, dst_qid, pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  Q_STATE_COPY_STAGE0(q_state_scratch)

  // If queue is empty exit
  if (QUEUE_EMPTY(q_state_scratch)) {
    // Evaluate and clear the doorbell if p_ndx == c_ndx
    //QUEUE_POP_DOORBELL_UPDATE

    exit();
  } else {
    // Increment the working consumer index. In ASM this should be a table write.
    QUEUE_POP(q_state_scratch)
   
    // In ASM, derive these from the K+I for stage 0
    modify_field(nvme_kivec_global.src_qaddr, 0);
    modify_field(nvme_kivec_global.src_lif, 0);
    modify_field(nvme_kivec_global.src_qtype, 0);
    modify_field(nvme_kivec_global.src_qid, 0);

    // Store fields needed in the K+I vector
    modify_field(nvme_kivec_t0_s2s.w_ndx, w_ndx);
    modify_field(nvme_kivec_t0_s2s.dst_lif, dst_lif);
    modify_field(nvme_kivec_t0_s2s.dst_qtype, dst_qtype);
    modify_field(nvme_kivec_t0_s2s.dst_qid, dst_qid);
    modify_field(nvme_kivec_t0_s2s.dst_qaddr, dst_qaddr);
  }
}


/*****************************************************************************
 *  handle_r2n_wqe: Read the R2N WQE posted by ROCE to get the status buffer
 *                  address. Load the status buffer for the next stage.
 *****************************************************************************/

action handle_r2n_wqe(handle, data_size, opcode, status) {

  // For D vector generation (type inference). No need to translate this to ASM.
  R2N_WQE_BASE_COPY(r2n_wqe_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // If opcode is set to process WQE
  if (r2n_wqe_scratch.opcode == R2N_OPCODE_PROCESS_WQE) {
    // Load the PVM VF SQ context for the next stage to push the NVME command
    CAPRI_LOAD_TABLE_ADDR(common_te0_phv, handle,
                          STORAGE_DEFAULT_TBL_LOAD_SIZE, process_be_status_start)
  }
}

/*****************************************************************************
 *  process_be_status: Process the backend status. If there is any error, 
 *                     set I/O status to error and it will be handled to ARM. 
 *****************************************************************************/

action process_be_status(time_us, be_status, is_q0, be_rsvd, r2n_buf_handle,
                         cspec, rsvd, sq_head, sq_id, cid, phase, status,
                         iob_addr, nvme_data_len, oper_status, is_read, 
                         is_remote, nvme_sq_qaddr) {

  // Save NVME Backend status header, NVME status to PHV
  NVME_BE_STA_HDR_COPY(nvme_be_sta_hdr)
  NVME_STATUS_COPY(nvme_sta)

  // For D vector generation (type inference). No need to translate this to ASM.
  IO_CTX_ENTRY_COPY(io_ctx_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // If I/O returned with error status, mark it in oper_status of I/O context
  if (status != NVME_STATUS_SUCESS) {
    modify_field(nvme_kivec_global.oper_status, IO_CTX_OPER_STATUS_BE_ERROR);
  }

  // Load the I/O context for processing in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, iob_addr + IO_BUF_IO_CTX_OFFSET,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, process_io_ctx_start)
}

/*****************************************************************************
 *  process_io_ctx: Process I/O context and finish the I/O if there are
 *                  no errors from backend processing:
 *                  1. For read commands which involve large I/O xfers or 
 *                     services kickstart sequencers for those services
 *                     (don't execute steps 2,3). Status should be sent only
 *                     after those operations are done.
 *                  2. For read commands which dont involve large I/O xfers
 *                     processing, form PDMA command to DMA data to host memory
 *                  3. For write commands read commands which dont involve 
 *                     large I/O xfers, start processing to send NVME status 
 *                     back to host.
 *****************************************************************************/

action process_io_ctx(iob_addr, nvme_data_len, oper_status, is_read, is_remote,
                      nvme_sq_qaddr) {

  // For D vector generation (type inference). No need to translate this to ASM.
  IO_CTX_ENTRY_COPY(io_ctx_scratch)

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // Form the oper status in the K+I vector 
  if ((nvme_kivec_global_scratch.oper_status != IO_CTX_OPER_STATUS_BE_ERROR) and
      (nvme_kivec_global_scratch.oper_status != IO_CTX_OPER_STATUS_TIMED_OUT)) {
    modify_field(nvme_kivec_global.oper_status, IO_CTX_OPER_STATUS_COMPLETED);
  }

  // DMA the Operation status. Keep it as the last DMA command so that the ARM
  // sees the update after the I/O is truly complete.
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_15, 
                           nvme_kivec_t0_s2s.iob_addr + IO_BUF_IO_CTX_OFFSET +
                           IO_CTX_ENTRY_OPER_STATUS_OFFSET,
                           PHV_FIELD_OFFSET(nvme_kivec_global.oper_status),
                           PHV_FIELD_OFFSET(nvme_kivec_global.oper_status),
                           0, 0, 0, 0)

  // Store the NVME SQ address as the destination (for now) in the K+I vector 
  // This will be used to derive the actual destination CQ with a lookup later
  modify_field(nvme_kivec_t0_s2s_scratch.dst_qaddr, nvme_sq_qaddr);
  
  // Set the fields in the K+I vector to xfer data for read command which
  // dont involve large I/O xfers
  // In ASM the first if check has to be conditional check and not PHV based
  if ((nvme_kivec_global_scratch.oper_status == IO_CTX_OPER_STATUS_COMPLETED) and
      (io_ctx_scratch.is_read == 1) and 
      (io_ctx_scratch.nvme_data_len <= NVME_READ_MAX_INLINE_DATA_SIZE)) {
    modify_field(nvme_kivec_t0_s2s.is_read, 1);
    modify_field(nvme_kivec_global.nvme_data_len, io_ctx_scratch.nvme_data_len);
  }

  // Load the I/O context for processing in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, 
                        nvme_kivec_t0_s2s.iob_addr + IO_BUF_PRP_LIST_OFFSET,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, send_read_data_start)
}

/*****************************************************************************
 *  send_read_data: Send read data for cases which don't involve large I/O
 *                  xfers
 *****************************************************************************/

action send_read_data(entry0, entry1, entry2, entry3, entry4, entry5, entry6,
                      entry7) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_PRP_LIST_COPY(prp_list_scratch)

  // Initialize the data length transferred
  modify_field(nvme_scratch.data_len_xferred, 0);

  // Start the data xfer (upto 4k) if this is a read command. 
  // This Macro takes care of the read command check and the fact that
  // prp list entry maybe set to 0 or oper_status may an error
  NVME_DATA_XFER_TO_HOST(nvme_kivec_t0_s2s, nvme_kivec_global_scratch, 
                         dma_m2m_1, dma_m2m_2,
                         prp_list_scratch.entry0, nvme_scratch)
  NVME_DATA_XFER_TO_HOST(nvme_kivec_t0_s2s, nvme_kivec_global_scratch,
                         dma_m2m_3, dma_m2m_4,
                         prp_list_scratch.entry1, nvme_scratch)

  // Load the NVME SQ (stored in dst_qaddr) to figure out the CQ to send
  // the NVME status to in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, nvme_kivec_t0_s2s.dst_qaddr,
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, lookup_sq_start)

}


/*****************************************************************************
 *  lookup_sq : Lookup the SQ and derive the CQ information. Also update the
 *              SQ head in the NVME status based on the c_ndx.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action lookup_sq(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
                 total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
                 num_entries, base_addr, entry_size, next_pc, vf_id, 
                 sq_id, cq_lif, cq_qtype, cq_qid, cq_qaddr,
                 arm_lif, arm_qtype, arm_qid, arm_qaddr,
                 io_map_base_addr, io_map_num_entries, iob_ring_base_addr,
                 pad) {

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_SQ_STATE_COPY(nvme_sq_state_scratch)

  // Set destination LIF parameters to point to the CQ 
  modify_field(nvme_kivec_t0_s2s.dst_lif, cq_lif);
  modify_field(nvme_kivec_t0_s2s.dst_qtype, cq_qtype);
  modify_field(nvme_kivec_t0_s2s.dst_qid, cq_qid);
  modify_field(nvme_kivec_t0_s2s.dst_qaddr, cq_qaddr);

  // Store the c_ndx in the NVME status as SQ head
  modify_field(nvme_sta.sq_head, c_ndx);

  // Load the NVME CQ to push the NVME status in the next stage
  CAPRI_LOAD_TABLE_ADDR(common_te0_phv, cq_qaddr, 
                        STORAGE_DEFAULT_TBL_LOAD_SIZE, push_cq_start)
}

/*****************************************************************************
 *  push_cq : Push the NVME status after setting the phase bit to the CQ. 
 *            Alter the phase bit in the CQ context in event wrapping around 
 *            the CQ.
 *****************************************************************************/

@pragma little_endian p_ndx c_ndx
action push_cq(pc_offset, rsvd, cosA, cosB, cos_sel, eval_last, 
               total_rings, host_rings, pid, p_ndx, c_ndx, w_ndx,
               num_entries, base_addr, entry_size, next_pc, vf_id, 
               intr_addr, intr_data, intr_en, phase, pad) {

  // Store the K+I vector into scratch to get the K+I generated correctly
  NVME_KIVEC_S2S_USE(nvme_kivec_t0_s2s_scratch, nvme_kivec_t0_s2s)
  NVME_KIVEC_GLOBAL_USE(nvme_kivec_global_scratch, nvme_kivec_global)

  // For D vector generation (type inference). No need to translate this to ASM.
  NVME_CQ_STATE_COPY(nvme_cq_state_scratch)

  // Set the phase bit in the NVME status
  modify_field(nvme_sta.phase, phase);

  // Check for queue full condition before pushing
  if (QUEUE_FULL(nvme_cq_state_scratch)) {

    // Exit pipeline here without error handling for now. This event of 
    // destination queue being full should never happen with constraints 
    // imposed by the control path programming.
    exit();

  } else {

    // DMA the NVME status entry to the CQ ring buffer
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_13,
                             nvme_cq_state_scratch.base_addr +
                             (nvme_cq_state_scratch.p_ndx * nvme_cq_state_scratch.entry_size),
                             PHV_FIELD_OFFSET(nvme_sta.cspec),
                             PHV_FIELD_OFFSET(nvme_sta.status),
                             0, 0, 0, 0)

    // Push the entry to the queue.  
    QUEUE_PUSH(nvme_cq_state_scratch)
   
    // In ASM, if new p_ndx has wrapped around, tblwr of phase bit to its complement
#if 0
    tblwr	d.phase, (d.phase + 1) % 2
#endif

    // Raise the MSIx interrupt if enabled
    if (nvme_cq_state_scratch.intr_en == 1) {
      modify_field(pci_intr_addr_scratch.addr, nvme_cq_state_scratch.intr_addr);
      modify_field(pci_intr_data.data, nvme_cq_state_scratch.intr_data);
      DMA_COMMAND_PHV2MEM_FILL(dma_p2m_14, 
                               0,
                               PHV_FIELD_OFFSET(pci_intr_data.data),
                               PHV_FIELD_OFFSET(pci_intr_data.data),
                               0, 0, 0, 0)
      // Fence the DMA command to set the interrupt
    }

    // Exit the pipeline here 
  }
}

/*****************************************************************************
 * Storage Tx NVME initiator status processing END
 *****************************************************************************/
