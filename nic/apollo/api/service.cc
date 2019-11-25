/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    service.cc
 *
 * @brief   service mapping entry handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup PDS_SVC_MAPPING - mapping entry functionality
 * @ingroup PDS_SERVICE
 * @{
 */

svc_mapping::svc_mapping() {
    stateless_ = true;
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

svc_mapping *
svc_mapping::build(pds_svc_mapping_key_t *key) {
    svc_mapping *mapping;

    // create service mapping entry with defaults, if any
    mapping = svc_mapping_db()->alloc();
    if (mapping) {
        new (mapping) svc_mapping();
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
    mapping->~svc_mapping();
    svc_mapping_db()->free(mapping);
}

sdk_ret_t
svc_mapping::init_config(api_ctxt_t *api_ctxt) {
    return SDK_RET_OK;
}

 sdk_ret_t
svc_mapping::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::program_config(obj_ctxt_t *obj_ctxt) {
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
svc_mapping::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping::activate_config(pds_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
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

sdk_ret_t
svc_mapping::add_to_db(void) {
    // service mappings are not added to s/w db, so its a no-op
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping::del_from_db(void) {
    // service mappings are not added to s/w db, so its a no-op
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SVC_MAPPING, this);
}

/** @} */    // end of PDS_SVC_MAPPING

}    // namespace api
