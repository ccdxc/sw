//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of policer
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/policer.hpp"
#include "nic/apollo/api/impl/apulu/policer_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_POLICER_IMPL - policer datapath implementation
/// \ingroup PDS_POLICER
/// \@{

#define vnic_policer_rx_info    action_u.vnic_policer_rx_vnic_policer_rx
#define vnic_policer_tx_info    action_u.vnic_policer_tx_vnic_policer_tx
static inline sdk_ret_t
program_vnic_policer_rx_entry_ (sdk::policer_t *policer, uint16_t idx, bool upd)
{
    p4pd_table_properties_t tbl_props = { 0 };

    p4pd_table_properties_get(P4TBL_ID_VNIC_POLICER_RX, &tbl_props);
    PROGRAM_POLICER_TABLE_ENTRY(policer, vnic_policer_rx,
                                P4TBL_ID_VNIC_POLICER_RX,
                                VNIC_POLICER_RX_VNIC_POLICER_RX_ID, idx, upd,
                                tbl_props.token_refresh_rate);
}

static inline sdk_ret_t
program_vnic_policer_tx_entry_ (sdk::policer_t *policer, uint16_t idx, bool upd)
{
    p4pd_table_properties_t tbl_props = { 0 };

    p4pd_table_properties_get(P4TBL_ID_VNIC_POLICER_TX, &tbl_props);
    PROGRAM_POLICER_TABLE_ENTRY(policer, vnic_policer_tx,
                                P4TBL_ID_VNIC_POLICER_TX,
                                VNIC_POLICER_TX_VNIC_POLICER_TX_ID, idx, upd,
                                tbl_props.token_refresh_rate);
}

policer_impl *
policer_impl::factory(pds_policer_spec_t *spec) {
    policer_impl *impl;

    impl = policer_impl_db()->alloc();
    new (impl) policer_impl();
    return impl;
}

void
policer_impl::destroy(policer_impl *impl) {
    impl->~policer_impl();
    policer_impl_db()->free(impl);
}

impl_base *
policer_impl::clone(void) {
    policer_impl *cloned_impl;

    cloned_impl = policer_impl_db()->alloc();
    new (cloned_impl) policer_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
policer_impl::free(policer_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_policer_spec_t *spec;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->policer_spec;
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        if (spec->dir == PDS_POLICER_DIR_INGRESS) {
            // reserve an entry in Rx policer table
            ret = policer_impl_db()->rx_idxr()->alloc(&idx);
        } else if (spec->dir == PDS_POLICER_DIR_EGRESS) {
            // reserve an entry in Tx policer table
            ret = policer_impl_db()->tx_idxr()->alloc(&idx);
        } else {
            return SDK_RET_INVALID_ARG;
        }
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in policer table for "
                          "policer %s, err %u", spec->key.str(), ret);
            return ret;
        }
        hw_id_ = idx;
        break;

    case API_OP_UPDATE:
        // we will use the same h/w resources as the original object
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::release_resources(api_base *api_obj) {
    policer_entry *policer = (policer_entry *)api_obj;

    if (hw_id_ != 0xFFFF) {
        if (policer->dir() == PDS_POLICER_DIR_INGRESS) {
            return policer_impl_db()->rx_idxr()->free(hw_id_);
        } else {
            return policer_impl_db()->tx_idxr()->free(hw_id_);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::nuke_resources(api_base *api_obj) {
    policer_entry *policer = (policer_entry *)api_obj;

    if (hw_id_ != 0xFFFF) {
        if (policer->dir() == PDS_POLICER_DIR_INGRESS) {
            return policer_impl_db()->rx_idxr()->free(hw_id_);
        } else {
            return policer_impl_db()->tx_idxr()->free(hw_id_);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::reprogram_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
policer_impl::activate_create_(pds_epoch_t epoch, policer_entry *policer,
                               pds_policer_spec_t *spec) {
    sdk_ret_t ret;
    sdk::policer_t pol;

    if (spec->dir == PDS_POLICER_DIR_INGRESS) {
        if (spec->type == sdk::POLICER_TYPE_PPS) {
            pol = { sdk::POLICER_TYPE_PPS, spec->pps, spec->pps_burst };
            ret = program_vnic_policer_rx_entry_(&pol, hw_id_, false);
        } else {
            pol = { sdk::POLICER_TYPE_BPS, spec->bps, spec->bps_burst };
            ret = program_vnic_policer_rx_entry_(&pol, hw_id_, false);
        }
    } else {
        if (spec->type == sdk::POLICER_TYPE_PPS) {
            pol = { sdk::POLICER_TYPE_PPS, spec->pps, spec->pps_burst };
            ret = program_vnic_policer_tx_entry_(&pol, hw_id_, false);
        } else {
            pol = { sdk::POLICER_TYPE_BPS, spec->bps, spec->bps_burst };
            ret = program_vnic_policer_tx_entry_(&pol, hw_id_, false);
        }
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to activate policer %s, hw id %u create",
                      spec->key.str(), hw_id_);
    }
    return ret;
}

sdk_ret_t
policer_impl::activate_update_(pds_epoch_t epoch, policer_entry *policer,
                               pds_policer_spec_t *spec) {
    sdk_ret_t ret;
    sdk::policer_t pol;

    if (spec->dir == PDS_POLICER_DIR_INGRESS) {
        if (spec->type == sdk::POLICER_TYPE_PPS) {
            pol = { sdk::POLICER_TYPE_PPS, spec->pps, spec->pps_burst };
            ret = program_vnic_policer_rx_entry_(&pol, hw_id_, true);
        } else {
            pol = { sdk::POLICER_TYPE_BPS, spec->bps, spec->bps_burst };
            ret = program_vnic_policer_rx_entry_(&pol, hw_id_, true);
        }
    } else {
        if (spec->type == sdk::POLICER_TYPE_PPS) {
            pol = { sdk::POLICER_TYPE_PPS, spec->pps, spec->pps_burst };
            ret = program_vnic_policer_tx_entry_(&pol, hw_id_, true);
        } else {
            pol = { sdk::POLICER_TYPE_BPS, spec->bps, spec->bps_burst };
            ret = program_vnic_policer_tx_entry_(&pol, hw_id_, true);
        }
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to activate policer %s, hw id %u update",
                      spec->key.str(), hw_id_);
    }
    return ret;
}

sdk_ret_t
policer_impl::activate_delete_(pds_epoch_t epoch, policer_entry *policer) {
    sdk_ret_t ret;
    sdk::policer_t pol;

    // while deactivating policer, exact policer type doesn't matter as values
    // are 0 for rate
    if (policer->dir() == PDS_POLICER_DIR_INGRESS) {
        pol = { sdk::POLICER_TYPE_PPS, 0, 0 };
        ret = program_vnic_policer_rx_entry_(&pol, hw_id_, false);
    } else {
        pol = { sdk::POLICER_TYPE_PPS, 0, 0 };
        ret = program_vnic_policer_tx_entry_(&pol, hw_id_, false);
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to de-activate policer %s, hw id %u",
                      policer->key().str(), hw_id_);
    }
    return ret;
}

sdk_ret_t
policer_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_policer_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->policer_spec;
        ret = activate_create_(epoch, (policer_entry *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (policer_entry *)api_obj);
        break;

    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->policer_spec;
        ret = activate_update_(epoch, (policer_entry *)api_obj, spec);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
policer_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                            api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

void
policer_impl::fill_stats_(pds_policer_stats_t *stats) {
}

void
policer_impl::fill_status_(pds_policer_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
policer_impl::fill_spec_(pds_policer_spec_t *spec) {
    p4pd_error_t p4pd_ret;
    p4pd_table_properties_t tbl_props = { 0 };
    vnic_policer_rx_actiondata_t rx_data;
    vnic_policer_tx_actiondata_t tx_data;
    uint64_t rate, burst;

    if (hw_id_ == 0xFFFF) {
        return sdk::SDK_RET_HW_READ_ERR;
    }

    if (spec->dir == PDS_POLICER_DIR_INGRESS) {
        p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_POLICER_RX, hw_id_,
                                          NULL, NULL, &rx_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read rx policer table at index %u", hw_id_);
            return sdk::SDK_RET_HW_READ_ERR;
        }
        POLICER_READ_HW_ENTRY(rx_data.vnic_policer_rx_info.rate, rate);
        POLICER_READ_HW_ENTRY(rx_data.vnic_policer_rx_info.burst, burst);

        p4pd_table_properties_get(P4TBL_ID_VNIC_POLICER_RX, &tbl_props);
        if (rx_data.vnic_policer_rx_info.pkt_rate) {
            spec->type = sdk::POLICER_TYPE_PPS;
            sdk::policer_token_to_rate(rate, burst,
                                       tbl_props.token_refresh_rate,
                                       &spec->pps, &spec->pps_burst);
        } else {
            spec->type = sdk::POLICER_TYPE_BPS;
            sdk::policer_token_to_rate(rate, burst,
                                       tbl_props.token_refresh_rate,
                                       &spec->bps, &spec->bps_burst);
        }
    } else if (spec->dir == PDS_POLICER_DIR_EGRESS) {
        p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_POLICER_TX, hw_id_,
                                          NULL, NULL, &tx_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read policer table at index %u", hw_id_);
            return sdk::SDK_RET_HW_READ_ERR;
        }
        POLICER_READ_HW_ENTRY(tx_data.vnic_policer_tx_info.rate, rate);
        POLICER_READ_HW_ENTRY(tx_data.vnic_policer_tx_info.burst, burst);

        p4pd_table_properties_get(P4TBL_ID_VNIC_POLICER_TX, &tbl_props);
        if (tx_data.vnic_policer_tx_info.pkt_rate) {
            spec->type = sdk::POLICER_TYPE_PPS;
            sdk::policer_token_to_rate(rate, burst,
                                       tbl_props.token_refresh_rate,
                                       &spec->pps, &spec->pps_burst);
        } else {
            spec->type = sdk::POLICER_TYPE_BPS;
            sdk::policer_token_to_rate(rate, burst,
                                       tbl_props.token_refresh_rate,
                                       &spec->bps, &spec->bps_burst);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_policer_info_t *policer = (pds_policer_info_t *)info;

    rv = fill_spec_(&policer->spec);
    if (rv != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read policer %s table entry",
                      api_obj->key2str().c_str());
        return rv;
    }
    fill_status_(&policer->status);
    fill_stats_(&policer->stats);
    return SDK_RET_OK;
}

#undef vnic_policer_tx_info
#undef vnic_policer_rx_info

/// \@}    // end of PDS_POLICER_IMPL

}    // namespace impl
}    // namespace api
