//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP datapath implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/artemis/tep_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL - tep datapath implementation
/// \ingroup PDS_TEP
/// @{

tep_impl *
tep_impl::factory(pds_tep_spec_t *spec) {
    tep_impl *impl;

    if (spec->type != PDS_TEP_TYPE_SERVICE) {
        // other type of TEPs are not supported in this pipeline
        return NULL;
    }

    // TODO: move to slab later
    impl = (tep_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TEP_IMPL,
                                  sizeof(tep_impl));
    new (impl) tep_impl();
    return impl;
}

void
tep_impl::destroy(tep_impl *impl) {
    impl->~tep_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_TEP_IMPL, impl);
}

sdk_ret_t
tep_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t idx;
    pds_tep_spec_t *tep_spec;

    tep_spec = &obj_ctxt->api_params->tep_spec;
    if (tep_spec->type != PDS_TEP_TYPE_SERVICE) {
        PDS_TRACE_ERR("Unsupported TEP type %u", tep_spec->type);
        return SDK_RET_INVALID_ARG;
    }

    // reserve an entry in TEP_table
    if (tep_impl_db()->remote_46_tep_idxr()->alloc(&idx) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate hw id for TEP %s",
                      ipaddr2str(&tep_spec->key.ip_addr));
        return sdk::SDK_RET_NO_RESOURCE;
    }
    hw_id_ = idx & 0xFFFF;
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::release_resources(api_base *api_obj) {
    if (hw_id_ != 0xFFFF) {
        tep_impl_db()->remote_46_tep_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::nuke_resources(api_base *api_obj) {
    if (hw_id_ != 0xFFFF) {
        tep_impl_db()->remote_46_tep_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    tep_entry *tep;
    p4pd_error_t p4pd_ret;
    pds_tep_spec_t *tep_spec;
    remote_46_mapping_actiondata_t remote_46_mapping_data = { 0 };

    tep_spec = &obj_ctxt->api_params->tep_spec;
    tep = (tep_entry *)api_obj;
    switch (tep_spec->type) {
    case PDS_TEP_TYPE_SERVICE:
        // program REMOTE_46_MAPPING table entry
        remote_46_mapping_data.action_id = REMOTE_46_MAPPING_REMOTE_46_INFO_ID;
        sdk::lib::memrev(remote_46_mapping_data.action_u.remote_46_mapping_remote_46_info.ipv6_tx_da,
                         tep_spec->ip_addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        p4pd_ret = p4pd_global_entry_write(P4_ARTEMIS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                           hw_id_, NULL, NULL,
                                           &remote_46_mapping_data);

        if (unlikely(p4pd_ret != P4PD_SUCCESS)) {
            PDS_TRACE_ERR("TEP table programming failed for TEP %s, "
                          "TEP hw id %u", api_obj->key2str().c_str(), hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        ret = SDK_RET_OK;
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }
    PDS_TRACE_DEBUG("Programmed TEP %s, hw id %u",
                    ipaddr2str(&tep_spec->key.ip_addr), hw_id_);
    return ret;
}

sdk_ret_t
tep_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

void
tep_impl::fill_status_(pds_tep_status_t *status) {
    status->hw_id = hw_id_;
}

void
tep_impl::fill_spec_(remote_46_mapping_actiondata_t *data,
                     pds_tep_spec_t *spec) {
    spec->type = PDS_TEP_TYPE_SERVICE;
    spec->ip_addr.af = IP_AF_IPV6;
    sdk::lib::memrev(spec->ip_addr.addr.v6_addr.addr8,
                     data->action_u.remote_46_mapping_remote_46_info.ipv6_tx_da,
                     IP6_ADDR8_LEN);
}

sdk_ret_t
tep_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    remote_46_mapping_actiondata_t remote_46_mapping_data = { 0 };
    p4pd_error_t p4pdret;
    pds_tep_info_t *tep_info = (pds_tep_info_t *)info;

    p4pdret = p4pd_global_entry_read(P4_ARTEMIS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                     hw_id_, NULL, NULL,
                                     &remote_46_mapping_data);
    if (unlikely(p4pdret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("Failed to read REMOTE_46_MAPPING table for TEP %s at "
                      "hw id %u, ret %d", api_obj->key2str().c_str(),
                      hw_id_, p4pdret);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    fill_spec_(&remote_46_mapping_data, &tep_info->spec);
    fill_status_(&tep_info->status);

    return sdk::SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
