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
#include "nic/apollo/api/impl/device_impl.hpp"    // TODO: think how to accomodate a new impl
#include "nic/apollo/api/impl/tep_impl.hpp"
#include "nic/apollo/api/impl/vnic_impl.hpp"
#include "nic/apollo/api/impl/mapping_impl.hpp"
#include "nic/apollo/api/impl/route_impl.hpp"
#include "nic/apollo/api/impl/mirror_impl.hpp"
#include "nic/apollo/api/impl/security_policy_impl.hpp"
#include "nic/apollo/api/impl/meter_impl.hpp"
#include "nic/apollo/api/impl/tag_impl.hpp"

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

void
impl_base::debug_dump(FILE *fp) {
    asic_impl_->debug_dump(fp);
    pipeline_impl_->debug_dump(fp);
}

impl_base *
impl_base::factory(impl_obj_id_t obj_id, void *args) {
    switch (obj_id) {
    case IMPL_OBJ_ID_DEVICE:
        return device_impl::factory((pds_device_spec_t *)args);

    case IMPL_OBJ_ID_TEP:
        return tep_impl::factory((pds_tep_spec_t *)args);

    case IMPL_OBJ_ID_VNIC:
        return vnic_impl::factory((pds_vnic_spec_t *)args);

    case IMPL_OBJ_ID_MAPPING:
        return mapping_impl::factory((pds_mapping_spec_t *)args);

    case IMPL_OBJ_ID_ROUTE_TABLE:
        return route_table_impl::factory((pds_route_table_spec_t *)args);

    case IMPL_OBJ_ID_SECURITY_POLICY:
        return security_policy_impl::factory((pds_policy_spec_t *)args);

    case IMPL_OBJ_ID_MIRROR_SESSION:
        return mirror_impl::factory((pds_mirror_session_spec_t *)args);

    case IMPL_OBJ_ID_METER:
        return meter_impl::factory((pds_meter_spec_t *)args);

    case IMPL_OBJ_ID_TAG:
        return tag_impl::factory((pds_tag_spec_t *)args);

    default:
        break;
    }
    return NULL;
}

impl_base *
impl_base::build(impl_obj_id_t obj_id, void *args) {
    switch (obj_id) {
    case IMPL_OBJ_ID_MAPPING:
        return mapping_impl::build((pds_mapping_key_t *)args);

    case IMPL_OBJ_ID_MIRROR_SESSION:
        return mirror_impl::build((pds_mirror_session_key_t *)args);

    default:
        break;
    }
    return NULL;
}

void
impl_base::soft_delete(impl_obj_id_t obj_id, impl_base *impl) {
    switch(obj_id) {
    case IMPL_OBJ_ID_MAPPING:
        mapping_impl::soft_delete((mapping_impl *)impl);
        break;

    case IMPL_OBJ_ID_MIRROR_SESSION:
        mirror_impl::soft_delete((mirror_impl *)impl);
        break;

    default:
        PDS_TRACE_ERR("Non-statless obj %u can't be soft deleted\n", obj_id);
        break;
    }
}

void
impl_base::destroy(impl_obj_id_t obj_id, impl_base *impl) {
    switch (obj_id) {
    case IMPL_OBJ_ID_DEVICE:
        return device_impl::destroy((device_impl *)impl);

    case IMPL_OBJ_ID_TEP:
        return tep_impl::destroy((tep_impl *)impl);

    case IMPL_OBJ_ID_VNIC:
        return vnic_impl::destroy((vnic_impl *)impl);

    case IMPL_OBJ_ID_MAPPING:
        return mapping_impl::destroy((mapping_impl *)impl);

    case IMPL_OBJ_ID_ROUTE_TABLE:
        return route_table_impl::destroy((route_table_impl *)impl);

    case IMPL_OBJ_ID_SECURITY_POLICY:
        return security_policy_impl::destroy((security_policy_impl *)impl);

    case IMPL_OBJ_ID_MIRROR_SESSION:
        return mirror_impl::destroy((mirror_impl *)impl);

    default:
        break;
    }
}

}    // namespace impl
}    // namespace api
