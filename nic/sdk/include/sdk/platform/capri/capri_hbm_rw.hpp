#ifndef __CAPRI_HBM_RW_HPP__
#define __CAPRI_HBM_RW_HPP__

#include "include/sdk/platform/capri/capri_cfg.hpp"
#include "nic/sdk/include/sdk/platform/utils/mpartition.hpp"
#include "nic/sdk/include/sdk/platform/p4loader/loader.hpp"

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


sdk_ret_t capri_hbm_parse(std::string cfg_path, std::string pgm_name);
mpartition_region_t *get_hbm_region(char *name);
uint64_t get_hbm_base(void);
uint64_t get_hbm_offset(const char *reg_name);
uint64_t get_start_offset(const char *reg_name);
uint32_t get_size_kb(const char *reg_name);
void reset_hbm_regions(capri_cfg_t *cfg);
int32_t capri_hbm_read_mem(uint64_t addr, uint8_t *buf, uint32_t size);
int32_t capri_hbm_write_mem(uint64_t addr, const uint8_t *buf, uint32_t size);


typedef enum asic_block_e {
    ASIC_BLOCK_PB,
    ASIC_BLOCK_TXD,
    ASIC_BLOCK_RXD,
    ASIC_BLOCK_MS,
    ASIC_BLOCK_PCIE,
    ASIC_BLOCK_MAX
} asic_block_t;

typedef struct asic_bw_s {
    double read;
    double write;
} asic_bw_t;

typedef struct asic_hbm_bw_s {
    asic_block_t type;
    uint64_t clk_diff;
    asic_bw_t max;
    asic_bw_t avg;
} asic_hbm_bw_t;


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

sdk_ret_t
capri_hbm_bw (uint32_t samples, uint32_t u_sleep,
              bool ms_pcie, asic_hbm_bw_t *hbm_bw_arr);

sdk_ret_t
capri_nx_get_llc_counters (uint32_t *rd_data);

sdk_ret_t
capri_nx_setup_llc_counters (uint32_t mask);

extern sdk_ret_t
capri_hbm_cache_init(capri_cfg_t *cfg);

extern sdk_ret_t
capri_hbm_cache_regions_init();

extern mpartition_region_t*
get_hbm_region_by_address(uint64_t addr);

} // namespace capri
} // namespace platform
} // namespace sdk


using sdk::platform::capri::get_start_offset;
using sdk::platform::capri::get_size_kb;
using sdk::platform::capri::capri_hbm_write_mem;
using sdk::platform::capri::capri_hbm_read_mem;
using sdk::platform::capri::asic_hbm_bw_t;
using sdk::platform::capri::asic_block_t::ASIC_BLOCK_MAX;
#endif    // __CAPRI_HBM_RW_HPP__
