//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of security policy
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/apulu/security_policy_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SECURITY_POLICY_IMPL - security policy datapath implementation
/// \ingroup PDS_SECURITY_POLICY
/// @{

security_policy_impl *
security_policy_impl::factory(pds_policy_spec_t *spec) {
    security_policy_impl    *impl;

    impl = security_policy_impl_db()->alloc();
    new (impl) security_policy_impl();
    return impl;
}

void
security_policy_impl::destroy(security_policy_impl *impl) {
    impl->~security_policy_impl();
    security_policy_impl_db()->free(impl);
}

impl_base *
security_policy_impl::clone(void) {
    security_policy_impl *cloned_impl;

    cloned_impl = security_policy_impl_db()->alloc();
    new (cloned_impl) security_policy_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
security_policy_impl::free(security_policy_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::reserve_resources(api_base *api_obj,
                                        api_obj_ctxt_t *obj_ctxt) {
    uint32_t             policy_block_id;
    pds_policy_spec_t    *spec;

    spec = &obj_ctxt->api_params->policy_spec;
    // record the fact that resource reservation was attempted
    // NOTE: even if we partially acquire resources and fail eventually,
    //       this will ensure that proper release of resources will happen
    api_obj->set_rsvd_rsc();
    // allocate available block for this security policy
    if (security_policy_impl_db()->security_policy_idxr(spec->af,
                                       spec->direction)->alloc(&policy_block_id) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    security_policy_root_addr_ =
        security_policy_impl_db()->security_policy_region_addr(spec->af,
                                       spec->direction) +
            (security_policy_impl_db()->security_policy_table_size(spec->af,
                                            spec->direction) * policy_block_id);
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::release_resources(api_base *api_obj) {
    uint32_t    policy_block_id;
    policy      *security_policy;

    security_policy = (policy *)api_obj;
    if (security_policy_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        policy_block_id =
            (security_policy_root_addr_ -
                 security_policy_impl_db()->security_policy_region_addr(security_policy->af(),
                                                                        security_policy->dir()))/
                 security_policy_impl_db()->security_policy_table_size(security_policy->af(),
                                                                       security_policy->dir());
        security_policy_impl_db()->security_policy_idxr(security_policy->af(),
                                                        security_policy->dir())->free(policy_block_id);
    }
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::nuke_resources(api_base *api_obj) {
    return this->release_resources(api_obj);
}

sdk_ret_t
security_policy_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t            ret;
    pds_policy_spec_t    *spec;
    rfc::policy_t        policy;

    spec = &obj_ctxt->api_params->policy_spec;

    memset(&policy, 0, sizeof(policy));
    policy.policy_type = spec->policy_type;
    policy.af = spec->af;
    policy.direction = spec->direction;
    policy.max_rules =
        (policy.af ==IP_AF_IPV4) ? PDS_MAX_RULES_PER_IPV4_SECURITY_POLICY:
                                   PDS_MAX_RULES_PER_IPV6_SECURITY_POLICY;
    policy.num_rules = spec->num_rules;
    policy.rules = spec->rules;
    PDS_TRACE_DEBUG("Processing security policy %s", spec->key.tostr());
    ret = rfc_policy_create(&policy, security_policy_root_addr_,
              security_policy_impl_db()->security_policy_table_size(spec->af,
                                             spec->direction));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build RFC policy table, err %u", ret);
    }
    return ret;
}

sdk_ret_t
security_policy_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                                api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
security_policy_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        // no programming needed in stage0 for these
    case API_OP_DELETE:
        // delete operation will free the hbm region in use altogether
    default:
        break;
    }
    return SDK_RET_OK;
}

void
security_policy_impl::fill_status_(pds_policy_status_t *status) {
    status->policy_base_addr = security_policy_root_addr_;
}

sdk_ret_t
security_policy_impl::read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) {
    pds_policy_info_t *policy_info = (pds_policy_info_t *)info;

    fill_status_(&policy_info->status);

    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
