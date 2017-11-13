#ifndef __BARCO_RINGS_HPP__
#define __BARCO_RINGS_HPP__
#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/barco_rings.pb.h"
#include "nic/gen/proto/hal/types.pb.h"

using std::string;
using barcoRings::GetOpaqueTagAddrRequest;
using barcoRings::GetOpaqueTagAddrResponse;
using barcoRings::GetOpaqueTagAddrRequestMsg;
using barcoRings::GetOpaqueTagAddrResponseMsg;

namespace hal {

hal_ret_t   GetOpaqueTagAddr(const GetOpaqueTagAddrRequest& request,
    GetOpaqueTagAddrResponse *response);

}    // namespace hal
#endif  /* __BARCO_RINGS_HPP__*/

