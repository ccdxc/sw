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
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/meter.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"
#include "nic/apollo/api/impl/artemis/meter_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "nic/sdk/asic/rw/asicrw.hpp"

namespace api {
namespace impl {

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

// reserve_resources() logic is same for both API_OP_CREATE and API_OP_UPDATE
// as update doesn't reuse any of the existing resources for this object
sdk_ret_t
meter_impl::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
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

    // TODO:
    // supporting only accounting for now, ideally we should walk
    // all the rules, count num_policers_ and num_stats_entries_
    // and allocate resources accordingly
    num_policers_ = 0;
    if (num_policers_) {
        // allocate all the policer indices
        if (meter_impl_db()->policer_idxr()->alloc_block(&policer_base_hw_idx_,
                                                         num_policers_) !=
                sdk::lib::indexer::SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate %u policers for meter %u",
                          num_policers_, spec->key.id);
            return sdk::SDK_RET_NO_RESOURCE;
        }
    }
    num_stats_entries_ = spec->num_rules;
    if (num_stats_entries_) {
        // allocate all the stats indices
        if (meter_impl_db()->stats_idxr()->alloc_block(&stats_base_hw_idx_,
                                                       num_stats_entries_) !=
                sdk::lib::indexer::SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate %u stats entries for meter %u",
                          num_stats_entries_, spec->key.id);
            return sdk::SDK_RET_NO_RESOURCE;
        }
    }

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
        if (policer_base_hw_idx_ != 0xFFFFFFFF) {
            for (uint32_t i = 0; i < num_policers_; i++) {
                meter_impl_db()->policer_idxr()->free(policer_base_hw_idx_ + i);
            }
        }
        // free all the stats indices allocated for this policy
        if (stats_base_hw_idx_ != 0xFFFFFFFF) {
            for (uint32_t i = 0; i < num_stats_entries_; i++) {
                meter_impl_db()->stats_idxr()->free(stats_base_hw_idx_ + i);
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
meter_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t           ret;
    pds_meter_spec_t    *spec;
    pds_obj_key_t       vpc_key;
    route_table_t       *rtable;
    vpc_entry           *vpc;
    meter_entry         *meter;
    uint32_t            n = 0, nh_id, num_prefixes = 0;

    spec = &obj_ctxt->api_params->meter_spec;
    // allocate memory for the library to build route table
    for (uint32_t i = 0; i < spec->num_rules; i++) {
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
    rtable->default_nhid = PDS_IMPL_RESERVED_METER_HW_ID;
    rtable->max_routes = meter_impl_db()->max_prefixes(rtable->af);
    rtable->num_routes = num_prefixes;
    for (uint32_t i = 0; i < spec->num_rules; i++) {
        if (spec->rules[i].type == PDS_METER_TYPE_ACCOUNTING) {
            nh_id = stats_base_hw_idx_ + i;
        } else {
            // TODO: other types are not supported
            SDK_ASSERT(FALSE);
        }
        for (uint32_t j = 0; j < spec->rules[i].num_prefixes; j++) {
            PDS_TRACE_DEBUG("Processing meter table %u, pfx %s prio %u",
                            spec->key.id,
                            ippfx2str(&spec->rules[i].prefixes[j]),
                            spec->rules[i].priority);
            rtable->routes[n].prefix = spec->rules[i].prefixes[j];
            rtable->routes[n].nhid = nh_id;
            rtable->routes[n].prio = spec->rules[i].priority;
            n++;
        }
    }
    ret = lpm_tree_create(rtable,
                          (spec->af == IP_AF_IPV4) ? ITREE_TYPE_METER_V4 :
                                                     ITREE_TYPE_METER_V6,
                          lpm_root_addr_,
                          meter_impl_db()->table_size(spec->af));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build LPM meter table %u, err : %u",
                      spec->key.id, ret);
    }
    SDK_FREE(PDS_MEM_ALLOC_ID_METER, rtable);
    return ret;
}

sdk_ret_t
meter_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                        pds_epoch_t epoch, api_op_t api_op,
                        api_obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_DELETE:
        // for meter create, there is no stage 0 programming
        // for meter delete, since all objects (e.g., vnics, subnets)
        // referring to this meter are already modified to point
        // to other meter table(s) or deleted (agent is expected to
        // ensure this), there is no need to program any tables during
        // activation stage
    case API_OP_UPDATE:
        // affected objects like affected vpc, subnet and vnic objects
        // are in the dependent object list by now and will be reprogrammed
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_impl::fill_stats_(pds_meter_info_t *info) {
    sdk_ret_t ret;
    pds_meter_stats_t stats = {0};
    uint64_t tx_offset = 0, rx_offset = 0;
    uint64_t start_addr = 0;

    info->spec.num_rules = num_stats_entries_;
    start_addr = api::g_pds_state.mempartition()->start_addr("meter_stats");
    for (uint32_t idx = stats_base_hw_idx_; idx < stats_base_hw_idx_
                                                + num_stats_entries_; idx++) {
        tx_offset = idx * 8; // Each statistics is 8B
        rx_offset = tx_offset + (METER_STATS_TABLE_SIZE << 2); // (SIZE * 8/2)
        stats.idx = idx;
        ret = sdk::asic::asic_mem_read(start_addr + tx_offset,
                                       (uint8_t *)&stats.tx_bytes, 8);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        ret = sdk::asic::asic_mem_read(start_addr + rx_offset,
                                       (uint8_t *)&stats.rx_bytes, 8);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        info->stats.tx_bytes += stats.tx_bytes;
        info->stats.rx_bytes += stats.rx_bytes;
    }
    return SDK_RET_OK;
}

sdk_ret_t
meter_impl::read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) {
    sdk_ret_t ret;
    pds_meter_info_t *meter_info = (pds_meter_info_t *)info;
    pds_meter_key_t *meter_key = (pds_meter_key_t *)key;

    meter_info->stats.idx = meter_key->id;
    ret = fill_stats_(meter_info);

    return ret;
}

}    // namespace impl
}    // namespace api
