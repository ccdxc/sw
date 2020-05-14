// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains function definitions for IPC helper functions
//----------------------------------------------------------------------------

#ifndef __API_INTERNAL_IPC_HPP__
#define __API_INTERNAL_IPC_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

sdk::sdk_ret_t pds_ipc_cfg_get(pds_ipc_id_t ipc_id, obj_id_t obj_id,
                               const pds_obj_key_t *key, void *info);
sdk::sdk_ret_t pds_ipc_cfg_get_all(pds_ipc_id_t ipc_id, obj_id_t obj_id,
                                   pds_cfg_get_all_rsp_t *reply, size_t sz);
void pds_cmd_response_handler_cb(sdk::ipc::ipc_msg_ptr msg, const void *ret);

}    // namespace api

#endif // __API_INTERNAL_IPC_HPP__
