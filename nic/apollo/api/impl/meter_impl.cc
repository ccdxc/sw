//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of metering
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/meter.hpp"
#include "nic/apollo/api/impl/meter_impl.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"
#include "nic/apollo/lpm/lpm.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_METER_IMPL - meter datapath implementation
/// \ingroup PDS_METER
/// \@{

meter_impl *
meter_impl::factory(pds_meter_spec_t *spec) {
    meter_impl    *impl;

    // TODO: move to slab later
    impl = (meter_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_METER_IMPL,
                                          sizeof(meter_impl));
    new (impl) meter_impl();
    return impl;
}

void
meter_impl::destroy(meter_impl *impl) {
    impl->~meter_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_METER_IMPL, impl);
}

// NOTE: reserve_resources() logic is same for both API_OP_CREATE and
//       API_OP_UPDATE as update doesn't reuse any of the existing resources
//       for this object
sdk_ret_t
meter_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t lpm_block_id;
    pds_meter_spec_t *spec = &obj_ctxt->api_params->meter_spec;

    // allocate free lpm slab for this meter table
    if (meter_impl_db()->lpm_idxr(spec->af)->alloc(&lpm_block_id) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate LPM block for meter %u",
                      spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    lpm_root_addr_ =
        meter_impl_db()->region_addr(spec->af) +
            meter_impl_db()->table_size(spec->af) * lpm_block_id;

    // allocate all the policer indices as well
    if (meter_impl_db()->policer_idxr()->alloc_block(&policer_idx_,
                                                     spec->num_rules) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate %u policers for meter %u",
                      spec->num_rules, spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    num_policers_ = spec->num_rules;
    return SDK_RET_OK;
}

sdk_ret_t
meter_impl::release_resources(api_base *api_obj) {
    uint32_t lpm_block_id;
    meter_entry *meter = (meter_entry *)api_obj;

    // just free the lpm block we are using
    if (lpm_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        lpm_block_id =
            (lpm_root_addr_ - meter_impl_db()->region_addr(meter->af()))/
                meter_impl_db()->table_size(meter->af());
        meter_impl_db()->lpm_idxr(meter->af())->free(lpm_block_id);
        // free all the policers allocated for this policy
        if (policer_idx_ != 0xFFFFFFFF) {
            for (uint32_t i = 0; i < num_policers_; i++) {
                meter_impl_db()->policer_idxr()->free(policer_idx_ + i);
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_impl::nuke_resources(api_base *api_obj) {
    // just free the lpm block we are using
    return this->release_resources(api_obj);
}

sdk_ret_t
meter_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t           ret;
    pds_meter_spec_t    *spec;
    pds_vpc_key_t       vpc_key;
    route_table_t       *rtable;
    vpc_entry           *vpc;
    meter_entry         *meter;
    uint32_t            n = 0, policer_id, num_prefixes;

    spec = &obj_ctxt->api_params->meter_spec;
    // allocate memory for the library to build route table
    for (uint32_t i = 0, num_prefixes = 0; i < spec->num_rules; i++) {
        num_prefixes += spec->rules[i].num_prefixes;
    }
    if (num_prefixes > PDS_MAX_PREFIX_PER_METER) {
        PDS_TRACE_ERR("No. of prefixes per meter %u exceed max. supported %u",
                      num_prefixes, PDS_MAX_PREFIX_PER_METER);
        return SDK_RET_INVALID_ARG;

    }
    rtable =
        (route_table_t *)
            SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                       sizeof(route_table_t) +
                           (num_prefixes * sizeof(route_t)));
    if (rtable == NULL) {
        return sdk::SDK_RET_OOM;
    }
    rtable->af = spec->af;
    rtable->default_nhid = PDS_RESERVED_METER_HW_ID;
    rtable->max_routes = meter_impl_db()->max_prefixes(rtable->af);
    rtable->num_routes = num_prefixes;
    for (uint32_t i = 0; i < spec->num_rules; i++) {
        // TODO: program the policer here when table is available !!!
        for (uint32_t j = 0; j < spec->rules[i].num_prefixes; j++) {
            rtable->routes[n].prefix = spec->rules[i].prefixes[j];
            rtable->routes[n].nhid = policer_idx_ + i;
            n++;
        }
    }
    ret = lpm_tree_create(rtable, lpm_root_addr_,
                          meter_impl_db()->table_size(spec->af));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build LPM meter table, err : %u", ret);
    }
    SDK_FREE(PDS_MEM_ALLOC_ID_METER, rtable);
    return ret;
}

sdk_ret_t
meter_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                        api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case api::API_OP_CREATE:
    case api::API_OP_DELETE:
        // for meter create, there is no stage 0 programming
        // for meter delete, since all objects (e.g., vnics, subnets)
        // referring to this meter are already modified to point
        // to other meter table(s) or deleted (agent is expected to
        // ensure this), there is no need to program any tables during
        // activation stage
    case api::API_OP_UPDATE:
        // affected objects like affected vpc, subnet and vnic objects
        // are in the dependent object list by now and will be reprogrammed
    default:
        break;
    }
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
