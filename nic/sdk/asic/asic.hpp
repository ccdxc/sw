// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASIC_HPP__
#define __SDK_ASIC_HPP__

#include <string>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/catalog.hpp"
#include "nic/sdk/include/sdk/platform/p4loader/loader.hpp"

namespace sdk {
namespace asic {

#define SDK_ASIC_PGM_CFG_MAX        3
#define SDK_ASIC_ASM_CFG_MAX        3

typedef enum asic_type_e {
    SDK_ASIC_TYPE_NONE,
    SDK_ASIC_TYPE_CAPRI,
} asic_type_t;

typedef struct asic_pgm_cfg_s {
    std::string                 path;
} asic_pgm_cfg_t;

typedef struct asic_asm_cfg_s {
    std::string          name;
    std::string          path;
    std::string          base_addr;
    mpu_pgm_sort_t       sort_func;
    mpu_pgm_symbols_t    symbols_func;
} asic_asm_cfg_t;

typedef struct asic_cfg_s {
    asic_type_t          asic_type;
    std::string          loader_info_file;
    std::string          default_config_dir;    // TODO: vasanth, pls. remove this up eventually
    uint32_t             admin_cos;
    uint32_t             repl_entry_width;
    std::string          cfg_path;
    std::string          pgm_name;
    uint8_t              num_pgm_cfgs;
    uint8_t              num_asm_cfgs;
    asic_pgm_cfg_t       pgm_cfg[SDK_ASIC_PGM_CFG_MAX];
    asic_asm_cfg_t       asm_cfg[SDK_ASIC_ASM_CFG_MAX];
    sdk::lib::catalog    *catalog;
    platform_type_t      platform;
} asic_cfg_t;

}    // namespace asic
}    // namespace sdk

using sdk::asic::asic_type_t;
using sdk::asic::asic_pgm_cfg_t;
using sdk::asic::asic_asm_cfg_t;
using sdk::asic::asic_cfg_t;

#endif    // __SDK_ASIC_HPP__
