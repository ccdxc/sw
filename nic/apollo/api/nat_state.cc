//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// NAT state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/nat_state.hpp"

using namespace sdk;

namespace api {

nat_state::nat_state() {
    nat_port_block_ht_ =
        ht::factory(PDS_MAX_NAT_PORT_BLOCK >> 2,
                    nat_port_block::nat_port_block_key_func_get,
                    sizeof(pds_nat_port_block_key_t));
    SDK_ASSERT(nat_port_block_ht_ != NULL);
    nat_port_block_slab_ = slab::factory("nat-port-block",
                                         PDS_SLAB_ID_NAT_PORT_BLOCK,
                                         sizeof(nat_port_block),
                                         16, true, true, NULL);
    SDK_ASSERT(nat_port_block_slab_ != NULL);
}

nat_state::~nat_state() {
    ht::destroy(nat_port_block_ht_);
    slab::destroy(nat_port_block_slab_);
}

nat_port_block *
nat_state::alloc(void) {
    return ((nat_port_block *)nat_port_block_slab_->alloc());
}

sdk_ret_t
nat_state::insert(nat_port_block *port_block) {
    return nat_port_block_ht_->insert_with_key(&port_block->key_, port_block,
                                               &port_block->ht_ctxt_);
}

nat_port_block *
nat_state::remove(nat_port_block *port_block) {
    return (nat_port_block *)(nat_port_block_ht_->remove(&port_block->key_));
}

void
nat_state::free(nat_port_block *port_block) {
    nat_port_block_slab_->free(port_block);
}

nat_port_block *
nat_state::find(pds_nat_port_block_key_t *key) const {
    return (nat_port_block *)(nat_port_block_ht_->lookup(key));
}

sdk_ret_t
nat_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(nat_port_block_slab_, ctxt);
    return SDK_RET_OK;
}

}    // namespace api
