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
    PROGRAM_POLICER_TABLE_ENTRY(policer, vnic_policer_rx,
                                P4TBL_ID_VNIC_POLICER_RX,
                                VNIC_POLICER_RX_VNIC_POLICER_RX_ID, idx, upd);
}

static inline sdk_ret_t
program_vnic_policer_tx_entry_ (sdk::policer_t *policer, uint16_t idx, bool upd)
{
    PROGRAM_POLICER_TABLE_ENTRY(policer, vnic_policer_tx,
                                P4TBL_ID_VNIC_POLICER_TX,
                                VNIC_POLICER_TX_VNIC_POLICER_TX_ID, idx, upd);
}
#undef vnic_policer_tx_info
#undef vnic_policer_rx_info

policer_impl *
policer_impl::factory(pds_policer_spec_t *spec) {
    policer_impl *impl;

    impl = (policer_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_POLICER_IMPL,
                                      sizeof(policer_impl));
    new (impl) policer_impl();
    return impl;
}

void
policer_impl::destroy(policer_impl *impl) {
    impl->~policer_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_POLICER_IMPL, impl);
}

sdk_ret_t
policer_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_policer_spec_t *spec;

    spec = &obj_ctxt->api_params->policer_spec;
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
        PDS_TRACE_ERR("Failed to reserve entry in policer table for policer %u,"
                      " err %u", spec->key.id, ret);
        return ret;
    }
    hw_id_ = idx;
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
policer_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
policer_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
policer_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
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
        PDS_TRACE_ERR("Failed to activate policer %u, hw id %u",
                      spec->key.id, hw_id_);
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
        PDS_TRACE_ERR("Failed to de-activate policer %u, hw id %u",
                      policer->key().id, hw_id_);
    }
    return ret;
}

sdk_ret_t
policer_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          obj_ctxt_t *obj_ctxt) {
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
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
policer_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                            api_op_t api_op) {
    return SDK_RET_OK;
}
void
policer_impl::fill_status_(pds_policer_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
policer_impl::fill_spec_(pds_policer_spec_t *spec) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
policer_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_POLICER_IMPL

}    // namespace impl
}    // namespace api
