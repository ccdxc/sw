/*****************************************************************************
 *  storage_asm_defines.h: Macros for common functions used across various 
 *                         storage ASMs. GPR r7 is strictly to be used for 
 *                         input/output to macros to carry over state 
 *                         information back and forth. GPR r6 is not to be 
 *                         used as it is used for storing state information
 *                         in programs. GPRs r1..r5 can be used freely.
 *****************************************************************************/

#ifndef STORAGE_ASM_DEFINES_H
#define STORAGE_ASM_DEFINES_H

#include "storage_common_defines.h"

// Macros for accessing fields in the storage K+I vector
#define STORAGE_KIVEC0_W_NDX		\
	k.storage_kivec0_w_ndx
#define STORAGE_KIVEC0_DST_LIF		\
	k.{storage_kivec0_dst_lif_sbit0_ebit7...storage_kivec0_dst_lif_sbit8_ebit10}
#define STORAGE_KIVEC0_DST_QTYPE	\
	k.storage_kivec0_dst_qtype
#define STORAGE_KIVEC0_DST_QID		\
	k.{storage_kivec0_dst_qid_sbit0_ebit1...storage_kivec0_dst_qid_sbit18_ebit23}
#define STORAGE_KIVEC0_DST_QADDR	\
	k.{storage_kivec0_dst_qaddr_sbit0_ebit1...storage_kivec0_dst_qaddr_sbit2_ebit33}
#define STORAGE_KIVEC0_PRP_ASSIST	\
	k.storage_kivec0_prp_assist
#define STORAGE_KIVEC0_IS_Q0		\
	k.storage_kivec0_is_q0

#define STORAGE_KIVEC1_SRC_LIF		\
	k.{storage_kivec1_src_lif_sbit0_ebit7...storage_kivec1_src_lif_sbit8_ebit10}
#define STORAGE_KIVEC1_SRC_QTYPE	\
	k.storage_kivec1_src_qtype
#define STORAGE_KIVEC1_SRC_QID		\
	k.{storage_kivec1_src_qid_sbit0_ebit1...storage_kivec1_src_qid_sbit18_ebit23}
#define STORAGE_KIVEC1_SRC_QADDR	\
	k.{storage_kivec1_src_qaddr_sbit0_ebit1...storage_kivec1_src_qaddr_sbit2_ebit33}

#define STAGE0_KIVEC_LIF		\
	k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define STAGE0_KIVEC_QTYPE		\
	k.p4_txdma_intr_qtype
#define STAGE0_KIVEC_QID		\
	k.p4_txdma_intr_qid
#define STAGE0_KIVEC_QADDR		\
	k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

// TODO: Fix these to use the values defined in hardware
#define CAPRI_DMA_PHV2MEM		3
#define CAPRI_DMA_MEM2MEM		5
#define CAPRI_DMA_M2M_TYPE_SRC		1
#define CAPRI_DMA_M2M_TYPE_DST		2

// Load a table based on absolute address
// PC input must be a 28-bit value
#define LOAD_TABLE_FOR_ADDR(_table_addr, _load_size, _pc)		\
  phvwri	p.app_header_table0_valid, 1;				\
  phvwri	p.app_header_table1_valid, 0;				\
  phvwri	p.app_header_table2_valid, 0;				\
  phvwri	p.app_header_table3_valid, 0;				\
  phvwr		p.common_te0_phv_table_pc, _pc;				\
  phvwr		p.common_te0_phv_table_addr, _table_addr;		\
  phvwr.e	p.common_te0_phv_table_raw_table_size, _load_size;	\
  nop;									\

// Load a table based on absolute address
// PC input is a .param resolved by the loader (34-bit value)
#define LOAD_TABLE_FOR_ADDR_PARAM(_table_addr, _load_size, _pc)		\
  addi		r1, r0, _pc;						\
  srl		r1, r1, 6;						\
  addi		r2, r0, _load_size;					\
  LOAD_TABLE_FOR_ADDR(_table_addr, r2, r1)				\

// Load a table based with a calculation based on index
// addr = _table_base + (_entry_index * (2 ^_entry_size))
// PC input must be a 28-bit value
#define LOAD_TABLE_FOR_INDEX(_table_base, _entry_index, _entry_size,	\
                             _load_size, _pc)				\
  add		r1, r0, _table_base;					\
  add 		r2, r0, _entry_size;					\
  sllv		r3, _entry_index, r2;					\
  add		r1, r1, r3;						\
  phvwri	p.app_header_table0_valid, 1;				\
  phvwri	p.app_header_table1_valid, 0;				\
  phvwri	p.app_header_table2_valid, 0;				\
  phvwri	p.app_header_table3_valid, 0;				\
  phvwr		p.common_te0_phv_table_pc, _pc;				\
  phvwr		p.common_te0_phv_table_addr, r1;			\
  phvwr.e	p.common_te0_phv_table_raw_table_size, _load_size;	\
  nop;									

// Load a table based with a calculation based on index
// addr = _table_base + (_entry_index * _entry_size)
// PC input is a .param resolved by the loader (34-bit value)
#define LOAD_TABLE_FOR_INDEX_PARAM(_table_base, _entry_index, 		\
                                   _entry_size, _load_size, _pc)	\
  addi		r1, r0, _pc;						\
  srl		r1, r1, 6;						\
  LOAD_TABLE_FOR_INDEX(_table_base, _entry_index, _entry_size,		\
                       _load_size, r1)					\

// Used in the last stage of a pipeline to clear all table valid bits
#define LOAD_NO_TABLES							\
  phvwri	p.app_header_table0_valid, 0;				\
  phvwri	p.app_header_table1_valid, 0;				\
  phvwri	p.app_header_table2_valid, 0;				\
  phvwri.e	p.app_header_table3_valid, 0;				\
  nop

// Capri PHV Bit to Byte Macros

#define CAPRI_PHV_FLIT_SIZE_BITS        512
#define CAPRI_PHV_FLIT_SIZE_BYTES       (CAPRI_PHV_FLIT_SIZE_BITS / 8)
#define CAPRI_PHV_BIT_TO_BYTE(x)                                        \
        (((x) / CAPRI_PHV_FLIT_SIZE_BITS) * CAPRI_PHV_FLIT_SIZE_BYTES) +\
         (CAPRI_PHV_FLIT_SIZE_BYTES) - 1 - (((x) % CAPRI_PHV_FLIT_SIZE_BITS) / 8)


// Phv2Mem DMA: Specify the address of the start and end fields in the PHV
//              and the destination address. Can specify 0 destination
//              address via GPR r0, which be update later.
#define DMA_PHV2MEM_SETUP(_start, _end, _addr, _dma_cmd_X)		\
   add		r1, r0, offsetof(p, _start);				\
   add		r1, r0, sizeof(p._start);				\
   add		r1, r0, offsetof(p, _end);				\
   phvwri	p._dma_cmd_X##_dma_cmd_phv_start_addr,			\
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) + 		\
                                      sizeof(p._start) - 1);		\
   phvwri	p._dma_cmd_X##_dma_cmd_phv_end_addr,			\
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _end));		\
   phvwri	p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_PHV2MEM;		\
   phvwr	p._dma_cmd_X##_dma_cmd_addr, _addr;			\
   srl		r1, _addr, 63;						\
   seq		c1, r1, 1;						\
   phvwr.c1	p._dma_cmd_X##_dma_cmd_host_addr, 1;			\

// DMA address update: Specify the destination address for the DMA command
#define DMA_ADDR_UPDATE(_addr, _dma_cmd_X)				\
   phvwr	p._dma_cmd_X##_dma_cmd_addr, _addr;			\
   srl		r1, _addr, 63;						\
   seq		c1, r1, 1;						\
   phvwr.c1	p._dma_cmd_X##_dma_cmd_host_addr, 1;			\
   
// Setup the start and end DMA pointers
#define DMA_PTR_SETUP(_start, _dma_cmd_eop, _dma_cmd_ptr)		\
   phvwri	p._dma_cmd_ptr,						\
                ((CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) + 		\
                                        sizeof(p._start) - 1))/16);	\
   phvwri	p._dma_cmd_eop, 1;					\

/*
#define DMA_PTR_SETUP(_start, _dma_cmd_eop, _dma_cmd_ptr)		\
   phvwri	p._dma_cmd_ptr, 32;					\
   phvwri	p._dma_cmd_eop, 1;					\
*/


// Setup the doorbell data. Write back the data in little endian format
#define DOORBELL_DATA_SETUP(_db_data, _index, _ring, _qid, _pid)	\
   add		r1, r0, _index;						\
   sll		r2, _ring, DOORBELL_DATA_RING_SHIFT;			\
   or		r1, r1, r2;						\
   sll		r2, _qid, DOORBELL_DATA_QID_SHIFT;			\
   or		r1, r1, r2;						\
   sll		r2, _pid, DOORBELL_DATA_PID_SHIFT;			\
   or		r1, r1, r2;						\
   phvwr	p._db_data, r1.dx;					\

// Setup the doorbell address. Output will be stored in GPR r7.
#define DOORBELL_ADDR_SETUP(_lif, _qtype, _sched_wr, _upd)		\
   sll		r7, _qtype, DOORBELL_ADDR_QTYPE_SHIFT;			\
   sll		r1, _lif, DOORBELL_ADDR_LIF_SHIFT;			\
   or		r7, r7, r1;						\
   addi		r1, r0, _sched_wr;					\
   sll		r1, r1, DOORBELL_ADDR_SCHED_WR_SHIFT;			\
   or		r7, r7, r1;						\
   addi		r1, r0, _upd;						\
   sll		r1, r1, DOORBELL_ADDR_UPD_SHIFT;			\
   or		r7, r7, r1;						\
   addi		r1, r0, DOORBELL_ADDR_WA_LOCAL_BASE;			\
   or		r7, r7, r1;						\

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

// Queue pop based on an increment value
#define QUEUE_POP_INCR(_w_ndx, _num_entries, _incr)			\
   tblmincri	_w_ndx, _num_entries, _incr;				\
   nop;									\

// Queue push (default increment value of 1)
#define QUEUE_POP(_w_ndx, _num_entries)					\
   QUEUE_POP_INCR(_w_ndx, _num_entries, 1)

// Queue push based on an increment value
#define QUEUE_PUSH_INCR(_p_ndx, _num_entries, _incr)			\
   tblmincri	_p_ndx, _num_entries, _incr;				\
   nop;									\

// Queue push (default increment value of 1)
#define QUEUE_PUSH(_p_ndx, _num_entries)				\
   QUEUE_PUSH_INCR(_p_ndx, _num_entries, 1)

#define QUEUE_PUSH_ADDR(_base_addr, _p_ndx, _size)			\
   add 		r1, r0, _size;						\
   mul		r1, r1, _p_ndx;						\
   add		r7, r1, _base_addr;					\

// Check if PRP assist can be done based on command parameters like opcode,
// data size, max assist size etc.
#define PRP_ASSIST_CHECK(_is_q0, _opc, _pdst, _nlb, _dptr1, _dptr2,	\
                         _branch_instr)					\
   sne		c1, r0, _is_q0;						\
   sne		c2, _opc, NVME_READ_CMD_OPCODE;				\
   sne		c3, _opc, NVME_WRITE_CMD_OPCODE;			\
   seq		c4, _pdst, r0;						\
   add		r1, r0, _nlb;						\
   sll		r1, r1, LB_SIZE_SHIFT;					\
   addi		r1, r1, 1;						\
   addi		r4, r0, MAX_ASSIST_SIZE;				\
   sle		c5, r1, r4;						\
   addi		r2, r0, PRP_SIZE_SUB;					\
   add		r5, r0, _dptr1;						\
   andi		r5, r5, PRP_SIZE_MASK;					\
   sub		r5, r2, r5;						\
   add		r3, r0, _dptr2;						\
   andi		r3, r3, PRP_SIZE_MASK;					\
   sub		r3, r2, r3;						\
   add		r7, r5, r3;						\
   sle		c6, r7, r4;						\
   bcf		![c1 | c2 | c3 | c4 | c5 | c6], _branch_instr;		\
   nop;									\
   
#endif     // STORAGE_ASM_DEFINES_H
