//
//  Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
///  @file
///  @brief   mirror session handling
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/mirror.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"

using sdk::lib::ht;

namespace api {

/// \defgroup PDS_MIRROR_SESSION - mirror session functionality
/// \ingroup PDS_MIRROR
/// \@{

mirror_session::mirror_session() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    stateless_ = true;
}

mirror_session *
mirror_session::factory(pds_mirror_session_spec_t *spec) {
    mirror_session *ms;

    // create mirror session entry with defaults, if any
    ms = mirror_session_db()->alloc();
    if (ms) {
        new (ms) mirror_session();
        ms->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_MIRROR_SESSION, spec);
        if (ms->impl_ == NULL) {
            mirror_session::destroy(ms);
            return NULL;
        }
    }
    return ms;
}

mirror_session::~mirror_session() {
    //SDK_SPINLOCK_DESTROY(&slock_);
}

void
mirror_session::destroy(mirror_session *ms) {
    ms->nuke_resources_();
    if (ms->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_MIRROR_SESSION, ms->impl_);
    }
    ms->~mirror_session();
    mirror_session_db()->free(ms);
}

api_base *
mirror_session::clone(api_ctxt_t *api_ctxt) {
    mirror_session *cloned_session;

    cloned_session = mirror_session_db()->alloc();
    if (cloned_session) {
        new (cloned_session) mirror_session();
        cloned_session->impl_ = impl_->clone();
        if (unlikely(cloned_session->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone mirror session %u impl", key_.id);
            goto error;
        }
        cloned_session->init_config(api_ctxt);
    }
    return cloned_session;

error:

    cloned_session->~mirror_session();
    mirror_session_db()->free(cloned_session);
    return NULL;
}

sdk_ret_t
mirror_session::free(mirror_session *session) {
    if (session->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_MIRROR_SESSION, session->impl_);
    }
    session->~mirror_session();
    mirror_session_db()->free(session);
    return SDK_RET_OK;
}

mirror_session *
mirror_session::build(pds_mirror_session_key_t *key) {
    mirror_session *ms;

    ms = mirror_session_db()->alloc();
    if (ms) {
        new (ms) mirror_session();
        ms->impl_ = impl_base::build(impl::IMPL_OBJ_ID_MIRROR_SESSION, key, ms);
        if (ms->impl_ == NULL) {
            mirror_session::destroy(ms);
            return NULL;
        }
    }
    return ms;
}

void
mirror_session::soft_delete(mirror_session *ms) {
    if (ms->impl_) {
        impl_base::soft_delete(impl::IMPL_OBJ_ID_MIRROR_SESSION, ms->impl_);
    }
    ms->~mirror_session();
    mirror_session_db()->free(ms);
}

sdk_ret_t
mirror_session::init_config(api_ctxt_t *api_ctxt) {
    pds_mirror_session_spec_t *spec =
        &api_ctxt->api_params->mirror_session_spec;

    memcpy(&this->key_, &spec->key, sizeof(pds_mirror_session_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
mirror_session::program_create(obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
mirror_session::nuke_resources_(void) {
    return (impl_ != NULL) ? (impl_->nuke_resources(this)) : SDK_RET_OK;
}

sdk_ret_t
mirror_session::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
mirror_session::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
mirror_session::read(pds_mirror_session_key_t *key,
                     pds_mirror_session_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)key,
                          (impl::obj_info_t *)info);
}

sdk_ret_t
mirror_session::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mirror_session::activate_config(pds_epoch_t epoch, api_op_t api_op,
                                api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 h/w programming for mapping , so nothing to activate
    return sdk::SDK_RET_OK;
}

sdk_ret_t
mirror_session::add_to_db(void) {
    // mirror sessions are not added to s/w db, so its a no-op
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session::del_from_db(void) {
    // mirror sessions are not added to s/w db, so its a no-op
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mirror_session::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_MIRROR_SESSION, this);
}

/// \@}    // end of PDS_MIRROR_SESSION

}    // namespace api
