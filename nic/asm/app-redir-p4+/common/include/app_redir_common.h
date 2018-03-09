#ifndef __APP_REDIR_COMMON_H
#define __APP_REDIR_COMMON_H

#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "../../../p4/iris/include/defines.h"
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
        addi.cf    reg, r0,  loword(imm64);                                     \
        addui.cf   reg, reg, hiword(imm64);
        
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
        add       data_r, ring_id, ring, DB_RING_SHIFT;                         \
        add       data_r, data_r, qid, DB_QID_SHIFT;

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

#endif //__APP_REDIR_COMMON_H

