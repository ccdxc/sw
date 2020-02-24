//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Basic types for impl classes
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_IMPL_HPP__
#define __FRAMEWORK_IMPL_HPP__

namespace api {
namespace impl {

/// \brief Impl object identifiers
typedef enum impl_obj_id_e {
    IMPL_OBJ_ID_NONE,
    IMPL_OBJ_ID_DEVICE,
    IMPL_OBJ_ID_VPC,
    IMPL_OBJ_ID_IF,
    IMPL_OBJ_ID_SUBNET,
    IMPL_OBJ_ID_TEP,
    IMPL_OBJ_ID_VNIC,
    IMPL_OBJ_ID_ROUTE_TABLE,
    IMPL_OBJ_ID_SECURITY_POLICY,
    IMPL_OBJ_ID_MAPPING,
    IMPL_OBJ_ID_MIRROR_SESSION,
    IMPL_OBJ_ID_METER,
    IMPL_OBJ_ID_TAG,
    IMPL_OBJ_ID_MAX,
    IMPL_OBJ_ID_SVC_MAPPING,
    IMPL_OBJ_ID_VPC_PEER,
    IMPL_OBJ_ID_NEXTHOP,
    IMPL_OBJ_ID_NEXTHOP_GROUP,
    IMPL_OBJ_ID_POLICER,
    IMPL_OBJ_ID_DHCP_RELAY,
} impl_obj_id_t;

// These are for read_hw apis
typedef void obj_info_t;
typedef void obj_key_t;

}    // namespace impl
}    // namespace api

using api::impl::impl_obj_id_t;

#endif    // __FRAMEWORK_IMPL_HPP__
