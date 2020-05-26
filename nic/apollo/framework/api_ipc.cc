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

// WARNING !! WARNING !! WARNING !!
// NOTE: keep ipc_peer_api_obj_map_t and api_obj_ipc_peer_map_t in sync always
// WARNING !! WARNING !! WARNING !!
static api_obj_ipc_peer_map_t g_api_obj_ipc_peer_map {
    // IPC peers interseted in DEVICE object
    { OBJ_ID_DEVICE,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
    // IPC peers interseted in VPC object
    { OBJ_ID_VPC,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
    // IPC peers interseted in SUBNET object
    { OBJ_ID_SUBNET,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
    // IPC peers interseted in VNIC object
    { OBJ_ID_VNIC,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
    // IPC peers interseted in MIRROR_SESSION object
    { OBJ_ID_MIRROR_SESSION,
        {
            { PDS_IPC_ID_ROUTING, false},
        }
    },
    // IPC peers interseted in NAT_PORT_BLOCK object
    { OBJ_ID_NAT_PORT_BLOCK,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
    // IPC peers interseted in DHCP_POLICY object
    { OBJ_ID_DHCP_POLICY,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
    // IPC peers interseted in SECURITY_PROFILE object
    { OBJ_ID_SECURITY_PROFILE,
        {
            { PDS_IPC_ID_VPP, false},
        }
    },
};

bool
api_obj_circulate (obj_id_t obj_id)
{
    if (g_api_obj_ipc_peer_map.find(obj_id) != g_api_obj_ipc_peer_map.end()) {
        return true;
    }
    return false;
}

api_obj_ipc_peer_list_t&
ipc_peer_list (obj_id_t obj_id)
{
    return g_api_obj_ipc_peer_map[obj_id];
}

}    // namespace api
