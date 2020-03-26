// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_CFG_HPP__
#define __CAPRI_CFG_HPP__

#include "asic/asic.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/utils/mpartition.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"
#include "p4/loader/loader.hpp"

namespace sdk {
namespace platform {
namespace capri {

sdk_ret_t capri_load_config(char *pathname);
sdk_ret_t capri_verify_config(char *config_dir);
sdk_ret_t capri_upgrade_config(asic_cfg_t *cfg);

sdk_ret_t capri_init(asic_cfg_t *cfg);
uint64_t capri_local_dbaddr(void);
uint64_t capri_local_db32_addr(void);
uint64_t capri_host_dbaddr(void);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

#endif    // __CAPRI_CFG_HPP__
