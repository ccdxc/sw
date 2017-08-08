/*****************************************************************************
 * queue.h: Definitions for modeling queue operations in P4+ pipeline of Capri
 *****************************************************************************/
 
#ifndef QUEUE_H
#define QUEUE_H

// Generic Queue Context Metadata. 
// Total size can be 64 bytes at most.
header_type q_ctx_t {
  fields {
    idx          : 16;     // Index of the Queue context
    state        : 16;     // State of the Queue context
    c_ndx        : 16;     // Consumer index
    p_ndx        : 16;     // Producer index
    p_ndx_db     : 64;     // Doorbell for Producer index
    c_ndx_db     : 64;     // Doorbell for Consumer index
    base_addr    : 64;     // Base address of Queue entries
    num_entries  : 16;     // Number of Queue entries
    paired_q_idx : 16;     // Index of Paired Queue to send commands (if any)
    rsvd         : 224;    // Align to 64 bytes
  }
}

// Generic Priority Queue Context Metadata (upto 3 priorities). 
// Total size can be 64 bytes at most.
// When modifying this, adjust the offsets in storagep4/common/defines.h
header_type pri_q_ctx_t {
  fields {
    idx          : 16;     // Index of the Queue context
    state        : 16;     // State of the Queue context
    c_ndx_lo     : 16;     // Consumer index (low priority)
    c_ndx_med    : 16;     // Consumer index (medium priority)
    c_ndx_hi     : 16;     // Consumer index (high priority)
    p_ndx_lo     : 16;     // Producer index (low priority)
    p_ndx_med    : 16;     // Producer index (medium priority)
    p_ndx_hi     : 16;     // Producer index (high priority)
    p_ndx_lo_db  : 64;     // Doorbell for Producer index (low priority)
    p_ndx_med_db : 64;     // Doorbell for Producer index (medium priority)
    p_ndx_hi_db  : 64;     // Doorbell for Producer index (high priority)
    base_addr    : 64;     // Base address of Queue entries
    num_entries  : 16;     // Number of Queue entries
    paired_q_idx : 16;     // Index of Paired Queue to send commands (if any)
    rsvd         : 96;     // Align to 64 bytes
  }
}

// VF Queue Context Metadata. 
// Total size can be 64 bytes at most.
header_type vf_q_ctx_t {
  fields {
    idx          : 16;     // Index of the Queue context
    state        : 16;     // State of the Queue context
    c_ndx        : 16;     // Consumer index
    p_ndx        : 16;     // Producer index
    p_ndx_db     : 64;     // Doorbell for Producer index
    c_ndx_db     : 64;     // Doorbell for Consumer index
    base_addr    : 64;     // Base address of Queue entries
    num_entries  : 16;     // Number of Queue entries
    vf_id        : 16;     // Virtual Function Id
    sq_id        : 16;     // Submission Queue Index (valid for SQ)
    cq_id        : 16;     // Completion Queue Index (paired CQ for SQ)
    sq_ctx_idx   : 16;     // Submission Queue Context Index (valid for SQ)
    cq_ctx_idx   : 16;     // Completion Queue Context Index (paired CQ for SQ)
    paired_q_idx : 16;     // Index of Paired Queue to send commands (if any)
    rsvd         : 144;    // Align to 64 bytes
  }
}


// Queue Cant Pop: If queue state is busy or queue empty
#define _QUEUE_CANT_POP(q,c_ndx,p_ndx)				\
	((q.state == Q_CTX_BUSY) or (c_ndx == p_ndx))


// Queue Cant Push: If queue state is full
#define _QUEUE_CANT_PUSH(q,c_ndx,p_ndx, increment)		\
	(c_ndx == ((p_ndx + increment) % q.num_entries))


// Queue Lock: Set the busy bit 
#define _QUEUE_LOCK(q)						\
  modify_field(q.state, Q_CTX_BUSY);

// Queue Lock and Load: Set the busy bit and load the CQ entry pointed to 
//                      consumer index for the next stage
#define _QUEUE_LOCK_AND_LOAD(q, i, entry_size, load_size, c_ndx, stage_entry)\
  _QUEUE_LOCK(q)							\
  CAPRI_LOAD_TABLE_IDX(i, q.base_addr, c_ndx, entry_size, load_size, stage_entry)	\


// Queue Pop: Increment consumer index. In ASM, memory write of the fields for 
//            consumer index and state from PHV (contiguous in memory).
//            Also DMA write of the consumer index to the doorbell register.
#define _QUEUE_POP(q, ctx_base, c_ndx)				\
  modify_field(c_ndx, (c_ndx + 1) % q.num_entries);		\
  modify_field(q.state, Q_CTX_FREE);
#if 0								
  memwr(ctx_base + (Q_CTX_SIZE * q.idx), q.state ..  q.c_ndx);
  dmawr(q.c_ndx_db, q.c_ndx);
#endif


// Queue Push: Increment producer index. In ASM, DMA write of producer index 
//             from PHV to the doorbell register
#define _QUEUE_PUSH(q, p_ndx)					\
  modify_field(p_ndx, (p_ndx + 1) % q.num_entries);
#if 0								
  // NOTE: Take care of right doorbell register when translating to assembly
  dmawr(q.p_ndx_db, q.p_ndx); 
#endif

// Generic Queue APIs
#define QUEUE_CANT_POP(q)					\
	_QUEUE_CANT_POP(q, q.c_ndx, q.p_ndx)
#define QUEUE_CANT_PUSH(q)					\
	_QUEUE_CANT_PUSH(q, q.c_ndx, q.p_ndx, 1)
#define QUEUE_CANT_PUSH2(q)					\
	_QUEUE_CANT_PUSH(q, q.c_ndx, q.p_ndx, 2)
#define QUEUE_POP(q, ctx_base)					\
	_QUEUE_POP(q, ctx_base, q.c_ndx)
#define QUEUE_PUSH(q)						\
	_QUEUE_PUSH(q, q.p_ndx)
#define QUEUE_LOCK_AND_LOAD(q, i, entry_size, load_size, stage_entry)	\
	_QUEUE_LOCK_AND_LOAD(q, i, entry_size, load_size, q.c_ndx, stage_entry)

// Priority Queue APIs
#define QUEUE_CANT_POP_LO(q)					\
	_QUEUE_CANT_POP(q, q.c_ndx_lo, q.p_ndx_lo)
#define QUEUE_CANT_PUSH_LO(q)					\
	_QUEUE_CANT_PUSH(q, q.c_ndx_lo, q.p_ndx_lo, 1)
#define QUEUE_POP_LO(q, ctx_base)				\
	_QUEUE_POP(q, ctx_base, q.c_ndx_lo)
#define QUEUE_PUSH_LO(q)					\
	_QUEUE_PUSH(q, q.p_ndx_lo)
#define QUEUE_LOCK_AND_LOAD_LO(q, i, entry_size, load_size, stage_entry)	\
	_QUEUE_LOCK_AND_LOAD(q, i, entry_size, load_size, q.c_ndx_lo, stage_entry)

#define QUEUE_CANT_POP_MED(q)					\
	_QUEUE_CANT_POP(q, q.c_ndx_med, q.p_ndx_med)
#define QUEUE_CANT_PUSH_MED(q)					\
	_QUEUE_CANT_PUSH(q, q.c_ndx_med, q.p_ndx_med, 1)
#define QUEUE_POP_MED(q, ctx_base)				\
	_QUEUE_POP(q, ctx_base, q.c_ndx_med)
#define QUEUE_PUSH_MED(q)					\
	_QUEUE_PUSH(q, q.p_ndx_med)
#define QUEUE_LOCK_AND_LOAD_MED(q, i, entry_size, load_size, stage_entry)	\
	_QUEUE_LOCK_AND_LOAD(q, i, entry_size, load_size, q.c_ndx_med, stage_entry)

#define QUEUE_CANT_POP_HI(q)					\
	_QUEUE_CANT_POP(q, q.c_ndx_hi, q.p_ndx_hi)
#define QUEUE_CANT_PUSH_HI(q)					\
	_QUEUE_CANT_PUSH(q, q.c_ndx_hi, q.p_ndx_hi, 1)
#define QUEUE_POP_HI(q, ctx_base)				\
	_QUEUE_POP(q, ctx_base, q.c_ndx_hi)
#define QUEUE_PUSH_HI(q)					\
	_QUEUE_PUSH(q, q.p_ndx_hi)
#define QUEUE_LOCK_AND_LOAD_HI(q, i, entry_size, load_size, stage_entry)	\
	_QUEUE_LOCK_AND_LOAD(q, i, entry_size, load_size, q.c_ndx_hi, stage_entry)

#define VF_QUEUE_CTX_COPY(q_ctx)			\
  modify_field(q_ctx.idx, idx);				\
  modify_field(q_ctx.state, state);			\
  modify_field(q_ctx.c_ndx, c_ndx);			\
  modify_field(q_ctx.p_ndx, p_ndx);			\
  modify_field(q_ctx.p_ndx_db, p_ndx_db);		\
  modify_field(q_ctx.c_ndx_db, c_ndx_db);		\
  modify_field(q_ctx.base_addr, base_addr);		\
  modify_field(q_ctx.num_entries, num_entries);		\
  modify_field(q_ctx.vf_id, vf_id);			\
  modify_field(q_ctx.sq_id, sq_id);			\
  modify_field(q_ctx.cq_id, cq_id);			\
  modify_field(q_ctx.sq_ctx_idx, sq_ctx_idx);		\
  modify_field(q_ctx.cq_ctx_idx, cq_ctx_idx);		\
  modify_field(q_ctx.paired_q_idx, paired_q_idx);	\
  modify_field(q_ctx.rsvd, rsvd);			\

#define QUEUE_CTX_COPY(q_ctx)				\
  modify_field(q_ctx.idx, idx);				\
  modify_field(q_ctx.state, state);			\
  modify_field(q_ctx.c_ndx, c_ndx);			\
  modify_field(q_ctx.p_ndx, p_ndx);			\
  modify_field(q_ctx.p_ndx_db, p_ndx_db);		\
  modify_field(q_ctx.c_ndx_db, c_ndx_db);		\
  modify_field(q_ctx.base_addr, base_addr);		\
  modify_field(q_ctx.num_entries, num_entries);		\
  modify_field(q_ctx.paired_q_idx, paired_q_idx);	\
  modify_field(q_ctx.rsvd, rsvd);			\

#define PRI_QUEUE_CTX_COPY(q_ctx)			\
  modify_field(q_ctx.idx, idx);				\
  modify_field(q_ctx.state, state);			\
  modify_field(q_ctx.c_ndx_lo, c_ndx_lo);		\
  modify_field(q_ctx.c_ndx_med, c_ndx_med);		\
  modify_field(q_ctx.c_ndx_hi, c_ndx_hi);		\
  modify_field(q_ctx.p_ndx_lo, p_ndx_lo);		\
  modify_field(q_ctx.p_ndx_med, p_ndx_med);		\
  modify_field(q_ctx.p_ndx_hi, p_ndx_hi);		\
  modify_field(q_ctx.p_ndx_lo_db, p_ndx_lo_db);		\
  modify_field(q_ctx.p_ndx_med_db, p_ndx_med_db);	\
  modify_field(q_ctx.p_ndx_hi_db, p_ndx_hi_db);		\
  modify_field(q_ctx.base_addr, base_addr);		\
  modify_field(q_ctx.num_entries, num_entries);		\
  modify_field(q_ctx.paired_q_idx, paired_q_idx);	\
  modify_field(q_ctx.rsvd, rsvd);			\

#endif     // QUEUE_H
