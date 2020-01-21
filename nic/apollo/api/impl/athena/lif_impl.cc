//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of lif
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/athena/pds_impl_state.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/impl/athena/athena_impl.hpp"
#include "nic/p4/common/defines.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/athena/include/p4pd.h"

namespace api {
namespace impl {

lif_impl *
lif_impl::factory (pds_lif_spec_t *spec) {
    lif_impl *impl;
    impl = lif_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) lif_impl(spec);
    return impl;
}

void
lif_impl::destroy(lif_impl *impl) {
    impl->~lif_impl();
    lif_impl_db()->free(impl);
}

lif_impl::lif_impl(pds_lif_spec_t *spec) {
    memcpy(&key_, &spec->key, sizeof(key_));
    id_ = spec->id;
    pinned_if_idx_ = spec->pinned_ifidx;
    type_ = spec->type;
    memcpy(mac_, spec->mac, ETH_ADDR_LEN);
    ifindex_ = LIF_IFINDEX(id_);
    nh_idx_ = 0xFFFFFFFF;
    vnic_hw_id_ = 0xFFFF;
    ht_ctxt_.reset();
    id_ht_ctxt_.reset();
}

#define  lif_egress_rl_params \
    action_u.tx_table_s5_t4_lif_rate_limiter_table_tx_stage5_lif_egress_rl_params
sdk_ret_t
lif_impl::program_tx_policer(uint32_t lif_id, sdk::policer_t *policer) {
    sdk_ret_t ret;
    tx_table_s5_t4_lif_rate_limiter_table_actiondata_t rlimit_data = { 0 };
    uint64_t refresh_interval_us = SDK_DEFAULT_POLICER_REFRESH_INTERVAL;
    uint64_t rate_tokens = 0, burst_tokens = 0, rate;

    rlimit_data.action_id =
        TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE_TX_STAGE5_LIF_EGRESS_RL_PARAMS_ID;
    if (policer->rate == 0) {
        rlimit_data.lif_egress_rl_params.entry_valid = 0;
    } else {
        rlimit_data.lif_egress_rl_params.entry_valid = 1;
        rlimit_data.lif_egress_rl_params.pkt_rate =
            (policer->type == sdk::policer_type_t::POLICER_TYPE_PPS) ? 1 : 0;
        rlimit_data.lif_egress_rl_params.rlimit_en = 1;
        ret = sdk::policer_to_token_rate(policer, refresh_interval_us,
                                         SDK_MAX_POLICER_TOKENS_PER_INTERVAL,
                                         &rate_tokens, &burst_tokens);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Error converting rate to token rate, err %u", ret);
            return ret;
        }
        memcpy(rlimit_data.lif_egress_rl_params.burst, &burst_tokens,
               sizeof(rlimit_data.lif_egress_rl_params.burst));
        memcpy(rlimit_data.lif_egress_rl_params.rate, &rate_tokens,
               sizeof(rlimit_data.lif_egress_rl_params.rate));
    }
    ret = lif_impl_db()->tx_rate_limiter_tbl()->insert_withid(&rlimit_data,
                                                              lif_id, NULL);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("LIF_TX_POLICER table write failure, lif %u, err %u",
                      lif_id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

#define nacl_redirect_action    action_u.nacl_nacl_redirect
sdk_ret_t
lif_impl::create_oob_mnic_(pds_lif_spec_t *spec) {
    sdk_ret_t              ret;
    nacl_swkey_t           key = { 0 };
    nacl_swkey_mask_t      mask = { 0 };
    nacl_actiondata_t      data =  { 0 };
    sdk_table_api_params_t tparams;
    uint32_t               idx;

    // ARM -> uplink
    key.capri_intrinsic_lif = id_;
    mask.capri_intrinsic_lif_mask = 0xFFFF;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport =
        g_pds_state.catalogue()->ifindex_to_tm_port(pinned_if_idx_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for mnic lif %u -> "
                      "uplink 0x%x, err %u", id_, pinned_if_idx_, ret);
        return ret;
    }

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // uplink -> ARM
    key.capri_intrinsic_lif =
        sdk::lib::catalog::ifindex_to_logical_port(pinned_if_idx_);
    mask.capri_intrinsic_lif_mask = 0xFFFF;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport = TM_PORT_DMA;
    data.nacl_redirect_action.lif = id_;
    //data.nacl_redirect_action.vlan_strip = spec->vlan_strip_en;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for uplink %u -> mnic "
                      "lif %u, err %u", pinned_if_idx_, id_, ret);
    }
    return ret;
}

sdk_ret_t
lif_impl::create_inb_mnic_(pds_lif_spec_t *spec) {
    sdk_ret_t              ret;
    nacl_swkey_t           key = { 0 };
    nacl_swkey_mask_t      mask = { 0 };
    nacl_actiondata_t      data =  { 0 };
    sdk_table_api_params_t tparams;
    uint32_t               idx;

    // ARM -> uplink (untag packets)
    key.capri_intrinsic_lif = id_;
    mask.capri_intrinsic_lif_mask = 0xFFFF;
    key.ctag_1_valid = 0;
    mask.ctag_1_valid_mask = 0xF;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport =
        g_pds_state.catalogue()->ifindex_to_tm_port(pinned_if_idx_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for mnic lif %u -> "
                      "uplink 0x%x, err %u", id_, pinned_if_idx_, ret);
        return ret;
    }

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // uplink -> ARM (untag packets)
    key.capri_intrinsic_lif =
        sdk::lib::catalog::ifindex_to_logical_port(pinned_if_idx_);
    mask.capri_intrinsic_lif_mask = 0xFFFF;
    key.ctag_1_valid = 0;
    mask.ctag_1_valid_mask = 0xF;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport = TM_PORT_DMA;
    data.nacl_redirect_action.lif = id_;
    //data.nacl_redirect_action.vlan_strip = spec->vlan_strip_en;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for uplink %u -> mnic "
                      "lif %u, err %u", pinned_if_idx_, id_, ret);
    }
    return ret;
}

sdk_ret_t
lif_impl::create_datapath_mnic_(pds_lif_spec_t *spec) {
    sdk_ret_t              ret;
    nacl_swkey_t           key = { 0 };
    nacl_swkey_mask_t      mask = { 0 };
    nacl_actiondata_t      data =  { 0 };
    sdk_table_api_params_t tparams = {0};

    PDS_TRACE_ERR("create_datapath_mnic_\n");

    // ARM -> uplink
    key.capri_intrinsic_lif = id_;
    mask.capri_intrinsic_lif_mask = 0xFFFF;

    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.redir_type = NACL_REDIR_UPLINK;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport =
        g_pds_state.catalogue()->ifindex_to_tm_port(pinned_if_idx_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    tparams.highest = true;
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program CPU-redirect NACL entry for mnic lif %u -> "
                      "uplink 0x%x, err %u\n", id_, pinned_if_idx_, ret);
        return ret;
    }

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // uplink -> ARM on flow miss
    key.control_metadata_flow_miss = 1;
    mask.control_metadata_flow_miss_mask = 1;
    key.capri_intrinsic_lif =
        sdk::lib::catalog::ifindex_to_logical_port(pinned_if_idx_);
    mask.capri_intrinsic_lif_mask = 0xFFFF;

    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.redir_type = NACL_REDIR_RXDMA;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport = TM_PORT_DMA;
    data.nacl_redirect_action.lif = id_;
    //data.nacl_redirect_action.vlan_strip = spec->vlan_strip_en;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    tparams.highest = true;
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program CPU-redirect NACL entry for uplink %u (key %u) -> "
                      "mnic lif %u, err %u\n", pinned_if_idx_, key.capri_intrinsic_lif,
                      id_, ret);
    }

    return ret;
}

typedef struct lif_internal_mgmt_ctx_s {
    lif_impl **lif;
    lif_type_t type;
} __PACK__ lif_internal_mgmt_ctx_t;

static bool
lif_internal_mgmt_cb_(void *api_obj, void *ctxt) {
    lif_impl *lif = (lif_impl *)api_obj;
    lif_internal_mgmt_ctx_t *cb_ctx = (lif_internal_mgmt_ctx_t *)ctxt;

    if (lif->type() == cb_ctx->type) {
        *cb_ctx->lif = lif;
        return true;
    }
    return false;
}

sdk_ret_t
lif_impl::create_internal_mgmt_mnic_(pds_lif_spec_t *spec) {
    uint32_t idx;
    sdk_ret_t ret;
    nacl_swkey_t key = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    nacl_actiondata_t data =  { 0 };
    lif_impl *host_mgmt_lif = NULL, *mnic_int_mgmt_lif = NULL;
    lif_internal_mgmt_ctx_t cb_ctx = {0};
    sdk_table_api_params_t  tparams;

    if (spec->type == sdk::platform::LIF_TYPE_HOST_MGMT) {
        host_mgmt_lif = this;
        cb_ctx.type = sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT;
        cb_ctx.lif = &mnic_int_mgmt_lif;
    } else if (spec->type == sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT) {
        mnic_int_mgmt_lif = this;
        cb_ctx.type = sdk::platform::LIF_TYPE_HOST_MGMT;
        cb_ctx.lif = &host_mgmt_lif;
    }
    lif_impl_db()->walk(lif_internal_mgmt_cb_, &cb_ctx);

    if (!host_mgmt_lif || !mnic_int_mgmt_lif) {
        return SDK_RET_OK;
    }

    PDS_TRACE_DEBUG("programming nacls for internal management:");
    // program host_mgmt -> mnic_int_mgmt
    key.capri_intrinsic_lif = host_mgmt_lif->id();
    mask.capri_intrinsic_lif_mask = 0xFFFF;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport = TM_PORT_DMA;
    data.nacl_redirect_action.lif = mnic_int_mgmt_lif->id();
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed NACL entry for host_mgmt -> mnic_int_mgmt "
                      "err %u", ret);
    }

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    memset(&tparams, 0, sizeof(tparams));

    // program mnic_int_mgmt -> host_mgmt
    key.capri_intrinsic_lif = mnic_int_mgmt_lif->id();
    mask.capri_intrinsic_lif_mask = 0xFFFF;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    data.nacl_redirect_action.oport = TM_PORT_DMA;
    data.nacl_redirect_action.lif = host_mgmt_lif->id();
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = athena_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed NACL entry for mnic_int_mgmt -> host_mgmt"
                      "err %u", ret);
    }
    return ret;
}

sdk_ret_t
lif_impl::create(pds_lif_spec_t *spec) {
    sdk_ret_t ret = SDK_RET_OK;

    switch (spec->type) {
    case sdk::platform::LIF_TYPE_MNIC_OOB_MGMT:
        ret = create_oob_mnic_(spec);
        break;
    case sdk::platform::LIF_TYPE_MNIC_INBAND_MGMT:
        //ret = create_inb_mnic_(spec);
        break;
    case sdk::platform::LIF_TYPE_MNIC_CPU:
        ret = create_datapath_mnic_(spec);
        break;
    case sdk::platform::LIF_TYPE_HOST_MGMT:
    case sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT:
        ret = create_internal_mgmt_mnic_(spec);
        break;
    default:
        return SDK_RET_INVALID_ARG;
    }
    return ret;
}

}    // namespace impl
}    // namespace api
