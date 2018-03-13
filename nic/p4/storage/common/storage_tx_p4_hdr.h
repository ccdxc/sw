/*****************************************************************************
 * storage_tx_p4_hdr.h: Defintions needed for storage_tx P4+ program
 *****************************************************************************/
 
#ifndef STORAGE_TX_P4_HDR_H
#define STORAGE_TX_P4_HDR_H

#include "storage_p4_hdr.h"


// Generic Queue State. Total size can be 64 bytes at most.
header_type q_state_t {
  fields {
    pc_offset	: 8;	// Program counter (relative offset)
    rsvd	: 8;	// Hardware reserved field
    cosA	: 4;	// Cos value A
    cosB	: 4;	// Cos value B
    cos_sel	: 8;	// Cos selector
    eval_last	: 8;	// Evaluator of "work ready" for ring
    total_rings	: 4;	// Total number of rings used by this qstate
    host_rings	: 4;	// Number of host facing rings used by this qstate
    pid		: 16;	// PID value to be compared with that from host
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    w_ndx	: 16;	// Working consumer index
    num_entries	: 16;	// Number of queue entries (power of 2 of this value)
    base_addr	: 64;	// Base address of queue entries
    entry_size	: 16;	// Size of each queue entry
    next_pc	: 28;	// Next program's PC
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
    dst_qaddr	: 34;	// Destination queue state address
    vf_id	: 16;   // VF id (valid only for NVME LIF)
    sq_id	: 16;   // Submission queue id (valid only for NVME LIF)
    ssd_bm_addr	: 34;	// Pointer to bitmap which is used to save SSD commands
    ssd_q_num	: 16;	// Number of entries in the SSD priority queue
    ssd_q_size	: 16;	// Size of each queue state entry in SSD priority queue
    ssd_ci_addr	: 34;	// Address of the consumer index in the SSD qstate
    pad		: 72;	// Align to 64 bytes
  }
}

// Pririority Queue State. Total size can be 64 bytes at most.
header_type pri_q_state_t {
  fields {
    pc_offset	: 8;	// Program counter (relative offset)
    rsvd	: 8;	// Hardware reserved field
    cosA	: 4;	// Cos value A
    cosB	: 4;	// Cos value B
    cos_sel	: 8;	// Cos selector
    eval_last	: 8;	// Evaluator of "work ready" for ring
    host_rings	: 4;	// Number of host facing rings used by this qstate
    total_rings	: 4;	// Total number of rings used by this qstate
    pid		: 16;	// PID value to be compared with that from host
    p_ndx_hi	: 16;	// Producer Index (high priority)
    c_ndx_hi	: 16;	// Consumer Index (high priority)
    p_ndx_med	: 16;	// Producer Index (medium priority)
    c_ndx_med	: 16;	// Consumer Index (medium priority)
    p_ndx_lo	: 16;	// Producer Index (low priority)
    c_ndx_lo	: 16;	// Consumer Index (low priority)
    w_ndx_hi	: 16;	// Working consumer index (high priority)
    w_ndx_med	: 16;	// Working consumer index (medium priority)
    w_ndx_lo	: 16;	// Working consumer index (low priority)
    num_entries	: 16;	// Number of queue entries (power of 2 of this value)
    base_addr	: 64;	// Base address of queue entries
    entry_size	: 16;	// Size of each queue entry
    hi_weight	: 8;	// Weight of high pri queue
    med_weight	: 8;	// Weight of medium pri queue
    lo_weight	: 8;	// Weight of low pri queue
    hi_running	: 8;	// Number of commands running from high pri queue
    med_running	: 8;	// Number of commands running from medium pri queue
    lo_running	: 8;	// Number of commands running from low pri queue
    num_running	: 8;	// Total number of commands running
    max_cmds	: 8;	// Maximum number of commands than can be running
    next_pc	: 28;	// Next program's PC
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
    dst_qaddr	: 34;	// Destination queue state address
    ssd_bm_addr	: 34;	// Pointer to bitmap which is used to save SSD commands
    pad		: 10;	// Align to 64 bytes
  }
}

// Queue State for access across the PCI bus - e.g. NVME driver, SSD, PVM. 
// Total size can be 64 bytes at most.
header_type pci_q_state_t {
  fields {
    pc_offset	: 8;	// Program counter (relative offset)
    rsvd	: 8;	// Hardware reserved field
    cosA	: 4;	// Cos value A
    cosB	: 4;	// Cos value B
    cos_sel	: 8;	// Cos selector
    eval_last	: 8;	// Evaluator of "work ready" for ring
    total_rings	: 4;	// Total number of rings used by this qstate
    host_rings	: 4;	// Number of host facing rings used by this qstate
    pid		: 16;	// PID value to be compared with that from host
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    w_ndx	: 16;	// Working consumer index
    num_entries	: 16;	// Number of queue entries (power of 2 of this value)
    base_addr	: 64;	// Base address of queue entries
    entry_size	: 16;	// Size of each queue entry
    push_addr	: 64;	// Address where the push data (pndx) is to be written
    intr_addr	: 64;	// Address where the MSI-X interrupt is to be raised
    intr_data	: 32;	// Preformed data for raising the MSI-X interrupt
    intr_en	:  1;	// 1 => Fire the MSI-X interrupt, 0 => don't fire
    pad		: 143;	// Align to 64 bytes
  }
}

// Barco ring 
header_type barco_ring_t {
  fields {
    p_ndx	: 32;	// Producer Index
  }
}

// ROCE CQ CB
header_type roce_cq_cb_t {
  fields {
    pc_offset	: 8;	// Program counter (relative offset)
    rsvd	: 8;	// Hardware reserved field
    cosA	: 4;	// Cos value A
    cosB	: 4;	// Cos value B
    cos_sel	: 8;	// Cos selector
    eval_last	: 8;	// Evaluator of "work ready" for ring
    total_rings	: 4;	// Total number of rings used by this qstate
    host_rings	: 4;	// Number of host facing rings used by this qstate
    pid		: 16;	// PID value to be compared with that from host
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    base_addr	: 64;	// Base address of queue entries
    page_size	: 5;	// Size of each page (power of 2 of this value)
    entry_size	: 5;	// Size of each WQE (power of 2 of this value)
    num_entries	: 5;	// Number of WQE (power of 2 of this value)
    rsvd0	: 1;	// ROCE reserved
    cq_id	: 24;	// Completion queue id
    eq_id	: 24;	// Event queue id
    rsvd1	: 64;	// ROCE specific values to end the CQ CB at 32 bytes
    w_ndx	: 16;	// Working consumer index
    next_pc	: 28;	// Next program's PC
    xlate_addr	: 34;	// Address of table to translate ROCE QP to PVM SQ
    rcq_lif	: 11;	// ROCE's CQ LIF number
    rcq_qtype	: 3;	// ROCE's CQ LIF type (within the LIF)
    rcq_qid	: 24;	// ROCE's CQ queue number (within the LIF)
    pad		: 108;	// Align to 64 bytes
  }
}

// ROCE RQ CB
header_type roce_rq_cb_t {
  fields {
    pc_offset	: 8;	// Program counter (relative offset)
    rsvd	: 8;	// Hardware reserved field
    cosA	: 4;	// Cos value A
    cosB	: 4;	// Cos value B
    cos_sel	: 8;	// Cos selector
    eval_last	: 8;	// Evaluator of "work ready" for ring
    total_rings	: 4;	// Total number of rings used by this qstate
    host_rings	: 4;	// Number of host facing rings used by this qstate
    pid		: 16;	// PID value to be compared with that from host
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    extra_rings	: 160;	// Additional rings used by ROCE RQ
    base_addr	: 64;	// Base address of queue entries
    rsvd0	: 72;	// Data used be ROCE internally
    pmtu	: 5;	// PMTU size (power of 2 of this value)
    page_size	: 5;	// Size of each page (power of 2 of this value)
    entry_size	: 5;	// Size of each WQE (power of 2 of this value)
    num_entries	: 5;	// Number of WQE (power of 2 of this value)
    pad		: 100;	// Align to 64 bytes
  }
}

// PVM's ROCE SQ CB - PVM's altered copy of the ROCE SQ CB. Uses only one ring
//                    and stores internal addresses which are not part of 
//                    ROCE CQ CB.
header_type pvm_roce_sq_cb_t {
  fields {
    pc_offset	: 8;	// Program counter (relative offset)
    rsvd	: 8;	// Hardware reserved field
    cosA	: 4;	// Cos value A
    cosB	: 4;	// Cos value B
    cos_sel	: 8;	// Cos selector
    eval_last	: 8;	// Evaluator of "work ready" for ring
    total_rings	: 4;	// Total number of rings used by this qstate
    host_rings	: 4;	// Number of host facing rings used by this qstate
    pid		: 16;	// PID value to be compared with that from host
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    base_addr	: 64;	// Base address of queue entries
    page_size	: 5;	// Size of each page (power of 2 of this value)
    entry_size	: 5;	// Size of each WQE (power of 2 of this value)
    num_entries	: 5;	// Number of WQE (power of 2 of this value)
    rsvd0	: 1;	// ROCE reserved
    roce_msn	: 32;	// ROCE message sequence number
    w_ndx	: 16;	// Working consumer index
    next_pc	: 28;	// Next program's PC
    rrq_lif	: 11;	// ROCE RQ LIF number
    rrq_qtype	: 3;	// ROCE RQ LIF type (within the LIF)
    rrq_qid	: 24;	// ROCE RQ queue number (within the LIF)
    rrq_qaddr	: 34;	// ROCE RQ queue state address
    rsq_lif	: 11;	// PVM's R2N LIF number
    rsq_qtype	: 3;	// PVM's R2N LIF type (within the LIF)
    rsq_qid	: 24;	// PVM's R2N queue number (within the LIF)
    pad		: 150;	// Align to 64 bytes
  }
}


// Trailer in the PVM cmd containing source information
header_type pvm_cmd_trailer_t {
  fields {
    tickreg	: 64;	// Microsecond reg from NIC
    vf_id	: 16;	// VF number
    sq_id	: 16;	// Submission queue id
    num_prps	: 8;	// Number of additional PRPs
  }
}

// Trailer in the PVM status used to push the status back
header_type pvm_sta_trailer_t {
  fields {
    // Information passed back by PVM
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
    dst_qaddr	: 34;	// Destination queue state address
  }
}

// WQE format of ROCE CQ. TODO: Is this a 64 byte or 32 byte entry ? Adjust accordingly
header_type roce_cq_wqe_t {
  fields {
    wrid_msn	: 64;	// WRID or MSN depending on op_type
    op_type	: 8;	// Operation type
    status	: 8;	// Status code
    qp		: 24;	// Queue pair id
    rsvd1	: 152;	// ROCE specific bits
  }
}

// WQE format of ROCE SQ
header_type roce_sq_wqe_t {
  fields {
    wrid		: 64;	// Work request id
    op_type		: 4;	// Operation type
    complete_notify	: 1;	
    fence		: 1;
    solicited_event	: 1;
    inline_data_vld	: 1;
    num_sges		: 8;	// Number of SGEs
    rsvd2		: 16;
    op_data		: 416;	// Storage P4+ does not interpret this
  }
}

// WQE format of ROCE RQ
header_type roce_rq_wqe_t {
  fields {
    wrid		: 64;	// Work request id
    num_sges		: 8;	// Number of SGEs
    rsvd0		: 184;
    sge_va		: 64;	// VA of the buffer
    sge_len		: 32;	// Length of buffer
    sge_l_key		: 32;	// L_KEY 
    rsvd1		: 128;	// Second SGE
  }
}

// The entry used to xlate ROCE SQ to PVM SQ for releasing the ROCE SQ entry.
// Implemented as an array indexed by ROCE QP+1. QP0 is reserved for R2N SQ.
// Note: Total size is made 64 bytes to fit in the d-vector. With 64 such 
//       SQs planned for PVM use, there should not be a space constraint. 
//       This can be revisited to pack more entries in one d-vector if 
//       the number of queues need to be scaled much higher.
header_type roce_sq_xlate_entry_t {
  fields {
    next_pc	: 28;	// Next program's PC
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
    dst_qaddr	: 34;	// Destination queue state address
    pad		: 412;	// Align to 64 byte boundary
  }
}

/********************************************************************
 *
 * R2N Buffer format (size of each entry in bytes indicated []):
 *
 * 1. roce_rq_wqe_t					[64]
 * 2. nvme_be_prp_list_t				[64]
 * 3. nvme_be_cmd_hdr_t + 376 bits pad + nvme_be_cmd_t	[128]
 * 4. nvme_be_sta_hdr_t + nvme_be_sta_t + 256 bits pad	[64]
 * 5. roce_sq_wqe_t (for sending write data over ROCE)	[64]
 * 6. roce_sq_wqe_t (for sending status over ROCE)	[64]
 *
 * Notes:
 *  a. nvme_be_cmd_t contains a ptr to the R2N buffer start.
 *  b. On the wire data from #3 to #6 is sent.
 *  c. ROCE gives ptr to #3 in its CQ WQE. R2N buffer ptr is derived.
 *  d. PVM gives ptr to #3 in its SQ WQE. R2N buffer ptr is derived.
 *  e. #1, #2, #5, #6 are not used by PVM for local commands.
 *
********************************************************************/

// WQE format of R2N layer. Used by:
// 1. R2N's SQ/CQ with local PVM for local target.
// 2. NVME backend's priority SQ/CQ with R2N.
// 3. R2N's HQ with local PVM for ROCE buf posting
header_type r2n_wqe_t {
  fields {
    // Note: These fields are set in the WQE from PVM and the WQE from ROCE
    handle		: 64;	// Pointer to R2N buffer (or) NVME BE command 
    data_size		: 32;	// Size pointed to by handle
    opcode		: 16;	// Each use case has a distinct opcode
    status		: 16;	// Success/failure status

    // Note: These fields are set in the WQE from PVM for ringing an additional
    //       doorbell on the response handling from the SSD on a per WQE basis.
    //
    // 1. Set in the WQE from PVM when sequencer is used for local target
    // 2. Only when db_enable is set to 1, the other db_* fields are valid.
    //    Also db_enable needs to be 2 bits for byte boundary alignment.
    // 3. The status is sent to PVM's R2N CQ and is not governed by db_enable.
    db_enable		: 2;	// Whether doorbell needs to be rung 
    db_lif		: 11;	// Doorbell LIF number
    db_qtype		: 3;	// Doorbell LIF type (within the LIF)
    db_qid		: 24;	// Doorbell queue number (within the LIF)
    db_index		: 16;	// Doorbell pindex

    // Note: This field is set to 0 in the WQE sent by PVM and set to 1 in the 
    //       WQE sent by ROCE CQ handler
    is_remote		: 8;	// Is the command from local or remote PVM	

    // Note: These fields are set in the WQE from PVM or ROCE path to provide
    //       a destination after complete processing of the R2N WQE.
    //
    // 1. When these fields are set in the WQE from ROCE, they point to the 
    //    PVM's ROCE SQ queue state structure and its doorbell. The is_remote
    //    flag is set to 1 in this path. These fields are looked up in the 
    //    response handler to determine the destination queue.
    //
    // 2. When these fields are set in the WQE from PVM, they point to the 
    //    ROCE RQ to post the R2N buffer. The is_remote flag is set to 0 
    //    in this path.
    dst_lif		: 11;	// LIF number used for response
    dst_qtype		: 3;	// LIF type (within the LIF) used for response
    dst_qid		: 24;	// Queue number (within the LIF) used for response
    dst_qaddr		: 34;	// Queue state address used for response

    // Note: These fields are not set in the WQE from PVM or the WQE from ROCE. 
    //       These are internally set and used by R2N module.
    src_queue_id	: 32;	// ROCE source queue id
    ssd_handle		: 16;	// SSD handle to select NVME backend
    io_priority		: 8;	// I/O priority to select ring with the queue
    is_read		: 8;	// If NVME command is a read
    r2n_buf_handle	: 64;	// Back pointer to the R2N buffer
    nvme_cmd_cid	: 16;	// Command identifier
    pri_qaddr		: 34;	// Priority queue state address for status
    pad			: 6;	// Align to byte boundary
    // Note: The total size of this cannot exceed 64 bytes
  }
}


// Bitmap of the list of outstanding commands sent to the SSD.
// Assumes a max of 64 outstanding per SSD.
header_type ssd_cmds_t {
  fields {
    bitmap	: 64;	// Max 64 outstanding commands per SSD
  }
}

// SSD's consumder index value stored in PHV for DMA
header_type ssd_ci_t {
  fields {
    c_ndx	: 16;	// SSD's Consumer index value
  }
}


// Sequencer metadata for PDMA
header_type seq_pdma_entry_t {
  fields {
    next_db_addr	: 64;	// 64 bit address of the next doorbell to ring
    next_db_data	: 64;	// 64 bit data of the next doorbell to ring
    src_addr		: 64;	// Source memory address for PDMA (HBM or Host)
    dst_addr		: 64;	// Destination memory address for PDMA (HBM or Host)
    data_size		: 32;	// Data size to be transferred 
    src_lif_override	: 1;	// Override the source LIF in the PDMA
    src_lif		: 11;	// Source LIF id used to override
    dst_lif_override	: 1;	// Override the source LIF in the PDMA
    dst_lif		: 11;	// Destination LIF id used to override
    intr_addr		: 64;	// Address where the MSI-X interrupt is to be raised
    intr_data		: 32;	// Preformed data for raising the MSI-X interrupt
    next_db_en		:  1;	// 1 => Ring next sequencer doorbell, 0 => don't ring
    intr_en		:  1;	// 1 => Fire the MSI-X interrupt, 0 => don't fire
				// NOTE: Don't enable intr_en and next_db_en together
				//       as only one will be serviced
				// Order of evaluation: 1. next_db_en 2. intr_en
  }
}

// Sequencer metadata for pushing Barco descriptor
header_type seq_barco_entry_t {
  fields {
    barco_desc_addr	: 64;	// Address of the descriptor to push
    barco_desc_size	: 32;	// Size of the descriptor to push
    barco_pndx_size	: 16;	// Size of the ring state to be loaded
    barco_pndx_addr	: 34;	// 64 bit address of the doorbell to ring
    barco_ring_addr	: 34;	// Address of the ring

    // special test batch mode: pndx value is given in entry
    barco_batch_pndx: 16;
    barco_batch_mode: 1;
    barco_batch_last: 1;
  }
}

// Sequencer metadata compression status entry
header_type seq_comp_status_desc_t {
  fields {
    next_db_addr	: 64;	// 64 bit address of the next doorbell to ring
    next_db_data	: 64;	// 64 bit data of the next doorbell to ring
    status_addr		: 64;	// Address where compression status will be placed
    data_addr		: 64;	// Address where compression data will be placed
    sgl_addr		: 64;	// Address where destination SGL will be placed for PDMA
    intr_addr		: 64;	// Address where interrupt needs to be written
    intr_data		: 32;	// Data that needs to be written for interrupt
    status_len		: 16;	// Length of the compression status
    data_len		: 16;	// Length of the compression data
    data_len_from_desc	:  1;	// 1 => Use the data length in the descriptor, 
				// 0 => Use the data lenghth in the status
    status_dma_en	:  1;	// 1 => DMA status, 0 => don't DMA 
    next_db_en		:  1;	// 1 => Ring next sequencer doorbell, 0 => don't ring
    intr_en		:  1;	// 1 => Fire the MSI-X interrupt, 0 => don't fire
				// NOTE: Don't enable intr_en and next_db_en together
				//       as only one will be serviced
				// Order of evaluation: 1. next_db_en 2. intr_en
  }
}

// Compression destination SGL metadata
header_type seq_comp_sgl_t {
  fields {
    status_addr		: 64;	// Status destination address
    addr0		: 64;	// SGL data buffer 0 address
    addr1		: 64;	// SGL data buffer 1 address
    addr2		: 64;	// SGL data buffer 2 address
    addr3		: 64;	// SGL data buffer 3 address
    len0		: 16;	// SGL data buffer 0 length
    len1		: 16;	// SGL data buffer 1 length
    len2		: 16;	// SGL data buffer 2 length
    len3		: 16;	// SGL data buffer 3 length
  }
}

// Compression status metadata
header_type seq_comp_status_t {
  fields {
    rsvd1		: 12;	// Reserved
    err			: 3;	// Error status (0: success: >0: failure)
    rsvd2		: 1;	// Reserved
    data_len		: 16;	// Output bits
    rsvd3		: 32;	// Reserved
  }
}

// Storage K+I vectors

// kivec0: header union with stage_2_stage for table 0
header_type storage_kivec0_t {
  fields {
    w_ndx		: 16;	// Working consumer index
    dst_lif		: 11;	// Destination LIF number
    dst_qtype		: 3;	// Destination LIF type (within the LIF)
    dst_qid		: 24;	// Destination queue number (within the LIF)
    dst_qaddr		: 34;	// Destination queue state address
    prp_assist		: 1;	// Download additional PRP entries (upto 16)
    is_q0		: 1;	// Is queue id 0 (admin queue) ?
    io_priority		: 8;	// I/O priority to select ring with the queue
    ssd_bm_addr		: 34;	// Pointer to bitmap used to save SSD commands
    cmd_index		: 8;	// Index into bitmap of saved SSD commands
    ssd_handle		: 16;	// SSD handle to select NVME backend
    is_remote		: 1;	// Is the command from local or remote PVM	
    is_read		: 1;	// If NVME command is a read
  }
}

// kivec1: header union with global
header_type storage_kivec1_t {
  fields {
    src_lif		: 11;	// Source LIF number
    src_qtype		: 3;	// Source LIF type (within the LIF)
    src_qid		: 24;	// Source queue number (within the LIF)
    src_qaddr		: 34;	// Source queue state address
    barco_desc_size	: 16;	// Barco descriptor size
    device_addr		: 34;	// Address of the consumer index in the SSD qstate (OR)
				// Barco ring address
    roce_cq_new_cmd	: 1;	// If ROCE CQ entry is a new commmand
  }
}

// kivec2: header union with to_stage_2
header_type storage_kivec2_t {
  fields {
    ssd_q_num	: 16;	// Number of entries in the SSD priority queue
    ssd_q_size	: 16;	// Size of each queue state entry in SSD priority queue
  }
}

// kivec3: header union with to_stage_3
header_type storage_kivec3_t {
  fields {
    roce_msn	: 32;	// ROCE message sequence number 
  }
}

// kivec4: header union with stage_2_stage for table 0
header_type storage_kivec4_t {
  fields {
    w_ndx		: 16;	// Working consumer index
    sgl_addr		: 64;	// Address where destination SGL will be placed for PDMA
    data_addr		: 64;	// Address where compression data will be placed
    data_len		: 16;	// Length of compression data (either from descriptor or 
				// from the compression status)
  }
}

// kivec5: header union with global
header_type storage_kivec5_t {
  fields {
    status_addr		: 64;	// Address where compression status will be placed
    status_len		: 16;	// Length of the compression status
    status_err		:  3;	// Error status (0: success: >0: failure)
    status_dma_en	:  1;	// 1 => DMA status, 0 => don't DMA status
    data_len_from_desc	:  1;	// 1 => Use the data length in the descriptor, 
				// 0 => Use the data lenghth in the status
  }
}
#define Q_STATE_COPY_INTRINSIC(q_state)			\
  modify_field(q_state.pc_offset, pc_offset);		\
  modify_field(q_state.rsvd, rsvd);			\
  modify_field(q_state.cosA, cosA);			\
  modify_field(q_state.cosB, cosB);			\
  modify_field(q_state.cos_sel, cos_sel);		\
  modify_field(q_state.eval_last, eval_last);		\
  modify_field(q_state.total_rings, total_rings);	\
  modify_field(q_state.host_rings, host_rings);		\
  modify_field(q_state.pid, pid);			\

#define Q_STATE_COPY_STAGE0(q_state)			\
  Q_STATE_COPY_INTRINSIC(q_state)			\
  modify_field(q_state.p_ndx, p_ndx);			\
  modify_field(q_state.c_ndx, c_ndx);			\
  modify_field(q_state.w_ndx, w_ndx);			\
  modify_field(q_state.num_entries, num_entries);	\
  modify_field(q_state.base_addr, base_addr);		\
  modify_field(q_state.entry_size, entry_size);		\
  modify_field(q_state.next_pc, next_pc);		\
  modify_field(q_state.dst_lif, dst_lif);		\
  modify_field(q_state.dst_qtype, dst_qtype);		\
  modify_field(q_state.dst_qid, dst_qid);		\
  modify_field(q_state.dst_qaddr, dst_qaddr);		\
  modify_field(q_state.vf_id, vf_id);			\
  modify_field(q_state.sq_id, sq_id);			\
  modify_field(q_state.ssd_bm_addr, ssd_bm_addr);	\
  modify_field(q_state.ssd_q_num, ssd_q_num);		\
  modify_field(q_state.ssd_q_size, ssd_q_size);		\
  modify_field(q_state.ssd_ci_addr, ssd_ci_addr);	\

#define Q_STATE_COPY(q_state)				\
  Q_STATE_COPY_STAGE0(q_state)				\
  modify_field(q_state.pad, pad);			\

#define PRI_Q_STATE_COPY_STAGE0(q_state)		\
  Q_STATE_COPY_INTRINSIC(q_state)			\
  modify_field(q_state.p_ndx_hi, p_ndx_hi);		\
  modify_field(q_state.c_ndx_hi, c_ndx_hi);		\
  modify_field(q_state.p_ndx_med, p_ndx_med);		\
  modify_field(q_state.c_ndx_med, c_ndx_med);		\
  modify_field(q_state.p_ndx_lo, p_ndx_lo);		\
  modify_field(q_state.c_ndx_lo, c_ndx_lo);		\
  modify_field(q_state.w_ndx_hi, w_ndx_hi);		\
  modify_field(q_state.w_ndx_med, w_ndx_med);		\
  modify_field(q_state.w_ndx_lo, w_ndx_lo);		\
  modify_field(q_state.num_entries, num_entries);	\
  modify_field(q_state.base_addr, base_addr);		\
  modify_field(q_state.entry_size, entry_size);		\
  modify_field(q_state.hi_weight, hi_weight);		\
  modify_field(q_state.med_weight, med_weight);		\
  modify_field(q_state.lo_weight, lo_weight);		\
  modify_field(q_state.hi_running, hi_running);		\
  modify_field(q_state.med_running, med_running);	\
  modify_field(q_state.lo_running, lo_running);		\
  modify_field(q_state.num_running, num_running);	\
  modify_field(q_state.max_cmds, max_cmds);		\
  modify_field(q_state.next_pc, next_pc);		\
  modify_field(q_state.dst_lif, dst_lif);		\
  modify_field(q_state.dst_qtype, dst_qtype);		\
  modify_field(q_state.dst_qid, dst_qid);		\
  modify_field(q_state.dst_qaddr, dst_qaddr);		\
  modify_field(q_state.ssd_bm_addr, ssd_bm_addr);	\

#define PRI_Q_STATE_COPY(q_state)			\
  PRI_Q_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pad, pad);			\

#define PCI_Q_STATE_COPY_STAGE0(q_state)		\
  Q_STATE_COPY_INTRINSIC(q_state)			\
  modify_field(q_state.p_ndx, p_ndx);			\
  modify_field(q_state.c_ndx, c_ndx);			\
  modify_field(q_state.w_ndx, w_ndx);			\
  modify_field(q_state.num_entries, num_entries);	\
  modify_field(q_state.base_addr, base_addr);		\
  modify_field(q_state.entry_size, entry_size);		\
  modify_field(q_state.push_addr, push_addr);		\
  modify_field(q_state.intr_addr, intr_addr);		\
  modify_field(q_state.intr_data, intr_data);		\
  modify_field(q_state.intr_en, intr_en);		\

#define PCI_Q_STATE_COPY(q_state)			\
  PCI_Q_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pad, pad);			\

#define ROCE_CQ_CB_COPY_STAGE0(cq_cb)			\
  Q_STATE_COPY_INTRINSIC(cq_cb)				\
  modify_field(cq_cb.p_ndx, p_ndx);			\
  modify_field(cq_cb.c_ndx, c_ndx);			\
  modify_field(cq_cb.base_addr, base_addr);		\
  modify_field(cq_cb.page_size, page_size);		\
  modify_field(cq_cb.entry_size, entry_size);		\
  modify_field(cq_cb.num_entries, num_entries);		\
  modify_field(cq_cb.rsvd0, rsvd0);			\
  modify_field(cq_cb.cq_id, cq_id);			\
  modify_field(cq_cb.eq_id, eq_id);			\
  modify_field(cq_cb.rsvd1, rsvd1);			\
  modify_field(cq_cb.w_ndx, w_ndx);			\
  modify_field(cq_cb.next_pc, next_pc);			\
  modify_field(cq_cb.xlate_addr, xlate_addr);		\
  modify_field(cq_cb.rcq_lif, rcq_lif);			\
  modify_field(cq_cb.rcq_qtype, rcq_qtype);		\
  modify_field(cq_cb.rcq_qid, rcq_qid);			\

#define ROCE_CQ_CB_COPY(cq_cb)				\
  ROCE_CQ_CB_COPY_STAGE0(cq_cb)				\
  modify_field(cq_cb.pad, pad);				\

#define PVM_ROCE_SQ_CB_COPY_STAGE0(sq_cb)		\
  Q_STATE_COPY_INTRINSIC(sq_cb)				\
  modify_field(sq_cb.p_ndx, p_ndx);			\
  modify_field(sq_cb.c_ndx, c_ndx);			\
  modify_field(sq_cb.base_addr, base_addr);		\
  modify_field(sq_cb.page_size, page_size);		\
  modify_field(sq_cb.entry_size, entry_size);		\
  modify_field(sq_cb.num_entries, num_entries);		\
  modify_field(sq_cb.rsvd0, rsvd0);			\
  modify_field(sq_cb.roce_msn, roce_msn);		\
  modify_field(sq_cb.w_ndx, w_ndx);			\
  modify_field(sq_cb.next_pc, next_pc);			\
  modify_field(sq_cb.rrq_lif, rrq_lif);			\
  modify_field(sq_cb.rrq_qtype, rrq_qtype);		\
  modify_field(sq_cb.rrq_qid, rrq_qid);			\
  modify_field(sq_cb.rrq_qaddr, rrq_qaddr);		\
  modify_field(sq_cb.rsq_lif, rsq_lif);			\
  modify_field(sq_cb.rsq_qtype, rsq_qtype);		\
  modify_field(sq_cb.rsq_qid, rsq_qid);			\

#define PVM_ROCE_SQ_CB_COPY(sq_cb)			\
  PVM_ROCE_SQ_CB_COPY_STAGE0(sq_cb)			\
  modify_field(sq_cb.pad, pad);				\

#define ROCE_RQ_CB_COPY_STAGE0(rq_cb)			\
  Q_STATE_COPY_INTRINSIC(rq_cb)				\
  modify_field(rq_cb.p_ndx, p_ndx);			\
  modify_field(rq_cb.c_ndx, c_ndx);			\
  modify_field(rq_cb.extra_rings, extra_rings);		\
  modify_field(rq_cb.base_addr, base_addr);		\
  modify_field(rq_cb.rsvd0, rsvd0);			\
  modify_field(rq_cb.pmtu, pmtu);			\
  modify_field(rq_cb.page_size, page_size);		\
  modify_field(rq_cb.entry_size, entry_size);		\
  modify_field(rq_cb.num_entries, num_entries);		\

#define ROCE_RQ_CB_COPY(rq_cb)				\
  ROCE_RQ_CB_COPY_STAGE0(rq_cb)				\
  modify_field(rq_cb.pad, pad);				\



// Copy the basic part of R2N WQE - the one sent by PVM
#define R2N_WQE_BASE_COPY(wqe)					\
  modify_field(wqe.handle, handle);				\
  modify_field(wqe.data_size, data_size);			\
  modify_field(wqe.opcode, opcode);				\
  modify_field(wqe.status, status);				\
  modify_field(wqe.db_enable, db_enable);			\
  modify_field(wqe.db_lif, db_lif);				\
  modify_field(wqe.db_qtype, db_qtype);				\
  modify_field(wqe.db_qid, db_qid);				\
  modify_field(wqe.db_index, db_index);				\
  modify_field(wqe.is_remote, is_remote);			\
  modify_field(wqe.dst_lif, dst_lif);				\
  modify_field(wqe.dst_qtype, dst_qtype);			\
  modify_field(wqe.dst_qid, dst_qid);				\
  modify_field(wqe.dst_qaddr, dst_qaddr);			\

// Copy the full R2N WQE - base from PVM + NVME backend header +
// additional fields saved before sending the NVME command to SSD

#define R2N_WQE_FULL_COPY(wqe)					\
  R2N_WQE_BASE_COPY(wqe)					\
  NVME_BE_CMD_HDR_COPY(wqe)					\
  modify_field(wqe.nvme_cmd_cid, nvme_cmd_cid);			\
  modify_field(wqe.pri_qaddr, pri_qaddr);			\
  modify_field(wqe.pad, pad);					\


#define ROCE_XLATE_ENTRY_COPY(entry)			\
  modify_field(entry.next_pc, next_pc);			\
  modify_field(entry.dst_lif, dst_lif);			\
  modify_field(entry.dst_qtype, dst_qtype);		\
  modify_field(entry.dst_qid, dst_qid);			\
  modify_field(entry.dst_qaddr, dst_qaddr);		\
  modify_field(entry.pad, pad);				\
  

#define STORAGE_KIVEC0_USE(scratch, kivec)				\
  modify_field(scratch.w_ndx, kivec.w_ndx);				\
  modify_field(scratch.dst_lif, kivec.dst_lif);				\
  modify_field(scratch.dst_qtype, kivec.dst_qtype);			\
  modify_field(scratch.dst_qid, kivec.dst_qid);				\
  modify_field(scratch.dst_qaddr, kivec.dst_qaddr);			\
  modify_field(scratch.prp_assist, kivec.prp_assist);			\
  modify_field(scratch.is_q0, kivec.is_q0);				\
  modify_field(scratch.io_priority, kivec.io_priority);			\
  modify_field(scratch.ssd_bm_addr, kivec.ssd_bm_addr);			\
  modify_field(scratch.cmd_index, kivec.cmd_index);			\
  modify_field(scratch.ssd_handle, kivec.ssd_handle);			\
  modify_field(scratch.is_remote, kivec.is_remote);			\
  modify_field(scratch.is_read, kivec.is_read);				\

#define STORAGE_KIVEC1_USE(scratch, kivec)				\
  modify_field(scratch.src_lif, kivec.src_lif);				\
  modify_field(scratch.src_qtype, kivec.src_qtype);			\
  modify_field(scratch.src_qid, kivec.src_qid);				\
  modify_field(scratch.src_qaddr, kivec.src_qaddr);			\
  modify_field(scratch.barco_desc_size, kivec.barco_desc_size);		\
  modify_field(scratch.device_addr, kivec.device_addr);			\

#define STORAGE_KIVEC2_USE(scratch, kivec)				\
  modify_field(scratch.ssd_q_num, kivec.ssd_q_num);			\
  modify_field(scratch.ssd_q_size, kivec.ssd_q_size);			\

#define STORAGE_KIVEC3_USE(scratch, kivec)				\
  modify_field(scratch.roce_msn, kivec.roce_msn);			\

#define STORAGE_KIVEC4_USE(scratch, kivec)				\
  modify_field(scratch.w_ndx, kivec.w_ndx);				\
  modify_field(scratch.sgl_addr, kivec.sgl_addr);			\
  modify_field(scratch.data_addr, kivec.data_addr);			\
  modify_field(scratch.data_len, kivec.data_len);			\

#define STORAGE_KIVEC5_USE(scratch, kivec)				\
  modify_field(scratch.status_addr, kivec.status_addr);			\
  modify_field(scratch.status_len, kivec.status_len);			\
  modify_field(scratch.status_err, kivec.status_err);			\
  modify_field(scratch.status_dma_en, kivec.status_dma_en);		\
  modify_field(scratch.data_len_from_desc, kivec.data_len_from_desc);	\


// Macros for ASM param addresses (hardcoded in P4)
#define q_state_push_start		0x80000000
#define pci_q_state_push_start		0x80001000
#define seq_q_state_push_start		0x80002000
#define pri_q_state_push_start		0x80003000
#define pri_q_state_incr_start		0x80004000
#define pri_q_state_decr_start		0x80005000
#define nvme_be_wqe_prep_start		0x80006000
#define nvme_be_wqe_save_start		0x80007000
#define nvme_be_wqe_release_start	0x80008000
#define nvme_be_wqe_handler_start	0x80009000
#define nvme_be_cmd_handler_start	0x8000A000
#define seq_barco_ring_push_start	0x8000B000
#define roce_sq_xlate_start		0x8000C000
#define roce_r2n_wqe_prep_start		0x8000D000
#define pvm_roce_sq_cb_update_start	0x8000E000
#define roce_rq_push_start		0x8000F000
#define pvm_roce_sq_cb_push_start	0x80010000
#define seq_pvm_roce_sq_cb_push_start	0x80020000
#define seq_comp_status_handler_start	0x80030000
#define seq_comp_sgl_handler_start	0x80040000


#endif     // STORAGE_TX_P4_HDR_H
