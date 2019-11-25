#ifndef __APP_REDIR_COMMON_H
#define __APP_REDIR_COMMON_H

#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "../../../p4/common/defines.h"
#include "../../../rdma/common/include/capri.h"
#include "app_redir_shared.h"
#include "app_redir_headers.h"
#include "../../cpu-p4plus/include/cpu-defines.h"
#include "../../cpu-p4plus/include/cpu-macros.h"

/*
 * Debug flags
 */
#define APP_REDIR_FREE_SEM_PINDEX_FULL_DEBUG 1
#define APP_REDIR_TXDMA_INVALID_AOL_DEBUG    1


/*
 * Struct field bit offset and size
 */
#define APP_REDIR_BIT_OFFS_STRUCT(struct_name, field)                           \
    offsetof(struct struct_name, field)
    
#define APP_REDIR_BIT_SIZE_STRUCT(struct_name, field)                           \
    sizeof(struct struct_name.field)
    
#define APP_REDIR_BIT_OFFS_PHV(phv_field)                                       \
    offsetof(p, phv_field)

#define APP_REDIR_BIT_OFFS_D_VEC(field)                                         \
    offsetof(d, field)
    
#define APP_REDIR_BIT_OFFS_DMA_MEM2PKT(field)                                   \
    APP_REDIR_BIT_OFFS_STRUCT(capri_dma_cmd_mem2pkt_t, field)
    
#define APP_REDIR_BIT_SIZE_DMA_MEM2PKT(field)                                   \
    APP_REDIR_BIT_SIZE_STRUCT(capri_dma_cmd_mem2pkt_t, field)

    
/*
 * Intrinsic fields
 */
#ifndef CAPRI_INTRINSIC_LIF
#define CAPRI_INTRINSIC_LIF                                                     \
        k.{p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10}
#endif
#ifndef CAPRI_INTRINSIC_TM_INSTANCE_TYPE
#define CAPRI_INTRINSIC_TM_INSTANCE_TYPE    k.{p4_intr_global_tm_instance_type}
#endif

#ifndef CAPRI_RXDMA_INTRINSIC_QTYPE
#define CAPRI_RXDMA_INTRINSIC_QTYPE         k.{p4_rxdma_intr_qtype}
#endif
#ifndef CAPRI_RXDMA_INTRINSIC_QID
#define CAPRI_RXDMA_INTRINSIC_QID           k.{p4_rxdma_intr_qid}
#endif
#ifndef CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR
#define CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR                                       \
            k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
#endif
#ifndef CAPRI_TXDMA_INTRINSIC_QTYPE
#define CAPRI_TXDMA_INTRINSIC_QTYPE     k.{p4_txdma_intr_qtype}
#endif
#ifndef CAPRI_TXDMA_INTRINSIC_QID
#define CAPRI_TXDMA_INTRINSIC_QID       k.{p4_txdma_intr_qid}
#endif
#ifndef CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
#define CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR                                       \
        k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
#endif

/*
 * From capri_hbm.hpp
 */
#ifndef JUMBO_FRAME_SIZE
#define JUMBO_FRAME_SIZE                9216
#endif
#ifndef ETH_FRAME_SIZE
#define ETH_FRAME_SIZE                  1536
#endif

#define APP_REDIR_PPAGE_SIZE            JUMBO_FRAME_SIZE
#define APP_REDIR_MPAGE_SIZE            ETH_FRAME_SIZE

/*
 * When semaphore access is not applicable, service chain queues may be directly
 * accessed from an HBM table. For example, if the next service queue were a
 * per-core ARQ, the following indices would represent core IDs.
 * 
 */
#define CHAIN_QIDXR_PI_0                0
#define CHAIN_QIDXR_PI_1                1
#define CHAIN_QIDXR_PI_2                2


/*
 * Load 64-bit immediate
 */
#define APP_REDIR_IMM64_LOAD(reg, imm64)                                        \
        addi        reg, r0,  loword(imm64);                                    \
        addui       reg, reg, hiword(imm64);
    
#define APP_REDIR_IMM64_LOAD_CF(cf, reg, imm64)                                 \
        addi.cf     reg, r0,  loword(imm64);                                    \
        addui.cf    reg, reg, hiword(imm64);
        
/*
 * Free semaphore pindex full trap
 */
#if APP_REDIR_FREE_SEM_PINDEX_FULL_DEBUG
#define APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()                                   \
        illegal;                                                                \
        nop;
#else
#define APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()                                   \
        nop;
#endif


/*
 * TxDMA invalid AOLs trap
 */
#if APP_REDIR_TXDMA_INVALID_AOL_DEBUG
#define APP_REDIR_TXDMA_INVALID_AOL_TRAP()                                      \
        illegal;                                                                \
        nop;
#else
#define APP_REDIR_TXDMA_INVALID_AOL_TRAP()                                      \
        nop;
#endif


/*
 * Doorbell setup
 */
#define APP_REDIR_SETUP_DB_ADDR(db_base, op, sched, lif, qtype, addr_r)         \
        add         addr_r, sched, op, DB_RING_UPD_SHIFT;                       \
        sll         addr_r, addr_r, DB_UPD_SHIFT;                               \
        add         addr_r, addr_r, lif, DB_LIF_SHIFT;                          \
        add         addr_r, addr_r, qtype, DB_QTYPE_SHIFT;                      \
        addi        addr_r, addr_r, db_base;

#define APP_REDIR_SETUP_DB_DATA(qid, ring, ring_id, data_r)                     \
        add         data_r, ring_id, ring, DB_RING_SHIFT;                       \
        add         data_r, data_r, qid, DB_QID_SHIFT;

/*
 * Launch error statistic increment
 */
#define APP_REDIR_STAT_INC_LAUNCH(table, qstate_addr_dst, qstate_addr_src,      \
                                  stat_table_offs, phv_inc_stat, launch_fn)     \
        add         qstate_addr_dst, qstate_addr_src, stat_table_offs;          \
        phvwri      phv_inc_stat, 1;                                            \
        CAPRI_NEXT_TABLE_READ(table, TABLE_LOCK_EN,                             \
                              launch_fn,                                        \
                              qstate_addr_dst,                                  \
                              TABLE_SIZE_512_BITS)

/*
 * Metrics related
 */
#define APP_REDIR_METRICS_VAL_SET(_kivec_metrics, _val)                         \
        phvwr       p._kivec_metrics, _val;                                     \
   
#define APP_REDIR_METRICS_VAL_SET_c(_cf, _kivec_metrics, _val)                  \
        phvwr._cf   p._kivec_metrics, _val;                                     \
   
#define APP_REDIR_METRICS_TBLADD_c(_cf, _metrics, _key)                         \
        sne         _cf, _key, r0;                                              \
        tbladd._cf  d._metrics, _key;                                           \

#define APP_REDIR_METRICS_TBLADD_c_e(_cf, _metrics, _key)                       \
        sne.e       _cf, _key, r0;                                              \
        tbladd._cf  d._metrics, _key;                                           \
    
#define RAWR_METRICS_VAL_SET(_metrics, _val)                                    \
        APP_REDIR_METRICS_VAL_SET(rawr_kivec9_##_metrics, _val)                 \
   
#define RAWR_METRICS_VAL_SET_c(_c, _metrics, _val)                              \
        APP_REDIR_METRICS_VAL_SET_c(_c, rawr_kivec9_##_metrics, _val)           \
   
#define RAWR_METRICS_SET(_metrics)                                              \
        RAWR_METRICS_VAL_SET(_metrics, 1)                                       \

#define RAWR_METRICS_SET_c(_c, _metrics)                                        \
        RAWR_METRICS_VAL_SET_c(_c, _metrics, 1)                                 \

#define RAWR_METRICS_CLR(_metrics)                                              \
        RAWR_METRICS_VAL_SET(_metrics, 0)                                       \

#define RAWR_METRICS_PARAMS()                                                   \
        .param      rawr_metrics0_commit;                                       \

#define RAWC_METRICS_VAL_SET(_metrics, _val)                                    \
        APP_REDIR_METRICS_VAL_SET(rawc_kivec9_##_metrics, _val)                 \
   
#define RAWC_METRICS_VAL_SET_c(_c, _metrics, _val)                              \
        APP_REDIR_METRICS_VAL_SET_c(_c, rawc_kivec9_##_metrics, _val)           \
   
#define RAWC_METRICS_SET(_metrics)                                              \
        RAWC_METRICS_VAL_SET(_metrics, 1)                                       \

#define RAWC_METRICS_SET_c(_c, _metrics)                                        \
        RAWC_METRICS_VAL_SET_c(_c, _metrics, 1)                                 \

#define RAWC_METRICS_CLR(_metrics)                                              \
        RAWC_METRICS_VAL_SET(_metrics, 0)                                       \

#define RAWC_METRICS_PARAMS()                                                   \
        .param      rawc_metrics0_commit;                                       \

/*
 * Launch commit phase for table based metrics
 */ 
#define RAWR_METRICS0_TABLE2_COMMIT(_src_qaddr)                                 \
        add         r_qstate_addr, _src_qaddr, RAWRCB_TABLE_METRICS0_OFFSET;    \
        CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN,                                 \
                              rawr_metrics0_commit,                             \
                              r_qstate_addr,                                    \
                              TABLE_SIZE_512_BITS)                              \

#define RAWR_METRICS0_TABLE2_COMMIT_e(_src_qaddr)                               \
        add         r_qstate_addr, _src_qaddr, RAWRCB_TABLE_METRICS0_OFFSET;    \
        CAPRI_NEXT_TABLE_READ_e(2, TABLE_LOCK_EN,                               \
                                rawr_metrics0_commit,                           \
                                r_qstate_addr,                                  \
                                TABLE_SIZE_512_BITS)                            \
        nop;                                                                    \
        
#define RAWC_METRICS0_TABLE2_COMMIT(_src_qaddr)                                 \
        add         r_qstate_addr, _src_qaddr, RAWCCB_TABLE_METRICS0_OFFSET;    \
        CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN,                                 \
                              rawc_metrics0_commit,                             \
                              r_qstate_addr,                                    \
                              TABLE_SIZE_512_BITS)                              \

#define RAWC_METRICS0_TABLE2_COMMIT_e(_src_qaddr)                               \
        add         r_qstate_addr, _src_qaddr, RAWCCB_TABLE_METRICS0_OFFSET;    \
        CAPRI_NEXT_TABLE_READ_e(2, TABLE_LOCK_EN,                               \
                                rawc_metrics0_commit,                           \
                                r_qstate_addr,                                  \
                                TABLE_SIZE_512_BITS)                            \
        nop;                                                                    \

/* 
 * Macros for accessing fields in K+I vectors
 */
#define RAWR_KIVEC0_CHAIN_RING_INDEX_SELECT                                     \
    k.{rawr_kivec0_chain_ring_index_select}
#define RAWR_KIVEC0_CHAIN_RING_SIZE_SHIFT                                       \
    k.{rawr_kivec0_chain_ring_size_shift}
#define RAWR_KIVEC0_RAWRCB_FLAGS                                                \
    k.{rawr_kivec0_rawrcb_flags}
#define RAWR_KIVEC0_PACKET_LEN                                                  \
    k.{rawr_kivec0_packet_len}
#define RAWR_KIVEC0_QSTATE_ADDR                                                 \
    k.{rawr_kivec0_qstate_addr_sbit0_ebit31...rawr_kivec0_qstate_addr_sbit32_ebit33}
#define RAWR_KIVEC0_PPAGE                                                       \
    k.{rawr_kivec0_ppage_sbit0_ebit5...rawr_kivec0_ppage_sbit38_ebit39}
#define RAWR_KIVEC0_CHAIN_TO_RXQ                                                \
    k.{rawr_kivec0_chain_to_rxq}
#define RAWR_KIVEC0_REDIR_SPAN_INSTANCE                                         \
    k.{rawr_kivec0_redir_span_instance}
#define RAWR_KIVEC0_PPAGE_SEM_PINDEX_FULL                                       \
    k.{rawr_kivec0_ppage_sem_pindex_full}
#define RAWR_KIVEC0_DO_CLEANUP_DISCARD                                          \
    k.{rawr_kivec0_do_cleanup_discard}
#define RAWR_KIVEC0_PKT_FREEQ_NOT_CFG                                           \
    k.{rawr_kivec0_pkt_freeq_not_cfg}

#define RAWR_KIVEC1_CHAIN_RING_INDICES_ADDR                                     \
    k.{rawr_kivec1_chain_ring_indices_addr}
#define RAWR_KIVEC1_ASCQ_SEM_INF_ADDR                                           \
    k.{rawr_kivec1_ascq_sem_inf_addr}
    
#define RAWR_KIVEC2_CHAIN_LIF                                                   \
    k.{rawr_kivec2_chain_lif_sbit0_ebit7...rawr_kivec2_chain_lif_sbit8_ebit10}
#define RAWR_KIVEC2_CHAIN_QTYPE                                                 \
    k.{rawr_kivec2_chain_qtype}
#define RAWR_KIVEC2_CHAIN_QID                                                   \
    k.{rawr_kivec2_chain_qid_sbit0_ebit1...rawr_kivec2_chain_qid_sbit18_ebit23}
#define RAWR_KIVEC2_CHAIN_RING_BASE                                             \
    k.{rawr_kivec2_chain_ring_base_sbit0_ebit1...rawr_kivec2_chain_ring_base_sbit58_ebit63}
#define RAWR_KIVEC2_CHAIN_ENTRY_SIZE_SHIFT                                      \
    k.{rawr_kivec2_chain_entry_size_shift_sbit0_ebit1...rawr_kivec2_chain_entry_size_shift_sbit2_ebit4}

#define RAWR_KIVEC3_ASCQ_BASE                                                   \
    k.{rawr_kivec3_ascq_base}
    
#define RAWR_KIVEC9_REDIR_PKTS                                                  \
    k.{rawr_kivec9_redir_pkts}
#define RAWR_KIVEC9_CB_NOT_READY_DISCARDS                                       \
    k.{rawr_kivec9_cb_not_ready_discards}
#define RAWR_KIVEC9_QSTATE_CFG_DISCARDS                                         \
    k.{rawr_kivec9_qstate_cfg_discards}
#define RAWR_KIVEC9_PKT_LEN_DISCARDS                                            \
    k.{rawr_kivec9_pkt_len_discards}
#define RAWR_KIVEC9_RXQ_FULL_DISCARDS                                           \
    k.{rawr_kivec9_rxq_full_discards}
#define RAWR_KIVEC9_TXQ_FULL_DISCARDS                                           \
    k.{rawr_kivec9_txq_full_discards}
#define RAWR_KIVEC9_PKT_ALLOC_ERRORS                                            \
    k.{rawr_kivec9_pkt_alloc_errors}
#define RAWR_KIVEC9_PKT_FREE_ERRORS                                             \
    k.{rawr_kivec9_pkt_free_errors}
#define RAWR_KIVEC9_METRICS0_RANGE                                              \
    k.{rawr_kivec9_metrics0_start...rawr_kivec9_metrics0_end}

#define RAWC_KIVEC0_CHAIN_TXQ_RING_SIZE_SHIFT                                   \
    k.{rawc_kivec0_chain_txq_ring_size_shift}
#define RAWC_KIVEC0_CHAIN_TXQ_ENTRY_SIZE_SHIFT                                  \
    k.{rawc_kivec0_chain_txq_entry_size_shift}
#define RAWC_KIVEC0_RAWCCB_FLAGS                                                \
    k.{rawc_kivec0_rawccb_flags}
#define RAWC_KIVEC0_QSTATE_ADDR                                                 \
    k.{rawc_kivec0_qstate_addr}
#define RAWC_KIVEC0_DO_CLEANUP_DISCARD                                          \
    k.{rawc_kivec0_do_cleanup_discard}
#define RAWC_KIVEC0_NEXT_SERVICE_CHAIN_ACTION                                   \
    k.{rawc_kivec0_next_service_chain_action}
#define RAWC_KIVEC0_PKT_FREEQ_NOT_CFG                                           \
    k.{rawc_kivec0_pkt_freeq_not_cfg}

#define RAWC_KIVEC1_CHAIN_TXQ_RING_INDICES_ADDR                                 \
    k.{rawc_kivec1_chain_txq_ring_indices_addr}

#define RAWC_KIVEC2_CHAIN_TXQ_BASE                                              \
    k.{rawc_kivec2_chain_txq_base}
#define RAWC_KIVEC2_CHAIN_TXQ_LIF                                               \
    k.{rawc_kivec2_chain_txq_lif_sbit0_ebit7...rawc_kivec2_chain_txq_lif_sbit8_ebit10}
#define RAWC_KIVEC2_CHAIN_TXQ_QTYPE                                             \
    k.{rawc_kivec2_chain_txq_qtype}
#define RAWC_KIVEC2_CHAIN_TXQ_QID                                               \
    k.{rawc_kivec2_chain_txq_qid_sbit0_ebit1...rawc_kivec2_chain_txq_qid_sbit18_ebit23}
#define RAWC_KIVEC2_CHAIN_TXQ_RING                                              \
    k.{rawc_kivec2_chain_txq_ring_sbit0_ebit1...rawc_kivec2_chain_txq_ring_sbit2_ebit2}

#define RAWC_KIVEC3_DESC                                                        \
    k.{rawc_kivec3_desc}
#define RAWC_KIVEC3_ASCQ_SEM_INF_ADDR                                           \
    k.{rawc_kivec3_ascq_sem_inf_addr}
#define RAWC_KIVEC3_LAST_MEM2PKT_PTR                                            \
    k.{rawc_kivec3_last_mem2pkt_ptr}
    
#define RAWC_KIVEC4_ASCQ_BASE                                                   \
    k.{rawc_kivec4_ascq_base}
    
#define RAWC_KIVEC9_CHAIN_PKTS                                                  \
    k.{rawc_kivec9_chain_pkts}
#define RAWC_KIVEC9_CB_NOT_READY_DISCARDS                                       \
    k.{rawc_kivec9_cb_not_ready_discards}
#define RAWC_KIVEC9_QSTATE_CFG_DISCARDS                                         \
    k.{rawc_kivec9_qstate_cfg_discards}
#define RAWC_KIVEC9_AOL_ERROR_DISCARDS                                          \
    k.{rawc_kivec9_aol_error_discards}
#define RAWC_KIVEC9_MY_TXQ_EMPTY_DISCARDS                                       \
    k.{rawc_kivec9_my_txq_empty_discards}
#define RAWC_KIVEC9_TXQ_FULL_DISCARDS                                           \
    k.{rawc_kivec9_txq_full_discards}
#define RAWC_KIVEC9_PKT_FREE_ERRORS                                             \
    k.{rawc_kivec9_pkt_free_errors}
#define RAWC_KIVEC9_METRICS0_RANGE                                              \
    k.{rawc_kivec9_metrics0_start...rawc_kivec9_metrics0_end}

    
#define RAWR_CLEANUP_STAGE              CAPRI_STAGE_5
#define RAWR_METRICS_STAGE              CAPRI_STAGE_7

#define RAWC_METRICS_STAGE              CAPRI_STAGE_7

#endif //__APP_REDIR_COMMON_H

