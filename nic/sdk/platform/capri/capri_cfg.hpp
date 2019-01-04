// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_CFG_HPP__
#define __CAPRI_CFG_HPP__

#include "nic/sdk/asic/asic.hpp"
#include "nic/sdk/include/sdk/catalog.hpp"
#include "include/sdk/bm_allocator.hpp"
#include "platform/p4loader/loader.hpp"

namespace sdk {
namespace platform {
namespace capri {

#define CAPRI_REPL_ENTRY_WIDTH          (64)
#define CAPRI_REPL_TABLE_DEPTH          (64*1024)
#define CAPRI_PGM_CFG_MAX               3
#define CAPRI_ASM_CFG_MAX               3


typedef struct capri_pgm_cfg_s {
    std::string                 path;
} capri_pgm_cfg_t;

typedef struct capri_asm_cfg_s {
    std::string                 name;
    std::string                 path;
    std::string                 base_addr;
    sdk::platform::mpu_pgm_sort_t              sort_func;
    sdk::platform::mpu_pgm_symbols_t           symbols_func;
} capri_asm_cfg_t;

typedef struct capri_cfg_s {
    std::string                 loader_info_file;
    std::string                 default_config_dir;
    uint32_t                    admin_cos;
    uint32_t                    repl_entry_width;
    bool                        p4_cache;
    bool                        p4plus_cache;
    bool                        llc_cache;
    std::string                 pgm_name;
    uint8_t                     num_pgm_cfgs;
    uint8_t                     num_asm_cfgs;
    capri_pgm_cfg_t             pgm_cfg[CAPRI_PGM_CFG_MAX];
    capri_asm_cfg_t             asm_cfg[CAPRI_ASM_CFG_MAX];
    std::string                 cfg_path;
    sdk::lib::catalog           *catalog;
    sdk::types::platform_type_t platform;
    sdk::asic::completion_cb_t completion_func;
} capri_cfg_t;

sdk_ret_t capri_load_config(char *pathname);
sdk_ret_t capri_verify_config(char *config_dir);

} // namespace capri
} // namespace platform
} // namespace sdk

using sdk::platform::capri::capri_cfg_t;
using sdk::platform::capri::capri_asm_cfg_t;
using sdk::platform::capri::capri_pgm_cfg_t;

#endif    // __CAPRI_CFG_HPP__
