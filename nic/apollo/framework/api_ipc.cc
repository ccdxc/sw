//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// certain API objects are of interest to various IPC endpoints and API engine
/// needs to circulate these API object (either in the form of sync req/rsp or
/// notifications (fire and forget semantics, without waiting for response from
/// the recipient(s). This file defines data structures and APIs that API engine
/// uses to keep track of API object subscriptions per IPC endpoint in the
/// system
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_ipc.hpp"

namespace api {

static api_obj_ipc_map_t const g_api_obj_ipc_map {
  { OBJ_ID_DEVICE, { { PDS_IPC_ID_VPP, false}, } },
  { OBJ_ID_VPC, { { PDS_IPC_ID_VPP, false}, } },
  { OBJ_ID_SUBNET, { { PDS_IPC_ID_VPP, false}, } },
  { OBJ_ID_VNIC, { { PDS_IPC_ID_VPP, false}, } },
  { OBJ_ID_NAT_PORT_BLOCK, { { PDS_IPC_ID_VPP, false}, } },
  { OBJ_ID_DHCP_POLICY, { { PDS_IPC_ID_VPP, false}, } },
  { OBJ_ID_SECURITY_PROFILE, { { PDS_IPC_ID_VPP, false}, } },
};

bool
api_obj_circulate (obj_id_t obj_id)
{
    if (g_api_obj_ipc_map.find(obj_id) != g_api_obj_ipc_map.end()) {
        return true;
    }
    return false;
}

}    // namespace api
