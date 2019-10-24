//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// @file
/// @brief   mirror session database handling
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/mirror_state.hpp"

using sdk::lib::ht;

namespace api {

/// \defgroup PDS_MIRROR_SESSION_STATE - mirror session database functionality
/// \ingroup PDS_MIRROR_SESSION
/// \@{

mirror_session_state::mirror_session_state() {
    mirror_session_slab_ =
        slab::factory("mirror-session", PDS_SLAB_ID_MIRROR_SESSION,
                      sizeof(mirror_session), 8, true, true, true, NULL);
    SDK_ASSERT(mirror_session_slab_ != NULL);
}

mirror_session_state::~mirror_session_state() {
    slab::destroy(mirror_session_slab_);
}

mirror_session *
mirror_session_state::alloc(void) {
    return ((mirror_session *)mirror_session_slab_->alloc());
}

void
mirror_session_state::free(mirror_session *ms) {
    mirror_session_slab_->free(ms);
}

sdk_ret_t
mirror_session_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(mirror_session_slab_, ctxt);
    return SDK_RET_OK;
}

#if 0
mirror_session *
mirror_session_state::find(pds_mirror_session_key_t *mirror_session_key) const {
    // TODO: rebuild from hw, if we need to support this
}
#endif

/// \@}    // end of PDS_MIRROR_SESSION_STATE

}    // namespace api
