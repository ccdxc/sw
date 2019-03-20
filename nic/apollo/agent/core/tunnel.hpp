//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_TUNNEL_HPP__
#define __CORE_TUNNEL_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"

namespace core {

sdk_ret_t tep_create(uint32_t key, pds_tep_spec_t *spec);
sdk_ret_t tep_delete(uint32_t key);
sdk_ret_t tep_get(uint32_t key, pds_tep_info_t *info);

}    // namespace core

#endif    // __CORE_TUNNEL_HPP__
