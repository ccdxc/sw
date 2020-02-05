//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file captures PDS memory related helpers
///
//----------------------------------------------------------------------------

#ifndef __CORE_MEM_HPP__
#define __CORE_MEM_HPP__

#include "nic/sdk/include/sdk/base.hpp"

namespace api {

/// \brief Unique slab identifiers
enum {
    PDS_SLAB_ID_MIN = 0,
    PDS_SLAB_ID_FRAMEWORK_MIN = PDS_SLAB_ID_MIN,
    PDS_SLAB_ID_API_PARAMS = PDS_SLAB_ID_FRAMEWORK_MIN,
    PDS_SLAB_ID_API_CTXT,
    PDS_SLAB_ID_API_MSG,
    PDS_SLAB_ID_API_OBJ_CTXT,
    PDS_SLAB_ID_FRAMEWORK_MAX = 31,

    PDS_SLAB_ID_API_OBJ_MIN = PDS_SLAB_ID_FRAMEWORK_MAX + 1,
    PDS_SLAB_ID_DEVICE = PDS_SLAB_ID_API_OBJ_MIN,
    PDS_SLAB_ID_TEP,
    PDS_SLAB_ID_IF,
    PDS_SLAB_ID_VPC,
    PDS_SLAB_ID_SUBNET,
    PDS_SLAB_ID_VNIC,
    PDS_SLAB_ID_MAPPING,
    PDS_SLAB_ID_ROUTE_TABLE,
    PDS_SLAB_ID_POLICY,
    PDS_SLAB_ID_MIRROR_SESSION,
    PDS_SLAB_ID_METER,
    PDS_SLAB_ID_TAG,
    PDS_SLAB_ID_SVC_MAPPING,
    PDS_SLAB_ID_VPC_PEER,
    PDS_SLAB_ID_NEXTHOP,
    PDS_SLAB_ID_NEXTHOP_GROUP,
    PDS_SLAB_ID_POLICER,
    PDS_SLAB_ID_NAT_PORT_BLOCK,
    PDS_SLAB_ID_DHCP_RELAY,
    PDS_SLAB_ID_DHCP_POLICY,
    PDS_SLAB_ID_SECURITY_PROFILE,
    PDS_SLAB_ID_API_OBJ_MAX,
    PDS_SLAB_ID_MAX = 16383
};

enum {
    PDS_MEM_ALLOC_DEVICE,
    PDS_MEM_ALLOC_ID_ROUTE_TABLE,
    PDS_MEM_ALLOC_SECURITY_POLICY,
    PDS_MEM_ALLOC_TAG,
    PDS_MEM_ALLOC_CMD_READ_IO,
    PDS_MEM_ALLOC_UPG,
    PDS_MEM_ALLOC_MAX = 16383,
};

#define PDS_DELAY_DELETE_MSECS       (TIME_MSECS_PER_SEC << 1)

/// \brief Wrapper function to delay delete slab elements
///
/// \param[in] slab_id Identifier of the slab to free the element to
/// \param[int] elem Element to free to the given slab
/// \return #SDK_RET_OK on success, failure status code on error
///
/// \remark
///   - Currently delay delete timeout is PDS_DELAY_DELETE_MSECS, it is
///     expected that other thread(s) using (a pointer to) this object should
///     be done using this object within this timeout or else this memory can
///     be freed and allocated for other objects and can result in corruptions.
///     Essentially, PDS_DELAY_DELETE is assumed to be infinite
sdk_ret_t delay_delete_to_slab(uint32_t slab_id, void *elem);

}    // namespace api

#endif // __CORE_MEM_HPP__
