//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mirror session datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/mirror_impl.hpp"
#include "nic/apollo/api/impl/apulu/mirror_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_MIRROR_IMPL_STATE - mirror session database functionality
/// \ingroup PDS_MIRROR
/// \@{

mirror_impl_state::mirror_impl_state(pds_state *state) {
    session_bmap_ = 0;
    mirror_impl_slab_ = slab::factory("mirror-impl", PDS_SLAB_ID_MIRROR_IMPL,
                                      sizeof(mirror_impl), 8, true, true);
    SDK_ASSERT(mirror_impl_slab_ != NULL);
}

mirror_impl_state::~mirror_impl_state() {
    slab::destroy(mirror_impl_slab_);
}

mirror_impl *
mirror_impl_state::alloc(void) {
    return ((mirror_impl *)mirror_impl_slab_->alloc());
}

void
mirror_impl_state::free(mirror_impl *impl) {
    mirror_impl_slab_->free(impl);
}

sdk_ret_t
mirror_impl_state::alloc_hw_id(pds_mirror_session_key_t *key, uint16_t *hw_id) {
    *hw_id = key->id - 1;
    if (*hw_id > (PDS_MAX_MIRROR_SESSION - 1)) {
        return SDK_RET_INVALID_ARG;
    } else if (session_bmap_ & (1 << *hw_id)) {
        return sdk::SDK_RET_ENTRY_EXISTS;
    }
    session_bmap_ |= (1 << *hw_id);
    PDS_TRACE_DEBUG("Allocated mirror session hw id %u", *hw_id);
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl_state::free_hw_id(uint16_t hw_id) {
    if (hw_id > (PDS_MAX_MIRROR_SESSION - 1)) {
        return SDK_RET_INVALID_ARG;
    }
    session_bmap_ &= ~(1 << hw_id);
    PDS_TRACE_DEBUG("Freed mirror session hw id %u", hw_id);
    return SDK_RET_OK;
}

sdk_ret_t
mirror_impl_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(mirror_impl_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
