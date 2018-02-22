// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_REPL_HPP__
#define __CAPRI_REPL_HPP__

#include "nic/hal/pd/capri/capri.hpp"

#define CAPRI_REPL_ENTRY_WIDTH          (64)
#define CAPRI_REPL_TABLE_DEPTH          (64*1024)

hal_ret_t capri_repl_init (capri_cfg_t *cfg);

#endif    // __CAPRI_REPL_HPP__
