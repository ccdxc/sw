//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// NAT API object handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/nat.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/internal/ipc.hpp"

namespace api {

// NAT port block API implementation
nat_port_block::nat_port_block() {
    ht_ctxt_.reset();
}

nat_port_block::~nat_port_block() {
}

nat_port_block *
nat_port_block::factory(pds_nat_port_block_spec_t *spec) {
    nat_port_block *port_block;

    // create NAT port block entry with defaults, if any
    port_block = nat_db()->alloc();
    if (port_block) {
        new (port_block) nat_port_block();
    }
    return port_block;
}

void
nat_port_block::destroy(nat_port_block *port_block) {
    port_block->~nat_port_block();
    nat_db()->free(port_block);
}

api_base *
nat_port_block::clone(api_ctxt_t *api_ctxt) {
    nat_port_block *cloned_port_block;

    cloned_port_block = nat_db()->alloc();
    if (cloned_port_block) {
        new (cloned_port_block) nat_port_block();
        if (cloned_port_block->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
    }
    return cloned_port_block;

error:

    cloned_port_block->~nat_port_block();
    nat_db()->free(cloned_port_block);
    return NULL;
}

sdk_ret_t
nat_port_block::free(nat_port_block *port_block) {
    port_block->~nat_port_block();
    nat_db()->free(port_block);
    return SDK_RET_OK;
}

nat_port_block *
nat_port_block::build(pds_obj_key_t *key) {
    nat_port_block *port_block;

    port_block = nat_db()->alloc();
    if (port_block) {
        new (port_block) nat_port_block();
        memcpy(&port_block->key_, key, sizeof(*key));
    }
    return port_block;
}

void
nat_port_block::soft_delete(nat_port_block *port_block) {
    port_block->del_from_db();
    port_block->~nat_port_block();
    nat_db()->free(port_block);
}

sdk_ret_t
nat_port_block::init_config(api_ctxt_t *api_ctxt) {
    key_ = api_ctxt->api_params->nat_port_block_spec.key;
    return SDK_RET_OK;
}

sdk_ret_t
nat_port_block::populate_msg(pds_msg_t *msg, api_obj_ctxt_t *obj_ctxt) {
    msg->cfg_msg.op = obj_ctxt->api_op;
    msg->cfg_msg.obj_id = OBJ_ID_NAT_PORT_BLOCK;
    if (obj_ctxt->api_op == API_OP_DELETE) {
        msg->cfg_msg.nat_port_block.key =
            obj_ctxt->api_params->key;
    } else {
        msg->cfg_msg.nat_port_block.spec =
            obj_ctxt->api_params->nat_port_block_spec;
    }
    return SDK_RET_OK;
}

void
nat_port_block::fill_spec_(pds_nat_port_block_spec_t *spec) {
}

typedef struct nat_port_block_read_params_s {
    nat_port_block_read_cb_t cb;
    bool read_all;
    void *ctxt;
} nat_port_block_read_params_t;

sdk_ret_t
nat_port_block::read(pds_nat_port_block_info_t *info) {
    return pds_ipc_cfg_get(PDS_IPC_ID_VPP, OBJ_ID_NAT_PORT_BLOCK,
                           &info->spec.key, info);
}

sdk_ret_t
nat_port_block::read_all(nat_port_block_read_cb_t cb, void *ctxt) {
    pds_cmd_msg_t request;
    pds_cmd_rsp_t reply;
    pds_cfg_get_all_rsp_t *payload;
    size_t payloadsz;
    sdk_ret_t ret;

    // send a cmd to VPP to retrieve object count
    request.id = PDS_CMD_MSG_OBJ_COUNT_GET;
    request.obj_count_get.obj_id = OBJ_ID_NAT_PORT_BLOCK;
    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, &request,
                            sizeof(request), api::pds_cmd_response_handler_cb,
                            &reply);
    // abort on error
    if (reply.status != (uint32_t )SDK_RET_OK) {
        return (sdk_ret_t )reply.status;
    }
    if (reply.obj_count == 0) {
        // no configured instances, we're done
        return SDK_RET_OK;
    }
    // allocate as much space as necessary
    payloadsz = sizeof(pds_cfg_get_all_rsp_t) +
                (reply.obj_count * sizeof(pds_nat_port_block_info_t));
    payload = (pds_cfg_get_all_rsp_t *)SDK_CALLOC(PDS_MEM_ALLOC_NAT_PORT_BLOCK,
                                                  payloadsz);
    ret = api::pds_ipc_cfg_get_all(PDS_IPC_ID_VPP, OBJ_ID_NAT_PORT_BLOCK,
                                   payload, payloadsz);
    if (ret != SDK_RET_OK) {
        SDK_FREE(PDS_MEM_ALLOC_NAT_PORT_BLOCK, payload);
        return ret;
    }
    for (uint32_t i = 0; i < payload->count; i++) {
        (*cb)(&payload->nat_port_block[i], ctxt);
    }
    ret = (sdk_ret_t )payload->status;
    SDK_FREE(PDS_MEM_ALLOC_NAT_PORT_BLOCK, payload);
    return ret;
}

sdk_ret_t
nat_port_block::add_to_db(void) {
    return nat_db()->insert(this);
}

sdk_ret_t
nat_port_block::del_from_db(void) {
    if (nat_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
nat_port_block::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (nat_db()->remove((nat_port_block *)orig_obj)) {
        return nat_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
nat_port_block::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_NAT_PORT_BLOCK, this);
}

}    // namespace api
