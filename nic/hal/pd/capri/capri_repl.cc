// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_repl.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"

/* HBM base address in System memory map; Cached once at the init time */
uint64_t capri_hbm_base;
uint64_t hbm_repl_table_offset;

hal_ret_t
capri_repl_init (capri_cfg_t *cfg)
{
    capri_hbm_base = get_hbm_base();
    hbm_repl_table_offset = get_hbm_offset(JP4_REPL);
    if (hbm_repl_table_offset != CAPRI_INVALID_OFFSET) {
        capri_tm_repl_table_base_addr_set(hbm_repl_table_offset / CAPRI_REPL_ENTRY_WIDTH);
        capri_tm_repl_table_token_size_set(cfg->repl_entry_width * 8);
    }

    return HAL_RET_OK;
}
