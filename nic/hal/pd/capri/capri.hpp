// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_HPP__
#define __CAPRI_HPP__

#include "nic/utils/bm_allocator/bm_allocator.hpp"

#define CAPRI_REPL_ENTRY_WIDTH          (64)
#define CAPRI_REPL_TABLE_DEPTH          (64*1024)

typedef struct capri_cfg_s {
    std::string      loader_info_file;
    std::string      default_config_dir;
    uint32_t         admin_cos;
    uint32_t         repl_entry_width;
    std::string      pgm_name;
    std::string      cfg_path;    // HAL config path
} capri_cfg_t;

hal_ret_t capri_init(capri_cfg_t *hal_cfg);

#endif    // __CAPRI_HPP__

