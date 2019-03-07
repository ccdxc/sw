//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_IRIS_TYPES_HPP__
#define __DEVAPI_IRIS_TYPES_HPP__

#include "devapi_types.hpp"
#include "gen/proto/kh.grpc.pb.h"

namespace iris {

#define LIF_DEFAULT_MAX_VLAN_FILTERS        4096
#define LIF_DEFAULT_MAX_MAC_FILTERS         4096
#define LIF_DEFAULT_MAX_MAC_VLAN_FILTERS    400

#define NATIVE_VLAN_ID 8192

typedef uint32_t uplink_id_t;
typedef kh::FilterType filter_type_t;
typedef std::tuple<mac_t, vlan_t> mac_vlan_t;
typedef std::tuple<filter_type_t, mac_t, vlan_t> mac_vlan_filter_t;

}     // namespace iris


#endif  // __DEVAPI_IRIS_TYPES_HPP__
