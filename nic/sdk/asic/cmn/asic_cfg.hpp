// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ASIC_CFG_HPP__
#define __ASIC_CFG_HPP__

#include "asic/asic.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/utils/mpartition.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"
#include "p4/loader/loader.hpp"

namespace sdk  {
namespace asic {

sdk_ret_t asic_load_config(char *pathname);
sdk_ret_t asic_verify_config(char *config_dir);

}    // namespace asic 
}    // namespace sdk

#endif    // __ASIC_CFG_HPP__
