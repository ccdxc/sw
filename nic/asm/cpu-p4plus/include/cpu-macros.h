/*********************************************************
 * cpu-macros.h
 ********************************************************/
#ifndef __CPU_MACROS_H__
#define __CPU_MACROS_H__

#include "capri-macros.h"

#define CPU_PIDX_SIZE                       16
#define CPU_PIDX_SHIFT                      4

#define CPU_ARQRX_TABLE_SIZE                (1024 * 8)
#define CPU_ARQRX_TABLE_SHIFT               13

#define NIC_ARQRX_ENTRY_SIZE                8
#define NIC_ARQRX_ENTRY_SIZE_SHIFT          3          /* for 8B */

#define CPU_ARQRX_QIDXR_OFFSET              64
#define ARQRX_QIDXR_DIR_ENTRY_SIZE_SHIFT    9          /* for 512B */

#define CPU_VALID_BIT_SHIFT                 63

#define CPU_PHV_RING_ENTRY_DESC_ADDR_START CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
#define CPU_PHV_RING_ENTRY_DESC_ADDR_END CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)

# define CPU_RX_ARQ_BASE_FOR_ID(_dest_r, \
                                _arqrx_base, \
                                _k_cpu_id) \
    add    _dest_r, r0, _k_cpu_id; \
    sll    _dest_r, _dest_r, CPU_ARQRX_TABLE_SHIFT; \
    add    _dest_r, _dest_r, _arqrx_base


#define CPU_RX_ENQUEUE(_dest_r, \
                       _descr_addr, \
                       _arqrx_pindex, \
                       _arqrx_base, \
                       _phv_desc_field_name, \
                       _dma_cmd_prefix, \
                       _eop_, \
                       _wr_fence_, \
                       _debug_dol_cr) \
 	add     _dest_r, r0, _arqrx_pindex; \
	sll     _dest_r, _dest_r, NIC_ARQRX_ENTRY_SIZE_SHIFT; \
	add     _dest_r, _dest_r, _arqrx_base; \
	phvwri  p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_PHV_TO_MEM; \
	phvwr   p.##_dma_cmd_prefix##_addr, _dest_r; \
    add     _dest_r, r0, r0; \
    add.!_debug_dol_cr  _dest_r, r0, 0x1; \
    sll.!_debug_dol_cr  _dest_r, _dest_r, CPU_VALID_BIT_SHIFT; \
    add     _dest_r, _dest_r, _descr_addr; \
	phvwr   p._phv_desc_field_name, _dest_r; \
	phvwri  p.##_dma_cmd_prefix##_phv_start_addr, CAPRI_PHV_START_OFFSET(_phv_desc_field_name); \
	phvwri  p.##_dma_cmd_prefix##_phv_end_addr, CAPRI_PHV_END_OFFSET(_phv_desc_field_name); \
    phvwri  p.##_dma_cmd_prefix##_eop, _eop_; \
    phvwri  p.##_dma_cmd_prefix##_wr_fence, _wr_fence_ 
   

#define CPU_ARQ_PIDX_READ_INC(_dest_r, _k_cpu_id, _d_struct, _pi0_field_name, _temp1_r, _temp2_r) \
    add     _temp1_r, r0, _k_cpu_id; \
    sll     _temp1_r, _temp1_r, CPU_PIDX_SHIFT; \
    tblrdp  _dest_r, _temp1_r, offsetof(_d_struct, _pi0_field_name), CPU_PIDX_SIZE; \
    addi    _temp2_r, _dest_r, 1; \
    tblwrp.f _temp1_r, offsetof(_d_struct, _pi0_field_name), CPU_PIDX_SIZE, _temp2_r 

#define CPU_ARQRX_QIDX_ADDR(_dir, _dest_r, _arqrx_qidxr_base) \
    addi   _dest_r, r0, _dir; \
    sll    _dest_r, _dest_r, ARQRX_QIDXR_DIR_ENTRY_SIZE_SHIFT; \
    addi   _dest_r, _dest_r, _arqrx_qidxr_base


#endif
