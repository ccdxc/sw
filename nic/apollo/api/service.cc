/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    service.cc
 *
 * @brief   service mapping entry handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"

namespace api {

/**
 * @defgroup PDS_SVC_MAPPING - mapping entry functionality
 * @ingroup PDS_SERVICE
 * @{
 */

svc_mapping::svc_mapping() {
    stateless_ = true;
    ht_ctxt_.reset();
    impl_ = NULL;
}

svc_mapping *
svc_mapping::factory(pds_svc_mapping_spec_t *spec) {
    svc_mapping *mapping;

    // create service mapping entry with defaults, if any
    mapping = svc_mapping_db()->alloc();
    if (mapping) {
        new (mapping) svc_mapping();
        mapping->impl_ =
            impl_base::factory(impl::IMPL_OBJ_ID_SVC_MAPPING, spec);
        if (mapping->impl_ == NULL) {
            svc_mapping::destroy(mapping);
            return NULL;
        }
    }
    return mapping;
}

svc_mapping::~svc_mapping() {
}

void
svc_mapping::destroy(svc_mapping *mapping) {
    mapping->nuke_resources_();
    if (mapping->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_SVC_MAPPING, mapping->impl_);
    }
    mapping->~svc_mapping();
    svc_mapping_db()->free(mapping);
}

api_base *
svc_mapping::clone(api_ctxt_t *api_ctxt) {
    svc_mapping *cloned_mapping;

    cloned_mapping = svc_mapping_db()->alloc();
    if (cloned_mapping) {
        new (cloned_mapping) svc_mapping();
        cloned_mapping->impl_ = impl_->clone();
        if (unlikely(cloned_mapping->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone mapping %s impl", key2str());
            goto error;
        }
        cloned_mapping->init_config(api_ctxt);
    }
    return cloned_mapping;

error:

    cloned_mapping->~svc_mapping();
    svc_mapping_db()->free(cloned_mapping);
    return NULL;
}

sdk_ret_t
svc_mapping::free(svc_mapping *mapping) {
    if (mapping->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_SVC_MAPPING, mapping->impl_);
    }
    mapping->~svc_mapping();
    svc_mapping_db()->free(mapping);
    return SDK_RET_OK;
}

svc_mapping *
svc_mapping::build(pds_svc_mapping_key_t *key) {
    svc_mapping *mapping;

    // create service mapping entry with defaults, if any
    mapping = svc_mapping_db()->alloc();
    if (mapping) {
        new (mapping) svc_mapping();
        memcpy(&mapping->key_, key, sizeof(*key));
        mapping->impl_ = impl_base::build(impl::IMPL_OBJ_ID_SVC_MAPPING,
                                          key, mapping);
        if (mapping->impl_ == NULL) {
            svc_mapping::destroy(mapping);
            return NULL;
        }
    }
    return mapping;
}

void
svc_mapping::soft_delete(svc_mapping *mapping) {
    if (mapping->impl_) {
        impl_base::soft_delete(impl::IMPL_OBJ_ID_SVC_MAPPING, mapping->impl_);
    }
    mapping->del_from_db();
    mapping->~svc_mapping();
    svc_mapping_db()->free(mapping);
}

sdk_ret_t
svc_mapping::init_config(api_ctxt_t *api_ctxt) {
    pds_svc_mapping_spec_t *spec = &api_ctxt->api_params->svc_mapping_spec;

    memcpy(&key_, &spec->key, sizeof(pds_svc_mapping_key_t));
    return SDK_RET_OK;
}

 sdk_ret_t
svc_mapping::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::program_create(obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
svc_mapping::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
svc_mapping::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping::activate_config(pds_epoch_t epoch, api_op_t api_op,
                             api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping::read(pds_svc_mapping_key_t *key, pds_svc_mapping_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)key,
                          (impl::obj_info_t *)info);
}

sdk_ret_t
svc_mapping::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // service mappings are not added to s/w db, so its a no-op, however, since
    // update operation is not supported on this object, we shouldn't endup here
    return sdk::SDK_RET_INVALID_OP;
}

// even though mapping object is stateless, we need to temporarily insert
// into the db as back-to-back operations on the same object can be issued
// in same batch
sdk_ret_t
svc_mapping::add_to_db(void) {
    return svc_mapping_db()->insert(this);
}

sdk_ret_t
svc_mapping::del_from_db(void) {
    if (svc_mapping_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
svc_mapping::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SVC_MAPPING, this);
}

/** @} */    // end of PDS_SVC_MAPPING

}    // namespace api
