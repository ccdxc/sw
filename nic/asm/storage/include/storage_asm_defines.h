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

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE                   8
#define SIZE_IN_BITS(bytes)             ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)             ((bits) / BITS_PER_BYTE)
#endif

// Macros for accessing fields in the storage K+I vector
#define STORAGE_KIVEC0_W_NDX                    \
    k.storage_kivec0_w_ndx
#define STORAGE_KIVEC0_DST_LIF                  \
    k.{storage_kivec0_dst_lif_sbit0_ebit7...storage_kivec0_dst_lif_sbit8_ebit10}
#define STORAGE_KIVEC0_DST_QTYPE                \
    k.storage_kivec0_dst_qtype
#define STORAGE_KIVEC0_DST_QID                  \
    k.{storage_kivec0_dst_qid_sbit0_ebit1...storage_kivec0_dst_qid_sbit18_ebit23}
#define STORAGE_KIVEC0_DST_QADDR                \
    k.{storage_kivec0_dst_qaddr_sbit0_ebit1...storage_kivec0_dst_qaddr_sbit18_ebit33}
#define STORAGE_KIVEC0_PRP_ASSIST               \
    k.storage_kivec0_prp_assist
#define STORAGE_KIVEC0_IS_Q0                    \
    k.storage_kivec0_is_q0
#define STORAGE_KIVEC0_IO_PRIORITY              \
    k.{storage_kivec0_io_priority_sbit0_ebit5...storage_kivec0_io_priority_sbit6_ebit7}
#define STORAGE_KIVEC0_SSD_BM_ADDR              \
    k.{storage_kivec0_ssd_bm_addr_sbit0_ebit5...storage_kivec0_ssd_bm_addr_sbit30_ebit33}
#define STORAGE_KIVEC0_CMD_INDEX                \
    k.{storage_kivec0_cmd_index_sbit0_ebit3...storage_kivec0_cmd_index_sbit4_ebit7}
#define STORAGE_KIVEC0_SSD_HANDLE               \
    k.{storage_kivec0_ssd_handle_sbit0_ebit3...storage_kivec0_ssd_handle_sbit12_ebit15}
#define STORAGE_KIVEC0_DST_REWRITE              \
    k.storage_kivec0_dst_rewrite
#define STORAGE_KIVEC0_IS_REMOTE                \
    k.storage_kivec0_is_remote
#define STORAGE_KIVEC0_IS_READ                  \
    k.storage_kivec0_is_read

#define STORAGE_KIVEC1_SRC_LIF                  \
    k.{storage_kivec1_src_lif_sbit0_ebit7...storage_kivec1_src_lif_sbit8_ebit10}
#define STORAGE_KIVEC1_SRC_QTYPE                \
    k.storage_kivec1_src_qtype
#define STORAGE_KIVEC1_SRC_QID                  \
    k.{storage_kivec1_src_qid_sbit0_ebit1...storage_kivec1_src_qid_sbit18_ebit23}
#define STORAGE_KIVEC1_SRC_QADDR                \
    k.{storage_kivec1_src_qaddr_sbit0_ebit1...storage_kivec1_src_qaddr_sbit26_ebit33}
#define STORAGE_KIVEC1_DEVICE_ADDR              \
    k.{storage_kivec1_device_addr_sbit0_ebit7...storage_kivec1_device_addr_sbit32_ebit33}
#define STORAGE_KIVEC1_ROCE_CQ_NEW_CMD          \
    k.storage_kivec1_roce_cq_new_cmd

#define STORAGE_KIVEC2_SSD_Q_NUM                \
    k.storage_kivec2_ssd_q_num
#define STORAGE_KIVEC2_SSD_Q_SIZE               \
    k.storage_kivec2_ssd_q_size
#define STORAGE_KIVEC2_SGL_OUT_AOL_ADDR         \
    k.storage_kivec2_sgl_out_aol_addr

#define STORAGE_KIVEC3_ROCE_MSN                 \
    k.storage_kivec3_roce_msn
#define STORAGE_KIVEC3_DATA_ADDR                \
    k.storage_kivec3_data_addr

#define STORAGE_KIVEC4_BARCO_RING_ADDR          \
    k.{storage_kivec4_barco_ring_addr_sbit0_ebit15...storage_kivec4_barco_ring_addr_sbit32_ebit33}
#define STORAGE_KIVEC4_BARCO_DESC_SIZE          \
    k.{storage_kivec4_barco_desc_size_sbit0_ebit1...storage_kivec4_barco_desc_size_sbit2_ebit3}
#define STORAGE_KIVEC4_BARCO_PNDX_ADDR          \
    k.{storage_kivec4_barco_pndx_addr_sbit0_ebit5...storage_kivec4_barco_pndx_addr_sbit30_ebit33}
#define STORAGE_KIVEC4_BARCO_PNDX_SHADOW_ADDR   \
    k.{storage_kivec4_barco_pndx_shadow_addr_sbit0_ebit3...storage_kivec4_barco_pndx_shadow_addr_sbit28_ebit33}
#define STORAGE_KIVEC4_BARCO_PNDX_SIZE          \
    k.storage_kivec4_barco_pndx_size
#define STORAGE_KIVEC4_BARCO_RING_SIZE          \
    k.{storage_kivec4_barco_ring_size_sbit0_ebit2...storage_kivec4_barco_ring_size_sbit3_ebit4}
#define STORAGE_KIVEC4_W_NDX                    \
    k.{storage_kivec4_w_ndx_sbit0_ebit5...storage_kivec4_w_ndx_sbit14_ebit15}

#define STORAGE_KIVEC5_INTR_ADDR                \
    k.{storage_kivec5_intr_addr_sbit0_ebit7...storage_kivec5_intr_addr_sbit40_ebit63}
#define STORAGE_KIVEC5_DATA_LEN                 \
    k.{storage_kivec5_data_len_sbit0_ebit7...storage_kivec5_data_len_sbit8_ebit15}
#define STORAGE_KIVEC5_PAD_LEN_SHIFT            \
    k.storage_kivec5_pad_len_shift
#define STORAGE_KIVEC5_STATUS_DMA_EN            \
    k.storage_kivec5_status_dma_en
#define STORAGE_KIVEC5_DATA_LEN_FROM_DESC       \
    k.storage_kivec5_data_len_from_desc
#define STORAGE_KIVEC5_STOP_CHAIN_ON_ERROR      \
    k.storage_kivec5_stop_chain_on_error
#define STORAGE_KIVEC5_COPY_SRC_DST_ON_ERROR    \
    k.storage_kivec5_copy_src_desc_on_error
#define STORAGE_KIVEC5_NEXT_DB_EN               \
    k.storage_kivec5_next_db_en
#define STORAGE_KIVEC5_AOL_PAD_XFER_EN          \
    k.storage_kivec5_aol_pad_xfer_en
#define STORAGE_KIVEC5_SGL_XFER_EN              \
    k.storage_kivec5_sgl_xfer_en
#define STORAGE_KIVEC5_INTR_EN                  \
    k.storage_kivec5_intr_en
#define STORAGE_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH\
    k.storage_kivec5_next_db_action_barco_push

#define STAGE0_KIVEC_LIF                        \
    k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#define STAGE0_KIVEC_QTYPE                      \
    k.p4_txdma_intr_qtype
#define STAGE0_KIVEC_QID                        \
    k.p4_txdma_intr_qid
#define STAGE0_KIVEC_QADDR                      \
    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}


#define NVME_KIVEC_T0_S2S_W_NDX                 \
    k.nvme_kivec_t0_s2s_w_ndx
#define NVME_KIVEC_T0_S2S_DST_LIF               \
    k.{nvme_kivec_t0_s2s_dst_lif_sbit0_ebit7...nvme_kivec_t0_s2s_dst_lif_sbit8_ebit10}
#define NVME_KIVEC_T0_S2S_DST_QTYPE             \
    k.nvme_kivec_t0_s2s_dst_qtype
#define NVME_KIVEC_T0_S2S_DST_QID               \
    k.{nvme_kivec_t0_s2s_dst_qid_sbit0_ebit1...nvme_kivec_t0_s2s_dst_qid_sbit18_ebit23}
#define NVME_KIVEC_T0_S2S_DST_QADDR             \
    k.{nvme_kivec_t0_s2s_dst_qaddr_sbit0_ebit1...nvme_kivec_t0_s2s_dst_qaddr_sbit2_ebit33}
#define NVME_KIVEC_T0_S2S_IOB_ADDR              \
    k.{nvme_kivec_t0_s2s_iob_addr_sbit0_ebit31...nvme_kivec_t0_s2s_iob_addr_sbit32_ebit33}
#define NVME_KIVEC_T0_S2S_IO_MAP_BASE_ADDR      \
    k.{nvme_kivec_t0_s2s_io_map_base_addr_sbit0_ebit5...nvme_kivec_t0_s2s_io_map_base_addr_sbit30_ebit33}
#define NVME_KIVEC_T0_S2S_PRP_ASSIST            \
    k.nvme_kivec_t0_s2s_prp_assist
#define NVME_KIVEC_T0_S2S_IS_REMOTE             \
    k.nvme_kivec_t0_s2s_is_remote
#define NVME_KIVEC_T0_S2S_IS_READ               \
    k.nvme_kivec_t0_s2s_is_read
#define NVME_KIVEC_T0_S2S_PUNT_TO_ARM           \
    k.nvme_kivec_t0_s2s_punt_to_arm

#define NVME_KIVEC_T1_S2S_W_NDX                 \
    k.nvme_kivec_t1_s2s_w_ndx
#define NVME_KIVEC_T1_S2S_DST_LIF               \
    k.{nvme_kivec_t1_s2s_dst_lif_sbit0_ebit7...nvme_kivec_t1_s2s_dst_lif_sbit8_ebit10}
#define NVME_KIVEC_T1_S2S_DST_QTYPE             \
    k.nvme_kivec_t1_s2s_dst_qtype
#define NVME_KIVEC_T1_S2S_DST_QID               \
    k.{nvme_kivec_t1_s2s_dst_qid_sbit0_ebit1...nvme_kivec_t1_s2s_dst_qid_sbit18_ebit23}
#define NVME_KIVEC_T1_S2S_DST_QADDR             \
    k.{nvme_kivec_t1_s2s_dst_qaddr_sbit0_ebit1...nvme_kivec_t1_s2s_dst_qaddr_sbit2_ebit33}
#define NVME_KIVEC_T1_S2S_IOB_ADDR              \
    k.{nvme_kivec_t1_s2s_iob_addr_sbit0_ebit31...nvme_kivec_t1_s2s_iob_addr_sbit32_ebit33}
#define NVME_KIVEC_T1_S2S_IO_MAP_BASE_ADDR      \
    k.{nvme_kivec_t1_s2s_io_map_base_addr_sbit0_ebit5...nvme_kivec_t1_s2s_io_map_base_addr_sbit30_ebit33}
#define NVME_KIVEC_T1_S2S_PRP_ASSIST            \
    k.nvme_kivec_t1_s2s_prp_assist
#define NVME_KIVEC_T1_S2S_IS_REMOTE             \
    k.nvme_kivec_t1_s2s_is_remote
#define NVME_KIVEC_T1_S2S_IS_READ               \
    k.nvme_kivec_t1_s2s_is_read
#define NVME_KIVEC_T1_S2S_PUNT_TO_ARM           \
    k.nvme_kivec_t1_s2s_punt_to_arm

#define NVME_KIVEC_GLOBAL_SRC_LIF               \
    k.{nvme_kivec_global_src_lif_sbit0_ebit7...nvme_kivec_global_src_lif_sbit8_ebit10}
#define NVME_KIVEC_GLOBAL_SRC_QTYPE             \
    k.nvme_kivec_global_src_qtype
#define NVME_KIVEC_GLOBAL_SRC_QID               \
    k.{nvme_kivec_global_src_qid_sbit0_ebit1...nvme_kivec_global_src_qid_sbit18_ebit23}
#define NVME_KIVEC_GLOBAL_SRC_QADDR             \
    k.{nvme_kivec_global_src_qaddr_sbit0_ebit1...nvme_kivec_global_src_qaddr_sbit2_ebit33}
#define NVME_KIVEC_GLOBAL_NVME_DATA_LEN         \
    k.nvme_kivec_global_nvme_data_len
#define NVME_KIVEC_GLOBAL_IO_MAP_NUM_ENTRIES    \
    k.nvme_kivec_global_io_map_num_entries
#define NVME_KIVEC_GLOBAL_OPER_STATUS           \
    k.nvme_kivec_global_oper_status

#define NVME_KIVEC_SQ_INFO_IS_ADMIN_Q           \
    k.nvme_kivec_sq_info_is_admin_q

#define NVME_KIVEC_IOB_RING3_BASE_ADDR          \
    k.{nvme_kivec_iob_ring3_base_addr_sbit0_ebit7...nvme_kivec_iob_ring3_base_addr_sbit32_ebit33}

#define NVME_KIVEC_IOB_RING4_BASE_ADDR          \
    k.{nvme_kivec_iob_ring4_base_addr_sbit0_ebit31...nvme_kivec_iob_ring4_base_addr_sbit32_ebit33}

#define NVME_KIVEC_PRP_BASE_PRP0                \
    k.nvme_kivec_prp_base_prp0
#define NVME_KIVEC_PRP_BASE_PRP1                \
    k.nvme_kivec_prp_base_prp1


#define NVME_KIVEC_ARM_DST6_ARM_LIF             \
    k.{nvme_kivec_arm_dst6_arm_lif_sbit0_ebit7...nvme_kivec_arm_dst6_arm_lif_sbit8_ebit10}
#define NVME_KIVEC_ARM_DST6_ARM_QTYPE           \
    k.nvme_kivec_arm_dst6_arm_qtype
#define NVME_KIVEC_ARM_DST6_ARM_QID             \
    k.{nvme_kivec_arm_dst6_arm_qid_sbit0_ebit1...nvme_kivec_arm_dst6_arm_qid_sbit18_ebit23}
#define NVME_KIVEC_ARM_DST6_ARM_QADDR           \
    k.{nvme_kivec_arm_dst6_arm_qaddr_sbit0_ebit1...nvme_kivec_arm_dst6_arm_qaddr_sbit2_ebit33}


#define NVME_KIVEC_ARM_DST7_ARM_LIF             \
    k.{nvme_kivec_arm_dst7_arm_lif_sbit0_ebit7...nvme_kivec_arm_dst7_arm_lif_sbit8_ebit10}
#define NVME_KIVEC_ARM_DST7_ARM_QTYPE           \
    k.nvme_kivec_arm_dst7_arm_qtype
#define NVME_KIVEC_ARM_DST7_ARM_QID             \
    k.{nvme_kivec_arm_dst7_arm_qid_sbit0_ebit1...nvme_kivec_arm_dst7_arm_qid_sbit18_ebit23}
#define NVME_KIVEC_ARM_DST7_ARM_QADDR           \
    k.{nvme_kivec_arm_dst7_arm_qaddr_sbit0_ebit1...nvme_kivec_arm_dst7_arm_qaddr_sbit2_ebit33}


// TODO: Fix these to use the values defined in hardware
#define CAPRI_DMA_NOP               0
#define CAPRI_DMA_PHV2MEM           3
#define CAPRI_DMA_MEM2MEM           6
#define CAPRI_DMA_M2M_TYPE_SRC      0
#define CAPRI_DMA_M2M_TYPE_DST      1

#define CLEAR_TABLE_VALID_e(_num)                                       \
  phvwri.e  p.app_header_table##_num##_valid, 0;                        \
  nop;                                                                  \

#define LOAD_TABLE_NO_LKUP_PC_IMM_e(_num, _pc)                          \
  addi      r1, r0, _pc[33:6];                                          \
  phvwri    p.app_header_table##_num##_valid, 1;                        \
  phvwri.e  p.{common_te##_num##_phv_table_lock_en...                   \
               common_te##_num##_phv_table_raw_table_size},             \
              (0 << 3 | STORAGE_TBL_LOAD_SIZE_0_BITS);                  \
  phvwrpair p.common_te##_num##_phv_table_pc, r1,                       \
            p.common_te##_num##_phv_table_addr, r0;                     \
        
// Load table 0 based on absolute address
// table 0 is always the last, so it should be ok to use .e modifiers
// _pc is in register or in a dvec, AND
// _table_addr is 64 bits
// _load_size is 3 bits
#define LOAD_TABLE0_FOR_ADDR64(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table0_valid, 1;                               \
  phvwrpair.e p.common_te0_phv_table_lock_en, 1,                        \
            p.common_te0_phv_table_raw_table_size, _load_size;          \
  phvwrpair p.common_te0_phv_table_pc, _pc,                             \
            p.common_te0_phv_table_addr, _table_addr;                   \

// _table_addr is 34 bits
#define LOAD_TABLE0_FOR_ADDR34(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table0_valid, 1;                               \
  phvwrpair p.common_te0_phv_table_lock_en, 1,                          \
            p.common_te0_phv_table_raw_table_size, _load_size;          \
  phvwr.e   p.common_te0_phv_table_pc, _pc;                             \
  phvwr     p.common_te0_phv_table_addr, _table_addr;                   \
            
// Load table 1 based on absolute address
// table 1 is never the last, so it is NOT ok to use .e modifiers
// _pc is in register or in a dvec, AND
// _table_addr is 64 bits
// _load_size is 3 bits
#define LOAD_TABLE1_FOR_ADDR64(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table1_valid, 1;                               \
  phvwrpair p.common_te1_phv_table_lock_en, 1,                          \
        p.common_te1_phv_table_raw_table_size, _load_size;              \
  phvwrpair p.common_te1_phv_table_pc, _pc,                             \
        p.common_te1_phv_table_addr, _table_addr;                       \

#define LOAD_TABLE1_FOR_ADDR64_e(_table_addr, _load_size, _pc)          \
  phvwri    p.app_header_table1_valid, 1;                               \
  phvwrpair.e p.common_te1_phv_table_lock_en, 1,                        \
        p.common_te1_phv_table_raw_table_size, _load_size;              \
  phvwrpair p.common_te1_phv_table_pc, _pc,                             \
        p.common_te1_phv_table_addr, _table_addr;                       \

// _table_addr is 34 bits
#define LOAD_TABLE1_FOR_ADDR34(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table0_valid, 1;                               \
  phvwrpair p.common_te1_phv_table_lock_en, 1,                          \
        p.common_te1_phv_table_raw_table_size, _load_size;              \
  phvwr     p.common_te1_phv_table_pc, _pc;                             \
  phvwr     p.common_te1_phv_table_addr, _table_addr;                   \


// Load a table based on absolute address,
// where phvwrpair is unusable because
//    _pc is a param resolved by the loader, OR
//    _table_addr is not 64 bits
#define LOAD_TABLE_FOR_ADDR_PC_IMM(_table_addr, _load_size, _pc)        \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE0_FOR_ADDR64(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE1_FOR_ADDR_PC_IMM(_table_addr, _load_size, _pc)       \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE1_FOR_ADDR64(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE1_FOR_ADDR_PC_IMM_e(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE1_FOR_ADDR64_e(_table_addr, _load_size, r1)                 \

#define LOAD_TABLE_FOR_ADDR34_PC_IMM(_table_addr, _load_size, _pc)      \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE0_FOR_ADDR34(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE_FOR_ADDR(_table_addr, _load_size, _pc)               \
  LOAD_TABLE0_FOR_ADDR64(_table_addr, _load_size, _pc)                  \
            
#define LOAD_TABLE1_FOR_ADDR34_PC_IMM(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE1_FOR_ADDR34(_table_addr, _load_size, r1)                   \


// Calculate a table address based on index and size
// addr = _table_base + (_entry_index * (2 ^_entry_size))
#define TABLE_ADDR_FOR_INDEX(_table_base, _entry_index, _entry_size)    \
  sll       r7, _entry_index, _entry_size;                              \
  add       r7, r7, _table_base;                                        \

// Load a table based with a calculation based on index
// addr = _table_base + (_entry_index * (2 ^_entry_size))
// PC input must be a 28-bit value
#define LOAD_TABLE_FOR_INDEX(_table_base, _entry_index, _entry_size,    \
                             _load_size, _pc)                           \
  TABLE_ADDR_FOR_INDEX(_table_base, _entry_index, _entry_size)          \
  LOAD_TABLE_FOR_ADDR(r7, _load_size, _pc)                              \

// Load a table based with a calculation based on index
// If index is passed as param it should be in GPR r7
// PC input is a .param resolved by the loader (34-bit value)
#define LOAD_TABLE_FOR_INDEX_PARAM(_table_base, _entry_index,           \
                                   _entry_size, _load_size, _pc)        \
  addi      r4, r0, _pc[33:6];                                          \
  LOAD_TABLE_FOR_INDEX(_table_base, _entry_index, _entry_size,          \
                       _load_size, r4)                                  \

// Load a table based with a calculation based on index and priority
// addr = _table_base + ((_entry_index + ( _pri * (2 ^ _num_entries))) 
//                       * (2 ^_entry_size))
// PC input must be a 28-bit value
#define LOAD_TABLE_FOR_PRI_INDEX(_table_base, _entry_index,             \
                                 _num_entries, _pri, _entry_size,       \
                                 _load_size, _pc)                       \
  sll       r3, _pri, _num_entries;                                     \
  add       r3, r3, _entry_index;                                       \
  sll       r3, r3, _entry_size;                                        \
  add       r1, r3, _table_base;                                        \
  LOAD_TABLE_FOR_ADDR(r1, _load_size, _pc)                              \

// Used to clear table 1 valid bits 
#define CLEAR_TABLE1                                                    \
  phvwri.e  p.app_header_table1_valid, 0;   				\
  nop;                                                                  \

// Used to clear all table valid bits and exit the pipeline
#define LOAD_NO_TABLES                                                  \
  phvwri.e  p.{app_header_table0_valid...app_header_table3_valid}, 0;   \
  nop;                                                                  \


// Capri PHV Bit to Byte Macros

#define CAPRI_PHV_FLIT_SIZE_BITS        512
#define CAPRI_PHV_FLIT_SIZE_BYTES       (CAPRI_PHV_FLIT_SIZE_BITS / 8)
#define CAPRI_PHV_BIT_TO_BYTE(x)                                        \
        (((x) / CAPRI_PHV_FLIT_SIZE_BITS) * CAPRI_PHV_FLIT_SIZE_BYTES) +\
         (CAPRI_PHV_FLIT_SIZE_BYTES) - 1 - (((x) % CAPRI_PHV_FLIT_SIZE_BITS) / 8)


// Phv2Mem DMA: Specify the address of the start and end fields in the PHV
//              and the destination address. Can specify 0 destination
//              address via GPR r0, which be update later.
// 
// _start and _end must resolve to 9-bit literals.
#define DMA_PHV2MEM_SETUP(_start, _end, _addr, _dma_cmd_X)              \
   phvwrpair p._dma_cmd_X##_dma_cmd_addr, _addr,                        \
             p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_PHV2MEM;            \
   phvwrpair p._dma_cmd_X##_dma_cmd_phv_end_addr,                       \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _end)),               \
             p._dma_cmd_X##_dma_cmd_phv_start_addr,                     \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +             \
                                      sizeof(p._start) - 1);            \
   phvwr    p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63];             \

// _start and _end can resolve to literals larger than 9 bits.
// _addr is a 64-bit value.
#define DMA_PHV2MEM_SETUP_ADDR64(_start, _end, _addr, _dma_cmd_X)       \
   phvwrpair p._dma_cmd_X##_dma_cmd_addr, _addr,                        \
             p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_PHV2MEM;            \
   phvwri    p._dma_cmd_X##_dma_cmd_phv_end_addr,                       \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _end));               \
   phvwri    p._dma_cmd_X##_dma_cmd_phv_start_addr,                     \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +             \
                                      sizeof(p._start) - 1);            \
   phvwr     p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63];            \
   
// _start and _end can resolve to literals larger than 9 bits
// _addr is less than 64-bit in size
#define DMA_PHV2MEM_SETUP_ADDR34(_start, _end, _addr, _dma_cmd_X)       \
   phvwrpair p._dma_cmd_X##_dma_cmd_addr[33:0], _addr,                  \
             p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_PHV2MEM;            \
   phvwri    p._dma_cmd_X##_dma_cmd_phv_end_addr,                       \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _end));               \
   phvwri    p._dma_cmd_X##_dma_cmd_phv_start_addr,                     \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +             \
                                      sizeof(p._start) - 1);            \
   
// Mem2Mem DMA:
// _addr is given in a register
#define DMA_MEM2MEM_SETUP_REG_ADDR(_type, _addr, _size,                 \
                                    _use_override_lif, _override_lif,   \
                                    _dma_cmd_X)                         \
   phvwrpair p._dma_cmd_X##_dma_cmd_mem2mem_type, _type,                \
             p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_MEM2MEM;            \
   phvwrpair p._dma_cmd_X##_dma_cmd_override_lif, _override_lif,        \
             p._dma_cmd_X##_dma_cmd_use_override_lif,_use_override_lif; \
   phvwrpair p._dma_cmd_X##_dma_cmd_size, _size,                        \
             p._dma_cmd_X##_dma_cmd_addr, _addr;                        \
   phvwr     p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63];            \

#define DMA_MEM2MEM_SETUP(_type, _addr, _size, _use_override_lif,       \
                          _override_lif, _dma_cmd_X)                    \
   add      r1, r0, _addr;                                              \
   DMA_MEM2MEM_SETUP_REG_ADDR(_type, r1, _size, _use_override_lif,      \
                              _override_lif, _dma_cmd_X)                \
   
// DMA fence update: Set the fence bit for the MEM2MEM DMA command
#define DMA_MEM2MEM_FENCE(_dma_cmd_X)                                   \
   phvwri   p._dma_cmd_X##_dma_cmd_wr_fence, 1;                         \

// DMA fence update: Set the fence bit for the PHV2MEM DMA command
#define DMA_PHV2MEM_FENCE(_dma_cmd_X)                                   \
   phvwri   p._dma_cmd_X##_dma_cmd_wr_fence, 1;                         \


// DMA address update: Specify the destination address for the DMA command
// _addr is given in a register
#define DMA_ADDR_UPDATE(_addr, _dma_cmd_X)                              \
   phvwr    p._dma_cmd_X##_dma_cmd_addr, _addr;                         \
   phvwr    p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63];             \
   
// Setup the start and end DMA pointers
#define DMA_PTR_SETUP(_start, _dma_cmd_eop, _dma_cmd_ptr)               \
   phvwri   p._dma_cmd_ptr,                                             \
                ((CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +           \
                                        sizeof(p._start) - 1))/16);     \
   phvwri   p._dma_cmd_eop, 1;                                          \

// Cancel a previously set DMA descriptor
#define DMA_CMD_CANCEL(_dma_cmd_X)                                      \
   phvwri   p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_NOP;                 \

// Setup the doorbell data. Write back the data in little endian format
#define DOORBELL_DATA_SETUP(_db_data, _index, _ring, _qid, _pid)        \
   add      r1, _index, _ring, DOORBELL_DATA_RING_SHIFT;                \
   add      r1, r1, _qid, DOORBELL_DATA_QID_SHIFT;                      \
   add      r1, r1, _pid, DOORBELL_DATA_PID_SHIFT;                      \
   phvwr    p._db_data, r1.dx;                                          \

// Setup the doorbell address. Output will be stored in GPR r7.
#define DOORBELL_ADDR_SETUP(_lif, _qtype, _sched_wr, _upd)              \
   addi     r7, r0, DOORBELL_ADDR_WA_LOCAL_BASE + _sched_wr + _upd;     \
   add      r7, r7, _qtype, DOORBELL_ADDR_QTYPE_SHIFT;                  \
   add      r7, r7, _lif, DOORBELL_ADDR_LIF_SHIFT;                      \


// Clear the doorbell as there was no work to be done. Note index can
// be 0 (r0) as there is no update.
#define QUEUE_DOORBELL_CLEAR(_ring, _wr_sched, _lif, _qtype, _qid)      \
   DOORBELL_DATA_SETUP(qpop_doorbell_data_data, r0, _ring, _qid, r0)    \
   DOORBELL_ADDR_SETUP(_lif, _qtype, _wr_sched, DOORBELL_UPDATE_NONE)   \
   DMA_PHV2MEM_SETUP(qpop_doorbell_data_data, qpop_doorbell_data_data,  \
                     r7, dma_p2m_0)                                     \

// Queue pop doorbell clear is done in two stages:
// 1. table write of w_ndx to c_ndx (this should make p_ndx == c_ndx)
// 2. scheduler bit clear with eval (this would eval p_ndx == c_ndx)
#define QUEUE_POP_DOORBELL_CLEAR                                        \
   tblwr    d.c_ndx, d.w_ndx;                                           \
   QUEUE_DOORBELL_CLEAR(r0, DOORBELL_SCHED_WR_EVAL,                     \
                        STAGE0_KIVEC_LIF,                               \
                        STAGE0_KIVEC_QTYPE,                             \
                        STAGE0_KIVEC_QID)                               \

// Queue pop doorbell clear is done in two stages:
// 1. table write of w_ndx to c_ndx 
// 2. scheduler bit clear with reset (eval won't work as p_ndx may not
//    be equal to c_ndx)
#define QUEUE_POP_DOORBELL_CLEAR_RESET                                  \
   tblwr    d.c_ndx, d.w_ndx;                                           \
   QUEUE_DOORBELL_CLEAR(r0, DOORBELL_SCHED_WR_RESET,                    \
                        STAGE0_KIVEC_LIF,                               \
                        STAGE0_KIVEC_QTYPE,                             \
                        STAGE0_KIVEC_QID)                               \

// Queue pop doorbell clear is done in two stages:
// 1. table write of w_ndx to c_ndx for all priorities 
//    (this should make p_ndx == c_ndx for all priorities)
// 2. scheduler bit clear with reset (eval won't work as p_ndx may not
//    be equal to c_ndx for all priorities)
#define PRI_QUEUE_POP_DOORBELL_CLEAR_RESET                              \
   tblwr d.c_ndx_lo, d.w_ndx_lo;                                        \
   tblwr d.c_ndx_med, d.w_ndx_med;                                      \
   tblwr d.c_ndx_hi, d.w_ndx_hi;                                        \
   QUEUE_DOORBELL_CLEAR(r0, DOORBELL_SCHED_WR_RESET,                    \
                        STAGE0_KIVEC_LIF,                               \
                        STAGE0_KIVEC_QTYPE,                             \
                        STAGE0_KIVEC_QID)                               \

// Queue pop doorbell clear is done in two stages:
// 1. table write of w_ndx to c_ndx (this should make p_ndx == c_ndx)
// 2. scheduler bit clear with eval (this would eval p_ndx == c_ndx)
#define NVME_QUEUE_POP_DOORBELL_CLEAR                                   \
   tblwr d.c_ndx, d.w_ndx;                                              \
   QUEUE_DOORBELL_CLEAR(r0, DOORBELL_SCHED_WR_RESET,                    \
                        NVME_KIVEC_GLOBAL_SRC_LIF,                      \
                        NVME_KIVEC_GLOBAL_SRC_QTYPE,                    \
                        NVME_KIVEC_GLOBAL_SRC_QID)                      \

// Setup the lif, type, qid, pindex for the doorbell push.  Set the fence 
// bit for the doorbell 
#define _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, _sched, _upd, _p_ndx,   \
                                  _lif, _qtype, _qid)                   \
   DOORBELL_DATA_SETUP(qpush_doorbell_data_data, _p_ndx, r0, _qid, r0)  \
   DOORBELL_ADDR_SETUP(_lif, _qtype, _sched, _upd)                      \
   DMA_PHV2MEM_SETUP(qpush_doorbell_data_data,qpush_doorbell_data_data, \
                     r7, _dma_cmd_ptr)                                  \
   DMA_PHV2MEM_FENCE(_dma_cmd_ptr)                                      \

#define QUEUE_PUSH_DOORBELL_RING(_dma_cmd_ptr)                          \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_NONE, d.p_ndx,             \
                             STORAGE_KIVEC0_DST_LIF,                    \
                             STORAGE_KIVEC0_DST_QTYPE,                  \
                             STORAGE_KIVEC0_DST_QID)                    \

#define QUEUE_PUSH_DOORBELL_UPDATE_RING(_dma_cmd_ptr, _p_ndx)           \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_P_NDX, _p_ndx,             \
                             STORAGE_KIVEC0_DST_LIF,                    \
                             STORAGE_KIVEC0_DST_QTYPE,                  \
                             STORAGE_KIVEC0_DST_QID)                    \

#define ROCE_QUEUE_PUSH_DOORBELL_RING(_dma_cmd_ptr)                     \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_P_NDX, d.p_ndx,            \
                             d.rsq_lif, d.rsq_qtype, d.rsq_qid)         \


#define NVME_SEQ_QUEUE_PUSH_DOORBELL_RING(_dma_cmd_ptr)                 \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_NONE, d.p_ndx,             \
                             NVME_KIVEC_T0_S2S_DST_LIF,                 \
                             NVME_KIVEC_T0_S2S_DST_QTYPE,               \
                             NVME_KIVEC_T0_S2S_DST_QID)                 \

// Setup the lif, type, qid, ring, pindex for the doorbell push. The I/O
// priority is used to select the ring. Set the fence bit for the doorbell.
#define _PRI_QUEUE_PUSH_DOORBELL_UPDATE(_dma_cmd_ptr, _p_ndx, _sched,   \
                                        _qid, _pri)                     \
   DOORBELL_DATA_SETUP(qpush_doorbell_data_data, _p_ndx,                \
                       _pri, _qid, r0)                                  \
   DOORBELL_ADDR_SETUP(STORAGE_KIVEC0_DST_LIF,                          \
                       STORAGE_KIVEC0_DST_QTYPE,                        \
                       _sched, DOORBELL_UPDATE_P_NDX)                   \
   DMA_PHV2MEM_SETUP(qpush_doorbell_data_data,qpush_doorbell_data_data, \
                     r7, _dma_cmd_ptr)                                  \
   DMA_PHV2MEM_FENCE(_dma_cmd_ptr)                                      \

// Setup the sequencer doorbell based on the lif, type, qid specified in
// the WQE and ring it.
#define SEQUENCER_DOORBELL_FORM_UPDATE_RING(_dma_cmd_ptr)               \
   DOORBELL_DATA_SETUP(seq_doorbell_data_data, d.db_index, r0,          \
                       d.db_qid, r0)                                    \
   DOORBELL_ADDR_SETUP(d.db_lif, d.db_qtype, DOORBELL_SCHED_WR_SET,     \
                       DOORBELL_UPDATE_P_NDX)                           \
   DMA_PHV2MEM_SETUP(seq_doorbell_data_data, seq_doorbell_data_data,    \
                     r7, _dma_cmd_ptr)                                  \

// Ring the sequencer doorbell based on the data provided in the 
// d-vector. Fence the interrupt with previous DMA writes.
#define SEQUENCER_DOORBELL_RING(_dma_cmd_ptr)                           \
   phvwr    p.seq_doorbell_data_data, d.next_db_data;                   \
   add      r7, r0, d.next_db_addr;                                     \
   DMA_PHV2MEM_SETUP(seq_doorbell_data_data, seq_doorbell_data_data,    \
                     r7, _dma_cmd_ptr)                                  \
   DMA_PHV2MEM_FENCE(_dma_cmd_ptr)                                      \

// Set the data to be pushed across the PCI layer to be the p_ndx. Issue
// DMA write of this data to the address in the d-vector. Set the fence
// bit for the data push.
#define PCI_QUEUE_PUSH_DATA_UPDATE(_dma_cmd_ptr)                        \
   add      r1, r0, d.p_ndx;                                            \
   phvwr    p.pci_push_data_data, r1.wx;                                \
   add      r7, r0, d.push_addr;                                        \
   DMA_PHV2MEM_SETUP(pci_push_data_data, pci_push_data_data,            \
                     r7, _dma_cmd_ptr)                                  \
   DMA_PHV2MEM_FENCE(_dma_cmd_ptr)                                      \


// Raise an interrupt across the PCI layer by using the address and data 
// specified in the d-vector. Fence the interrupt with previous 
// DMA writes.
#define PCI_SET_INTERRUPT_DATA()                                        \
   phvwr    p.pci_intr_data_data, d.{intr_data}.wx;                     \
   
#define PCI_SET_INTERRUPT_ADDR_DMA(_intr_addr, _dma_cmd_ptr)            \
   add      r7, r0, _intr_addr;                                         \
   DMA_PHV2MEM_SETUP(pci_intr_data_data, pci_intr_data_data,            \
                     r7, _dma_cmd_ptr)                                  \
   DMA_PHV2MEM_FENCE(_dma_cmd_ptr)                                      \

#define PCI_RAISE_INTERRUPT(_dma_cmd_ptr)                               \
   PCI_SET_INTERRUPT_DATA()                                             \
   PCI_SET_INTERRUPT_ADDR_DMA(d.intr_addr, _dma_cmd_ptr)                \
   
// Queue full check based on an increment value
#define _QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, _incr, _branch_instr) \
   add      r1, r0, _p_ndx;                                             \
   mincr    r1, _num_entries, _incr;                                    \
   seq      c1, r1, _c_ndx;                                             \
   bcf      [c1], _branch_instr;                                        \
   nop;                                                                 \

#define QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, _branch_instr)         \
   _QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, 1, _branch_instr)

#define QUEUE_FULL2(_p_ndx, _c_ndx, _num_entries, _branch_instr)        \
   _QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, 2, _branch_instr)

// Queue empty check
#define QUEUE_EMPTY(_p_ndx, _w_ndx, _branch_instr)                      \
   seq      c1, _p_ndx, _w_ndx;                                         \
   bcf      [c1], _branch_instr;                                        \
   nop;                                                                 \

// Queue pop based on an increment value
#define QUEUE_POP_INCR(_w_ndx, _num_entries, _incr)                     \
   tblmincri    _w_ndx, _num_entries, _incr;                            \

// Queue push (default increment value of 1)
#define QUEUE_POP(_w_ndx, _num_entries)                                 \
   QUEUE_POP_INCR(_w_ndx, _num_entries, 1)

// Queue push based on an increment value
#define QUEUE_PUSH_INCR(_p_ndx, _num_entries, _incr)                    \
   tblmincri    _p_ndx, _num_entries, _incr;                            \

// Queue push (default increment value of 1)
#define QUEUE_PUSH(_p_ndx, _num_entries)                                \
   QUEUE_PUSH_INCR(_p_ndx, _num_entries, 1)

// Get the address to push the entry to. Return value is in GPR r7.
#define QUEUE_PUSH_ADDR(_base_addr, _p_ndx, _entry_size)                \
   sll      r2, _p_ndx, _entry_size;                                    \
   add      r7, r2, _base_addr;                                         \

// Priority queue pop check - based on queue empty AND 
// priority counter < priority weight
#define PRI_QUEUE_CAN_POP(_p_ndx, _w_ndx, _pri_running, _pri_weight,    \
                          _branch_instr)                                \
   QUEUE_EMPTY(_p_ndx, _w_ndx, _branch_instr)                           \
   slt      c1, _pri_running, _pri_weight;                              \
   bcf      ![c1], _branch_instr;                                       \
   nop;                                                                 \

// Service the priority queue by popping the entry and setting up the
// next stage to handle the entry. The w_ndx to be used is saved in 
// GPR r6 for use later as the tblmincr alters the d-vector.
#define SERVICE_PRI_QUEUE(_w_ndx_pri, _pri_val)                         \
   add      r6, r0, _w_ndx_pri;                                         \
   QUEUE_POP(_w_ndx_pri, d.num_entries)                                 \
   phvwrpair p.storage_kivec0_w_ndx, _w_ndx_pri,                        \
             p.storage_kivec0_io_priority, _pri_val;                    \
   LOAD_TABLE_FOR_PRI_INDEX(d.base_addr, r6, d.num_entries, _pri_val,   \
                            d.entry_size, d.entry_size[2:0], d.next_pc) \
   
   
// Derive the priority queue push address and store it in register r7
// addr = base_addr + (entry_size * priority * num_entries) + (entry_size * p_ndx)
// Both entry_size and num_entries are to used as powers of 2.
#define PRI_QUEUE_PUSH_ADDR(_pri, _base_addr, _p_ndx, _num_entries,     \
                            _entry_size)                                \
   sll      r2, _pri, _num_entries;                                     \
   add      r2, r2, _p_ndx;                                             \
   sll      r2, r2, _entry_size;                                        \
   add      r7, r2, _base_addr;                                         \

// Pushing into a priority queue:
// 1. Check and branch if the priority queue is full.
// 2. Calculate the address to which the command has to be written to in the
//    priority queue. Output will be stored in GPR r7.
// 3. Update DMA command 1 with the address stored in GPR r7.
// 4. Push the entry to the queue (this increments p_ndx and writes to table).
// 5. Form and ring the doorbell for the recipient of the push. 
#define PRI_QUEUE_PUSH(_pri_vec, _pri_val, _p_ndx, _c_ndx, _base_addr,  \
                       _num_entries, _entry_size, _branch_instr1,       \
                       _branch_instr2)                                  \
   sne      c1, _pri_vec, _pri_val;                                     \
   bcf      [c1], _branch_instr1;                                       \
   QUEUE_FULL(_p_ndx, _c_ndx, _num_entries, _branch_instr2)             \
   PRI_QUEUE_PUSH_ADDR(_pri_vec, _base_addr, _p_ndx, _num_entries,      \
                       _entry_size)                                     \
   DMA_ADDR_UPDATE(r7, dma_p2m_1)                                       \
   QUEUE_PUSH(_p_ndx, _num_entries)                                     \
   add      r5, STORAGE_KIVEC0_DST_QID, STORAGE_KIVEC0_SSD_HANDLE;      \
   _PRI_QUEUE_PUSH_DOORBELL_UPDATE(dma_p2m_3, _p_ndx,                   \
                                   DOORBELL_SCHED_WR_SET, r5, _pri_vec) \


// Increment the priority running counter and the total running counter.
// This writes back the updated values to the table.
#define PRI_QUEUE_INCR(_pri_vec, _pri_val, _pri_ctr, _tot_ctr,          \
                       _branch_instr)                                   \
   sne      c1, _pri_vec, _pri_val;                                     \
   bcf      [c1], _branch_instr;                                        \
   addi     r1, r0, 1;                                                  \
   tbladd   _pri_ctr, r1;                                               \
   tbladd   _tot_ctr, r1;                                               \

// Decrement the priority running counter and the total running counter.
// This writes back the updated values to the table.
#define PRI_QUEUE_DECR(_pri_vec, _pri_val, _pri_ctr, _tot_ctr,          \
                       _branch_instr)                                   \
   sne      c1, _pri_vec, _pri_val;                                     \
   bcf      [c1], _branch_instr;                                        \
   addi     r1, r0, 1;                                                  \
   tblsub   _pri_ctr, r1;                                               \
   tblsub   _tot_ctr, r1;                                               \

// Check if PRP assist can be done based on command parameters like opcode,
// data size, max assist size etc.
#define PRP_ASSIST_CHECK(_is_q0, _opc, _pdst, _nlb, _dptr1, _dptr2,     \
                         _branch_instr)                                 \
   sne      c1, r0, _is_q0;                                             \
   sne      c2, _opc, NVME_READ_CMD_OPCODE;                             \
   sne      c3, _opc, NVME_WRITE_CMD_OPCODE;                            \
   seq      c4, _pdst, r0;                                              \
   sll      r1, _nlb, LB_SIZE_SHIFT;                                    \
   addi     r1, r1, 1;                                                  \
   sle      c5, r1, MAX_ASSIST_SIZE;                                    \
   addi     r2, r0, PRP_SIZE_SUB;                                       \
   and      r5, _dptr1, PRP_SIZE_MASK;                                  \
   sub      r5, r2, r5;                                                 \
   and      r3, _dptr2, PRP_SIZE_MASK;                                  \
   sub      r3, r2, r3;                                                 \
   add      r7, r5, r3;                                                 \
   sle      c6, r7, MAX_ASSIST_SIZE;                                    \
   bcf      ![c1 | c2 | c3 | c4 | c5 | c6], _branch_instr;              \
   nop;                                                                 \

#define R2N_WQE_BASE_COPY                                               \
   phvwr    p.{r2n_wqe_handle...r2n_wqe_dst_qaddr},                     \
            d.{handle...dst_qaddr};                                     \

#define R2N_WQE_FULL_COPY                                               \
   phvwr    p.{r2n_wqe_handle...r2n_wqe_nvme_cmd_cid},                  \
            d.{handle...nvme_cmd_cid};                                  \
   phvwr    p.{r2n_wqe_pri_qaddr...r2n_wqe_pad},                        \
            d.{pri_qaddr...pad};                                        \

// Calculate the table address based on the command index offset into
// the SSD's list of outstanding commands
// address = (SSD_CMDS_HEADER_SIZE + (cmd_index * SSD_CMDS_ENTRY_SIZE))
// Input: cmd_index stored in GRP r6. Output: Address tored in GPR r7
#define SSD_CMD_ENTRY_ADDR_CALC                                         \
   add      r7, STORAGE_KIVEC0_SSD_BM_ADDR, SSD_CMDS_HEADER_SIZE;       \
   add      r7, r7, r6, SSD_CMDS_ENTRY_SHIFT;                           \
   
// Setup the source of the mem2mem DMA into DMA cmd 1:
//   Src Addr:  ROCE RQ WQE address calculated based on the R2N buffer 
//              passed in the R2N WQE.  
//   Length:     Fixed length of ROCE RQ WQE
//   For now, not using any override LIF parameters.
//
// Setup the destination of the mem2mem DMA into DMA cmd 2 (just fill
// the size, address will be filled by the push operation). 
//
#define R2N_BUF_POST_SETUP(_addr)                                       \
   sub      r4, _addr, R2N_BUF_NVME_BE_CMD_OFFSET;                      \
   addi     r5, r0, ROCE_RQ_WQE_SIZE;                                   \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, r4, r5, r0, r0, dma_m2m_1) \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, r0, r5, r0, r0, dma_m2m_2) \
   
// Setup the compression data buffer DMA based on flat source buffer 
// and destination SGL (processing one SGL entry in this macro).
// Notes: These GPRs are used for input/output to/from this macro
//  1. r6 stores the running count of data remaining to be xfered
//  2. r7 stores the offeset of the source data buffer from where
//     the current xfer is to be done.
// Steps:
//  1. Adjust data len (0 => 64L xfer) and store it in r4
//  2. Setup the DMA size based on the min size in r4 vs r6. 
//  3. Source of the DMA is stored in r7.
//  4. Destination of the DMA is based on the address in SGL.
//  5. Update data remaining (r6) and address offset (r7) 
//  6. If data xfer is complete, jump to the branch instruction
//  7. Use a branch delay slot of nop to avoid spurious updates after 
//     this macro is invoked 
#define COMP_SGL_DMA(_dma_cmd_ptr_src, _dma_cmd_ptr_dst, _addr, _len,   \
                     _branch_instr)                                     \
   seq      c1, _len, r0;                                               \
   cmov     r4, c1, 65536, _len;                                        \
   sle      c2, r6, r4;                                                 \
   add.c2   r4, r0, r6;                                                 \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, r7, r4, 0, 0,              \
                     _dma_cmd_ptr_src)                                  \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, _addr, r4, 0, 0,           \
                     _dma_cmd_ptr_dst)                                  \
   add      r7, r7, r4;                                                 \
   bcf      [c2], _branch_instr;                                        \
   sub      r6, r6, r4;                                                 \


#define NVME_DATA_XFER_FROM_HOST(_prp_entry, _dst_addr, _data_len,      \
                                 _xfer_len,  _src_dma_cmd, _dst_dma_cmd,\
                                 _branch_instr)                         \
   sne      c1, _prp_entry, 0;                                          \
   slt      c2, _xfer_len, _data_len;                                   \
   bcf      [!c1 | !c2], _branch_instr;                                 \
   sub      r3, _data_len, _xfer_len;                                   \
   slt      c3, r3, PRP_DATA_XFER_SIZE;                                 \
   add.c3   r3, r0, PRP_DATA_XFER_SIZE;                                 \
   add      r4, _dst_addr, _xfer_len;                                   \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, _prp_entry, r3, 0, 0,      \
                     _src_dma_cmd)                                      \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, r4, r3, 0, 0,              \
                     _dst_dma_cmd)                                      \
   add      _xfer_len, _xfer_len, r3;                                   \
   

#define NVME_DATA_XFER_TO_HOST(_prp_entry, _dst_addr, _data_len,        \
                               _xfer_len,  _src_dma_cmd, _dst_dma_cmd,  \
                               _branch_instr)                           \
   sne      c1, _prp_entry, 0;                                          \
   slt      c2, _xfer_len, _data_len;                                   \
   bcf      [!c1 | !c2], _branch_instr;                                 \
   sub      r3, _data_len, _xfer_len;                                   \
   slt      c3, r3, PRP_DATA_XFER_SIZE;                                 \
   add.c3   r3, r0, PRP_DATA_XFER_SIZE;                                 \
   add      r4, _dst_addr, _xfer_len;                                   \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, r4, r3, 0, 0,              \
                     _src_dma_cmd)                                      \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, _prp_entry, r3, 0, 0,      \
                     _dst_dma_cmd)                                      \
   add      _xfer_len, _xfer_len, r3;                                   \


#endif     // STORAGE_ASM_DEFINES_H
