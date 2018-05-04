/*****************************************************************************
 * storage_p4_hdr.h: Common header definitions across all storage P4+ programs
 *****************************************************************************/
 
#ifndef STORAGE_P4_HDR_H
#define STORAGE_P4_HDR_H

#include "storage_common_defines.h"

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

// NVME backend command header base + R2N buffer handle
header_type nvme_be_cmd_hdr_t {
  fields {
    src_queue_id	: 32;	// ROCE source queue id
    ssd_handle		: 16;	// SSD handle to select NVME backend
    io_priority		: 8;	// I/O priority to select ring with the queue
    is_read		: 8;	// If NVME command is a read
    r2n_buf_handle	: 64;	// Back pointer to the R2N buffer
  }
}

// NVME backend status header carried over ROCE
header_type nvme_be_sta_hdr_t {
  fields {
    time_us		: 32;	// Timestamp in usec
    be_status		: 8;	// Backend status
    is_q0		: 8;	// Is queue 0 (admin queue)
    be_rsvd		: 16;	// Padding for 64 bit alignment
    r2n_buf_handle	: 64;	// Back pointer to the R2N buffer
  }
}

// NVME backend PRP list. TODO: Split into fields once finalized
header_type nvme_be_prp_list_t {
  fields {
    prp_list		: 1024;
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
    is_remote		: 8;	// Is the command from local or remote PVM	
  }
}

// Capri doorbell address, data definitions and macros to manipulate them
header_type storage_capri_addr_t {
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

// IO buffer address
header_type iob_addr_t {
  fields {
    iob_addr		: 34;	// Base address of the I/O buffer
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
    base_addr	: 32;	// Base address of queue entries
    page_size	: 5;	// Size of each page (power of 2 of this value)
    entry_size	: 5;	// Size of each WQE (power of 2 of this value)
    num_entries	: 5;	// Number of WQE (power of 2 of this value)
    pad		: 209;	// Align to 64 bytes
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
    rrq_base	: 34;	// ROCE RQ queue base address
    post_buf	: 1;	// Whether a buffer needs to be posted back on ACK
    pad		: 115;	// Align to 64 bytes
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
// Copy macros

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

#define NVME_BE_CMD_HDR_COPY(hdr)			\
  modify_field(hdr.src_queue_id, src_queue_id);		\
  modify_field(hdr.ssd_handle, ssd_handle);		\
  modify_field(hdr.io_priority, io_priority);		\
  modify_field(hdr.is_read, is_read);			\
  modify_field(hdr.r2n_buf_handle, r2n_buf_handle);	\

#define NVME_BE_STA_HDR_COPY(hdr)			\
  modify_field(hdr.time_us, time_us);			\
  modify_field(hdr.be_status, be_status);		\
  modify_field(hdr.is_q0, is_q0);			\
  modify_field(hdr.be_rsvd, be_rsvd);			\
  modify_field(hdr.r2n_buf_handle, r2n_buf_handle);	\

#define SEQ_R2N_ENTRY_COPY(entry)			\
  modify_field(entry.r2n_wqe_addr, r2n_wqe_addr);	\
  modify_field(entry.r2n_wqe_size, r2n_wqe_size);	\
  modify_field(entry.dst_lif, dst_lif);			\
  modify_field(entry.dst_qtype, dst_qtype);		\
  modify_field(entry.dst_qid, dst_qid);			\
  modify_field(entry.dst_qaddr, dst_qaddr);		\
  modify_field(entry.is_remote, is_remote);		\

#define IOB_ADDR_COPY(entry)				\
  modify_field(entry.iob_addr, iob_addr);		\

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
  modify_field(sq_cb.rrq_base, rrq_base);		\
  modify_field(sq_cb.post_buf, post_buf);		\

#define PVM_ROCE_SQ_CB_COPY(sq_cb)			\
  PVM_ROCE_SQ_CB_COPY_STAGE0(sq_cb)			\
  modify_field(sq_cb.pad, pad);				\

#define ROCE_RQ_CB_COPY_STAGE0(rq_cb)			\
  Q_STATE_COPY_INTRINSIC(rq_cb)				\
  modify_field(rq_cb.p_ndx, p_ndx);			\
  modify_field(rq_cb.c_ndx, c_ndx);			\
  modify_field(rq_cb.extra_rings, extra_rings);		\
  modify_field(rq_cb.base_addr, base_addr);		\
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

// Macro to return the (byte) offset of a field within a PHV. Dummy in the
// P4 land, implement this in ASM.
#define PHV_FIELD_OFFSET(field)		0

// Load a table based on fixed base, index and fixed size
// Size * index is used to determine offset beyond table base
#define CAPRI_LOAD_TABLE_NO_LKUP(i, _pc)				\
  modify_field(i.table_pc, _pc);					\
  modify_field(i.table_addr, 0);	                                \
  modify_field(i.table_raw_table_size, STORAGE_TBL_LOAD_SIZE_0_BITS);   \
  
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

// Queue empty macros
#define _QUEUE_EMPTY(_p_ndx, _c_ndx)	(_p_ndx == _c_ndx)
#define QUEUE_EMPTY(q)	_QUEUE_EMPTY(q.p_ndx, q.c_ndx)

// Queue full macros
#define _QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, _increment)	\
	(_c_ndx == ((_p_ndx + _increment) % _num_entries))
#define QUEUE_FULL(q)						\
	_QUEUE_FULL(q.p_ndx, q.c_ndx, q.num_entries, 1)
#define QUEUE_FULL2(q)						\
	_QUEUE_FULL(q.p_ndx, q.c_ndx, q.num_entries, 2)

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
  modify_field(doorbell_addr_scratch.addr,			\
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
#define QUEUE_PUSH_DOORBELL_UPDATE(_lif, _qtype, _qid, _ring, 	\
                                   _w_ndx, _dma_cmd)		\
  modify_field(doorbell_addr_scratch.addr,			\
        STORAGE_DOORBELL_ADDRESS(_qtype,			\
                                 _lif,				\
                                 DOORBELL_SCHED_WR_RESET,	\
                                 DOORBELL_UPDATE_P_NDX_INCR));	\
  modify_field(qpop_doorbell_data.data,				\
        STORAGE_DOORBELL_DATA(storage_kivec0.w_ndx, _ring, _qid,\
                              0));				\
  DMA_COMMAND_PHV2MEM_FILL(_dma_cmd, 0,				\
        PHV_FIELD_OFFSET(qpush_doorbell_data.data),		\
        PHV_FIELD_OFFSET(qpush_doorbell_data.data),		\
        0, 0, 0, 0)

#endif     // STORAGE_P4_HDR_H
