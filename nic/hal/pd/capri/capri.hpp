// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_HPP__
#define __CAPRI_HPP__

typedef struct capri_cfg_s {
    std::string      loader_info_file;
    bool             init_with_pbc_hbm;
    uint32_t         admin_cos;
} capri_cfg_t;

hal_ret_t capri_init(capri_cfg_t *hal_cfg);
hal_ret_t capri_repl_init(void);

#endif    // __CAPRI_HPP__
