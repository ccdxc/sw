/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping.cc
 *
 * @brief   mapping entry handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup PDS_MAPPING_ENTRY - mapping entry functionality
 * @ingroup PDS_MAPPING
 * @{
 */

mapping_entry::mapping_entry() {
    stateless_ = true;
}

mapping_entry *
mapping_entry::factory(pds_mapping_spec_t *pds_mapping) {
    mapping_entry *mapping;

    // create mapping entry with defaults, if any
    mapping = mapping_db()->mapping_alloc();
    if (mapping) {
        new (mapping) mapping_entry();
        mapping->impl_ =
            impl_base::factory(impl::IMPL_OBJ_ID_MAPPING, pds_mapping);
        if (mapping->impl_ == NULL) {
            mapping_entry::destroy(mapping);
            return NULL;
        }
    }
    return mapping;
}

mapping_entry::~mapping_entry() {
}

void
mapping_entry::destroy(mapping_entry *mapping) {
    mapping->nuke_resources_();
    if (mapping->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_MAPPING, mapping->impl_);
    }
    mapping->~mapping_entry();
    mapping_db()->mapping_free(mapping);
}

sdk_ret_t
mapping_entry::init_config(api_ctxt_t *api_ctxt) {
    return SDK_RET_OK;
}

 sdk_ret_t
mapping_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
mapping_entry::program_config(obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
mapping_entry::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
mapping_entry::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
mapping_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
mapping_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // update operation is not supported on mapping
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
    // there is no stage 0 h/w programming for mapping , so nothing to activate
    return SDK_RET_OK;
}

sdk_ret_t
mapping_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // mappings are not added to s/w db, so its a no-op, however, since update
    // operation is not supported on this object, we shouldn't endup here
    return sdk::SDK_RET_INVALID_OP;
}


sdk_ret_t
mapping_entry::add_to_db(void) {
    // mappings are not added to s/w db, so its a no-op
    return SDK_RET_OK;
}

sdk_ret_t
mapping_entry::del_from_db(void) {
    // mappings are not added to s/w db, so its a no-op
    return SDK_RET_OK;
}

sdk_ret_t
mapping_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_MAPPING, this);
}

/** @} */    // end of PDS_MAPPING_ENTRY

}    // namespace api
