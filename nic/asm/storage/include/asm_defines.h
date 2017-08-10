/*****************************************************************************
 *  asm_defines.h: Macros for common functions used across various ASMs. 
 *                 GPR r7 is strictly to be used for input/output to macros
 *                 to carry over state information back and forth.
 *****************************************************************************/

#ifndef ASM_DEFINES_H
#define ASM_DEFINES_H

#include "defines.h"


// TODO: Fix these to use the values defined in hardware
#define CAPRI_DMA_PHV2MEM		1
#define CAPRI_DMA_MEM2MEM		2
#define CAPRI_DMA_M2M_TYPE_SRC		1
#define CAPRI_DMA_M2M_TYPE_DST		2

// Load a table based on fixed base, index and fixed size
// Size * index is used to determine offset beyond table base
#define LOAD_TABLE_FBASE_IDX(_table_base, _idx, _table_size,		\
                             _load_size, _stage_entry)			\
  addi		r1, r0, _table_base;					\
  add 		r2, r0, _idx;						\
  muli		r3, r2, _table_size;					\
  add		r1, r1, r3;						\
  phvwri	p.common_te0_phv_table_pc, _stage_entry;		\
  phvwr		p.common_te0_phv_table_addr, r1;			\
  phvwri.e	p.common_te0_phv_table_raw_table_size, _load_size;	\
  nop;									\

// Load a table based on calculated base, index and fixed size
// Size * index is used to determine offset beyond table base
#define LOAD_TABLE_CBASE_IDX(_table_base, _idx, _table_size,		\
                             _load_size, _stage_entry)			\
  add		r1, r0, _table_base;					\
  add 		r2, r0, _idx;						\
  muli		r3, r2, _table_size;					\
  add		r1, r1, r3;						\
  phvwri	p.common_te0_phv_table_pc, _stage_entry;		\
  phvwr		p.common_te0_phv_table_addr, r1;			\
  phvwri.e	p.common_te0_phv_table_raw_table_size, _load_size;	\
  nop;									

// Load a table based on calculated base, fixed offset and fixed size
// Offset is used as is beyond table base
#define LOAD_TABLE_CBASE_OFFSET(_table_base, _offset, _load_size,	\
                                _stage_entry)				\
  add		r1, r0, _table_base;					\
  addi 		r2, r0, _offset;					\
  add		r1, r1, r2;						\
  phvwri	p.common_te0_phv_table_pc, _stage_entry;		\
  phvwr		p.common_te0_phv_table_addr, r1;			\
  phvwri.e	p.common_te0_phv_table_raw_table_size, _load_size;	\
  nop;									\

// Load no table, but call a handler to execute a skip stage
#define LOAD_NO_TABLE(_stage_entry)					\
  phvwri	p.common_te0_phv_table_pc, _stage_entry;		\
  phvwri	p.common_te0_phv_table_addr, 0;				\
  phvwri.e	p.common_te0_phv_table_raw_table_size, 0;		\
  nop;									\

// Phv2Mem DMA: Specify the address of the start and end fields in the PHV
//              and the destination address
#define DMA_PHV2MEM_SETUP(_start, _end, _addr, _fn_start, _fn_end,	\
                          _fn_type, _fn_addr, _fn_host_addr)		\
   phvwri	p._fn_start, (sizeof (p) >> 3) - ((offsetof(p, _start) +\
                                                sizeof(p._start)) >> 3);\
   phvwri	p._fn_end, (sizeof(p) >> 3) - (offsetof(p, _end) >> 3);	\
   phvwri	p._fn_type, CAPRI_DMA_PHV2MEM;				\
   phvwr	p._fn_addr, _addr;					\
   srl		r1, _addr, 63;						\
   seq		c1, r1, 1;						\
   phvwr.c1	p._fn_host_addr, 1;					\
   

// Mem2Mem DMA: Specify the address and size (call once each for src and dest)
#define DMA_MEM2MEM_SETUP_RSIZE(_addr, _size, _mtype, _fn_ctype,	\
                                _fn_addr, _fn_size, _fn_mtype,		\
                                _fn_host_addr)				\
   phvwri	p._fn_ctype, CAPRI_DMA_MEM2MEM;				\
   phvwr	p._fn_addr, _addr;					\
   phvwr	p._fn_size, _size;					\
   phvwri	p._fn_mtype, _mtype;					\
   srl		r1, _addr, 63;						\
   seq		c1, r1, 1;						\
   phvwr.c1	p._fn_host_addr, 1;					\

#define DMA_MEM2MEM_SETUP(_addr, _size, _mtype, _fn_ctype, _fn_addr,	\
                          _fn_size, _fn_mtype, _fn_host_addr)		\
   addi		r1, r0, _size;						\
   DMA_MEM2MEM_SETUP_RSIZE(_addr, r1, _mtype, _fn_ctype, _fn_addr, 	\
                           _fn_size, _fn_mtype, _fn_host_addr)		\

// Setup the write fence for DMA
#define DMA_WRITE_FENCE_SETUP(_fn_wfence)				\
   phvwri	p._fn_wfence, 1;					\

// Setup the start and end DMA pointers
#define DMA_PTR_SETUP(_start, _fn_eop, _fn_ptr)				\
   phvwri	p._fn_ptr, (sizeof (p) >> 7) - ((offsetof(p, _start) +	\
                                                sizeof(p._start)) >> 7);\
   phvwri	p._fn_eop, 1;						\

// Service NVME backend priority queue based on priority running counter
// and the weight. Load the table address of the priority queue if it can
// be serviced.
#define NVME_BE_SERVICE_PRI_QUEUE(_run_ctr, _pri_run_ctr, _pri_weight,	\
                                  _pri, _p_ndx, _c_ndx, _base_addr,	\
                                  _num_entries, _table_size, _load_size,\
                                  _branch_instr)			\
   slt		c1, _pri_run_ctr, _pri_weight;				\
   sne		c2, _p_ndx, _c_ndx;					\
   bcf		[!c1 | !c2], _branch_instr;				\
   addi		r1, r0, _pri;						\
   muli		r1, r1, _table_size;					\
   add		r2, r0, _num_entries;					\
   sllv		r1, r1, r2;						\
   add		r1, r1, _base_addr;					\
   addi		r2, r0, 1;						\
   tbladd	_run_ctr, r2;						\
   tbladd	_pri_run_ctr, r2;					\
   phvwri	p.nvme_tgt_kivec0_io_priority, _pri;			\
   LOAD_TABLE_CBASE_IDX(r1, _c_ndx, _table_size, _load_size,		\
                        nvme_be_sq_entry_copy_start)			\

// Queue context locked check by checking busy state
#define QUEUE_CTX_LOCKED(_state, _branch_instr)				\
   addi		r1, r0, Q_CTX_BUSY;					\
   seq		c1, _state, r1;						\
   bcf		[c1], _branch_instr;					\
   nop;									\
   
// Queue context locking by setting busy state
#define QUEUE_CTX_LOCK(_state)						\
   addi		r1, r0, Q_CTX_BUSY;					\
   tblwr	_state, r1;						\

// Queue full check based on an increment value
#define QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, _branch_instr)		\
   add		r1, r0, _p_ndx;						\
   mincr	r1, _num_entries, 1;					\
   seq		c1, r1, _c_ndx;						\
   bcf		[c1], _branch_instr;					\
   nop;									\

// Queue empty check
#define QUEUE_EMPTY(_p_ndx, _c_ndx, _branch_instr)			\
   seq		c1, _p_ndx, _c_ndx;					\
   bcf		[c1], _branch_instr;					\
   nop;									\

// Queue push based on an increment value
#define QUEUE_PUSH_INCR(_p_ndx, _num_entries, _incr)			\
   tblmincri	_p_ndx, _num_entries, _incr;				\

// Queue push (default increment value of 1)
#define QUEUE_PUSH(_p_ndx, _num_entries)				\
   QUEUE_PUSH_INCR(_p_ndx,  _num_entries, 1)

// Queue pop by incrementing the c_ndx, setting state to free and 
// memory write of state and c_ndx 
#define QUEUE_POP(_c_ndx, _num_entries,	_q_idx, _table_base,		\
                  _state_offset, _c_ndx_offset)				\
   add		r1, r0, _c_ndx;						\
   mincr	r1, _num_entries, 1;					\
   addi		r2, r0, Q_CTX_FREE;					\
   add		r3, r0, _q_idx;						\
   muli		r3, r3, Q_CTX_SIZE;					\
   addi		r3, r3, _table_base;					\
   addi		r4, r3, _state_offset;					\
   memwr.h	r4, r2;							\
   addi		r4, r3, _c_ndx_offset;					\
   memwr.h	r4, r1;							\

// Derive the queue push address and store it in register r7
#define QUEUE_PUSH_ADDR_RSIZE(_base_addr, _p_ndx, _size)			\
   add	 	r1, r0, _size;						\
   mul		r1, r1, _p_ndx;						\
   add		r7, r1, _base_addr;					\

#define QUEUE_PUSH_ADDR(_base_addr, _p_ndx, _size)			\
   addi 	r1, r0, _size;						\
   mul		r1, r1, _p_ndx;						\
   add		r7, r1, _base_addr;					\

// Derive the queue push offset and store it in register r7
#define QUEUE_PUSH_ADDR_OFFSET(_offset, _size)				\
   addi 	r1, r0, _size;						\
   add		r7, r1, _offset;					\

// Queue push by incrmenting and saving p_ndx into a saved address space
#define QUEUE_PUSH_INCR_SAVE_PNDX(_p_ndx, _num_entries, _incr,		\
                                  _saved_p_ndx)				\
   QUEUE_PUSH_INCR(_p_ndx, _num_entries, _incr);			\
   add		r1, r0, _p_ndx;						\
   mincr	r1, _num_entries, _incr;				\
   phvwr	_saved_p_ndx, r1;					\

// Queue push by incrementing (by 1) and saving p_ndx into a saved address space
#define QUEUE_PUSH_SAVE_PNDX(_p_ndx, _num_entries, _saved_p_ndx)	\
   QUEUE_PUSH_INCR_SAVE_PNDX(_p_ndx, _num_entries, 1, _saved_p_ndx)

// Check if all prirority queues are empty
#define PRI_QUEUE_ALL_EMPTY(_p_ndx_lo, _c_ndx_lo, _p_ndx_med,		\
                            _c_ndx_med, _p_ndx_hi, _c_ndx_hi,		\
                            _branch_instr)				\
   seq		c1, _p_ndx_lo, _c_ndx_lo;				\
   seq		c2, _p_ndx_med, _c_ndx_med;				\
   seq		c3, _p_ndx_hi, _c_ndx_hi;				\
   bcf		[c1 & c2 & c3], _branch_instr;				\
   nop;									\

// Queue pop from a priority queu by incrementing the c_ndx, setting state to 
// free and memory write of state and c_ndx 
#define PRI_QUEUE_POP(_pri_vec, _pri_val, _c_ndx, _num_entries, 	\
                      _q_idx, _table_base, _state_offset, _c_ndx_offset,\
                      _branch_instr)					\
   addi		r1, r0, _pri_val;					\
   seq		c1, _pri_vec, r1;					\
   bcf		[!c1], _branch_instr;					\
   add		r1, r0, _c_ndx;						\
   mincr	r1, _num_entries, 1;					\
   add		r2, r0, Q_CTX_FREE;					\
   add		r3, r0, _q_idx;						\
   muli		r3, r3, Q_CTX_SIZE;					\
   addi		r3, r3, _table_base;					\
   addi		r4, r3, _state_offset;					\
   memwr.h	r4, r2;							\
   addi		r4, r3, _c_ndx_offset;					\
   memwr.h	r4, r1;							\

// Derive the priority queue push address and store it in register r7
// base_addr + (size * priority * num_entries) + (size * p_ndx)
#define PRI_QUEUE_PUSH_ADDR(_pri_vec, _base_addr, _p_ndx,		\
                            _num_entries, _size)			\
   addi 	r1, r0, _size;						\
   add		r2, r0, _num_entries;					\
   mul		r3, r1, _pri_vec;					\
   sllv		r3, r3, r2;						\
   mul		r4, r1, _p_ndx;						\
   add		r4, r4, r3;						\
   add		r7, r4, _base_addr;					\

// Pushing into a priority queue:
// 1. Check and branch if the priority queue is full
// 2. Calculate the address to which the command has to be written to in the
//    priority queue. Output will be stored in GPR r7.
// 3. DMA write of fields in the Rx buffer entry into the priroity queue
// 4. Push the entry
// 5. DMA NVME backend SQ p_ndx to p_ndx_db (doorbell) register
//    TODO: Check of this is by scheduler bit or doorbell write
// 6. Setup the start and end DMA pointers
#define PRI_QUEUE_PUSH(_pri_vec, _pri_val, _c_ndx, _p_ndx, _p_ndx_db,	\
                       _fn_p_ndx, _num_entries, _src, _table_base,	\
                       _table_size, _branch_instr1, _branch_instr2)	\
   sne		c1, _pri_vec, _pri_val;					\
   bcf		[c1], _branch_instr1;					\
   QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, _branch_instr2)		\
   DMA_MEM2MEM_SETUP(_src, R2N_RX_BUF_ENTRY_BE_CMD_SIZE,		\
                     CAPRI_DMA_M2M_TYPE_SRC, dma_cmd6_cmdtype,		\
                     dma_cmd6_addr, dma_cmd6_data_size,			\
                     dma_cmd6_mem2mem_type, dma_cmd6_host_addr)		\
   PRI_QUEUE_PUSH_ADDR(_pri_vec, _table_base, _p_ndx, _num_entries,	\
                       _table_size)					\
   DMA_MEM2MEM_SETUP(r7, R2N_RX_BUF_ENTRY_BE_CMD_SIZE,			\
                     CAPRI_DMA_M2M_TYPE_DST, dma_cmd7_cmdtype,		\
                     dma_cmd7_addr, dma_cmd7_data_size,			\
                     dma_cmd7_mem2mem_type, dma_cmd7_host_addr)		\
   QUEUE_PUSH_SAVE_PNDX(_p_ndx, _num_entries, p._fn_p_ndx)		\
   DMA_PHV2MEM_SETUP(_fn_p_ndx, _fn_p_ndx, _p_ndx_db,			\
                     dma_cmd0_phv_start, dma_cmd0_phv_end,		\
                     dma_cmd0_cmdtype, dma_cmd0_addr,			\
                     dma_cmd0_host_addr)				\
   DMA_PTR_SETUP(dma_cmd6_rsvd, dma_cmd0_cmdeop, p4_txdma_intr_dma_cmd_ptr)	\


// Address of the SSD command entry based on SSD idx and command index
#define SSD_CMD_ENTRY_ADDR(_idx, _cmd_index)				\
   add		r1, r0, _idx;						\
   muli		r1, r1, SSD_CMDS_ENTRY_SIZE;				\
   add		r2, r0, _cmd_index;					\
   muli		r2, r2, NVME_BE_SQ_ENTRY_SIZE;				\
   add		r7, r1, r2;						\
   addi		r7, r7, SSD_CMDS_HEADER_SIZE;				\
   addi		r7, r7, SSD_CMDS_TABLE_BASE;				\
 
// Update the SSD priority queue to which the command is to be sent and
// load the table address for that queue. Note: Use a branch delay slot 
// to avoid spurious table writes.
#define SSD_INFO_UPDATE_AND_LOAD(_pri_vec, _pri_val, _pri_ctr, 		\
                                 _global_ctr, _decr_val, _branch_instr,	\
                                 _stage_entry)				\
   add		r4, r0, _pri_vec;					\
   bnei         r4, _pri_val, _branch_instr;				\
   addi		r5, r0, _decr_val;					\
   tblsub       _pri_ctr, r5;						\
   tblsub       _global_ctr, r5;					\
   LOAD_NO_TABLE(_stage_entry);						\

// Check if PRP assist can be done based on command parameters like opcode,
// data size, max assist size etc.
#define PRP_ASSIST_CHECK(_sq_id, _opc, _pdst, _nlb, _dptr1, _dptr2,	\
                         _branch_instr)					\
   sle		c1, r0, _sq_id;						\
   sne		c2, _opc, NVME_READ_CMD_OPCODE;				\
   sne		c3, _opc, NVME_WRITE_CMD_OPCODE;			\
   seq		c4, _pdst, r0;						\
   add		r1, r0, _nlb;						\
   sll		r1, r1, LB_SIZE_SHIFT;					\
   addi		r1, r1, 1;						\
   addi		r5, r0, MAX_ASSIST_SIZE;				\
   sle		c5, r1, r5;						\
   addi		r2, r0, PRP_SIZE_SUB;					\
   add		r6, r0, _dptr1;						\
   andi		r6, r6, PRP_SIZE_MASK;					\
   sub		r6, r2, r6;						\
   add		r3, r0, _dptr2;						\
   andi		r3, r3, PRP_SIZE_MASK;					\
   sub		r3, r2, r3;						\
   add		r7, r6, r3;						\
   sle		c6, r7, r5;						\
   bcf		![c1 | c2 | c3 | c4 | c5 | c6], _branch_instr;		\
   nop;									\
   
#endif     // ASM_DEFINES_H
