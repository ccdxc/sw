#ifndef __SDK_CAPRI_HBM_HPP__
#define __SDK_CAPRI_HBM_HPP__

#include "nic/include/capri_common.h"
#include "nic/include/base.hpp"
#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {

#define CAPRI_HBM_MEM_REG_NAME_MAX_LEN 80
#define CARPI_HBM_MEM_NUM_MEM_REGS 128

#define CAPRI_INVALID_OFFSET 0xFFFFFFFFFFFFFFFF

#define JKEY_REGIONS             "regions"
#define JKEY_REGION_NAME         "name"
#define JKEY_SIZE_KB             "size_kb"
#define JKEY_CACHE_PIPE          "cache"
#define JKEY_RESET_REGION        "reset"
#define JKEY_START_OFF           "start_offset"

#define HBM_OFFSET(x)       CAPRI_HBM_OFFSET(x)

typedef enum capri_hbm_cache_pipe_s {
    // Note: Values are used in bitmap
    CAPRI_HBM_CACHE_PIPE_NONE       = 0,
    CAPRI_HBM_CACHE_PIPE_P4IG       = 1,
    CAPRI_HBM_CACHE_PIPE_P4EG       = 2,
    CAPRI_HBM_CACHE_PIPE_P4IG_P4EG  = 3,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_TXDMA     = 4,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_RXDMA     = 8,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_PCIE_DB   = 16,
    CAPRI_HBM_CACHE_PIPE_P4PLUS_ALL       = 28,  // TxDMA + RxDMA + PCIE + DB blocks
} capri_hbm_cache_pipe_t;

typedef struct capri_hbm_region_s {
    char                    mem_reg_name[CAPRI_HBM_MEM_REG_NAME_MAX_LEN];
    uint32_t                size_kb;
    uint64_t                start_offset;
    capri_hbm_cache_pipe_t  cache_pipe;
    bool                    reset;    // true to bzero this region during init
} capri_hbm_region_t;

class capri {
private:
    capri_hbm_region_t *hbm_regions_;
    int num_hbm_regions_;

public:
    sdk_ret_t capri_hbm_parse(const char *hbm_mem_json_file);
    capri_hbm_region_t *get_hbm_region(char *name);
    uint64_t get_hbm_base(void);
    uint64_t get_hbm_offset(const char *reg_name);
    uint64_t get_start_offset(const char *reg_name);
    uint32_t get_size_kb(const char *reg_name);

    capri_hbm_region_t* get_hbm_region_by_address(uint64_t addr);

    capri_hbm_region_t *hbm_region(int i) {
        return i < num_hbm_regions_ ? &hbm_regions_[i] : NULL;
    }

    int num_hbm_regions() { return num_hbm_regions_; }
};

}   // namespace platform
}   // namespace sdk

#endif    // __SDK_CAPRI_HBM_HPP__
