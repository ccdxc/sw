/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping.cc
 *
 * @brief   mapping entry handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"

namespace api {

/**
 * @defgroup PDS_MAPPING_ENTRY - mapping entry functionality
 * @ingroup PDS_MAPPING
 * @{
 */

mapping_entry::mapping_entry() {
    stateless_ = true;
    is_local_ = false;
    public_ip_valid_ = false;
    ht_ctxt_.reset();
}

mapping_entry *
mapping_entry::factory(pds_mapping_spec_t *spec) {
    mapping_entry *mapping;

    // create mapping entry with defaults, if any
    mapping = mapping_db()->alloc();
    if (mapping) {
        new (mapping) mapping_entry();
        if (spec->is_local) {
            mapping->is_local_ = true;
        }
        mapping->impl_ =
            impl_base::factory(impl::IMPL_OBJ_ID_MAPPING, spec);
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
    mapping_db()->free(mapping);
}

mapping_entry *
mapping_entry::build(pds_mapping_key_t *key) {
    mapping_entry *mapping;

    // create mapping entry with defaults, if any
    mapping = mapping_db()->alloc();
    if (mapping) {
        new (mapping) mapping_entry();
        memcpy(&mapping->key_, key, sizeof(*key));
        mapping->impl_ = impl_base::build(impl::IMPL_OBJ_ID_MAPPING,
                                          key, mapping);
        if (mapping->impl_ == NULL) {
            mapping_entry::destroy(mapping);
            return NULL;
        }
    }
    return mapping;
}

void
mapping_entry::soft_delete(mapping_entry *mapping) {
    if (mapping->impl_) {
        impl_base::soft_delete(impl::IMPL_OBJ_ID_MAPPING, mapping->impl_);
    }
    mapping->del_from_db();
    mapping->~mapping_entry();
    mapping_db()->free(mapping);
}

sdk_ret_t
mapping_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_mapping_spec_t *spec = &api_ctxt->api_params->mapping_spec;

    memcpy(&key_, &spec->key, sizeof(pds_mapping_key_t));
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
    if (this->impl_) {
        return impl_->nuke_resources(this);
    }
    return SDK_RET_OK;
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
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

sdk_ret_t
mapping_entry::read(pds_mapping_key_t *key, pds_mapping_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)key,
                          (impl::obj_info_t *)info);
}

sdk_ret_t
mapping_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // mappings are not added to s/w db, so its a no-op, however, since update
    // operation is not supported on this object, we shouldn't endup here
    return sdk::SDK_RET_INVALID_OP;
}

// even though mapping object is stateless, we need to temporarily insert
// into the db as back-to-back operations on the same object can be issued
// in same batch
sdk_ret_t
mapping_entry::add_to_db(void) {
    return mapping_db()->insert(this);
}

sdk_ret_t
mapping_entry::del_from_db(void) {
    if (mapping_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
mapping_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_MAPPING, this);
}

/** @} */    // end of PDS_MAPPING_ENTRY

}    // namespace api
