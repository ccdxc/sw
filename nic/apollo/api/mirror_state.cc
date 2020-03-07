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
    memset(db_, 0, sizeof(db_));
}

mirror_session_state::~mirror_session_state() {
    slab::destroy(mirror_session_slab_);
}

mirror_session *
mirror_session_state::alloc(void) {
    return ((mirror_session *)mirror_session_slab_->alloc());
}

sdk_ret_t
mirror_session_state::insert(mirror_session *ms) {
    for (uint32_t i = 0; i < PDS_MAX_MIRROR_SESSION; i++) {
        if (db_[i] == NULL) {
            db_[i] = ms;
            return SDK_RET_OK;
        }
    }
    return sdk::SDK_RET_TABLE_FULL;
}

mirror_session *
mirror_session_state::remove(mirror_session *ms) {
    for (uint32_t i = 0; i < PDS_MAX_MIRROR_SESSION; i++) {
        if (db_[i] && (db_[i]->key() == ms->key())) {
            db_[i] = NULL;
            return ms;
        }
    }
    return NULL;
}

void
mirror_session_state::free(mirror_session *ms) {
    mirror_session_slab_->free(ms);
}

mirror_session *
mirror_session_state::find(pds_obj_key_t *key) const {
    for (uint32_t i = 0; i < PDS_MAX_MIRROR_SESSION; i++) {
        if (db_[i] && (db_[i]->key() == *key)) {
            return db_[i];
        }
    }
    return NULL;
}

sdk_ret_t
mirror_session_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    bool stop_walk;

    for (uint32_t i = 0; i < PDS_MAX_MIRROR_SESSION; i++) {
        if (db_[i]) {
            stop_walk = walk_cb(db_[i], ctxt);
            if (stop_walk) {
                return SDK_RET_OK;
            }
        }
    }
    return SDK_RET_OK;
}
sdk_ret_t
mirror_session_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(mirror_session_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_MIRROR_SESSION_STATE

}    // namespace api
