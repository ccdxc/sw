//
// Copyright (c) 2019 Pensando Systems, Inc.
//----------------------------------------------------------------------------
///
/// \file
/// service mapping entry handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"

namespace api {

svc_mapping::svc_mapping() {
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
            PDS_TRACE_ERR("Failed to clone mapping %s impl", key2str().c_str());
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
svc_mapping::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
svc_mapping::nuke_resources_(void) {
    if (impl_ == NULL) {
        return SDK_RET_OK;
    }
    return impl_->nuke_resources(this);
}

sdk_ret_t
svc_mapping::init_config(api_ctxt_t *api_ctxt) {
    pds_svc_mapping_spec_t *spec = &api_ctxt->api_params->svc_mapping_spec;

    memcpy(&key_, &spec->key, sizeof(pds_svc_mapping_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping::program_create(api_obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
svc_mapping::compute_update(api_obj_ctxt_t *obj_ctxt) {
    // we can compare the (VIP, svc port) and optimize for no update case in
    // future but ideally we shouldn't hit that case as agent/controller
    // shouldn't come down to HAL in such cases
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->update_hw(orig_obj, this, obj_ctxt);
}

sdk_ret_t
svc_mapping::activate_config(pds_epoch_t epoch, api_op_t api_op,
                             api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
svc_mapping::read(pds_svc_mapping_key_t *key, pds_svc_mapping_info_t *info) {
    return impl_->read_hw(this, (impl::obj_key_t *)key,
                          (impl::obj_info_t *)info);
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
svc_mapping::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (svc_mapping_db()->remove((svc_mapping *)orig_obj)) {
        return svc_mapping_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
svc_mapping::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SVC_MAPPING, this);
}

string
svc_mapping::key2str(void) const {
    std::string ret("svc-(");

    ret += std::to_string(key_.vpc.id) + "," + ipaddr2str(&key_.backend_ip);
    ret += ":" + std::to_string(key_.backend_port) + ")";

    return ret;
}

/** @} */    // end of PDS_SVC_MAPPING

}    // namespace api
