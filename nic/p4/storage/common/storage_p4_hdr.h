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

// Queue empty macros
#define _QUEUE_EMPTY(_p_ndx, _c_ndx)	(_p_ndx == _c_ndx)
#define QUEUE_EMPTY(q)	_QUEUE_EMPTY(q.p_ndx, q.c_ndx)

// Queue full macros
#define _QUEUE_FULL(_c_ndx, _p_ndx, _num_entries, _increment)	\
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
