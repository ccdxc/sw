/*****************************************************************************
 * storage_nvme_p4_hdr.h: Defintions needed for storage_nvme P4+ program
 *****************************************************************************/
 
#ifndef STORAGE_NVME_P4_HDR_H
#define STORAGE_NVME_P4_HDR_H

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
    pad		: 204;	// Align to 64 bytes
  }
}

// NVME SQ Queue State. Total size can be 64 bytes at most.
header_type nvme_sq_state_t {
  fields {
    pc_offset		: 8;	// Program counter (relative offset)
    rsvd		: 8;	// Hardware reserved field
    cosA		: 4;	// Cos value A
    cosB		: 4;	// Cos value B
    cos_sel		: 8;	// Cos selector
    eval_last		: 8;	// Evaluator of "work ready" for ring
    total_rings		: 4;	// Total number of rings used by this qstate
    host_rings		: 4;	// Number of host facing rings used by this qstate
    pid			: 16;	// PID value to be compared with that from host
    p_ndx		: 16;	// Producer Index
    c_ndx		: 16;	// Consumer Index
    w_ndx		: 16;	// Working consumer index
    num_entries		: 16;	// Number of queue entries (power of 2 of this value)
    base_addr		: 64;	// Base address of queue entries
    entry_size		: 16;	// Size of each queue entry
    next_pc		: 28;	// Next program's PC
    vf_id		: 16;   // VF id (valid only for NVME LIF)
    sq_id		: 16;   // Submission queue id (valid only for NVME LIF)
    cq_lif		: 11;	// NVME CQ LIF number
    cq_qtype		: 3;	// NVME CQ LIF type (within the LIF)
    cq_qid		: 24;	// NVME CQ queue number (within the LIF)
    cq_qaddr		: 34;	// NVME CQ queue state address
    arm_lif		: 11;	// ARM Q LIF number
    arm_qtype		: 3;	// ARM Q LIF type (within the LIF)
    arm_base_qid	: 24;	// ARM Base Q queue number (within the LIF)
    arm_base_qaddr	: 34;	// ARM Base Q queue state address
    io_map_base_addr	: 34;	// IO map table base address (entry = base + (nsid * size))
    io_map_num_entries	: 16;	// Number of entries in the IO map table
    iob_ring_base_addr	: 34;	// IOB ring base address (free list of IOBs)
    pad			: 16;	// Align to 64 bytes
  }
}

// NVME CQ Queue State. Total size can be 64 bytes at most.
header_type nvme_cq_state_t {
  fields {
    pc_offset		: 8;	// Program counter (relative offset)
    rsvd		: 8;	// Hardware reserved field
    cosA		: 4;	// Cos value A
    cosB		: 4;	// Cos value B
    cos_sel		: 8;	// Cos selector
    eval_last		: 8;	// Evaluator of "work ready" for ring
    total_rings		: 4;	// Total number of rings used by this qstate
    host_rings		: 4;	// Number of host facing rings used by this qstate
    pid			: 16;	// PID value to be compared with that from host
    p_ndx		: 16;	// Producer Index
    c_ndx		: 16;	// Consumer Index
    w_ndx		: 16;	// Working consumer index
    num_entries		: 16;	// Number of queue entries (power of 2 of this value)
    base_addr		: 64;	// Base address of queue entries
    entry_size		: 16;	// Size of each queue entry
    next_pc		: 28;	// Next program's PC
    intr_addr		: 32;	// MSI-X interrupt address
    intr_data		: 32;	// MSI-X interrupt data
    intr_en		: 1;	// 1 => Fire the MSI-X interrupt, 0 => don't fire
    phase		: 1;	// Phase bit
    pad			: 210;	// Align to 64 bytes
  }
}

// ARM  Queue State.  Total size can be 64 bytes at most.
header_type arm_q_state_t {
  fields {
    pc_offset		: 8;	// Program counter (relative offset)
    rsvd		: 8;	// Hardware reserved field
    cosA		: 4;	// Cos value A
    cosB		: 4;	// Cos value B
    cos_sel		: 8;	// Cos selector
    eval_last		: 8;	// Evaluator of "work ready" for ring
    total_rings		: 4;	// Total number of rings used by this qstate
    host_rings		: 4;	// Number of host facing rings used by this qstate
    pid			: 16;	// PID value to be compared with that from host
    p_ndx		: 16;	// Producer Index
    c_ndx		: 16;	// Consumer Index
    w_ndx		: 16;	// Working consumer index
    num_entries		: 16;	// Number of queue entries (power of 2 of this value)
    base_addr		: 64;	// Base address of queue entries
    entry_size		: 16;	// Size of each queue entry
    next_pc		: 28;	// Next program's PC
    dst_lif		: 11;	// Destination LIF number
    dst_qtype		: 3;	// Destination LIF type (within the LIF)
    dst_qid		: 24;	// Destination queue number (within the LIF)
    dst_qaddr		: 34;	// Destination queue state address
    intr_addr		: 32;	// MSI-X interrupt address
    intr_data		: 32;	// MSI-X interrupt data
    intr_en		: 1;	// 1 => Fire the MSI-X interrupt, 0 => don't fire
    phase		: 1;	// Phase bit
    iob_ring_base_addr	: 34;	// IOB ring base address (free list of IOBs)
    pad			: 104;	// Align to 64 bytes
  }
}


// Generic Ring State. Total size can be 64 bytes at most.
header_type ring_state_t {
  fields {
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    wp_ndx	: 16;	// Working producer index
    num_entries	: 16;	// Number of queue entries (power of 2 of this value)
    base_addr	: 64;	// Base address of queue entries
    entry_size	: 16;	// Size of each queue entry
    pad		: 368;	// Align to 64 bytes
  }
}

// Doorbell cleanup state
header_type doorbell_cleanup_q_state_t {
  fields {
    p_ndx	: 16;	// Producer Index
    c_ndx	: 16;	// Consumer Index
    w_ndx	: 16;	// Working consumer index
  }
}

// Queue push p_ndx data
header_type qpush_pndx_data_t {
  fields {
    p_ndx	: 16;	// Producer Index
  }
}

// Sequencer doorbell information
header_type seq_db_info_t {
  fields {
    lif		: 11;	// LIF number
    qtype	: 3;	// LIF type (within the LIF)
    qid		: 24;	// Queue number (within the LIF)
    qaddr	: 34;	// Queue state address
  }
}

// IO Map table entry
header_type io_map_entry_t {
  fields {
    entry_addr		: 64;	// Address of the IO map entry (points to itself)
    nsid		: 32;	// Namespace identifier
    vf_id		: 16;	// VF number
    dst_flags		: 32;	// Destination Flags (encryption, compression etc)
    r2n_lif		: 11;	// Destination LIF number
    r2n_qtype		: 3;	// Destination LIF type (within the LIF)
    r2n_qid		: 24;	// Destination queue number (within the LIF)
    r2n_qaddr		: 34;	// Destination queue state address
    roce_lif		: 11;	// Destination LIF number
    roce_qtype		: 3;	// Destination LIF type (within the LIF)
    roce_qid		: 24;	// Destination queue number (within the LIF)
    roce_qaddr		: 34;	// Destination queue state address
    src_queue_id	: 32;	// ROCE source queue id
    ssd_handle		: 16;	// SSD handle to select NVME backend
    io_priority		: 8;	// I/O priority to select ring with the queue
    pad			: 168;
  }
}

// IO context entry
header_type io_ctx_entry_t {
  fields {
    oper_status		: 8;	// Free/InUse/TimedOut/Punt2Arm/Error etc
    iob_addr		: 34;	// Base address of the I/O buffer
    nvme_data_len	: 32;	// Data length
    is_read		: 8;	// Whether it is read command
    is_remote		: 8;	// Whether destination is remote
    nvme_sq_qaddr	: 34;	// NVME VF SQ address for which this IOB was used
  }
}

// IO buffer address
header_type iob_addr_t {
  fields {
    iob_addr		: 34;	// Base address of the I/O buffer
  }
}

// Scratch metadata used only in P4 (mirrored in registers)
header_type nvme_scratch_t {
  fields {
    data_len_xferred	: 32;	// Data length that is transferred
  }
}

// Common header format used with R2N implementation on the target side
header_type r2n_wqe_t {
  fields {
    // Note: These fields are set in the WQE from PVM and the WQE from ROCE
    handle		: 64;	// Pointer to R2N buffer (or) NVME BE command 
    data_size		: 32;	// Size pointed to by handle
    opcode		: 16;	// Each use case has a distinct opcode
    status		: 16;	// Success/failure status
  }
}

// PRP list holding upto 8 entries (64 bytes)
header_type nvme_prp_list_t {
  fields {
    entry0		: 64;
    entry1		: 64;
    entry2		: 64;
    entry3		: 64;
    entry4		: 64;
    entry5		: 64;
    entry6		: 64;
    entry7		: 64;
  }
}

// Storage K+I vectors

// header union with stage_2_stage for table 0 and table 1
header_type nvme_kivec_s2s_t {
  fields {
    w_ndx		: 16;	// Working consumer index
    dst_lif		: 11;	// Destination LIF number
    dst_qtype		: 3;	// Destination LIF type (within the LIF)
    dst_qid		: 24;	// Destination queue number (within the LIF)
    dst_qaddr		: 34;	// Destination queue state address
    iob_addr		: 34;	// Address of the allocated I/O buffer
    io_map_base_addr	: 34;	// IO map table base address (entry = base + (nsid * size))
    prp_assist		: 1;	// Whether PRP list needs to be downloaded
    is_remote		: 1;	// Is the command from local or remote PVM	
    is_read		: 1;	// If NVME command is a read
    punt_to_arm		: 1;	// If destination is ARM
  }
}

// header union with global
header_type nvme_kivec_global_t {
  fields {
    src_lif		: 11;	// Source LIF number
    src_qtype		: 3;	// Source LIF type (within the LIF)
    src_qid		: 24;	// Source queue number (within the LIF)
    src_qaddr		: 34;	// Source queue state address
    nvme_data_len	: 32;	// Data length as indicated in the NVME command
    io_map_num_entries	: 16;	// Number of entries in the IO map table
    oper_status		: 8;	// Free/InUse/TimedOut/Punt2Arm/Error etc
  }
}

// header union with to_stage_3
header_type nvme_kivec_sq_info_t {
  fields {
    is_admin_q		: 1;	// Whether the SQ is an admin queue
  }
}

// header union with to_stage_3 and to_stage_4
header_type nvme_kivec_iob_ring_t {
  fields {
    base_addr	: 34;		// IOB ring base address (free list of IOBs)
  }
}

// header union with to_stage_5
header_type nvme_kivec_prp_base_t {
  fields {
    prp0		: 64;	// PRP0 from NVME command
    prp1		: 64;	// PRP1 from NVME command
  }
}

// header union with to_stage_6 and to_stage_7
header_type nvme_kivec_arm_dst_t {
  fields {
    arm_lif		: 11;	// ARM Q LIF number
    arm_qtype		: 3;	// ARM Q LIF type (within the LIF)
    arm_qid		: 24;	// ARM Q queue number (within the LIF)
    arm_qaddr		: 34;	// ARM Q queue state address
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

#define Q_STATE_COPY(q_state)				\
  Q_STATE_COPY_STAGE0(q_state)				\
  modify_field(q_state.pad, pad);			\

#define NVME_SQ_STATE_COPY_STAGE0(q_state)			\
  Q_STATE_COPY_INTRINSIC(q_state)				\
  modify_field(q_state.p_ndx, p_ndx);				\
  modify_field(q_state.c_ndx, c_ndx);				\
  modify_field(q_state.w_ndx, w_ndx);				\
  modify_field(q_state.num_entries, num_entries);		\
  modify_field(q_state.base_addr, base_addr);			\
  modify_field(q_state.entry_size, entry_size);			\
  modify_field(q_state.next_pc, next_pc);			\
  modify_field(q_state.vf_id, vf_id);				\
  modify_field(q_state.sq_id, sq_id);				\
  modify_field(q_state.cq_lif, cq_lif);				\
  modify_field(q_state.cq_qtype, cq_qtype);			\
  modify_field(q_state.cq_qid, cq_qid);				\
  modify_field(q_state.cq_qaddr, cq_qaddr);			\
  modify_field(q_state.arm_lif, arm_lif);			\
  modify_field(q_state.arm_qtype, arm_qtype);			\
  modify_field(q_state.arm_base_qid, arm_base_qid);		\
  modify_field(q_state.arm_base_qaddr, arm_base_qaddr);		\
  modify_field(q_state.io_map_base_addr, io_map_base_addr);	\
  modify_field(q_state.io_map_num_entries, io_map_num_entries);	\
  modify_field(q_state.iob_ring_base_addr, iob_ring_base_addr);	\

#define NVME_SQ_STATE_COPY(q_state)			\
  NVME_SQ_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pad, pad);			\

#define NVME_CQ_STATE_COPY_STAGE0(q_state)		\
  Q_STATE_COPY_INTRINSIC(q_state)			\
  modify_field(q_state.p_ndx, p_ndx);			\
  modify_field(q_state.c_ndx, c_ndx);			\
  modify_field(q_state.w_ndx, w_ndx);			\
  modify_field(q_state.num_entries, num_entries);	\
  modify_field(q_state.base_addr, base_addr);		\
  modify_field(q_state.entry_size, entry_size);		\
  modify_field(q_state.next_pc, next_pc);		\
  modify_field(q_state.intr_addr, intr_addr);		\
  modify_field(q_state.intr_data, intr_data);		\
  modify_field(q_state.intr_en, intr_en);		\
  modify_field(q_state.phase, phase);			\

#define NVME_CQ_STATE_COPY(q_state)			\
  NVME_CQ_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pad, pad);			\


#define ARM_Q_STATE_COPY_STAGE0(q_state)		\
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
  modify_field(q_state.intr_addr, intr_addr);		\
  modify_field(q_state.intr_data, intr_data);		\
  modify_field(q_state.intr_en, intr_en);		\
  modify_field(q_state.phase, phase);			\
  modify_field(q_state.iob_ring_base_addr,		\
               iob_ring_base_addr);			\

#define ARM_Q_STATE_COPY(q_state)			\
  ARM_Q_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pad, pad);			\


#define RING_STATE_COPY_STAGE0(ring_state)		\
  modify_field(ring_state.p_ndx, p_ndx);		\
  modify_field(ring_state.c_ndx, c_ndx);		\
  modify_field(ring_state.wp_ndx, wp_ndx);		\
  modify_field(ring_state.num_entries, num_entries);	\
  modify_field(ring_state.base_addr, base_addr);	\
  modify_field(ring_state.entry_size, entry_size);	\

#define RING_STATE_COPY(ring)				\
  RING_STATE_COPY_STAGE0(ring)				\
  modify_field(ring.pad, pad);				\


#define SEQ_DB_INFO_COPY(entry)				\
  modify_field(entry.lif, lif);				\
  modify_field(entry.qtype, qtype);			\
  modify_field(entry.qid, qid);				\
  modify_field(entry.qaddr, qaddr);			\

#define IO_MAP_ENTRY_COPY(entry)			\
  modify_field(entry.entry_addr, entry_addr);		\
  modify_field(entry.nsid, nsid);			\
  modify_field(entry.vf_id, vf_id);			\
  modify_field(entry.dst_flags, dst_flags);		\
  modify_field(entry.r2n_lif, r2n_lif);			\
  modify_field(entry.r2n_qtype, r2n_qtype);		\
  modify_field(entry.r2n_qid, r2n_qid);			\
  modify_field(entry.r2n_qaddr, r2n_qaddr);		\
  modify_field(entry.roce_lif, roce_lif);		\
  modify_field(entry.roce_qtype, roce_qtype);		\
  modify_field(entry.roce_qid, roce_qid);		\
  modify_field(entry.roce_qaddr, roce_qaddr);		\
  modify_field(entry.src_queue_id, src_queue_id);	\
  modify_field(entry.ssd_handle, ssd_handle);		\
  modify_field(entry.io_priority, io_priority);		\
  modify_field(entry.pad, pad);				\

#define IO_CTX_ENTRY_COPY(entry)			\
  modify_field(entry.iob_addr, iob_addr);		\
  modify_field(entry.nvme_data_len, nvme_data_len);	\
  modify_field(entry.oper_status, oper_status);		\
  modify_field(entry.is_read, is_read);			\
  modify_field(entry.is_remote, is_remote);		\
  modify_field(entry.nvme_sq_qaddr, nvme_sq_qaddr);	\

#define IOB_ADDR_COPY(entry)				\
  modify_field(entry.iob_addr, iob_addr);		\

#define NVME_PRP_LIST_COPY(list)			\
  modify_field(list.entry0, entry0);			\
  modify_field(list.entry1, entry1);			\
  modify_field(list.entry2, entry2);			\
  modify_field(list.entry3, entry3);			\
  modify_field(list.entry4, entry4);			\
  modify_field(list.entry5, entry5);			\
  modify_field(list.entry6, entry6);			\
  modify_field(list.entry7, entry7);			\

#define R2N_WQE_BASE_COPY(wqe)				\
  modify_field(wqe.handle, handle);			\
  modify_field(wqe.data_size, data_size);		\
  modify_field(wqe.opcode, opcode);			\
  modify_field(wqe.status, status);			\

#define NVME_KIVEC_S2S_USE(scratch, kivec)				\
  modify_field(scratch.w_ndx, kivec.w_ndx);				\
  modify_field(scratch.dst_lif, kivec.dst_lif);				\
  modify_field(scratch.dst_qtype, kivec.dst_qtype);			\
  modify_field(scratch.dst_qid, kivec.dst_qid);				\
  modify_field(scratch.dst_qaddr, kivec.dst_qaddr);			\
  modify_field(scratch.iob_addr, kivec.iob_addr);		\
  modify_field(scratch.io_map_base_addr, kivec.io_map_base_addr);	\
  modify_field(scratch.prp_assist, kivec.prp_assist);			\
  modify_field(scratch.is_remote, kivec.is_remote);			\
  modify_field(scratch.is_read, kivec.is_read);				\
  modify_field(scratch.punt_to_arm, kivec.punt_to_arm);			\

#define NVME_KIVEC_GLOBAL_USE(scratch, kivec)				\
  modify_field(scratch.src_lif, kivec.src_lif);				\
  modify_field(scratch.src_qtype, kivec.src_qtype);			\
  modify_field(scratch.src_qid, kivec.src_qid);				\
  modify_field(scratch.src_qaddr, kivec.src_qaddr);			\
  modify_field(scratch.nvme_data_len, kivec.nvme_data_len);		\
  modify_field(scratch.io_map_num_entries, kivec.io_map_num_entries);	\
  modify_field(scratch.oper_status, kivec.oper_status);			\

#define NVME_KIVEC_SQ_INFO_USE(scratch, kivec)				\
  modify_field(scratch.is_admin_q, kivec.is_admin_q);			\

#define NVME_KIVEC_PRP_BASE_USE(scratch, kivec)				\
  modify_field(scratch.prp0, kivec.prp0);				\
  modify_field(scratch.prp1, kivec.prp1);				\

#define NVME_KIVEC_ARM_DST_USE(scratch, kivec)				\
  modify_field(scratch.arm_lif, kivec.arm_lif);				\
  modify_field(scratch.arm_qtype, kivec.arm_qtype);			\
  modify_field(scratch.arm_qid, kivec.arm_qid);				\
  modify_field(scratch.arm_qaddr, kivec.arm_qaddr);			\

#define NVME_KIVEC_IOB_RING_USE(scratch, kivec)				\
  modify_field(scratch.base_addr, kivec.base_addr);	                \

// PRP entry based data xfer marcos from host (for write command)
// TODO: FIXME: In ASM, use min(remaining_len, PRP_DATA_XFER_SIZE)
#define NVME_DATA_XFER_FROM_HOST(s2s_kivec, global_kivec, src_dma_cmd,	\
                                 dst_dma_cmd, prp_entry, scratch)	\
  if ((s2s_kivec.punt_to_arm == 0) and					\
      (s2s_kivec.is_read == 0) and					\
      (prp_entry != 0) and						\
      (scratch.data_len_xferred < global_kivec.nvme_data_len)) {	\
    DMA_COMMAND_MEM2MEM_FILL(src_dma_cmd, dst_dma_cmd, prp_entry,	\
                             s2s_kivec.iob_addr +			\
                             IO_BUF_DATA_OFFSET +			\
                             scratch.data_len_xferred,			\
                             0, 0, PRP_DATA_XFER_SIZE, 0, 0, 0)		\
    modify_field(nvme_scratch.data_len_xferred,				\
                 nvme_scratch.data_len_xferred + PRP_DATA_XFER_SIZE);	\
  }									\

// PRP entry based data xfer marcos to host (for read command)
// TODO: FIXME: In ASM, use min(scratch.data_len_xferred, PRP_DATA_XFER_SIZE)
#define NVME_DATA_XFER_TO_HOST(s2s_kivec, global_kivec, src_dma_cmd,	\
                               dst_dma_cmd, prp_entry, scratch)		\
  if ((global_kivec.oper_status == IO_CTX_OPER_STATUS_COMPLETED) and	\
      (s2s_kivec.is_read == 1) and					\
      (prp_entry != 0) and						\
      (scratch.data_len_xferred < global_kivec.nvme_data_len)) {	\
    DMA_COMMAND_MEM2MEM_FILL(src_dma_cmd, dst_dma_cmd, 			\
                             s2s_kivec.iob_addr +			\
                             IO_BUF_DATA_OFFSET +			\
                             scratch.data_len_xferred,			\
                             prp_entry,					\
                             0, 0, PRP_DATA_XFER_SIZE, 0, 0, 0)		\
    modify_field(nvme_scratch.data_len_xferred,				\
                 nvme_scratch.data_len_xferred + PRP_DATA_XFER_SIZE);	\
  }									\


// Dummy function pointers for P4
#define allocate_iob_start		0x81000000
#define pop_sq_start			0x81010000
#define handle_cmd_start		0x81020000
#define send_cmd_free_iob_start		0x81030000
#define process_io_map_start		0x81040000
#define handle_no_prp_list_start	0x81050000
#define handle_prp_list0_start		0x81060000
#define handle_prp_list1_start		0x81070000
#define save_io_ctx_start		0x81080000
#define process_dst_seq_start		0x81090000
#define push_arm_q_start		0x810A0000
#define push_dst_seq_q_start		0x810B0000
#define handle_r2n_wqe_start		0x810C0000
#define process_be_status_start		0x810D0000
#define process_io_ctx_start		0x810E0000
#define send_read_data_start		0x810F0000
#define lookup_sq_start			0x81100000
#define push_cq_start			0x81110000
#define send_sta_free_iob_start		0x81120000
#define cleanup_iob_start		0x81130000
#define cleanup_io_ctx_start		0x81140000
#define free_iob_start			0x81150000
#define timeout_iob_skip_start		0x81160000
#define timeout_io_ctx_start		0x81170000
#define save_iob_addr_start		0x81180000


#endif     // STORAGE_NVME_P4_HDR_H
