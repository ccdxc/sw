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
    impl_ = NULL;
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
        if (cloned_session->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        cloned_session->impl_ = impl_->clone();
        if (unlikely(cloned_session->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone mirror session %u impl", key_.id);
            goto error;
        }
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

sdk_ret_t
mirror_session::reserve_resources(api_base *orig_obj,
                                  api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
mirror_session::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
mirror_session::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
mirror_session::init_config(api_ctxt_t *api_ctxt) {
    vpc_entry *vpc;
    if_entry *intf;
    pds_mirror_session_spec_t *spec;

    spec = &api_ctxt->api_params->mirror_session_spec;
    switch (spec->type) {
    case PDS_MIRROR_SESSION_TYPE_ERSPAN:
        vpc = vpc_find(&spec->erspan_spec.vpc);
        if (vpc == NULL) {
            PDS_TRACE_ERR("Failed to initialize mirror session %s, vpc %s "
                          "not found", spec->key.str(),
                          spec->erspan_spec.vpc.str());
            return SDK_RET_INVALID_ARG;
        }
        erspan_.vpc_ = spec->erspan_spec.vpc;
        if (vpc->type() == PDS_VPC_TYPE_UNDERLAY) {
            // if this underlay VPC, make sure we know the destination (as a
            // tunnel)
            if (tep_find(&spec->erspan_spec.tep) == NULL) {
                PDS_TRACE_ERR("Failed to initialize mirror session %s, "
                              "tunnel %s not found", spec->key.str(),
                              spec->erspan_spec.tep.str());
                return SDK_RET_INVALID_ARG;
            }
            erspan_.tep_ = spec->erspan_spec.tep;
        } else {
            // TODO: should we validate mapping ?
            erspan_.mapping_ = spec->erspan_spec.mapping;
        }
        break;
    case PDS_MIRROR_SESSION_TYPE_RSPAN:
        // TODO: validate uplink if ?
        intf = if_find(&spec->rspan_spec.uplink_if);
        if (intf == NULL) {
            PDS_TRACE_ERR("Failed to find uplink interface %s in RSPAN %s "
                          "config", spec->rspan_spec.uplink_if.str(),
                          spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        if (intf->type() != PDS_IF_TYPE_ETH) {
            PDS_TRACE_ERR("Invalid interface type %u in RSPAN config %s",
                          spec->rspan_spec.uplink_if.str(),
                          spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        if ((spec->rspan_spec.encap.type != PDS_ENCAP_TYPE_NONE) &&
            (spec->rspan_spec.encap.type != PDS_ENCAP_TYPE_DOT1Q)) {
            PDS_TRACE_ERR("Unsupported encap %s in RSPAN config %s",
                          pds_encap2str(&spec->rspan_spec.encap),
                          spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        rspan_.uplink_if_ = spec->rspan_spec.uplink_if;
        rspan_.encap_ = spec->rspan_spec.encap;
        break;
    default:
        PDS_TRACE_ERR("Unknown mirror session type %u", spec->type);
        return SDK_RET_INVALID_ARG;
    }
    memcpy(&this->key_, &spec->key, sizeof(key_));
    return SDK_RET_OK;
}

sdk_ret_t
mirror_session::program_create(api_obj_ctxt_t *obj_ctxt) {
    // TODO: may not need this at all !!
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
mirror_session::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

// TODO: handle updates
sdk_ret_t
mirror_session::compute_update(api_obj_ctxt_t *obj_ctxt) {
#if 0
    pds_mirror_session_spec_t *spec;

    spec = &obj_ctxt->api_params->mirror_session_spec;
#endif
    return SDK_RET_INVALID_OP;
}

// TODO: fixme
sdk_ret_t
mirror_session::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mirror_session::activate_config(pds_epoch_t epoch, api_op_t api_op,
                                api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
mirror_session::fill_spec_(pds_mirror_session_spec_t *spec) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mirror_session::read(pds_mirror_session_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
mirror_session::add_to_db(void) {
    return mirror_session_db()->insert(this);
}

sdk_ret_t
mirror_session::del_from_db(void) {
    if (mirror_session_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
mirror_session::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (mirror_session_db()->remove((mirror_session *)orig_obj)) {
        return mirror_session_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
mirror_session::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_MIRROR_SESSION, this);
}

/// \@}    // end of PDS_MIRROR_SESSION

}    // namespace api
