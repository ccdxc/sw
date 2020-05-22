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

#ifndef __FRAMEWORK_API_IPC_HPP__
#define __FRAMEWORK_API_IPC_HPP__

#include "nic/apollo/include/globals.hpp"
#include "nic/apollo/framework/api.h"

#include <vector>
#include <unordered_map>

using std::unordered_map;
using std::vector;

namespace api {

// per API object, IPC peer endpoint info per API object of interest
typedef struct api_obj_ipc_peer_info_s {
    pds_ipc_id_t ipc_id;
    bool ntfn;
} api_obj_ipc_peer_info_t;

// list of IPC endpoint infos per API object
typedef vector<api_obj_ipc_peer_info_t> api_obj_ipc_peer_list_t;

// per API object IP endpoint info map
typedef unordered_map<obj_id_t, api_obj_ipc_peer_list_t> api_obj_ipc_peer_map_t;

/// \brief return true if this API object is of interest to any IPC peers
/// \param[in] obj_id    API object id
/// \return true if this object if of interest to any of the peers
bool api_obj_circulate(obj_id_t obj_id);

/// \brief  return list of IPC endpoints interested in the given API object
/// \param[in] obj_id    API object id
/// \return list of IPC endpoints interested in the object
api_obj_ipc_peer_list_t& ipc_peer_list(obj_id_t obj_id);

}    // namespace api

#endif    // __FRAMEWORK_API_IPC_HPP__
