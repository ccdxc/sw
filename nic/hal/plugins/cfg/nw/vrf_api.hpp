//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __VRF_API_HPP__
#define __VRF_API_HPP__

#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"

namespace hal {

vrf_t *vrf_get_infra_vrf(void);
bool is_mytep(vrf_t& vrf, const ipvx_addr_t *ipaddr);
bool vrf_if_is_designated_uplink(vrf_t *vrf, if_t *hal_if);

} // namespace hal


#endif // __VRF_API_HPP__
