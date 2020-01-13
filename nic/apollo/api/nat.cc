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
        cloned_port_block->init_config(api_ctxt);
    }
    return cloned_port_block;
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
    msg->id = PDS_CFG_MSG_ID_NAT_PORT_BLOCK;
    msg->cfg_msg.op = obj_ctxt->api_op;
    msg->cfg_msg.obj_id = OBJ_ID_NAT_PORT_BLOCK;
    if (obj_ctxt->api_op == API_OP_DELETE) {
        msg->cfg_msg.nat_port_block.key =
            obj_ctxt->api_params->nat_port_block_key;
    } else {
        msg->cfg_msg.nat_port_block.spec =
            obj_ctxt->api_params->nat_port_block_spec;
    }
    return SDK_RET_OK;
}

void
nat_port_block::fill_spec_(pds_nat_port_block_spec_t *spec) {
}

sdk_ret_t
nat_port_block::read(pds_nat_port_block_info_t *info) {
    return SDK_RET_INVALID_OP;
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
