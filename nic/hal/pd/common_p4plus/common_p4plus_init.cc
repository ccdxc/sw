// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "platform/capri/capri_pxb_pcie.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "platform/capri/capri_common.hpp"

namespace hal {
namespace pd {

#define IPSEC_N2H_GLOBAL_STATS_OFFSET 512 

#define P4PLUS_SYMBOLS_MAX 52

uint32_t
common_p4plus_symbols_init (void **p4plus_symbols, platform_type_t platform_type)
{
    uint32_t    i = 0;
    uint64_t    offset;
    uint32_t capri_coreclk_freq; //Mhz

    capri_coreclk_freq = (uint32_t)(sdk::platform::capri::capri_get_coreclk_freq(platform_type) / 1000000);

    HAL_TRACE_DEBUG("Capri core clock freq is {} Mhz", capri_coreclk_freq);


    *p4plus_symbols = (sdk::p4::p4_param_info_t *)
        HAL_CALLOC(hal::HAL_MEM_ALLOC_PD,
                   P4PLUS_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = TCP_PROXY_STATS;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = TCP_OOQ_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TCP_OOO_QBASE_RING);
    i++;

    symbols[i].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDPR_BIG_TX);
    i++;

    symbols[i].name = TLS_PROXY_GLOBAL_STATS;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TLS_PROXY_GLOBAL_STATS);
    i++;

    symbols[i].name = BRQ_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_BRQ);
    i++;

    symbols[i].name = BRQ_GCM1_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_BARCO_RING_GCM1);
    i++;

    symbols[i].name = IPSEC_PAD_BYTES_HBM_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_PAD_TABLE);
    i++;

    symbols[i].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = IPSEC_CB_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSECCB);
    i++;

    symbols[i].name = IPSEC_RNMPR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_NMPR_RX);
    i++;

    symbols[i].name = IPSEC_TNMPR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_NMPR_TX);
    i++;

    symbols[i].name = IPSEC_RNMDR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_NMDR_RX);
    i++;

    symbols[i].name = IPSEC_TNMDR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_NMDR_TX);
    i++;

    symbols[i].name = IPSEC_PAGE_ADDR_RX;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_PAGE_BIG_RX) + (4096 * 9216);
    i++;

    symbols[i].name = IPSEC_PAGE_ADDR_TX;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_PAGE_BIG_TX)+ (4096 * 9216);
    i++;

    symbols[i].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + IPSEC_N2H_GLOBAL_STATS_OFFSET;
    i++;

    symbols[i].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = CPU_RX_DPR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_CPU_RX_DPR);
    i++;

    symbols[i].name = CPU_TX_DOT1Q_HDR_OFFSET;
    symbols[i].val = sizeof(hal::pd::cpu_to_p4plus_header_t) + \
        sizeof(hal::pd::p4plus_to_p4_header_t) + L2HDR_DOT1Q_OFFSET;
    i++;

    symbols[i].name = ARQRX_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_ARQRX);
    i++;

    if ((offset = get_mem_addr("flow_hash")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = P4_FLOW_HASH_BASE;
        symbols[i].val = offset;
        i++;
        if ((offset = get_mem_addr("flow_hash_overflow")) != INVALID_MEM_ADDRESS) {
            symbols[i].name = P4_FLOW_HASH_OVERFLOW_BASE;
            symbols[i].val = offset;
            i++;
        }
    }

    if ((offset = get_mem_addr("flow_info")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = P4_FLOW_INFO_BASE;
        symbols[i].val = offset;
        i++;
    }

    if ((offset = get_mem_addr("session_state")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = P4_SESSION_STATE_BASE;
        symbols[i].val = offset;
        i++;
    }

    if ((offset = get_mem_addr("flow_stats")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = P4_FLOW_STATS_BASE;
        symbols[i].val = offset;
        i++;

        symbols[i].name = P4_FLOW_ATOMIC_STATS_BASE;
        symbols[i].val = get_mem_addr(CAPRI_HBM_REG_P4_ATOMIC_STATS);
        i++;
    }

    symbols[i].name = RNMDR_GC_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDR_RX_GC);
    i++;

    symbols[i].name = TNMDR_GC_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDR_TX_GC);
    i++;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TCP/TLS global resources.
     */
    symbols[i].name = GC_GLOBAL_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                    CAPRI_GC_GLOBAL_TABLE;
    i++;

    symbols[i].name = BRQ_MPP1_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_BARCO_RING_MPP1);
    i++;

    symbols[i].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);
    i++;

    symbols[i].name = RDMA_PCIE_ATOMIC_BASE_ADDR;
    symbols[i].val = CAPRI_PCIE_ATOMIC_BASE_ADDR;
    i++;

    symbols[i].name = P4PT_TCB_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_P4PT_TCB_MEM);
    i++;

    symbols[i].name = P4PT_TCB_ISCSI_REC_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_P4PT_TCB_ISCSI_REC_MEM);
    i++;

    symbols[i].name = P4PT_TCB_ISCSI_STATS_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_P4PT_TCB_ISCSI_STATS_MEM);
    i++;

    symbols[i].name = P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_MEM);
    i++;

    symbols[i].name = P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_MEM);
    i++;

    symbols[i].name = RNMDR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDPR_SMALL_RX);
    i++;

    symbols[i].name = RNMPR_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDPR_SMALL_RX);
    i++;

    symbols[i].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDPR_SMALL_RX);
    i++;

    symbols[i].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].val = capri_coreclk_freq;
    i++;

    symbols[i].name = CAPRI_CPU_HASH_MASK;
    symbols[i].val = 0x1;
    i++;

    symbols[i].name = CAPRI_CPU_MAX_ARQID;
    symbols[i].val = 0x0;
    i++;

    symbols[i].name = BRQ_MPP2_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_BARCO_RING_MPP2);
    i++;

    symbols[i].name = BRQ_MPP3_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_BARCO_RING_MPP3);
    i++;

    symbols[i].name = TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE);
    i++;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = RNMDR_TLS_GC_TABLE_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_NMDR_RX_GC) +
        CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE * CAPRI_HBM_RNMDR_ENTRY_SIZE * CAPRI_RNMDR_GC_TLS_RING_PRODUCER;
    i++;

    symbols[i].name = LIF_STATS_BASE;
    symbols[i].val = get_mem_addr(CAPRI_HBM_REG_LIF_STATS);
    i++;

    // Please increment CAPRI_P4PLUS_NUM_SYMBOLS when you want to add more below
    SDK_ASSERT(i <= P4PLUS_SYMBOLS_MAX);

    return i;
}

}    // namespace pd
}    // namespace hal
