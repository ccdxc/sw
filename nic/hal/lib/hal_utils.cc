// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/lib/hal_utils.hpp"

sdk::lib::catalog* 
hal_lib_catalog_init (void)
{
    std::string     catalog_file   = "catalog.json";
    char  *cfg_path = NULL;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        catalog_file = std::string(cfg_path) + "/" + catalog_file;
    }

    return sdk::lib::catalog::factory(catalog_file);
}
