//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Base object definition for all impl objects
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/impl.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_device.hpp"

namespace api {
namespace impl {

asic_impl_base *impl_base::asic_impl_  = NULL;
pipeline_impl_base *impl_base::pipeline_impl_  = NULL;

sdk_ret_t
impl_base::init(pds_init_params_t *params, asic_cfg_t *asic_cfg) {
    pipeline_cfg_t pipeline_cfg;

    // instanitiate asic implementaiton object
    asic_impl_ = asic_impl_base::factory(asic_cfg);
    SDK_ASSERT(asic_impl_ != NULL);

    // instanitiate pipeline implementaiton object
    pipeline_cfg.name = params->pipeline;
    pipeline_impl_ = pipeline_impl_base::factory(&pipeline_cfg);
    SDK_ASSERT(pipeline_impl_ != NULL);

    // initialize program and asm specific configs
    pipeline_impl_->program_config_init(params, asic_cfg);
    pipeline_impl_->asm_config_init(params, asic_cfg);
    pipeline_impl_->ring_config_init(asic_cfg);

    // perform asic initialization
    asic_impl_->asic_init(asic_cfg);

    // followed by pipeline initialization
    pipeline_impl_->pipeline_init();

    return SDK_RET_OK;
}

void
impl_base::destroy(void) {
    pipeline_impl_->destroy(pipeline_impl_);
    asic_impl_->destroy(asic_impl_);
}

impl_base *
impl_base::factory(impl_obj_id_t obj_id, void *args) {
    switch (obj_id) {
    default:
        break;
    }
    return NULL;
}

impl_base *
impl_base::build(impl_obj_id_t obj_id, void *key, api_base *api_obj) {
    switch (obj_id) {
    default:
        break;
    }
    return NULL;
}

void
impl_base::soft_delete(impl_obj_id_t obj_id, impl_base *impl) {
    switch(obj_id) {
    default:
        PDS_TRACE_ERR("Non-statless obj %u can't be soft deleted\n", obj_id);
        break;
    }
}

void
impl_base::destroy(impl_obj_id_t obj_id, impl_base *impl) {
    switch (obj_id) {
    default:
        break;
    }
}

sdk_ret_t
impl_base::free(impl_obj_id_t obj_id, impl_base *impl) {
    return SDK_RET_INVALID_OP;
}

}    // namespace impl
}    // namespace api
