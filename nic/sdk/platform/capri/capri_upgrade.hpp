//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __CAPRI_UPGRADE_HPP__
#define __CAPRI_UPGRADE_HPP__

#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {
namespace capri {

sdk_ret_t capri_upg_table_property_set(p4pd_pipeline_t pipeline,
                                       p4_upg_table_property_t *cfg,
                                       uint32_t ncfgs);
}   // namespace capri
}   // namespace platform
}   // namespace sdk

#endif    // __CAPRI_UPGRADE_HPP__
