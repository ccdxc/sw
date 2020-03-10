//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc peer state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/vpc_peer_state.hpp"

namespace api {

/// \defgroup PDS_VPC_PEER_STATE - vpc peering database functionality
/// \ingroup PDS_VPC
/// \@{

vpc_peer_state::vpc_peer_state() {
    vpc_peer_slab_ = slab::factory("vpc-peer", PDS_SLAB_ID_VPC_PEER,
                                   sizeof(vpc_peer_entry), 16, true,
                                   true, true, NULL);
    SDK_ASSERT(vpc_peer_slab_ != NULL);
}

vpc_peer_state::~vpc_peer_state() {
    slab::destroy(vpc_peer_slab_);
}

vpc_peer_entry *
vpc_peer_state::alloc(void) {
    return ((vpc_peer_entry *)vpc_peer_slab_->alloc());
}

void
vpc_peer_state::free(vpc_peer_entry *vpc_peer) {
    vpc_peer_slab_->free(vpc_peer);
}

sdk_ret_t
vpc_peer_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(vpc_peer_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_VPC_PEER_STATE

}    // namespace api
