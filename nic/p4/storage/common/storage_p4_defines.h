/*****************************************************************************
 * storage_p4_defines.h: Storage defintions needed in P4
 *****************************************************************************/
 
#ifndef STORAGE_P4_DEFINES_H
#define STORAGE_P4_DEFINES_H

#include "storage_common_defines.h"

// Macro to return the (byte) offset of a field within a PHV. Dummy in the
// P4 land, implement this in ASM.
#define PHV_FIELD_OFFSET(field)		0

// Load a table based on fixed base, index and fixed size
// Size * index is used to determine offset beyond table base
#define CAPRI_LOAD_TABLE_IDX(i, _table_base, _idx, _entry_size, 	\
                             _load_size, _pc)				\
  modify_field(i.table_pc, _pc);					\
  modify_field(i.table_addr, _table_base + (_idx * _entry_size));	\
  modify_field(i.table_raw_table_size, _load_size);			\

// Load a table based on fixed base, index and fixed size
// Size * index is used to determine offset beyond table base
#define CAPRI_LOAD_TABLE_ADDR(i, _table_addr, _load_size, _pc)		\
  modify_field(i.table_pc, _pc);					\
  modify_field(i.table_addr, _table_addr); 				\
  modify_field(i.table_raw_table_size, _load_size);			\

// Load a table based on fixed base, index, priority, number of entries
// and fixed size.  Each priority queue starts at offset 
// (size * pri * num_entries) beyond table base.  Size * index is used 
// to determine offset beyond the start of each priority queue.
#define CAPRI_LOAD_TABLE_PRI_IDX(i, _table_base, _idx, _num_entries,	\
                                 _pri, _entry_size, _load_size, _pc)	\
  modify_field(i.table_pc, _pc);					\
  modify_field(i.table_addr, _table_base + 				\
                         _entry_size * ((_num_entries * _pri) + _idx));	\
  modify_field(i.table_raw_table_size, _load_size);			\


// Doorbell update macros
#define STORAGE_DOORBELL_ADDRESS(qtype, lif, sched_wr, upd)	\
	(DOORBELL_ADDR_WA_LOCAL_BASE | 				\
	 qtype << DOORBELL_ADDR_QTYPE_SHIFT |			\
	 lif << DOORBELL_ADDR_LIF_SHIFT |			\
	 sched_wr << DOORBELL_ADDR_SCHED_WR_SHIFT |		\
	 upd << DOORBELL_ADDR_UPD_SHIFT)			\

#define STORAGE_DOORBELL_DATA(index, ring, qid, pid)		\
	(index | 						\
	 ring << DOORBELL_DATA_RING_SHIFT |			\
	 qid << DOORBELL_DATA_QID_SHIFT |			\
	 pid << DOORBELL_DATA_PID_SHIFT)			\

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
    dst_qaddr	: 34;	// Destination queue state address
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
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
    dst_qaddr	: 34;	// Destination queue state address
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
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
    intr_en	: 1;	// If interrupts are enabled 
    pad		: 143;	// Align to 64 bytes
  }
}

// Barco XTS ring 
header_type barco_xts_ring_t {
  fields {
    base_addr	: 64;	// Base address of queue entries
    num_entries	: 32;	// Number of entries in the barco ring
    p_ndx	: 32;	// Producer Index
    opa_tag	: 64;	// Operation tag
    c_ndx	: 32;	// Consumer Index
    status	: 32;	// Status from barco
  }
}

#define Q_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pc_offset, pc_offset);		\
  modify_field(q_state.rsvd, rsvd);			\
  modify_field(q_state.cosA, cosA);			\
  modify_field(q_state.cosB, cosB);			\
  modify_field(q_state.cos_sel, cos_sel);		\
  modify_field(q_state.eval_last, eval_last);		\
  modify_field(q_state.total_rings, total_rings);	\
  modify_field(q_state.host_rings, host_rings);		\
  modify_field(q_state.pid, pid);			\
  modify_field(q_state.p_ndx, p_ndx);			\
  modify_field(q_state.c_ndx, c_ndx);			\
  modify_field(q_state.w_ndx, w_ndx);			\
  modify_field(q_state.num_entries, num_entries);	\
  modify_field(q_state.base_addr, base_addr);		\
  modify_field(q_state.entry_size, entry_size);		\
  modify_field(q_state.next_pc, next_pc);		\
  modify_field(q_state.dst_qaddr, dst_qaddr);		\
  modify_field(q_state.dst_lif, dst_lif);		\
  modify_field(q_state.dst_qtype, dst_qtype);		\
  modify_field(q_state.dst_qid, dst_qid);		\
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
  modify_field(q_state.pc_offset, pc_offset);		\
  modify_field(q_state.rsvd, rsvd);			\
  modify_field(q_state.cosA, cosA);			\
  modify_field(q_state.cosB, cosB);			\
  modify_field(q_state.cos_sel, cos_sel);		\
  modify_field(q_state.eval_last, eval_last);		\
  modify_field(q_state.total_rings, total_rings);	\
  modify_field(q_state.host_rings, host_rings);		\
  modify_field(q_state.pid, pid);			\
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
  modify_field(q_state.dst_qaddr, dst_qaddr);		\
  modify_field(q_state.dst_lif, dst_lif);		\
  modify_field(q_state.dst_qtype, dst_qtype);		\
  modify_field(q_state.dst_qid, dst_qid);		\
  modify_field(q_state.ssd_bm_addr, ssd_bm_addr);	\

#define PRI_Q_STATE_COPY(q_state)			\
  PRI_Q_STATE_COPY_STAGE0(q_state)			\
  modify_field(q_state.pad, pad);			\

#define PCI_Q_STATE_COPY_STAGE0(q_state)		\
  modify_field(q_state.pc_offset, pc_offset);		\
  modify_field(q_state.rsvd, rsvd);			\
  modify_field(q_state.cosA, cosA);			\
  modify_field(q_state.cosB, cosB);			\
  modify_field(q_state.cos_sel, cos_sel);		\
  modify_field(q_state.eval_last, eval_last);		\
  modify_field(q_state.total_rings, total_rings);	\
  modify_field(q_state.host_rings, host_rings);		\
  modify_field(q_state.pid, pid);			\
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

// Queue empty macros
#define _QUEUE_EMPTY(_p_ndx, _c_ndx)	(_p_ndx == _c_ndx)
#define QUEUE_EMPTY(q)	_QUEUE_EMPTY(q.p_ndx, q.c_ndx)

// Queue full macros
#define _QUEUE_FULL(_c_ndx, _p_ndx, _num_entries, _increment)	\
	(_c_ndx == ((_p_ndx + _increment) % _num_entries))
#define QUEUE_FULL(q)						\
	_QUEUE_FULL(q.p_ndx, q.c_ndx, q.num_entries, 1)

// Queue pop macros
#define _QUEUE_POP(_w_ndx, _num_entries)			\
	modify_field(_w_ndx, (_w_ndx + 1) % _num_entries);
#define QUEUE_POP(q)		_QUEUE_POP(q.w_ndx, q.num_entries)

// Queue push macros
#define _QUEUE_PUSH(_p_ndx, _num_entries)			\
  modify_field(_p_ndx, (_p_ndx + 1) % _num_entries);
#define QUEUE_PUSH(q)	_QUEUE_PUSH(q.p_ndx, q.num_entries)

// Priority queue check macros
#define PRI_QUEUE_CAN_POP_HI(q)					\
  ((not(_QUEUE_EMPTY(q.p_ndx_hi, q.c_ndx_hi))) and		\
   (q.hi_running < q.hi_weight))
#define PRI_QUEUE_CAN_POP_MED(q)				\
  ((not(_QUEUE_EMPTY(q.p_ndx_med, q.c_ndx_med))) and		\
   (q.med_running < q.med_weight))
#define PRI_QUEUE_CAN_POP_LO(q)					\
  ((not(_QUEUE_EMPTY(q.p_ndx_lo, q.c_ndx_lo))) and		\
   (q.lo_running < q.lo_weight))

// Service priority queue
#define SERVICE_PRI_QUEUE(_kivec, _q, _i, _w_ndx_pri, _pri_val)	\
    _QUEUE_POP(_w_ndx_pri, _q.num_entries)			\
    modify_field(_kivec.w_ndx, _w_ndx_pri);			\
    modify_field(_kivec.io_priority, _pri_val);			\
    CAPRI_LOAD_TABLE_PRI_IDX(_i, _q.base_addr, _q.w_ndx_hi,	\
                             _q.num_entries, _pri_val,		\
                             _q.entry_size, _q.entry_size,	\
                             _q.next_pc)			\

// Check if the correct priority queue is full
#define PRI_QUEUE_FULL(q, _pri)					\
   (((_pri == NVME_BE_PRIORITY_HI) and				\
     _QUEUE_FULL(q.c_ndx_hi, q.p_ndx_hi, q.num_entries, 1)) or	\
    ((_pri == NVME_BE_PRIORITY_MED) and				\
     _QUEUE_FULL(q.c_ndx_med, q.p_ndx_med, q.num_entries, 1)) or\
    ((_pri == NVME_BE_PRIORITY_LO) and				\
     _QUEUE_FULL(q.c_ndx_lo, q.p_ndx_lo, q.num_entries, 1)))

// Push to the  priority queue
#define PRI_QUEUE_PUSH(_q, _p_ndx_pri, _pri_val)		\
    DMA_COMMAND_PHV2MEM_FILL(dma_p2m_1,				\
                             _q.base_addr +			\
                             (_pri_val * _q.entry_size * 	\
                              _q.num_entries) +			\
                             (_p_ndx_pri * _q.entry_size),	\
                             0, 0, 0, 0, 0, 0)			\
    _QUEUE_PUSH(_p_ndx_pri, _q.num_entries)

// Queue pop doorbell update
// Form the doorbell and setup the DMA command to push the entry by
// incrementing p_ndx. In ASM use the correct index, ring, queue
// when constructing the data.
#define QUEUE_POP_DOORBELL_UPDATE				\
  modify_field(doorbell_addr.addr,				\
        STORAGE_DOORBELL_ADDRESS(storage_kivec1.src_qtype,	\
                                 storage_kivec1.src_lif,	\
                                 DOORBELL_SCHED_WR_RESET, 	\
                                 DOORBELL_UPDATE_C_NDX));	\
  modify_field(qpop_doorbell_data.data,				\
        STORAGE_DOORBELL_DATA(storage_kivec0.w_ndx, 0, 0, 0));	\
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_0, 0,			\
        PHV_FIELD_OFFSET(qpop_doorbell_data.data),		\
        PHV_FIELD_OFFSET(qpop_doorbell_data.data),		\
        0, 0, 0, 0)

// Queue push doorbell update
// Form the doorbell and setup the DMA command to push the entry by
// incrementing p_ndx. In ASM use the correct index, ring, queue
// when constructing the data.
#define QUEUE_PUSH_DOORBELL_UPDATE				\
  modify_field(doorbell_addr.addr,				\
        STORAGE_DOORBELL_ADDRESS(storage_kivec0.dst_qtype,	\
                                 storage_kivec0.dst_lif,	\
                                 DOORBELL_SCHED_WR_RESET,	\
                                 DOORBELL_UPDATE_P_NDX_INCR));	\
  modify_field(qpop_doorbell_data.data,				\
        STORAGE_DOORBELL_DATA(storage_kivec0.w_ndx, 0, 0, 0));	\
  DMA_COMMAND_PHV2MEM_FILL(dma_p2m_2, 0,			\
        PHV_FIELD_OFFSET(qpush_doorbell_data.data),		\
        PHV_FIELD_OFFSET(qpush_doorbell_data.data),		\
        0, 0, 0, 0)


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
  }
}

// kivec1: header union with global
header_type storage_kivec1_t {
  fields {
    src_lif	: 11;	// Source LIF number
    src_qtype	: 3;	// Source LIF type (within the LIF)
    src_qid	: 24;	// Source queue number (within the LIF)
    src_qaddr	: 34;	// Source queue state address
    xts_desc_size	: 16;	// Barco XTS descriptor size
    ssd_ci_addr	: 34;	// Address of the consumer index in the SSD qstate
  }
}

// kivec2: header union with to_stage_2
header_type storage_kivec2_t {
  fields {
    ssd_q_num	: 16;	// Number of entries in the SSD priority queue
    ssd_q_size	: 16;	// Size of each queue state entry in SSD priority queue
  }
}

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

#define STORAGE_KIVEC1_USE(scratch, kivec)				\
  modify_field(scratch.src_lif, kivec.src_lif);				\
  modify_field(scratch.src_qtype, kivec.src_qtype);			\
  modify_field(scratch.src_qid, kivec.src_qid);				\
  modify_field(scratch.src_qaddr, kivec.src_qaddr);			\
  modify_field(scratch.xts_desc_size, kivec.xts_desc_size);		\
  modify_field(scratch.ssd_ci_addr, kivec.ssd_ci_addr);			\

#define STORAGE_KIVEC2_USE(scratch, kivec)				\
  modify_field(scratch.ssd_q_num, kivec.ssd_q_num);			\
  modify_field(scratch.ssd_q_size, kivec.ssd_q_size);			\

// PRP size calculation
#define PRP_SIZE(p)				(PRP_SIZE_SUB - (p & PRP_SIZE_MASK))

// LB size calculation
#define LB_SIZE(nlb)				(nlb << LB_SIZE_SHIFT)

// NVME command definition
header_type nvme_cmd_t {
  fields {
    // NVME command Dword 0
    opc		: 8;	// Opcode
    fuse	: 2;	// Fusing 2 simple commands
    rsvd0	: 4; 
    psdt	: 2;	// PRP or SGL
    cid		: 16;	// Command identifier
  
    // NVME command Dword 1
    nsid	: 32;	// Namespace identifier

    // NVME command Dword 2
    rsvd2	: 32;

    // NVME command Dword 3
    rsvd3	: 32;

    // NVME command Dwords 4 and 5 
    mptr	: 64;	// Metadata pointer

    // NVME command Dwords 6,7,8 & 9 form the data pointer (PRP or SGL)
    dptr1	: 64;	// PRP1 or address of SGL
    dptr2	: 64;	// PRP2 or size/type/sub_type in SGL

    // NVME command Dwords 10 and 11 
    slba	: 64;	// Starting LBA (for Read/Write) commands

    // NVME command Dword 12
    nlb		: 16;	// Number of logical blocks
    rsvd12	: 10;	
    prinfo	: 4;	// Protection information field
    fua		: 1;	// Force unit access
    lr		: 1;	// Limited retry
 
    // NVME command Dword 13
    dsm		: 8;	// Dataset management
    rsvd13	: 24;

    // NVME command Dword 14
    dw14	: 32;

    // NVME command Dword 15
    dw15	: 32;
  }
}

// NVME status definition
header_type nvme_sta_t {
  fields {
    // NVME status Dword 0
    cspec	: 32;	// Command specific

    // NVME status Dword 1
    rsvd	: 32;

    // NVME status Dword 2
    sq_head	: 16;	// Submission queue head pointer
    sq_id	: 16;	// Submission queue identifier
	
    // NVME status Dword 3
    cid		: 16;	// Command identifier
    phase	: 1;	// Phase bit
    status	: 15;	// Status
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

// NVME backend command header carried over ROCE
header_type nvme_be_cmd_hdr_t {
  fields {
    src_queue_id	: 32;	// ROCE source queue id
    ssd_handle		: 16;	// SSD handle to select NVME backend
    io_priority		: 8;	// I/O priority to select ring with the queue
    is_read		: 8;	// If NVME command is a read
    r2n_buf_handle	: 64;	// Back pointer to the R2N buffer
    is_local		: 8;	// Is the command from local or remove PVM	
  }
}

// NVME backend status header carried over ROCE
header_type nvme_be_sta_hdr_t {
  fields {
    time_us		: 32;	// Timestamp in usec
    be_status		: 8;	// Backend status
    is_q0		: 8;	// Is queue 0 (admin queue)
    rsvd		: 16;	// Padding for 64 bit alignment
    r2n_buf_handle	: 64;	// Back pointer to the R2N buffer
  }
}

// NVME backend PRP list. TODO: Split into fields once finalized
header_type nvme_be_prp_list_t {
  fields {
    prp_list		: 1024;
  }
}

// WQE format of ROCE SQ. TODO: Split into fields once finalized
header_type roce_sq_wqe_t {
  fields {
    data		: 512;	// Timestamp in usec
  }
}

// WQE format of ROCE RQ. TODO: Split into fields once finalized
header_type roce_rq_wqe_t {
  fields {
    data		: 512;	// Timestamp in usec
  }
}

// WQE format of R2N layer. Used by:
// 1. R2N's SQ/CQ with local PVM for local target.
// 2. NVME backend's priority SQ/CQ with R2N.
// 3. R2N's HQ with local PVM for ROCE buf posting
header_type r2n_wqe_t {
  fields {
    // Note: These fields are provided in the WQE from PVM
    handle		: 64;	// Pointer to R2N buffer (or) NVME BE command 
    data_size		: 32;	// Size pointed to by handle
    opcode		: 16;	// Each use case has a distinct opcode
    status		: 16;	// Success/failure status

    // Note: These fields are not part of the WQE from PVM
    src_queue_id	: 32;	// ROCE source queue id
    ssd_handle		: 16;	// SSD handle to select NVME backend
    io_priority		: 8;	// I/O priority to select ring with the queue
    is_read		: 8;	// If NVME command is a read
    r2n_buf_handle	: 64;	// Back pointer to the R2N buffer
    is_local		: 8;	// Is the command from local or remove PVM	
    nvme_cmd_cid	: 16;	// Command identifier
    pri_qaddr		: 34;	// Priority queue state address for status

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


// Pads for storage, vary this based on PHV allocation to align DMA commands
// to 16 byte boundary, not to have NCC generated pads in DMA regions etc.
header_type storage_pad0_t {
  fields {
    pad		: 88;	
  }
}
header_type storage_pad1_t {
  fields {
    pad		: 48;	
  }
}

// Capri doorbell address, data definitions and macros to manipulate them
header_type storage_doorbell_addr_t {
  fields {
    addr	: 64;	// 64 bit address
  }
}

header_type storage_doorbell_data_t {
  fields {
    data	: 64;	// 64 bit data
  }
}

// PCI push/interrupt data defintion 
header_type storage_pci_data_t  {
  fields {
    data	: 32;	// 32 bit data
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
  }
}

// Sequencer metadata for pushing R2N WQE
header_type seq_r2n_entry_t {
  fields {
    r2n_wqe_addr	: 64;	// Address of the R2N WQE to push
    r2n_wqe_size	: 32;	// Size of the R2N WQE to push
    dst_lif		: 11;	// Destination LIF number
    dst_qtype		: 3;	// Destination LIF type (within the LIF)
    dst_qid		: 24;	// Destination queue number (within the LIF)
    dst_qaddr		: 34;	// Destination queue state address
  }
}

// Sequencer metadata for pushing Barco XTS descriptor
header_type seq_barco_entry_t {
  fields {
    xts_desc_addr	: 64;	// Address of the XTS descriptor to push
    xts_desc_size	: 32;	// Size of the XTS descriptor to push
    xts_db_addr		: 64;	// 64 bit address of the XTS doorbell to ring
    xts_db_data		: 64;	// 64 bit data of the XTS doorbell to ring
    xts_ring_load_size	: 16;	// Size of the XTS ring state to be loaded
    xts_ring_addr	: 34;	// Address of the XTS ring
  }
}


  // Carry forward NVME command information to be sent to PVM in the PHV 
#define NVME_CMD_COPY(cmd)			\
  modify_field(cmd.opc, opc);			\
  modify_field(cmd.fuse, fuse);			\
  modify_field(cmd.rsvd0, rsvd0);		\
  modify_field(cmd.psdt, psdt);			\
  modify_field(cmd.cid, cid);			\
  modify_field(cmd.nsid, nsid);			\
  modify_field(cmd.rsvd2, rsvd2);		\
  modify_field(cmd.rsvd3, rsvd3);		\
  modify_field(cmd.mptr, mptr);			\
  modify_field(cmd.dptr1, dptr1);		\
  modify_field(cmd.dptr2, dptr2);		\
  modify_field(cmd.slba, slba);			\
  modify_field(cmd.nlb, nlb);			\
  modify_field(cmd.rsvd12, rsvd12);		\
  modify_field(cmd.prinfo, prinfo);		\
  modify_field(cmd.fua, fua);			\
  modify_field(cmd.lr, lr);			\
  modify_field(cmd.dsm, dsm);			\
  modify_field(cmd.rsvd13, rsvd13);		\
  modify_field(cmd.dw14, dw14);			\
  modify_field(cmd.dw15, dw15);			\

#define NVME_STATUS_COPY(sta)			\
  modify_field(sta.cspec, cspec);		\
  modify_field(sta.rsvd, rsvd);			\
  modify_field(sta.sq_head, sq_head);		\
  modify_field(sta.sq_id, sq_id);		\
  modify_field(sta.cid, cid);			\
  modify_field(sta.phase, phase);		\
  modify_field(sta.status, status);		\

// Copy the basic part of R2N WQE - the one sent by PVM
#define R2N_WQE_BASE_COPY(wqe)					\
  modify_field(wqe.handle, handle);				\
  modify_field(wqe.data_size, data_size);			\
  modify_field(wqe.opcode, opcode);				\
  modify_field(wqe.status, status);

// Copy the NVME backend command header - also present in R2N WQE
#define NVME_BE_CMD_HDR_COPY(hdr)				\
  modify_field(hdr.src_queue_id, src_queue_id);			\
  modify_field(hdr.ssd_handle, ssd_handle);			\
  modify_field(hdr.io_priority, io_priority);			\
  modify_field(hdr.is_read, is_read);				\
  modify_field(hdr.r2n_buf_handle, r2n_buf_handle);		\
  modify_field(hdr.is_local, is_local);				\

// Copy the full R2N WQE - base from PVM + NVME backend header +
// additional fields saved before sending the NVME command to SSD
#define R2N_WQE_FULL_COPY(wqe)					\
  R2N_WQE_BASE_COPY(wqe)					\
  NVME_BE_CMD_HDR_COPY(wqe)					\
  modify_field(wqe.nvme_cmd_cid, nvme_cmd_cid);			\
  modify_field(wqe.pri_qaddr, pri_qaddr);

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

#endif     // STORAGE_P4_DEFINES_H
