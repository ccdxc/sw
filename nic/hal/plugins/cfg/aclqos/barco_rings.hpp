//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __BARCO_RINGS_HPP__
#define __BARCO_RINGS_HPP__
#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/proto/internal.pb.h"
#include "gen/proto/types.pb.h"

using std::string;
using internal::GetOpaqueTagAddrRequest;
using internal::GetOpaqueTagAddrResponse;
using internal::GetOpaqueTagAddrRequestMsg;
using internal::GetOpaqueTagAddrResponseMsg;
using internal::BarcoGetReqDescrEntryRequest;
using internal::BarcoGetReqDescrEntryRequestMsg;
using internal::BarcoGetReqDescrEntryResponse;
using internal::BarcoGetReqDescrEntryResponseMsg;
using internal::BarcoGetRingMetaRequest;
using internal::BarcoGetRingMetaRequestMsg;
using internal::BarcoGetRingMetaResponse;
using internal::BarcoGetRingMetaResponseMsg;
using internal::BarcoGetRingMetaConfigRequest;
using internal::BarcoGetRingMetaConfigRequestMsg;
using internal::BarcoGetRingMetaConfigResponse;
using internal::BarcoGetRingMetaConfigResponseMsg;

namespace hal {

hal_ret_t barco_get_opaque_tag_addr(types::BarcoRings ring_type, uint64_t* addr);

}    // namespace hal
#endif  /* __BARCO_RINGS_HPP__*/

