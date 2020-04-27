// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_HBM_RW_HPP__
#define __CAPRI_HBM_RW_HPP__

#include "asic/asic.hpp"
#include "platform/utils/mpartition.hpp"
#include "p4/loader/loader.hpp"
#include "asic/asic.hpp"
#include "asic/cmn/asic_hbm.hpp"

namespace sdk {
namespace platform {
namespace capri {

using sdk::platform::utils::mpartition_region_t;

#define RBM_BRIDGE_(x)  (CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_BRIDGE_##x##_BYTE_ADDRESS)
#define RBM_AGENT_(x)   (CAP_ADDR_BASE_MS_RBM_OFFSET + NS_SOC_IP_MEMORYMAP_RBM_M_NOC_REGISTERS_AGENT_##x##_BYTE_ADDRESS)

#define PAGE_SCRATCH_SIZE               512
#define PAGE_SCRATCH_SIZE_BYTES         8

#define    JUMBO_FRAME_SIZE             9216
#define    ETH_FRAME_SIZE               1536

// Below regions to skip the programming. This should be fixed as these are
// pipeline dependent
#define MEM_REGION_RSS_INDIR_TABLE_NAME "rss_indir_table"
#define MEM_REGION_MCAST_REPL_NAME "mcast_repl"

mpartition_region_t *capri_get_mem_region(char *name);
uint64_t capri_get_mem_base(void);
uint64_t capri_get_mem_offset(const char *reg_name);
uint64_t capri_get_mem_addr(const char *reg_name);
uint32_t capri_get_mem_size_kb(const char *reg_name);

typedef struct capri_descr_s {
        uint64_t        scratch[8];
        uint64_t        A0;
        uint32_t        O0;
        uint32_t        L0;
        uint64_t        A1;
        uint32_t        O1;
        uint32_t        L1;
        uint64_t        A2;
        uint32_t        O2;
        uint32_t        L2;
        uint64_t        next_addr;
        uint64_t        reserved;
} capri_descr_t;

typedef struct capri_big_page_s {
        uint64_t        scratch[PAGE_SCRATCH_SIZE_BYTES];
        char            data[JUMBO_FRAME_SIZE];
} capri_big_page_t;

typedef struct capri_small_page_s {
        uint64_t        scratch[PAGE_SCRATCH_SIZE_BYTES];
        char            data[ETH_FRAME_SIZE];
} capri_small_page_t;

sdk_ret_t capri_hbm_bw(uint32_t samples, uint32_t u_sleep,
                       bool ms_pcie, asic_hbm_bw_t *hbm_bw_arr);
sdk_ret_t capri_nx_get_llc_counters(uint32_t *mask, uint32_t *rd_data);
sdk_ret_t capri_nx_setup_llc_counters(uint32_t mask);
sdk_ret_t capri_hbm_cache_init(asic_cfg_t *cfg);
sdk_ret_t capri_hbm_cache_regions_init(asic_cfg_t *cfg);
mpartition_region_t* capri_get_hbm_region_by_address(uint64_t addr);
uint32_t capri_freq_get(void);
sdk_ret_t capri_init (asic_cfg_t *cfg);
uint64_t capri_local_dbaddr (void);
uint64_t capri_local_db32_addr (void);
uint64_t capri_host_dbaddr (void);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

using sdk::platform::capri::capri_get_mem_base;
using sdk::platform::capri::capri_get_mem_offset;
using sdk::platform::capri::capri_get_mem_addr;
using sdk::platform::capri::capri_get_mem_size_kb;
using sdk::platform::capri::capri_get_mem_region;
using sdk::platform::capri::capri_get_hbm_region_by_address;

#endif    // __CAPRI_HBM_RW_HPP__
