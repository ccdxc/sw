// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_HPP__
#define __CAPRI_HPP__

#include "nic/utils/bm_allocator/bm_allocator.hpp"

typedef struct capri_cfg_s {
    std::string      loader_info_file;
    std::string      default_config_dir;
    uint32_t         admin_cos;
    std::string      pgm_name;
} capri_cfg_t;

// TODO: Need to redefine class according to factory pattern
class capri_state_pd {
public:
    // get APIs for TXS scheduler related state
    hal::BMAllocator *txs_scheduler_map_idxr(void) { return txs_scheduler_map_idxr_; }
    // TXS scheduler related state
    struct {
        hal::BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;
};

hal_ret_t capri_init(capri_cfg_t *hal_cfg);
hal_ret_t capri_repl_init(void);

#endif    // __CAPRI_HPP__
