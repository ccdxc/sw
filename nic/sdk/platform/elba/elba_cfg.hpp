// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_CFG_HPP__
#define __ELBA_CFG_HPP__

#include "asic/asic.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/utils/mpartition.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"
#include "p4/loader/loader.hpp"

namespace sdk {
namespace platform {
namespace elba {

sdk_ret_t elba_load_config(char *pathname);
sdk_ret_t elba_verify_config(char *config_dir);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    // __ELBA_CFG_HPP__
