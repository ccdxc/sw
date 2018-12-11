// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_HPP__
#define __CAPRI_HPP__

#include "nic/sdk/include/sdk/catalog.hpp"
#include "include/sdk/bm_allocator.hpp"
#include "include/sdk/platform/p4loader/loader.hpp"

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
} capri_cfg_t;

#endif    // __CAPRI_HPP__
