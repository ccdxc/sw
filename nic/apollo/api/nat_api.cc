//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements NAT related CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/nat.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"

static sdk_ret_t
pds_nat_port_block_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                               pds_obj_key_t *key,
                               pds_nat_port_block_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(OBJ_ID_NAT_PORT_BLOCK, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->nat_port_block_key = *key;
        } else {
            api_ctxt->api_params->nat_port_block_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

//----------------------------------------------------------------------------
// NAT APIs entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_nat_port_block_create (_In_ pds_nat_port_block_spec_t *spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_nat_port_block_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

void
nat_port_block_read_cb (sdk::ipc::ipc_msg_ptr msg, const void *cookie)
{
    pds_msg_t *reply = (pds_msg_t *)msg->data();
    pds_nat_port_block_info_t *info = (pds_nat_port_block_info_t *)cookie;

    info->spec = reply->cfg_msg.nat_port_block.spec;
    info->stats = reply->cfg_msg.nat_port_block.stats;
    info->status = reply->cfg_msg.nat_port_block.status;
}

sdk_ret_t
pds_nat_port_block_read (_In_ pds_obj_key_t *key,
                         _Out_ pds_nat_port_block_info_t *info)
{
    pds_msg_t request;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    memset(info, 0, sizeof(pds_nat_port_block_info_t));
    info->spec.key = *key;

    memset(&request, 0, sizeof(request));

    request.id = PDS_CFG_MSG_ID_NAT_PORT_BLOCK;
    request.cfg_msg.op = API_OP_NONE;
    request.cfg_msg.obj_id = OBJ_ID_NAT_PORT_BLOCK;
    request.cfg_msg.nat_port_block.key = info->spec.key;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, &request,
                      sizeof(pds_msg_t), nat_port_block_read_cb, info);
    if (info->spec.nat_ip_range.ip_lo.v4_addr == 0) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return SDK_RET_OK;
}

sdk_ret_t
pds_nat_port_block_read_all (nat_port_block_read_cb_t cb, void *ctxt)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
pds_nat_port_block_update (_In_ pds_nat_port_block_spec_t *spec,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_nat_port_block_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_nat_port_block_delete (_In_ pds_obj_key_t *key,
                           _In_ pds_batch_ctxt_t bctxt)
{
    return pds_nat_port_block_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
