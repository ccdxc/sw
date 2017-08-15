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
  modify_field(i.table_pc, _pc);						\
  modify_field(i.table_addr, _table_base + (_idx * _entry_size));	\
  modify_field(i.table_raw_table_size, _load_size);			\

// Load a table based on fixed base, index and fixed size
// Size * index is used to determine offset beyond table base
#define CAPRI_LOAD_TABLE_ADDR(i, _table_addr, _load_size, _pc)		\
  modify_field(i.table_pc, _pc);					\
  modify_field(i.table_addr, _table_addr); 				\
  modify_field(i.table_raw_table_size, _load_size);			\

#define pvm_sq_entry_push_start		0x80000000

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
    pad		: 172;	// Align to 64 bytes
  }
}

#define Q_STATE_COPY(q_state)				\
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
#define QUEUE_POP(q)	_QUEUE_POP(q.w_ndx, q.num_entries)

// Queue push macros
#define _QUEUE_PUSH(_p_ndx, _num_entries)			\
  modify_field(_p_ndx, (_p_ndx + 1) % _num_entries);
#define QUEUE_PUSH(q)	_QUEUE_PUSH(q.p_ndx, q.num_entries)

header_type storage_kivec0_t {
   fields {
    w_ndx	: 16;	// Working consumer index
    dst_lif	: 11;	// Destination LIF number
    dst_qtype	: 3;	// Destination LIF type (within the LIF)
    dst_qid	: 24;	// Destination queue number (within the LIF)
    src_qaddr	: 34;	// Source queue state address
    dst_qaddr	: 34;	// Destination queue state address
    prp_assist	: 1;	// Download additional PRP entries (upto 16)
    is_q0	: 1;	// Is queue id 0 ? Used to distinguish admin queue
   }
}

#define STORAGE_KIVEC0_USE(scratch, kivec)				\
  modify_field(scratch.w_ndx, kivec.w_ndx);				\
  modify_field(scratch.dst_lif, kivec.dst_lif);				\
  modify_field(scratch.dst_qtype, kivec.dst_qtype);			\
  modify_field(scratch.dst_qid, kivec.dst_qid);				\
  modify_field(scratch.prp_assist, kivec.prp_assist);			\
  modify_field(scratch.is_q0, kivec.is_q0);				\
  modify_field(scratch.src_qaddr, kivec.src_qaddr);			\
  modify_field(scratch.dst_qaddr, kivec.dst_qaddr);			\

// PRP size calculation
#define PRP_SIZE(p)				(PRP_SIZE_SUB - (p & PRP_SIZE_MASK))

// LB size calculation
#define LB_SIZE(nlb)				(nlb << LB_SIZE_SHIFT)

// NVME command sent to PVM
header_type pvm_cmd_t {
  fields {
    // NVME command Dword 0
    opc          : 8;    // Opcode
    fuse         : 2;    // Fusing 2 simple commands
    rsvd0        : 4; 
    psdt         : 2;    // PRP or SGL
    cid          : 16;   // Command identifier
  
    // NVME command Dword 1
    nsid         : 32;   // Namespace identifier

    // NVME command Dword 2
    rsvd2        : 32;

    // NVME command Dword 3
    rsvd3        : 32;

    // NVME command Dwords 4 and 5 
    mptr         : 64;   // Metadata pointer

    // NVME command Dwords 6,7,8 & 9 form the data pointer (PRP or SGL)
    dptr1        : 64;   // PRP1 or address of SGL
    dptr2        : 64;   // PRP2 or size/type/sub_type in SGL

    // NVME command Dwords 10 and 11 
    slba         : 64;   // Starting LBA (for Read/Write) commands

    // NVME command Dword 12
    nlb          : 16;   // Number of logical blocks
    rsvd12       : 10;   
    prinfo       : 4;    // Protection information field
    fua          : 1;    // Force unit access
    lr           : 1;    // Limited retry
 
    // NVME command Dword 13
    dsm          : 8;    // Dataset management
    rsvd13       : 24;

    // NVME command Dword 14
    dw14         : 32;

    // NVME command Dword 15
    dw15         : 32;

    // Information passed to PVM
    tickreg      : 64;   // Microsecond reg from NIC
    vf_id        : 16;   // VF number
    sq_id        : 16;   // Submission queue id
    num_prps     : 8;    // Number of additional PRPs
  }
}

header_type storage_pad_t {
  fields {
    pad		: 88;	// Align DMA commands to 16 byte to boundary
  }
}

#endif     // STORAGE_P4_DEFINES_H
