//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/lif_state.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"

namespace api {

/// \defgroup PDS_LIF_STATE - lif database functionality
/// \ingroup PDS_LIF
/// \@{

lif_state::lif_state() {
}

lif_state::~lif_state() {
}

void
lif_state::impl_state_set(void *impl_state) {
    lif_impl_state_ = impl_state;
}

void *
lif_state::find(pds_lif_id_t *key) const {
    if (lif_impl_state_) {
        return ((api::impl::lif_impl_state *)lif_impl_state_)->find(key);
    }
    return NULL;
}

void *
lif_state::find(pds_obj_key_t *key) const {
    if (lif_impl_state_) {
        return ((api::impl::lif_impl_state *)lif_impl_state_)->find(key);
    }
    return NULL;
}

sdk_ret_t
lif_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    if (lif_impl_state_) {
        return ((api::impl::lif_impl_state *)lif_impl_state_)->walk(walk_cb,
                                                                    ctxt);
    }
    return SDK_RET_OK;
}

/// \@}    // end of PDS_LIF_STATE

}    // namespace api
