/*****************************************************************************
 *  storage_asm_defines.h: Macros for common functions used across various 
 *                         storage ASMs. GPR r7 is strictly to be used for 
 *                         input/output to macros to carry over state 
 *                         information back and forth. GPR r6 is not to be 
 *                         used as it is used for storing state information
 *                         in programs. GPRs r1..r5 can be used freely.
 *****************************************************************************/

#ifndef __STORAGE_ASM_DEFINES_H__
#define __STORAGE_ASM_DEFINES_H__

#include "storage_common_defines.h"
#include "storage_seq_common.h"

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE                   8
#define SIZE_IN_BITS(bytes)             ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)             ((bits) / BITS_PER_BYTE)
#endif

#define STORAGE_PHYS_ADDR_HOST_POS      63

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
    k.{storage_kivec0_dst_qaddr_sbit0_ebit1...storage_kivec0_dst_qaddr_sbit2_ebit33}
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
    k.{storage_kivec1_src_qaddr_sbit0_ebit1...storage_kivec1_src_qaddr_sbit2_ebit33}
#define STORAGE_KIVEC1_DEVICE_ADDR              \
    k.{storage_kivec1_device_addr_sbit0_ebit31...storage_kivec1_device_addr_sbit32_ebit33}
#define STORAGE_KIVEC1_ROCE_CQ_NEW_CMD          \
    k.storage_kivec1_roce_cq_new_cmd
#define STORAGE_KIVEC1_ROCE_POST_BUF            \
    k.storage_kivec1_roce_post_buf

#define STORAGE_KIVEC2_SSD_Q_NUM                \
    k.storage_kivec2_ssd_q_num
#define STORAGE_KIVEC2_SSD_Q_SIZE               \
    k.storage_kivec2_ssd_q_size

#define STORAGE_KIVEC3_ROCE_MSN                 \
    k.storage_kivec3_roce_msn
#define STORAGE_KIVEC3_DATA_ADDR                \
    k.storage_kivec3_data_addr


#define STORAGE_KIVEC6_SSD_CI_ADDR              \
    k.storage_kivec6_ssd_ci_addr
    
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
#define NVME_KIVEC_ARM_DST6_RRQ_DESC_ADDR       \
    k.{nvme_kivec_arm_dst6_rrq_desc_addr_sbit0_ebit31...nvme_kivec_arm_dst6_rrq_desc_addr_sbit32_ebit33}


#define NVME_KIVEC_ARM_DST7_ARM_LIF             \
    k.{nvme_kivec_arm_dst7_arm_lif_sbit0_ebit7...nvme_kivec_arm_dst7_arm_lif_sbit8_ebit10}
#define NVME_KIVEC_ARM_DST7_ARM_QTYPE           \
    k.nvme_kivec_arm_dst7_arm_qtype
#define NVME_KIVEC_ARM_DST7_ARM_QID             \
    k.{nvme_kivec_arm_dst7_arm_qid_sbit0_ebit1...nvme_kivec_arm_dst7_arm_qid_sbit18_ebit23}
#define NVME_KIVEC_ARM_DST7_ARM_QADDR           \
    k.{nvme_kivec_arm_dst7_arm_qaddr_sbit0_ebit1...nvme_kivec_arm_dst7_arm_qaddr_sbit2_ebit33}
#define NVME_KIVEC_ARM_DST7_RRQ_DESC_ADDR       \
    k.{nvme_kivec_arm_dst7_rrq_desc_addr_sbit0_ebit31...nvme_kivec_arm_dst7_rrq_desc_addr_sbit32_ebit33}


#define SEQ_KIVEC1_SRC_LIF                      \
    k.{seq_kivec1_src_lif_sbit0_ebit7...seq_kivec1_src_lif_sbit8_ebit10}
#define SEQ_KIVEC1_SRC_QTYPE                    \
    k.seq_kivec1_src_qtype
#define SEQ_KIVEC1_SRC_QID                      \
    k.{seq_kivec1_src_qid_sbit0_ebit1...seq_kivec1_src_qid_sbit18_ebit23}
#define SEQ_KIVEC1_SRC_QADDR                    \
    k.{seq_kivec1_src_qaddr_sbit0_ebit1...seq_kivec1_src_qaddr_sbit26_ebit33}

#define SEQ_KIVEC2_SGL_PDMA_DST_ADDR            \
    k.seq_kivec2_sgl_pdma_dst_addr
#define SEQ_KIVEC2_SGL_VEC_ADDR                 \
    k.seq_kivec2_sgl_vec_addr

#define SEQ_KIVEC2XTS_SGL_PDMA_DST_ADDR         \
    k.seq_kivec2xts_sgl_pdma_dst_addr
#define SEQ_KIVEC2XTS_DECR_BUF_ADDR             \
    k.seq_kivec2xts_decr_buf_addr
    
#define SEQ_KIVEC3_COMP_BUF_ADDR                \
    k.seq_kivec3_comp_buf_addr
#define SEQ_KIVEC3_PAD_LEN                      \
    k.seq_kivec3_pad_len
#define SEQ_KIVEC3_LAST_BLK_LEN                 \
    k.{seq_kivec3_last_blk_len}
#define SEQ_KIVEC3_NUM_BLKS                     \
    k.seq_kivec3_num_blks
#define SEQ_KIVEC3_SGL_TUPLE_NO                 \
    k.seq_kivec3_sgl_tuple_no
#define SEQ_KIVEC3_PAD_BOUNDARY_SHIFT           \
    k.{seq_kivec3_pad_boundary_shift_sbit0_ebit0...seq_kivec3_pad_boundary_shift_sbit1_ebit4}

#define SEQ_KIVEC3XTS_DECR_BUF_ADDR             \
    k.seq_kivec3xts_decr_buf_addr

#define SEQ_KIVEC4_BARCO_DESC_ADDR              \
    k.{seq_kivec4_barco_desc_addr_sbit0_ebit7...seq_kivec4_barco_desc_addr_sbit40_ebit63}
#define SEQ_KIVEC4_BARCO_RING_ADDR              \
    k.{seq_kivec4_barco_ring_addr_sbit0_ebit7...seq_kivec4_barco_ring_addr_sbit32_ebit33}
#define SEQ_KIVEC4_BARCO_DESC_SIZE              \
    k.seq_kivec4_barco_desc_size
#define SEQ_KIVEC4_BARCO_PNDX_SHADOW_ADDR       \
    k.{seq_kivec4_barco_pndx_shadow_addr_sbit0_ebit5...seq_kivec4_barco_pndx_shadow_addr_sbit30_ebit33}
#define SEQ_KIVEC4_BARCO_PNDX_SIZE              \
    k.seq_kivec4_barco_pndx_size
#define SEQ_KIVEC4_BARCO_RING_SIZE              \
    k.seq_kivec4_barco_ring_size
#define SEQ_KIVEC4_BARCO_NUM_DESCS              \
    k.{seq_kivec4_barco_num_descs_sbit0_ebit7...seq_kivec4_barco_num_descs_sbit8_ebit9}
#define SEQ_KIVEC4_PAD_BOUNDARY_SHIFT           \
    k.seq_kivec4_pad_boundary_shift

#define SEQ_KIVEC5_SRC_QADDR                    \
    k.{seq_kivec5_src_qaddr_sbit0_ebit7...seq_kivec5_src_qaddr_sbit32_ebit33}
#define SEQ_KIVEC5_DATA_LEN                     \
    k.{seq_kivec5_data_len_sbit0_ebit3...seq_kivec5_data_len_sbit12_ebit16}
#define SEQ_KIVEC5_ALT_DATA_LEN                 \
    k.{seq_kivec5_alt_data_len_sbit0_ebit2...seq_kivec5_alt_data_len_sbit11_ebit16}
#define SEQ_KIVEC5_PAD_BUF_ADDR                  \
    k.{seq_kivec5_pad_buf_addr_sbit0_ebit5...seq_kivec5_pad_buf_addr_sbit30_ebit33}
#define SEQ_KIVEC5_STATUS_DMA_EN                \
    k.seq_kivec5_status_dma_en
#define SEQ_KIVEC5_DATA_LEN_FROM_DESC           \
    k.seq_kivec5_data_len_from_desc
#define SEQ_KIVEC5_STOP_CHAIN_ON_ERROR          \
    k.seq_kivec5_stop_chain_on_error
#define SEQ_KIVEC5_CHAIN_ALT_DESC_ON_ERROR      \
    k.seq_kivec5_chain_alt_desc_on_error
#define SEQ_KIVEC5_NEXT_DB_EN                   \
    k.seq_kivec5_next_db_en
#define SEQ_KIVEC5_AOL_UPDATE_EN                \
    k.seq_kivec5_aol_update_en
#define SEQ_KIVEC5_SGL_UPDATE_EN                \
    k.seq_kivec5_sgl_update_en
#define SEQ_KIVEC5_SGL_SPARSE_FORMAT_EN         \
    k.seq_kivec5_sgl_sparse_format_en
#define SEQ_KIVEC5_SGL_PDMA_EN                  \
    k.seq_kivec5_sgl_pdma_en
#define SEQ_KIVEC5_SGL_PDMA_PAD_ONLY            \
    k.seq_kivec5_sgl_pdma_pad_only
#define SEQ_KIVEC5_SGL_PDMA_ALT_SRC_ON_ERROR    \
    k.seq_kivec5_sgl_pdma_alt_src_on_error
#define SEQ_KIVEC5_INTR_EN                      \
    k.seq_kivec5_intr_en
#define SEQ_KIVEC5_NEXT_DB_ACTION_BARCO_PUSH    \
    k.seq_kivec5_next_db_action_barco_push
#define SEQ_KIVEC5_DESC_VEC_PUSH_EN             \
    k.seq_kivec5_desc_vec_push_en
#define SEQ_KIVEC5_INTEG_DATA0_WR_EN            \
    k.seq_kivec5_integ_data0_wr_en
#define SEQ_KIVEC5_INTEG_DATA_NULL_EN           \
    k.seq_kivec5_integ_data_null_en
#define SEQ_KIVEC5_DESC_DLEN_UPDATE_EN          \
    k.seq_kivec5_desc_dlen_update_en
#define SEQ_KIVEC5_HDR_VERSION_WR_EN            \
    k.seq_kivec5_hdr_version_wr_en
#define SEQ_KIVEC5_CP_HDR_UPDATE_EN             \
    k.seq_kivec5_cp_hdr_update_en
#define SEQ_KIVEC5_RATE_LIMIT_EN                \
    k.seq_kivec5_rate_limit_en
#define SEQ_KIVEC5_RATE_LIMIT_SRC_EN            \
    k.seq_kivec5_rate_limit_src_en
#define SEQ_KIVEC5_RATE_LIMIT_DST_EN            \
    k.seq_kivec5_rate_limit_dst_en
#define SEQ_KIVEC5_STATUS_LEN_NO_HDR            \
    k.seq_kivec5_status_len_no_hdr
#define SEQ_KIVEC5_PADDING_EN                   \
    k.seq_kivec5_padding_en

#define SEQ_KIVEC5XTS_SRC_QADDR                 \
    k.{seq_kivec5xts_src_qaddr_sbit0_ebit7...seq_kivec5xts_src_qaddr_sbit32_ebit33}
#define SEQ_KIVEC5XTS_DATA_LEN                  \
    k.{seq_kivec5xts_data_len_sbit0_ebit5...seq_kivec5xts_data_len_sbit30_ebit31}
#define SEQ_KIVEC5XTS_BLK_BOUNDARY_SHIFT        \
    k.seq_kivec5xts_blk_boundary_shift
#define SEQ_KIVEC5XTS_STATUS_DMA_EN             \
    k.seq_kivec5xts_status_dma_en
#define SEQ_KIVEC5XTS_STOP_CHAIN_ON_ERROR       \
    k.seq_kivec5xts_stop_chain_on_error
#define SEQ_KIVEC5XTS_NEXT_DB_EN                \
    k.seq_kivec5xts_next_db_en
#define SEQ_KIVEC5XTS_COMP_LEN_UPDATE_EN        \
    k.seq_kivec5xts_comp_len_update_en
#define SEQ_KIVEC5XTS_COMP_SGL_SRC_EN           \
    k.seq_kivec5xts_comp_sgl_src_en
#define SEQ_KIVEC5XTS_COMP_SGL_SRC_VEC_EN       \
    k.seq_kivec5xts_comp_sgl_src_vec_en
#define SEQ_KIVEC5XTS_SGL_SPARSE_FORMAT_EN      \
    k.seq_kivec5xts_sgl_sparse_format_en
#define SEQ_KIVEC5XTS_INTR_EN                   \
    k.seq_kivec5xts_intr_en
#define SEQ_KIVEC5XTS_NEXT_DB_ACTION_BARCO_PUSH \
    k.seq_kivec5xts_next_db_action_barco_push
#define SEQ_KIVEC5XTS_SGL_PDMA_EN               \
    k.seq_kivec5xts_sgl_pdma_en
#define SEQ_KIVEC5XTS_SGL_PDMA_LEN_FROM_DESC    \
    k.seq_kivec5xts_sgl_pdma_len_from_desc
#define SEQ_KIVEC5XTS_DESC_VEC_PUSH_EN          \
    k.seq_kivec5xts_desc_vec_push_en
#define SEQ_KIVEC5XTS_RATE_LIMIT_EN             \
    k.seq_kivec5xts_rate_limit_en
#define SEQ_KIVEC5XTS_RATE_LIMIT_SRC_EN         \
    k.seq_kivec5xts_rate_limit_src_en
#define SEQ_KIVEC5XTS_RATE_LIMIT_DST_EN         \
    k.seq_kivec5xts_rate_limit_dst_en
    
#define SEQ_KIVEC6_AOL_SRC_VEC_ADDR             \
    k.seq_kivec6_aol_src_vec_addr
#define SEQ_KIVEC6_AOL_DST_VEC_ADDR             \
    k.seq_kivec6_aol_dst_vec_addr
    
#define SEQ_KIVEC7XTS_COMP_DESC_ADDR            \
    k.seq_kivec7xts_comp_desc_addr
#define SEQ_KIVEC7XTS_COMP_SGL_SRC_ADDR         \
    k.seq_kivec7xts_comp_sgl_src_addr

#define SEQ_KIVEC8_ALT_BUF_ADDR                 \
    k.seq_kivec8_alt_buf_addr
#define SEQ_KIVEC8_ALT_BUF_ADDR_EN              \
    k.seq_kivec8_alt_buf_addr_en

#define SEQ_KIVEC9_METRICS0_RANGE               \
    k.{seq_kivec9_metrics0_start...seq_kivec9_metrics0_end}
#define SEQ_KIVEC9_INTERRUPTS_RAISED            \
    k.seq_kivec9_interrupts_raised
#define SEQ_KIVEC9_NEXT_DB_RUNG                 \
    k.seq_kivec9_next_db_rung
#define SEQ_KIVEC9_DESCS_PROCESSED              \
    k.seq_kivec9_descs_processed
#define SEQ_KIVEC9_DESCS_ABORTED                \
    k.{seq_kivec9_descs_aborted_sbit0_ebit3...seq_kivec9_descs_aborted_sbit12_ebit15}
#define SEQ_KIVEC9_STATUS_PDMA_XFERS            \
    k.seq_kivec9_status_pdma_xfers
#define SEQ_KIVEC9_HW_DESC_XFERS                \
    k.{seq_kivec9_hw_desc_xfers_sbit0_ebit2...seq_kivec9_hw_desc_xfers_sbit11_ebit15}
#define SEQ_KIVEC9_HW_BATCH_ERRS                \
    k.seq_kivec9_hw_batch_errs
#define SEQ_KIVEC9_HW_OP_ERRS                   \
    k.seq_kivec9_hw_op_errs
    
#define SEQ_KIVEC9_METRICS1_RANGE               \
    k.{seq_kivec9_metrics1_start...seq_kivec9_metrics1_end}
#define SEQ_KIVEC9_AOL_UPDATE_REQS              \
    k.seq_kivec9_aol_update_reqs
#define SEQ_KIVEC9_SGL_UPDATE_REQS              \
    k.seq_kivec9_sgl_update_reqs
#define SEQ_KIVEC9_SGL_PDMA_XFERS               \
    k.seq_kivec9_sgl_pdma_xfers
#define SEQ_KIVEC9_SGL_PDMA_ERRS                \
    k.seq_kivec9_sgl_pdma_errs
#define SEQ_KIVEC9_SGL_PAD_ONLY_XFERS           \
    k.seq_kivec9_sgl_pad_only_xfers
#define SEQ_KIVEC9_SGL_PAD_ONLY_ERRS            \
    k.seq_kivec9_sgl_pad_only_errs
#define SEQ_KIVEC9_ALT_DESCS_TAKEN              \
    k.seq_kivec9_alt_descs_taken
#define SEQ_KIVEC9_ALT_BUFS_TAKEN               \
    k.seq_kivec9_alt_bufs_taken
    
#define SEQ_KIVEC9_METRICS2_RANGE               \
    k.{seq_kivec9_metrics2_start...seq_kivec9_metrics2_end}
#define SEQ_KIVEC9_LEN_UPDATES                  \
    k.seq_kivec9_len_updates
#define SEQ_KIVEC9_CP_HEADER_UPDATES            \
    k.seq_kivec9_cp_header_updates
#define SEQ_KIVEC9_SEQ_HW_BYTES                 \
    k.{seq_kivec9_seq_hw_bytes_sbit0_ebit2...seq_kivec9_seq_hw_bytes_sbit27_ebit31}

#define SEQ_KIVEC10_INTR_ADDR                   \
    k.seq_kivec10_intr_addr
#define SEQ_KIVEC10_NUM_ALT_DESCS               \
    k.seq_kivec10_num_alt_descs
#define SEQ_KIVEC10_ALT_DESCS_SELECT            \
    k.seq_kivec10_alt_descs_select
    
/*
 * MPU stages
 */
#define CAPRI_STAGE_0                           0
#define CAPRI_STAGE_1                           1
#define CAPRI_STAGE_2                           2
#define CAPRI_STAGE_3                           3
#define CAPRI_STAGE_4                           4
#define CAPRI_STAGE_5                           5
#define CAPRI_STAGE_6                           6
#define CAPRI_STAGE_7                           7

#define SEQ_INTR_OVERRIDE_STAGE                 CAPRI_STAGE_4
#define SEQ_METRICS_STAGE                       CAPRI_STAGE_5

/*
 * Debug flags
 */
#define SEQ_COMP_SGL_PDMA_XFER_DEBUG            0
#define SEQ_COMP_SGL_PDMA_PAD_ONLY_DEBUG        0
#define DMA_CMD_MEM2MEM_SIZE_DEBUG              0

/*
 * Qstate size definitions
 */
#define CAPRI_QSTATE_SINGLE_SIZE                64

#define SEQ_QSTATE_SINGLE_SIZE                  CAPRI_QSTATE_SINGLE_SIZE

#define SEQ_QSTATE_METRICS0_SIZE                CAPRI_QSTATE_SINGLE_SIZE
#define SEQ_QSTATE_METRICS1_SIZE                CAPRI_QSTATE_SINGLE_SIZE
#define SEQ_QSTATE_METRICS2_SIZE                CAPRI_QSTATE_SINGLE_SIZE

#define SEQ_QSTATE_METRICS0_OFFSET              \
    (SEQ_QSTATE_SINGLE_SIZE)
#define SEQ_QSTATE_METRICS1_OFFSET              \
    (SEQ_QSTATE_METRICS0_OFFSET + SEQ_QSTATE_METRICS0_SIZE)
#define SEQ_QSTATE_METRICS2_OFFSET              \
    (SEQ_QSTATE_METRICS1_OFFSET + SEQ_QSTATE_METRICS1_SIZE)

/*
 * Barco SGL rearranged to little-endian layout
 */
struct barco_sgl_le_t {
    rsvd        : 64;
    link        : 64;
    rsvd2       : 32;
    len2        : 32;
    addr2       : 64;
    rsvd1       : 32;
    len1        : 32;
    addr1       : 64;
    rsvd0       : 32;
    len0        : 32;
    addr0       : 64;
};

#define BARCO_SGL_DESC_SIZE         64
#define BARCO_SGL_DESC_SIZE_SHIFT   6

#define BARCO_SGL_TUPLE0            0
#define BARCO_SGL_TUPLE1            1
#define BARCO_SGL_TUPLE2            2
#define BARCO_SGL_NUM_TUPLES_MAX    3

/*
 * Barco AOL rearranged to little-endian layout
 */
struct barco_aol_le_t {
    rsvd        : 64;
    next_addr   : 64;
    L2          : 32;
    O2          : 32;
    A2          : 64;
    L1          : 32;
    O1          : 32;
    A1          : 64;
    L0          : 32;
    O0          : 32;
    A0          : 64;
};

#define BARCO_AOL_DESC_SIZE         64
#define BARCO_AOL_DESC_SIZE_SHIFT   6

/*
 * Specification of a single chain SGL PDMA tuple for use with tblrdp/tblwrp.
 */
struct chain_sgl_pdma_tuple_t {
    addr        : 64;
    len         : 32;
};

/*
 * Compression descriptor rearranged to little-endian layout
 */
struct comp_desc_le_t {
    status_data     : 32;
    opaque_tag_data : 32;
    opaque_tag_addr : 64;
    doorbell_data   : 64;
    doorbell_addr   : 64;
    status_addr     : 64;
    threshold_len   : 16;
    extended_len    : 16;
    datain_len      : 16;
    cmd             : 16;
    dst             : 64;
    src             : 64;
};

#define COMP_DESC_SIZE              64
#define COMP_DESC_SIZE_SHIFT        6

/*
 * Compression header
 */
struct seq_comp_hdr_t {
    cksum           : 32;
    data_len        : 16;
    version         : 16;
};

// TODO: Fix these to use the values defined in hardware
#define CAPRI_DMA_NOP               0
#define CAPRI_DMA_PHV2MEM           3
#define CAPRI_DMA_MEM2MEM           6
#define CAPRI_DMA_M2M_TYPE_SRC      0
#define CAPRI_DMA_M2M_TYPE_DST      1

// Copied from rdma/common/include/capri.h
#define DMA_CMD_MEM2MEM_T struct capri_dma_cmd_mem2mem_t

// mem2mem size below is maximally 14 bits
#define DMA_CMD_MEM2MEM_SIZE_MAX    ((1 << 14) - 1)

struct capri_dma_cmd_mem2mem_t {
    rsvd            : 16;
    size            : 14;
    rsvd1           : 1;
    override_lif    : 11;
    addr            : 52;
    barrier         : 1;
    round           : 1;
    pcie_msg        : 1;
    use_override_lif: 1;
    phv_end         : 10;
    phv_start       : 10;
    wr_fence_fence  : 1;
    wr_fence        : 1;
    cache           : 1;
    host_addr       : 1;
    mem2mem_type    : 2;
    cmdeop          : 1;
    cmdtype         : 3;
};

#define DMA_M2M_PTR_WRITE(_field, _val)                                 \
  phvwrp    r_curr_dma_cmd_ptr, offsetof(DMA_CMD_MEM2MEM_T, _field),    \
            sizeof(DMA_CMD_MEM2MEM_T._field), _val;                     \

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
        
#define LOAD_TABLE_NO_LKUP_PC_IMM(_num, _pc)                            \
  addi      r1, r0, _pc[33:6];                                          \
  phvwri    p.app_header_table##_num##_valid, 1;                        \
  phvwri    p.{common_te##_num##_phv_table_lock_en...                   \
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

#define LOAD_TABLE0_FOR_ADDR64_CONT(_table_addr, _load_size, _pc)       \
  phvwri    p.app_header_table0_valid, 1;                               \
  phvwrpair p.common_te0_phv_table_lock_en, 1,                          \
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

// Same as LOAD_TABLE0_FOR_ADDR34() but does not exit            
#define LOAD_TABLE0_FOR_ADDR34_CONT(_table_addr, _load_size, _pc)       \
  phvwri    p.app_header_table0_valid, 1;                               \
  phvwrpair p.common_te0_phv_table_lock_en, 1,                          \
            p.common_te0_phv_table_raw_table_size, _load_size;          \
  phvwr     p.common_te0_phv_table_pc, _pc;                             \
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
  phvwri    p.app_header_table1_valid, 1;                               \
  phvwrpair p.common_te1_phv_table_lock_en, 1,                          \
        p.common_te1_phv_table_raw_table_size, _load_size;              \
  phvwr     p.common_te1_phv_table_pc, _pc;                             \
  phvwr     p.common_te1_phv_table_addr, _table_addr;                   \

#define LOAD_TABLE1_FOR_ADDR34_e(_table_addr, _load_size, _pc)          \
  phvwri    p.app_header_table1_valid, 1;                               \
  phvwrpair p.common_te1_phv_table_lock_en, 1,                          \
        p.common_te1_phv_table_raw_table_size, _load_size;              \
  phvwr.e   p.common_te1_phv_table_pc, _pc;                             \
  phvwr     p.common_te1_phv_table_addr, _table_addr;                   \

#define LOAD_TABLE2_FOR_ADDR34(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table2_valid, 1;                               \
  phvwrpair p.common_te2_phv_table_lock_en, 1,                          \
        p.common_te2_phv_table_raw_table_size, _load_size;              \
  phvwr     p.common_te2_phv_table_pc, _pc;                             \
  phvwr     p.common_te2_phv_table_addr, _table_addr;                   \

#define LOAD_TABLE2_FOR_ADDR34_e(_table_addr, _load_size, _pc)          \
  phvwri    p.app_header_table2_valid, 1;                               \
  phvwrpair p.common_te2_phv_table_lock_en, 1,                          \
        p.common_te2_phv_table_raw_table_size, _load_size;              \
  phvwr.e   p.common_te2_phv_table_pc, _pc;                             \
  phvwr     p.common_te2_phv_table_addr, _table_addr;                   \

#define LOAD_TABLE3_FOR_ADDR34(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table3_valid, 1;                               \
  phvwrpair p.common_te3_phv_table_lock_en, 1,                          \
        p.common_te3_phv_table_raw_table_size, _load_size;              \
  phvwr     p.common_te3_phv_table_pc, _pc;                             \
  phvwr     p.common_te3_phv_table_addr, _table_addr;                   \

#define LOAD_TABLE3_FOR_ADDR34_e(_table_addr, _load_size, _pc)          \
  phvwri    p.app_header_table3_valid, 1;                               \
  phvwrpair p.common_te3_phv_table_lock_en, 1,                          \
        p.common_te3_phv_table_raw_table_size, _load_size;              \
  phvwr.e   p.common_te3_phv_table_pc, _pc;                             \
  phvwr     p.common_te3_phv_table_addr, _table_addr;                   \

#define LOAD_TABLE2_FOR_ADDR64(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table2_valid, 1;                               \
  phvwrpair p.common_te2_phv_table_lock_en, 1,                          \
        p.common_te2_phv_table_raw_table_size, _load_size;              \
  phvwrpair p.common_te2_phv_table_pc, _pc,                             \
        p.common_te2_phv_table_addr, _table_addr;                       \

#define LOAD_TABLE2_FOR_ADDR64_e(_table_addr, _load_size, _pc)          \
  phvwri    p.app_header_table2_valid, 1;                               \
  phvwrpair.e p.common_te2_phv_table_lock_en, 1,                        \
        p.common_te2_phv_table_raw_table_size, _load_size;              \
  phvwrpair p.common_te2_phv_table_pc, _pc,                             \
        p.common_te2_phv_table_addr, _table_addr;                       \

#define LOAD_TABLE3_FOR_ADDR64(_table_addr, _load_size, _pc)            \
  phvwri    p.app_header_table3_valid, 1;                               \
  phvwrpair p.common_te3_phv_table_lock_en, 1,                          \
        p.common_te3_phv_table_raw_table_size, _load_size;              \
  phvwrpair p.common_te3_phv_table_pc, _pc,                             \
        p.common_te3_phv_table_addr, _table_addr;                       \

#define LOAD_TABLE3_FOR_ADDR64_e(_table_addr, _load_size, _pc)          \
  phvwri    p.app_header_table3_valid, 1;                               \
  phvwrpair.e p.common_te3_phv_table_lock_en, 1,                        \
        p.common_te3_phv_table_raw_table_size, _load_size;              \
  phvwrpair p.common_te3_phv_table_pc, _pc,                             \
        p.common_te3_phv_table_addr, _table_addr;                       \

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

#define LOAD_TABLE_FOR_ADDR34_PC_IMM_CONT(_table_addr, _load_size, _pc) \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE0_FOR_ADDR34_CONT(_table_addr, _load_size, r1)              \

#define LOAD_TABLE_FOR_ADDR(_table_addr, _load_size, _pc)               \
  LOAD_TABLE0_FOR_ADDR64(_table_addr, _load_size, _pc)                  \
            
#define LOAD_TABLE1_FOR_ADDR34_PC_IMM(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE1_FOR_ADDR34(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE1_FOR_ADDR34_PC_IMM_e(_table_addr, _load_size, _pc)   \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE1_FOR_ADDR34_e(_table_addr, _load_size, r1)                 \

#define LOAD_TABLE2_FOR_ADDR_PC_IMM(_table_addr, _load_size, _pc)       \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE2_FOR_ADDR64(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE2_FOR_ADDR_PC_IMM_e(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE2_FOR_ADDR64_e(_table_addr, _load_size, r1)                 \

#define LOAD_TABLE2_FOR_ADDR34_PC_IMM(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE2_FOR_ADDR34(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE2_FOR_ADDR34_PC_IMM_e(_table_addr, _load_size, _pc)   \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE2_FOR_ADDR34_e(_table_addr, _load_size, r1)                 \

#define LOAD_TABLE3_FOR_ADDR_PC_IMM(_table_addr, _load_size, _pc)       \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE3_FOR_ADDR64(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE3_FOR_ADDR_PC_IMM_e(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE3_FOR_ADDR64_e(_table_addr, _load_size, r1)                 \

#define LOAD_TABLE3_FOR_ADDR34_PC_IMM(_table_addr, _load_size, _pc)     \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE3_FOR_ADDR34(_table_addr, _load_size, r1)                   \

#define LOAD_TABLE3_FOR_ADDR34_PC_IMM_e(_table_addr, _load_size, _pc)   \
  addi      r1, r0, _pc[33:6];                                          \
  LOAD_TABLE3_FOR_ADDR34_e(_table_addr, _load_size, r1)                 \

// Special API to load table1 without setting the header valid bits
// pc is an immediate value
// _table_addr is 34 bits
#define LOAD_TABLE1_NO_VALID_BIT_e(_table_addr, _load_size, _pc)        \
  addi      r1, r0, _pc[33:6];                                          \
  phvwrpair p.common_te1_phv_table_lock_en, 1,                          \
        p.common_te1_phv_table_raw_table_size, _load_size;              \
  phvwr.e   p.common_te1_phv_table_pc, r1;                              \
  phvwr     p.common_te1_phv_table_addr, _table_addr;                   \


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
  add       r3, r3, _table_base;                                        \
  LOAD_TABLE_FOR_ADDR_PC_IMM(r3, _load_size, _pc)                       \

// Used to set/clear table N valid bit 
#define SET_TABLE0                                                      \
  phvwri    p.app_header_table0_valid, 1;                               \

#define CLEAR_TABLE0                                                    \
  phvwri    p.app_header_table0_valid, 0;                               \

#define CLEAR_TABLE0_e                                                  \
  phvwri.e  p.app_header_table0_valid, 0;                               \
  nop;                                                                  \

#define SET_TABLE1                                                      \
  phvwri    p.app_header_table1_valid, 1;                               \

#define CLEAR_TABLE1                                                    \
  phvwri    p.app_header_table1_valid, 0;                               \

#define CLEAR_TABLE1_e                                                  \
  phvwri.e  p.app_header_table1_valid, 0;                               \
  nop;                                                                  \

#define SET_TABLE2                                                      \
  phvwri    p.app_header_table2_valid, 1;                               \

#define CLEAR_TABLE2                                                    \
  phvwri    p.app_header_table2_valid, 0;                               \

#define CLEAR_TABLE2_e                                                  \
  phvwri.e  p.app_header_table2_valid, 0;                               \
  nop;                                                                  \

#define SET_TABLE3                                                      \
  phvwri    p.app_header_table3_valid, 1;                               \

#define CLEAR_TABLE3                                                    \
  phvwri    p.app_header_table3_valid, 0;                               \

#define CLEAR_TABLE3_e                                                  \
  phvwri.e  p.app_header_table3_valid, 0;                               \
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
         
#define CAPRI_PHV_FLIT_SIZE_BITS_SHIFT  9
#define CAPRI_TXDMA_DESC_SIZE_BITS      128
#define CAPRI_NUM_TXDMA_DESCS_PER_FLIT                                  \
    (CAPRI_PHV_FLIT_SIZE_BITS / CAPRI_TXDMA_DESC_SIZE_BITS)

// Given a flit number, calculate the bit offset to the first TxDMA descriptor
// in that flit, assuming the entire flit holds only TxDMA descriptors.
// _flit_no is an immediate value.
// 
// Note: PHV flit memory is in big endian layout, so the next adjacent
// descriptor is at a LOWER address! Hence, the subi instruction below.
#define CAPRI_FLIT_TO_DMA_PTR(_flit_no)                                 \
    tblwr.l     l_dma_desc_count, 0;                                    \
    addi        r_curr_dma_cmd_ptr, r0, ((_flit_no + 1) *               \
                CAPRI_PHV_FLIT_SIZE_BITS) - CAPRI_TXDMA_DESC_SIZE_BITS; \
    subi        r_last_dma_cmd_ptr, r_curr_dma_cmd_ptr,                 \
                CAPRI_TXDMA_DESC_SIZE_BITS *                            \
                    (CAPRI_NUM_TXDMA_DESCS_PER_FLIT - 1);               \

// Alternatively, the caller can pass in the 1st available and last
// descriptors in a flit, from which r_curr_dma_cmd_ptr and
// r_last_dma_cmd_ptr can be calculated.
#define CAPRI_FLIT_DMA_PTR_INITIAL(_dma_desc_avail, _dma_desc_last)     \
    tblwr.l     l_dma_desc_count, 0;                                    \
    addi        r_curr_dma_cmd_ptr, r0,                                 \
                PHV_DMA_CMD_START_OFFSET(_dma_desc_avail);              \
    addi        r_last_dma_cmd_ptr, r0,                                 \
                PHV_DMA_CMD_START_OFFSET(_dma_desc_last);               \

// Given r_curr_dma_cmd_ptr, calculate the next adjacent descriptor, taking
// into account of flit boundary crossing. When crossing the flit, rewind
// back to the first descriptor of the flit (which means adding 
// CAPRI_PHV_FLIT_SIZE_BITS minus CAPRI_TXDMA_DESC_SIZE_BITS),
// then add CAPRI_TXDMA_DESC_SIZE_BITS to advance to the first descriptor
// in the next flit.
// 
// Note: PHV flit memory is in big endian layout, so the next adjacent
// descriptor is at a LOWER address! Hence, the subi instruction below.
#define CAPRI_FLIT_DMA_PTR_ADVANCE(_outer_label)                        \
    tbladd.l    l_dma_desc_count, 1;                                    \
    bne         r_curr_dma_cmd_ptr, r_last_dma_cmd_ptr, _outer_label;   \
    subi        r_curr_dma_cmd_ptr, r_curr_dma_cmd_ptr,                 \
                CAPRI_TXDMA_DESC_SIZE_BITS;                             \
    addi        r_curr_dma_cmd_ptr, r_curr_dma_cmd_ptr,                 \
                (CAPRI_PHV_FLIT_SIZE_BITS * 2) -                        \
                     CAPRI_TXDMA_DESC_SIZE_BITS;                        \
    subi        r_last_dma_cmd_ptr, r_curr_dma_cmd_ptr,                 \
                CAPRI_TXDMA_DESC_SIZE_BITS *                            \
                    (CAPRI_NUM_TXDMA_DESCS_PER_FLIT - 1);               \

// Validate that the number of DMA descriptors (i.e. commands) consumed
// so far does not exceed limit (_final_no minus _initial_no).
// 
// _initial_no and _final_no must be literals.
#define CAPRI_FLIT_DMA_PTR_FINAL_CHECK(_dma_desc_initial, _initial_no,  \
                                       _dma_desc_final, _final_no,      \
                                       _error_label)                    \
    sle         c1, l_dma_desc_count, _final_no - _initial_no;          \
    bcf         [!c1], _error_label;                                    \
    nop;                                                                \
    
// Offset/size operands for tblrdp/tblwrp instruction
#define CAPRI_TBLRWP_FIELD_OP(_struct, _field)                          \
    offsetof(struct _struct, _field),                                   \
    sizeof(struct _struct._field)                                       \


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
// _addr is a 64-bit value in k/d-vector.
#define DMA_PHV2MEM_SETUP_ADDR_VEC(_start, _end, _addr, _dma_cmd_X)     \
   phvwr     p._dma_cmd_X##_dma_cmd_addr, _addr;                        \
   phvwrpair p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63],            \
             p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_PHV2MEM ;           \
   phvwri    p._dma_cmd_X##_dma_cmd_phv_end_addr,                       \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _end));               \
   phvwri    p._dma_cmd_X##_dma_cmd_phv_start_addr,                     \
                CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +             \
                                      sizeof(p._start) - 1);            \
   
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
   
// Mem2Mem DMA:
// assume no LIF override
// _addr is given in a register
#define DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(_type, _addr, _size,          \
                                          _dma_cmd_X)                   \
   phvwrpair p._dma_cmd_X##_dma_cmd_mem2mem_type, _type,                \
             p._dma_cmd_X##_dma_cmd_type, CAPRI_DMA_MEM2MEM;            \
   phvwrpair p._dma_cmd_X##_dma_cmd_size, _size,                        \
             p._dma_cmd_X##_dma_cmd_addr, _addr;                        \
   phvwr     p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63];            \

#define DMA_MEM2MEM_NO_LIF_SETUP(_type, _addr, _size, _dma_cmd_X)       \
   add      r1, r0, _addr;                                              \
   DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(_type, r1, _size, _dma_cmd_X)      \
   
// Mem2Mem DMA setup via pointer:
// assume no LIF override
// _addr is 64 bits
#define DMA_MEM2MEM_PTR_SETUP_ADDR(_type, _addr, _size)                 \
   DMA_M2M_PTR_WRITE(mem2mem_type, _type)                               \
   DMA_M2M_PTR_WRITE(cmdtype, CAPRI_DMA_MEM2MEM)                        \
   DMA_M2M_PTR_WRITE(size, _size)                                       \
   DMA_M2M_PTR_WRITE(addr, _addr)                                       \
   DMA_M2M_PTR_WRITE(host_addr, _addr[63])                              \

// Mem2Mem DMA setup via pointer:
// assume no LIF override
// _addr is 34 bits
#define DMA_MEM2MEM_PTR_SETUP_ADDR34(_type, _addr, _size)               \
   DMA_M2M_PTR_WRITE(mem2mem_type, _type)                               \
   DMA_M2M_PTR_WRITE(cmdtype, CAPRI_DMA_MEM2MEM)                        \
   DMA_M2M_PTR_WRITE(size, _size)                                       \
   DMA_M2M_PTR_WRITE(addr, _addr)                                       \
   
// DMA fence update: Set the fence bit for the MEM2MEM DMA command
#define DMA_MEM2MEM_FENCE(_dma_cmd_X)                                   \
   phvwri   p._dma_cmd_X##_dma_cmd_wr_fence, 1;                         \

// DMA fence update: Set the fence bit for the PHV2MEM DMA command
#define DMA_PHV2MEM_FENCE(_dma_cmd_X)                                   \
   phvwri   p._dma_cmd_X##_dma_cmd_wr_fence, 1;                         \

#define DMA_PHV2MEM_FENCE_FENCE(_dma_cmd_X)                             \
   phvwrpair p._dma_cmd_X##_dma_cmd_fence_fence, 1,                     \
             p._dma_cmd_X##_dma_cmd_wr_fence, 0;                        \

// DMA address update: Specify the destination address for the DMA command
// _addr is given in a register
#define DMA_ADDR_UPDATE(_addr, _dma_cmd_X)                              \
   phvwr    p._dma_cmd_X##_dma_cmd_addr, _addr;                         \
   phvwr    p._dma_cmd_X##_dma_cmd_host_addr, _addr[63:63];             \
   
// DMA size update: Specify the size for the DMA command
#define DMA_SIZE_UPDATE(_size, _dma_cmd_X)                              \
   phvwr    p._dma_cmd_X##_dma_cmd_size, _size;                         \
   
// Setup the start and end DMA pointers
#define DMA_PTR_SETUP(_start, _dma_cmd_eop, _dma_cmd_ptr)               \
   phvwri   p._dma_cmd_ptr,                                             \
                ((CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +           \
                                        sizeof(p._start) - 1))/16);     \
   phvwri   p._dma_cmd_eop, 1;                                          \

#define DMA_PTR_SETUP_e(_start, _dma_cmd_eop, _dma_cmd_ptr)             \
   phvwri.e p._dma_cmd_ptr,                                             \
                ((CAPRI_PHV_BIT_TO_BYTE(offsetof(p, _start) +           \
                                        sizeof(p._start) - 1))/16);     \
   phvwri   p._dma_cmd_eop, 1;                                          \
   
// Cancel a previously set Comp next doorbell ring command.
// Since such a command is likely an EOP, NOP cannot be used
// so work around that using a harmless PHV2MEM commmand.
#define SEQ_COMP_NEXT_DB_CANCEL(_dma_cmd_X)                             \
   add      r_src_qaddr, SEQ_KIVEC5_SRC_QADDR      ,                    \
            SEQ_QSTATE_SINGLE_SIZE - 1;                                 \
   DMA_PHV2MEM_SETUP_ADDR64(null_byte_len, null_byte_len,               \
                            r_src_qaddr, _dma_cmd_X)                    \
   SEQ_METRICS_CLR(next_db_rung)                                        \

// Cancel a previously set XTS next doorbell ring command.
// Since such a command is likely an EOP, NOP cannot be used
// so work around that using a harmless PHV2MEM commmand.
#define SEQ_XTS_NEXT_DB_CANCEL(_dma_cmd_X)                              \
   add      r_src_qaddr, SEQ_KIVEC5XTS_SRC_QADDR,                       \
            SEQ_QSTATE_SINGLE_SIZE - 1;                                 \
   DMA_PHV2MEM_SETUP_ADDR64(null_byte_len, null_byte_len,               \
                            r_src_qaddr, _dma_cmd_X)                    \
   SEQ_METRICS_CLR(next_db_rung)                                        \

// Setup the doorbell data. Write back the data in little endian format
#define DOORBELL_DATA_SETUP_REG(_reg, _index, _ring, _qid, _pid)        \
   add      _reg, _index, _ring, DOORBELL_DATA_RING_SHIFT;              \
   add      _reg, _reg, _qid, DOORBELL_DATA_QID_SHIFT;                  \
   add      _reg, _reg, _pid, DOORBELL_DATA_PID_SHIFT;                  \

#define DOORBELL_DATA_SETUP(_db_data, _index, _ring, _qid, _pid)        \
   DOORBELL_DATA_SETUP_REG(r1, _index, _ring, _qid, _pid)               \
   phvwr    p._db_data, r1.dx;                                          \

// Setup the doorbell address. Output will be stored in GPR.
#define DOORBELL_ADDR_SETUP_REG(_reg, _lif, _qtype, _sched_wr, _upd)    \
   addi     _reg, r0, DOORBELL_ADDR_WA_LOCAL_BASE + _sched_wr + _upd;   \
   add      _reg, _reg, _qtype, DOORBELL_ADDR_QTYPE_SHIFT;              \
   add      _reg, _reg, _lif, DOORBELL_ADDR_LIF_SHIFT;                  \

#define DOORBELL_ADDR_SETUP(_lif, _qtype, _sched_wr, _upd)              \
   DOORBELL_ADDR_SETUP_REG(r7, _lif, _qtype, _sched_wr, _upd)           \

// Clear the doorbell as there was no work to be done. Note index can
// be 0 (r0) as there is no update.
#define QUEUE_DOORBELL_CLEAR(_ring, _wr_sched, _lif, _qtype, _qid)      \
   DOORBELL_DATA_SETUP(qpop_doorbell_data_data, r0, _ring, _qid, r0)    \
   DOORBELL_ADDR_SETUP(_lif, _qtype, _wr_sched, DOORBELL_UPDATE_NONE)   \
   DMA_PHV2MEM_SETUP(qpop_doorbell_data_data, qpop_doorbell_data_data,  \
                     r7, dma_p2m_0)                                     \

// Same as QUEUE_DOORBELL_CLEAR but execute the op inline without DMA.
#define QUEUE_DOORBELL_CLEAR_INLINE(_ring, _wr_sched,                   \
                                    _lif, _qtype, _qid)                 \
   DOORBELL_DATA_SETUP_REG(r_db_data_scratch, r0, _ring, _qid, r0)      \
   DOORBELL_ADDR_SETUP_REG(r_db_addr_scratch, _lif, _qtype,             \
                           _wr_sched, DOORBELL_UPDATE_NONE)             \
   memwr.dx   r_db_addr_scratch, r_db_data_scratch;                     \

#define QUEUE_DOORBELL_CLEAR_INLINE_e(_ring, _wr_sched,                 \
                                      _lif, _qtype, _qid)               \
   DOORBELL_DATA_SETUP_REG(r_db_data_scratch, r0, _ring, _qid, r0)      \
   DOORBELL_ADDR_SETUP_REG(r_db_addr_scratch, _lif, _qtype,             \
                           _wr_sched, DOORBELL_UPDATE_NONE)             \
   memwr.dx.e r_db_addr_scratch, r_db_data_scratch;                     \
   nop;                                                                 \

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
   QUEUE_DOORBELL_CLEAR(r0, DOORBELL_SCHED_WR_EVAL,                     \
                        NVME_KIVEC_GLOBAL_SRC_LIF,                      \
                        NVME_KIVEC_GLOBAL_SRC_QTYPE,                    \
                        NVME_KIVEC_GLOBAL_SRC_QID)                      \

// Setup the lif, type, qid, pindex for the doorbell push.  Set the fence 
// bit for the doorbell 
#define _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, _sched, _upd, _p_ndx,   \
                                 _lif, _qtype, _qid, _db_data)          \
   DOORBELL_DATA_SETUP(_db_data, _p_ndx, r0, _qid, r0)                  \
   DOORBELL_ADDR_SETUP(_lif, _qtype, _sched, _upd)                      \
   DMA_PHV2MEM_SETUP(_db_data, _db_data, r7, _dma_cmd_ptr)              \
   DMA_PHV2MEM_FENCE(_dma_cmd_ptr)                                      \

#define QUEUE_PUSH_DOORBELL_RING(_dma_cmd_ptr)                          \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_NONE, d.p_ndx,             \
                             STORAGE_KIVEC0_DST_LIF,                    \
                             STORAGE_KIVEC0_DST_QTYPE,                  \
                             STORAGE_KIVEC0_DST_QID,                    \
                             qpush_doorbell_data_data)                  \

#define QUEUE_PUSH_DOORBELL_UPDATE_RING(_dma_cmd_ptr, _p_ndx)           \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_P_NDX, _p_ndx,             \
                             STORAGE_KIVEC0_DST_LIF,                    \
                             STORAGE_KIVEC0_DST_QTYPE,                  \
                             STORAGE_KIVEC0_DST_QID,                    \
                             qpush_doorbell_data_data)                  \

#define ROCE_QUEUE_PUSH_DOORBELL_RING(_dma_cmd_ptr)                     \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_P_NDX, d.p_ndx,            \
                             d.rsq_lif, d.rsq_qtype, d.rsq_qid,         \
                             qpush_doorbell_data_data)                  \


#define NVME_SEQ_QUEUE_PUSH_DOORBELL_RING(_dma_cmd_ptr)                 \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_NONE, d.p_ndx,             \
                             NVME_KIVEC_T0_S2S_DST_LIF,                 \
                             NVME_KIVEC_T0_S2S_DST_QTYPE,               \
                             NVME_KIVEC_T0_S2S_DST_QID,                 \
                             qpush_doorbell_data_data)                  \

#define NVME_SEND_STA_FREE_IOB_DOORBELL_RING(_dma_cmd_ptr)              \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_NONE, d.p_ndx,             \
                             NVME_KIVEC_ARM_DST7_ARM_LIF,               \
                             NVME_KIVEC_ARM_DST7_ARM_QTYPE,             \
                             NVME_KIVEC_ARM_DST7_ARM_QID,               \
                             qpush_doorbell_data_data)                  \

#define NVME_ROCE_RQ_PUSH_DOORBELL_RING(_dma_cmd_ptr)                   \
   _QUEUE_PUSH_DOORBELL_FORM(_dma_cmd_ptr, DOORBELL_SCHED_WR_SET,       \
                             DOORBELL_UPDATE_NONE, d.p_ndx,             \
                             NVME_KIVEC_T1_S2S_DST_LIF,                 \
                             NVME_KIVEC_T1_S2S_DST_QTYPE,               \
                             NVME_KIVEC_T1_S2S_DST_QID,                 \
                             qpush_doorbell_data_1_data)                \

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
#define SEQUENCER_DOORBELL_RING_NO_FENCE(_dma_cmd_ptr)                  \
   phvwr    p.seq_doorbell_data_data, d.next_db_data;                   \
   add      r7, r0, d.next_db_addr;                                     \
   DMA_PHV2MEM_SETUP(seq_doorbell_data_data, seq_doorbell_data_data,    \
                     r7, _dma_cmd_ptr)                                  \

#define SEQUENCER_DOORBELL_RING(_dma_cmd_ptr)                           \
   SEQUENCER_DOORBELL_RING_NO_FENCE(_dma_cmd_ptr)                       \
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
#define SERVICE_PRI_QUEUE(_w_ndx_pri, _pri_val, _next_pc)               \
   add      r6, r0, _w_ndx_pri;                                         \
   QUEUE_POP(_w_ndx_pri, d.num_entries)                                 \
   phvwrpair p.storage_kivec0_w_ndx, _w_ndx_pri,                        \
             p.storage_kivec0_io_priority, _pri_val;                    \
   LOAD_TABLE_FOR_PRI_INDEX(d.base_addr, r6, d.num_entries, _pri_val,   \
                            d.entry_size, d.entry_size[2:0], _next_pc)  \
   
   
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
   _PRI_QUEUE_PUSH_DOORBELL_UPDATE(dma_p2m_4, _p_ndx,                   \
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
   phvwr    p.{r2n_wqe_handle...r2n_wqe_pad},                           \
            d.{handle...pad};                                           \

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
#define _R2N_BUF_POST_SETUP(_buf)                                         \
   addi     r5, r0, ROCE_RQ_WQE_SIZE;                                     \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, _buf, r5, r0, r0, dma_m2m_1) \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, r0, r5, r0, r0, dma_m2m_2)   \

#define R2N_BUF_POST_SETUP_CMD(_addr)                                     \
   sub      r4, _addr, R2N_BUF_NVME_BE_CMD_OFFSET;                        \
   _R2N_BUF_POST_SETUP(r4)                                                \
   
#define R2N_BUF_POST_SETUP_STATUS(_addr)                                  \
   sub      r4, _addr, R2N_BUF_STATUS_BUF_OFFSET;                         \
   _R2N_BUF_POST_SETUP(r4)                                                \

// Offset to the start of a TxDMA descriptor in PHV
#define PHV_DMA_CMD_START_OFFSET(_dma_cmd) offsetof(p, _dma_cmd##_dma_cmd_type)

// Set up the initial d-vector chain_sgl_pdma tuple pointer in
// r_sgl_tuple_p, for use by tblrdp/tblwrp. 
// Note that tblrdp/tblwrp uses positive incremental offset, while 
// chain_sgl_pdma embedded in descriptor are ordered from low to high as 
// len/addr tuples. Hence, the initial starting offset below is len0.
#define CAPRI_CHAIN_SGL_PDMA_TUPLE_INITIAL()                            \
    addi    r_sgl_tuple_p, r0, offsetof(d, len0);                       \

// Given the current chain_sgl_pdma tuple pointer in r_sgl_tuple_p,
// advance to the next tuple. Due to Capri big endian layout, each 
// subsequent tuple is *BELOW* its predecessor, hence the subi 
// sinstruction below.
#define CAPRI_CHAIN_SGL_PDMA_TUPLE_ADVANCE(_error_label)                \
    subi    r_sgl_tuple_p, r_sgl_tuple_p,                               \
            sizeof(struct chain_sgl_pdma_tuple_t);                      \
    blei.s  r_sgl_tuple_p, offsetof(d, pad0), _error_label;             \
    nop;                                                                \
            
// Setup the compression data buffer DMA based on flat source buffer 
// and destination SGL (processing one SGL entry in this macro).
// Notes: These GPRs are used for input/output to/from this macro
//  1. r_src_len stores the running count of data remaining to be xfered
//  2. r_src_addr stores the offset of the source data buffer from where
//     the current xfer is to be done.
// Steps:
//  1. Setup the DMA size based on the min size in r_xfer_len vs r_src_len. 
//  2. Source of the DMA is stored in r_src_addr.
//  3. Destination of the DMA is based on the address in SGL.
//  4. Update data remaining (r_src_len) and address offset (r_src_addr) 
//  5. If data xfer is complete, jump to the branch instruction
#define CHAIN_SGL_PDMA(_dma_cmd_ptr_src, _dma_cmd_ptr_dst,              \
                       _addr, _len, _outer_label, _error_label)         \
   add      r_xfer_len, _len, r0;                                       \
   sle      c2, r_src_len, r_xfer_len;                                  \
   add.c2   r_xfer_len, r0, r_src_len;                                  \
   sle      c3, r_xfer_len, DMA_CMD_MEM2MEM_SIZE_MAX;                   \
   bcf      [!c3], _error_label;                                        \
   add      r_dst_addr, r0, _addr;                                      \
   DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_SRC, r_src_addr,\
                                     r_xfer_len, _dma_cmd_ptr_src)      \
   DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r_dst_addr,\
                                     r_xfer_len, _dma_cmd_ptr_dst)      \
   add      r_src_addr, r_src_addr, r_xfer_len;                         \
   add      r_dst_addr, r_dst_addr, r_xfer_len;                         \
   sub      r_sgl_len, _len, r_xfer_len;                                \
   bcf      [c2], _outer_label;                                         \
   sub      r_src_len, r_src_len, r_xfer_len;                           \

// Same as CHAIN_SGL_PDMA() but uses DMA descriptor pointer and
// SGL tuple pointer, which are a necessity when post PDMA padding
// comes into play.
#define CHAIN_SGL_PDMA_PTR(_inner_label0, _inner_label1,                \
                           _outer_label, _error_label)                  \
   tblrdp.wx r_xfer_len, r_sgl_tuple_p,                                 \
             CAPRI_TBLRWP_FIELD_OP(chain_sgl_pdma_tuple_t, len);        \
   sle      c2, r_src_len, r_xfer_len;                                  \
   add.c2   r_xfer_len, r0, r_src_len;                                  \
   sle      c3, r_xfer_len, DMA_CMD_MEM2MEM_SIZE_MAX;                   \
   bcf      [!c3], _error_label;                                        \
   tblrdp.dx r_dst_addr, r_sgl_tuple_p,                                 \
             CAPRI_TBLRWP_FIELD_OP(chain_sgl_pdma_tuple_t, addr);       \
   DMA_MEM2MEM_PTR_SETUP_ADDR(CAPRI_DMA_M2M_TYPE_SRC, r_src_addr,       \
                              r_xfer_len)                               \
   CAPRI_FLIT_DMA_PTR_ADVANCE(_inner_label0)                            \
_inner_label0:;                                                         \
   DMA_MEM2MEM_PTR_SETUP_ADDR(CAPRI_DMA_M2M_TYPE_DST, r_dst_addr,       \
                              r_xfer_len)                               \
   CAPRI_FLIT_DMA_PTR_ADVANCE(_inner_label1)                            \
_inner_label1:;                                                         \
   add      r_src_addr, r_src_addr, r_xfer_len;                         \
   add      r_dst_addr, r_dst_addr, r_xfer_len;                         \
   bcf      [c2], _outer_label;                                         \
   sub      r_src_len, r_src_len, r_xfer_len;                           \

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
   add.!c3  r3, r0, PRP_DATA_XFER_SIZE;                                 \
   add      r4, _dst_addr, _xfer_len;                                   \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, r4, r3, 0, 0,              \
                     _src_dma_cmd)                                      \
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, _prp_entry, r3, 0, 0,      \
                     _dst_dma_cmd)                                      \
   add      _xfer_len, _xfer_len, r3;                                   \

/*
 * Set a PHV metrics field to a value
 */
#define SEQ_METRICS_VAL_SET(_metrics, _val)                             \
   phvwr    p.seq_kivec9_##_metrics, _val;                              \
   
#define SEQ_METRICS_SET(_metrics)                                       \
   SEQ_METRICS_VAL_SET(_metrics, 1)                                     \

#define SEQ_METRICS_CLR(_metrics)                                       \
   SEQ_METRICS_VAL_SET(_metrics, 0)                                     \

#define SEQ_METRICS_PARAMS()                                            \
   .param storage_seq_metrics0_commit;                                  \
   .param storage_seq_metrics1_commit;                                  \
   .param storage_seq_metrics2_commit;                                  \

#define SEQ_METRICS_TBLADD_c(_cf, _metrics, _key)                       \
    sne        _cf, _key, r0;                                           \
    tbladd._cf d._metrics, _key;                                        \

#define SEQ_METRICS_TBLADD_c_e(_cf, _metrics, _key)                     \
    sne.e      _cf, _key, r0;                                           \
    tbladd._cf d._metrics, _key;                                        \

/*
 * Launch commit phase for table based metrics
 */ 
#define SEQ_METRICS0_TABLE1_COMMIT(_src_qaddr)                          \
   add  r_src_qaddr, _src_qaddr, SEQ_QSTATE_METRICS0_OFFSET;            \
   LOAD_TABLE1_FOR_ADDR_PC_IMM(r_src_qaddr,                             \
        STORAGE_DEFAULT_TBL_LOAD_SIZE, storage_seq_metrics0_commit)     \

#define SEQ_METRICS0_TABLE1_COMMIT_e(_src_qaddr)                        \
   add  r_src_qaddr, _src_qaddr, SEQ_QSTATE_METRICS0_OFFSET;            \
   LOAD_TABLE1_FOR_ADDR_PC_IMM_e(r_src_qaddr,                           \
        STORAGE_DEFAULT_TBL_LOAD_SIZE, storage_seq_metrics0_commit)     \

#define SEQ_METRICS1_TABLE2_COMMIT(_src_qaddr)                          \
   add  r_src_qaddr, _src_qaddr, SEQ_QSTATE_METRICS1_OFFSET;            \
   LOAD_TABLE2_FOR_ADDR_PC_IMM(r_src_qaddr,                             \
        STORAGE_DEFAULT_TBL_LOAD_SIZE, storage_seq_metrics1_commit)     \

#define SEQ_METRICS1_TABLE2_COMMIT_e(_src_qaddr)                        \
   add  r_src_qaddr, _src_qaddr, SEQ_QSTATE_METRICS1_OFFSET;            \
   LOAD_TABLE2_FOR_ADDR_PC_IMM_e(r_src_qaddr,                           \
        STORAGE_DEFAULT_TBL_LOAD_SIZE, storage_seq_metrics1_commit)     \

#define SEQ_METRICS2_TABLE3_COMMIT(_src_qaddr)                          \
   add  r_src_qaddr, _src_qaddr, SEQ_QSTATE_METRICS2_OFFSET;            \
   LOAD_TABLE3_FOR_ADDR_PC_IMM(r_src_qaddr,                             \
        STORAGE_DEFAULT_TBL_LOAD_SIZE, storage_seq_metrics2_commit)     \

#define SEQ_METRICS2_TABLE3_COMMIT_e(_src_qaddr)                        \
   add  r_src_qaddr, _src_qaddr, SEQ_QSTATE_METRICS2_OFFSET;            \
   LOAD_TABLE3_FOR_ADDR_PC_IMM_e(r_src_qaddr,                           \
        STORAGE_DEFAULT_TBL_LOAD_SIZE, storage_seq_metrics2_commit)     \

/*
 * Set HW Tx rate limiter, with scale factor given in _rl_units_scale.
 */
#define SEQ_RATE_LIMIT_ENABLE_CHECK(_rate_limit_en, _cf)                \
    seq     _cf, _rate_limit_en[0], 1;                                  \

#define SEQ_RATE_LIMIT_DATA_LEN_LOAD_c(_cf, _data_len)                  \
    add._cf r_rl_len, _data_len, STORAGE_SEQ_RL_UNITS_SCALE_DFLT - 1;   \
    add.!_cf r_rl_len, r0, r0;                                          \

#define SEQ_RATE_LIMIT_DATA_LEN_ADD_c(_cf, _data_len)                   \
    add._cf r_rl_len, r_rl_len, _data_len;                              \

#define SEQ_RATE_LIMIT_CLR()                                            \
    phvwr   p.p4_intr_packet_len, r0;                                   \

#define SEQ_RATE_LIMIT_SET_c(_cf)                                       \
    srl._cf r_rl_len, r_rl_len, STORAGE_SEQ_RL_UNITS_SCALE_SHFT;        \
    phvwr._cf p.p4_intr_packet_len, r_rl_len;                           \

#define SEQ_RATE_LIMIT_LOAD_SET_c(_cf, _data_len)                       \
    SEQ_RATE_LIMIT_DATA_LEN_LOAD_c(_cf, _data_len)                      \
    SEQ_RATE_LIMIT_SET_c(_cf)                                           \

/*
 * Compression SGL PDMA transfer length error
 */
#if SEQ_COMP_SGL_PDMA_XFER_DEBUG
#define SEQ_COMP_SGL_PDMA_XFER_ERROR_TRAP()                             \
        illegal;                                                        \
        nop;
#else
#define SEQ_COMP_SGL_PDMA_XFER_ERROR_TRAP()                             \
        nop;
#endif

/*
 * Compression SGL PDMA pad-only error
 */
#if SEQ_COMP_SGL_PDMA_PAD_ONLY_DEBUG
#define SEQ_COMP_SGL_PDMA_PAD_ONLY_ERROR_TRAP()                         \
        illegal;                                                        \
        nop;
#else
#define SEQ_COMP_SGL_PDMA_PAD_ONLY_ERROR_TRAP()                         \
        nop;
#endif

/*
 * MEM2MEM transfer length error
 */
#if DMA_CMD_MEM2MEM_SIZE_DEBUG
#define DMA_CMD_MEM2MEM_SIZE_ERROR_TRAP()                               \
        illegal;                                                        \
        nop;
#else
#define DMA_CMD_MEM2MEM_SIZE_ERROR_TRAP()                               \
        nop;
#endif

#endif     // __STORAGE_ASM_DEFINES_H__
