// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains miscellaneous IPC back-end routines for PDS API
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/internal/ipc.hpp"

namespace api {

/// \brief callback to handle command msg reply
void
pds_cmd_response_handler_cb (sdk::ipc::ipc_msg_ptr msg, const void *ret)
{
    pds_cmd_rsp_t *response = (pds_cmd_rsp_t *)ret;

    if (msg->length() != sizeof(pds_cmd_rsp_t)) {
        response->status = sdk::SDK_RET_INVALID_ARG;
        return;
    }
    memcpy(response, msg->data(), msg->length());
}

/// \brief callback function for handling IPC reply message
static void
cfg_get_ipc_reply_cb (sdk::ipc::ipc_msg_ptr msg, const void *reply)
{
    memcpy((void *)reply, msg->data(), sizeof(pds_cfg_get_rsp_t));
}

/// \brief read a single config object via IPC
///        currently supports dhcp policy and security profile only
sdk::sdk_ret_t
pds_ipc_cfg_get (pds_ipc_id_t ipc_id, obj_id_t obj_id,
                 const pds_obj_key_t *key, void *info)
{
    pds_cfg_get_req_t request;
    pds_cfg_get_rsp_t reply = { 0 };
    sdk::sdk_ret_t retval = sdk::SDK_RET_OK;

    request.obj_id = obj_id;
    request.key = *key;
    if ((ipc_id == PDS_IPC_ID_VPP) &&
        (api::g_pds_state.ipc_mock() == false)) {
        sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CFG_OBJ_GET, &request,
                          sizeof(request), cfg_get_ipc_reply_cb, &reply);
        retval = (sdk::sdk_ret_t )reply.status;
        if (retval != sdk::SDK_RET_OK) {
            return retval;
        }
    }
    // if endpoint unavailable, the memcpy will just copy zeros
    switch (obj_id) {
    case OBJ_ID_DHCP_POLICY:
        memcpy(info, &reply.dhcp_policy, sizeof(pds_dhcp_policy_info_t));
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        memcpy(info, &reply.nat_port_block, sizeof(pds_nat_port_block_info_t));
        break;

    case OBJ_ID_SECURITY_PROFILE:
        memcpy(info, &reply.security_profile,
               sizeof(pds_security_profile_info_t));
        break;

    default:
        return sdk::SDK_RET_INVALID_OP;
    }

    return retval;
}

typedef struct cfg_get_all_ctxt_s {
    pds_cfg_get_all_rsp_t *reply;
    size_t sz;
} cfg_get_all_ctxt_t;

/// \brief callback function to handle cfg get all IPC response
static void
cfg_get_all_ipc_reply_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    cfg_get_all_ctxt_t *gactxt = (cfg_get_all_ctxt_t *)ctx;

    memcpy((void *)(gactxt->reply), msg->data(), gactxt->sz);
}

/// \brief API to retrieve all configured objects of specified type
sdk::sdk_ret_t
pds_ipc_cfg_get_all (pds_ipc_id_t ipc_id, obj_id_t obj_id,
                     pds_cfg_get_all_rsp_t *reply, size_t sz)
{
    pds_cfg_get_all_req_t request;
    cfg_get_all_ctxt_t ctxt;

    request.obj_id = obj_id;
    ctxt.reply = reply;
    ctxt.sz = sz;
    if ((ipc_id == PDS_IPC_ID_VPP) &&
        (api::g_pds_state.ipc_mock() == false)) {
        sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CFG_OBJ_GET_ALL,
                          &request, sizeof(request),
                          cfg_get_all_ipc_reply_cb, &ctxt);
    } else {
        reply->status = sdk::SDK_RET_OK;
        reply->count = 0;
    }

    return sdk::SDK_RET_OK;
}

}    // namespace api
