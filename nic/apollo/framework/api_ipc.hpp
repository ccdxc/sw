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

#include <unordered_map>
#include <vector>

using std::unordered_map;
using std::vector;

namespace api {

// per IPC endpoint info per API object of interest
typedef struct api_obj_ipc_ep_info_s {
    pds_ipc_id_t ipc_id;
    bool ntfn;
} api_obj_ipc_ep_info_t;

// list of IPC endpoint infos per API object
typedef vector<api_obj_ipc_ep_info_t> api_obj_ipc_ep_list_t;

// per API object IP endpoint info map
typedef unordered_map<obj_id_t, api_obj_ipc_ep_list_t> api_obj_ipc_map_t;

bool api_obj_circulate(obj_id_t obj_id);

}    // namespace api

#endif    // __FRAMEWORK_API_IPC_HPP__
