//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/tag.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/// \@defgroup PDS_TAG - tag functionality
/// \@ingroup PDS_ROUTE
/// \@{

typedef struct tag_update_ctxt_s {
    tag_entry *tag;
    obj_ctxt_t *obj_ctxt;
} __PACK__ tag_entry_update_ctxt_t;

tag_entry::tag_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

tag_entry *
tag_entry::factory(pds_tag_spec_t *spec) {
    tag_entry    *tag;

    // create tag instance with defaults, if any
    tag = tag_db()->alloc();
    if (tag) {
        new (tag) tag_entry();
        tag->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_TAG, spec);
        if (tag->impl_ == NULL) {
            tag_entry::destroy(tag);
            return NULL;
        }
    }
    return tag;
}

tag_entry::~tag_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

void
tag_entry::destroy(tag_entry *tag) {
    tag->nuke_resources_();
    if (tag->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_TAG, tag->impl_);
    }
    tag->~tag_entry();
    tag_db()->free(tag);
}

sdk_ret_t
tag_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_tag_spec_t    *spec;

    spec = &api_ctxt->api_params->tag_spec;
    memcpy(&this->key_, &spec->key, sizeof(pds_tag_key_t));
    this->af_ = spec->af;
    return SDK_RET_OK;
}

sdk_ret_t
tag_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
tag_entry::program_config(obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Programming tag %u", key_);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
tag_entry::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
tag_entry::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
tag_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
tag_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                            obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating tag %u config", key_);
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

sdk_ret_t
tag_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
tag_entry::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding tag %u to db", key_);
    return tag_db()->insert(this);
}

sdk_ret_t
tag_entry::del_from_db(void) {
    if (tag_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
tag_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_TAG, this);
}

sdk_ret_t
tag_entry::add_deps(obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_TAG

}    // namespace api
