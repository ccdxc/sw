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
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"
#include "nic/apollo/api/impl/apulu/if_impl.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/p4/common/defines.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_LIF_IMPL - lif entry datapath implementation
/// \ingroup PDS_LIF
/// \@{

lif_impl *
lif_impl::factory(pds_lif_spec_t *spec) {
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
    pinned_if_idx_ = spec->pinned_ifidx;
    type_ = spec->type;
    nh_idx_ = 0xFFFFFFFF;
    vnic_hw_id_ = 0xFFFF;
}

#define lif_action              action_u.lif_lif_info
static inline sdk_ret_t
program_lif_table_ (uint16_t lif_hw_id, uint16_t vnic_hw_id)
{
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    lif_actiondata_t lif_data = { 0 };

    // program the LIF table
    lif_data.action_id = LIF_LIF_INFO_ID;
    lif_data.lif_action.lif_type = P4_LIF_TYPE_HOST;
    lif_data.lif_action.vnic_id = vnic_hw_id;
    lif_data.lif_action.bd_id = PDS_IMPL_RSVD_BD_HW_ID;
    lif_data.lif_action.vpc_id = PDS_IMPL_RSVD_VPC_HW_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_LIF, lif_hw_id,
                                       NULL, NULL, &lif_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program LIF table for oob lif %u", lif_hw_id);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

#define lif_egress_rl_params       action_u.tx_table_s5_t4_lif_rate_limiter_table_tx_stage5_lif_egress_rl_params
sdk_ret_t
lif_impl::program_tx_policer(uint32_t lif_id, sdk::policer_t *policer) {
    sdk_ret_t ret;
    uint64_t rate_tokens = 0, burst_tokens = 0, rate;
    uint64_t refresh_interval_us = SDK_DEFAULT_POLICER_REFRESH_INTERVAL;
    tx_table_s5_t4_lif_rate_limiter_table_actiondata_t rlimit_data = { 0 };

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
#define nexthop_info            action_u.nexthop_nexthop_info
sdk_ret_t
lif_impl::create_oob_mnic_(pds_lif_spec_t *spec) {
    uint32_t idx;
    if_impl *intf;
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    pds_ifindex_t if_index;
    nacl_swkey_t key = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    nacl_actiondata_t data = { 0 };
    nexthop_actiondata_t nh_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Creating oob lif %u", key_);
    // TODO: fix this once block indexer starts working
    // allocate required nexthops
    //ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_, 2);
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entries for oob "
                      "lif %u, err %u", key_, ret);
        return ret;
    }

    // program the nexthop for ARM to uplink traffic
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.port =
        g_pds_state.catalogue()->ifindex_to_tm_port(pinned_if_idx_);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for oob lif %u "
                      "at idx %u", key_, nh_idx_);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // install NACL for ARM to uplink traffic (all vlans)
    key.capri_intrinsic_lif = key_;
    mask.capri_intrinsic_lif_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_redirect_action.nexthop_id = nh_idx_;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for oob lif %u to "
                      "uplink 0x%x, err %u", key_, pinned_if_idx_, ret);
        goto error;
    }
    PDS_TRACE_DEBUG("nacl lif %u -> nh type %u, idx %u, nh lif %u, port %u",
                    key.capri_intrinsic_lif, NEXTHOP_TYPE_NEXTHOP, nh_idx_,
                    nh_data.nexthop_info.lif, nh_data.nexthop_info.port);

    // TODO: fix this once block indexer starts working
    // allocate required nexthops
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entries for oob "
                      "lif %u, err %u", key_, ret);
        return ret;
    }
    // program the nexthop for uplink to ARM traffic
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.lif = key_;
    nh_data.nexthop_info.port = TM_PORT_DMA;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_, //nh_idx_ + 1,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for oob lif %u "
                      "at idx %u", key_, nh_idx_); //nh_idx_ + 1);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // install for NACL for uplink to ARM (untagged) traffic
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    memset(&tparams, 0, sizeof(tparams));
    if_index = ETH_IFINDEX_TO_UPLINK_IFINDEX(pinned_if_idx_);
    intf = (if_impl *)if_db()->find(&if_index)->impl();
    key.capri_intrinsic_lif = intf->hw_id();
    mask.capri_intrinsic_lif_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_redirect_action.nexthop_id = nh_idx_; //nh_idx_ + 1;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for uplink %u to oob "
                      "lif %u, err %u", pinned_if_idx_, key_, ret);
        goto error;
    }
    PDS_TRACE_DEBUG("nacl lif %u -> nh type %u, idx %u, nh lif %u, port %u",
                    key.capri_intrinsic_lif, NEXTHOP_TYPE_NEXTHOP,
                    nh_idx_, nh_data.nexthop_info.lif,
                    //nh_idx_ + 1, nh_data.nexthop_info.lif,
                    nh_data.nexthop_info.port);

    // allocate vnic h/w id for this lif
    if ((ret = vnic_impl_db()->vnic_idxr()->alloc(&idx)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate vnic h/w id for lif %u, err %u",
                      key_, ret);
        return ret;
    }
    // program the LIF table
    ret = program_lif_table_(key_, idx);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    return SDK_RET_OK;

error:

    nexthop_impl_db()->nh_idxr()->free(nh_idx_, 2);
    nh_idx_ = 0xFFFFFFFF;
    return ret;
}

sdk_ret_t
lif_impl::create_inb_mnic_(pds_lif_spec_t *spec) {
    uint32_t idx;
    if_impl *intf;
    sdk_ret_t ret;
    pds_ifindex_t if_index;
    p4pd_error_t p4pd_ret;
    nacl_swkey_t key = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    nacl_actiondata_t data =  { 0 };
    lif_actiondata_t lif_data = { 0 };
    nexthop_actiondata_t nh_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Creating inband lif %u", key_);
    // allocate required nexthops
    //ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_, 2);
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entries for inband "
                      "lif %u, err %u", key_, ret);
        return ret;
    }

    // program the nexthop for ARM to uplink traffic
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.port =
        g_pds_state.catalogue()->ifindex_to_tm_port(pinned_if_idx_);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for inb lif %u at "
                      "idx %u", key_, nh_idx_);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // install NACL for ARM to uplink traffic (all vlans)
    key.capri_intrinsic_lif = key_;
    mask.capri_intrinsic_lif_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_redirect_action.nexthop_id = nh_idx_;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for inb lif %u to "
                      "uplink 0x%x, err %u", key_, pinned_if_idx_, ret);
        return ret;
    }
    PDS_TRACE_DEBUG("nacl lif %u -> nh type %u, idx %u, nh lif %u, port %u",
                    key.capri_intrinsic_lif, NEXTHOP_TYPE_NEXTHOP, nh_idx_,
                    nh_data.nexthop_info.lif, nh_data.nexthop_info.port);


    // TODO: clean this up once block indexer is fixed
    // allocate required nexthops
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entries for inband "
                      "lif %u, err %u", key_, ret);
        return ret;
    }

    // program the nexthop for uplink to ARM traffic
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.lif = key_;
    nh_data.nexthop_info.port = TM_PORT_DMA;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_, //nh_idx_ + 1,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for oob lif %u "
                      "at idx %u", key_, nh_idx_); //nh_idx_ + 1);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // install for NACL for uplink to ARM (untagged) traffic
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    memset(&tparams, 0, sizeof(tparams));
    if_index = ETH_IFINDEX_TO_UPLINK_IFINDEX(pinned_if_idx_);
    intf = (if_impl *)if_db()->find(&if_index)->impl();
    key.capri_intrinsic_lif = intf->hw_id();
    mask.capri_intrinsic_lif_mask = ~0;
    key.control_metadata_tunneled_packet = 0;
    mask.control_metadata_tunneled_packet_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_redirect_action.nexthop_id = nh_idx_; //nh_idx_ + 1;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for uplink %u to inb "
                      "lif %u, err %u", pinned_if_idx_, key_, ret);
        goto error;
    }
    PDS_TRACE_DEBUG("nacl lif %u -> nh type %u, idx %u, nh lif %u, port %u",
                    key.capri_intrinsic_lif, NEXTHOP_TYPE_NEXTHOP,
                    nh_idx_, nh_data.nexthop_info.lif,
                    //nh_idx_ + 1, nh_data.nexthop_info.lif,
                    nh_data.nexthop_info.port);

    // allocate vnic h/w id for this lif
    if ((ret = vnic_impl_db()->vnic_idxr()->alloc(&idx)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate vnic h/w id for lif %u, err %u",
                      key_, ret);
        return ret;
    }
    // program the LIF table
    ret = program_lif_table_(key_, idx);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    return SDK_RET_OK;

error:

    nexthop_impl_db()->nh_idxr()->free(nh_idx_, 2);
    nh_idx_ = 0xFFFFFFFF;
    return ret;
}

#define nacl_flow_miss_action    action_u.nacl_nacl_flow_miss
sdk_ret_t
lif_impl::create_datapath_mnic_(pds_lif_spec_t *spec) {
    uint32_t idx;
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    nacl_swkey_t key = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    nacl_actiondata_t data =  { 0 };
    nexthop_actiondata_t nh_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Creating s/w datapath lif %u", key_);
    // allocate required nexthop to point to ARM datapath lif
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entry for flow miss, "
                      "lif %u, err %u", key_, ret);
        return ret;
    }

    // program the nexthop
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.lif = key_;
    nh_data.nexthop_info.port = TM_PORT_DMA;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for datapath lif %u "
                      "at idx %u", key_, nh_idx_);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // flow miss packet coming back from txdma to s/w datapath
    // lif (i.e., dpdk/vpp lif)
    key.control_metadata_flow_miss = 1;
    mask.control_metadata_flow_miss_mask = ~0;
    data.action_id = NACL_NACL_FLOW_MISS_ID;
    data.nacl_flow_miss_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_flow_miss_action.nexthop_id = nh_idx_;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NACL entry for redirect to arm, "
                      "lif %u, err %u", key_, ret);
        goto error;
    }

    // allocate vnic h/w id for this lif
    if ((ret = vnic_impl_db()->vnic_idxr()->alloc(&idx)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate vnic h/w id for lif %u, err %u",
                      key_, ret);
        return ret;
    }
    // program the LIF table
    ret = program_lif_table_(key_, idx);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    return SDK_RET_OK;

error:

    nexthop_impl_db()->nh_idxr()->free(nh_idx_);
    nh_idx_ = 0xFFFFFFFF;
    return ret;
}

typedef struct lif_internal_mgmt_ctx_s {
    lif_impl **lif;
    lif_type_t type;
} __PACK__ lif_internal_mgmt_ctx_t;

static bool
lif_internal_mgmt_cb_ (void *api_obj, void *ctxt) {
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
    p4pd_error_t p4pd_ret;
    nacl_swkey_t key = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    nacl_actiondata_t data =  { 0 };
    nexthop_actiondata_t nh_data = { 0 };
    lif_internal_mgmt_ctx_t cb_ctx = { 0 };
    sdk_table_api_params_t  tparams = { 0 };
    lif_impl *host_mgmt_lif = NULL, *int_mgmt_lif = NULL;

    if (spec->type == sdk::platform::LIF_TYPE_HOST_MGMT) {
        PDS_TRACE_DEBUG("Creating host mgmt. lif %u", key_);
        host_mgmt_lif = this;
        int_mgmt_lif =
            lif_impl_db()->find(sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT);
    } else if (spec->type == sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT) {
        PDS_TRACE_DEBUG("Creating internal mgmt. lif %u", key_);
        int_mgmt_lif = this;
        host_mgmt_lif = lif_impl_db()->find(sdk::platform::LIF_TYPE_HOST_MGMT);
    }
    if (!host_mgmt_lif || !int_mgmt_lif) {
        // we will program when both lifs are available
        return SDK_RET_OK;
    }

    PDS_TRACE_DEBUG("Programming NACLs for internal management");
    // TOOD: fix this once block indexer starts working
    // allocate required nexthops
    //ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_, 2);
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entries for internal mgmt. "
                      "lifs %u, %u, err %u", host_mgmt_lif->key(),
                      int_mgmt_lif->key(), ret);
        return ret;
    }

    // program the nexthop for host mgmt. lif to internal mgmt. lif traffic
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.lif = int_mgmt_lif->key();
    nh_data.nexthop_info.port = TM_PORT_DMA;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for host mgmt. lif %u "
                      "to internal mgmt. lif %u traffic at idx %u",
                      host_mgmt_lif->key(), int_mgmt_lif->key(), nh_idx_);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // program NACL for host mgmt lif to internal mgmt lif traffic
    key.capri_intrinsic_lif = host_mgmt_lif->key();
    mask.capri_intrinsic_lif_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_redirect_action.nexthop_id = nh_idx_;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to install NACL entry for host mgmt lif %u to "
                      "internal mgmt lif %u traffic, err %u",
                      host_mgmt_lif->key(), int_mgmt_lif->key(), ret);
        goto error;
    }

    // TOOD: fix this once block indexer starts working
    // allocate required nexthops
    ret = nexthop_impl_db()->nh_idxr()->alloc(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate nexthop entries for internal mgmt. "
                      "lifs %u, %u, err %u", host_mgmt_lif->key(),
                      int_mgmt_lif->key(), ret);
        return ret;
    }

    // program the nexthop for internal mgmt. lif to host mgmt. lif traffic
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.nexthop_info.lif = host_mgmt_lif->key();
    nh_data.nexthop_info.port = TM_PORT_DMA;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, nh_idx_, //nh_idx_ + 1,
                                       NULL, NULL, &nh_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table for internal mgmt. "
                      "lif %u to host mgmt. lif %u traffic at idx %u",
                      int_mgmt_lif->key(), host_mgmt_lif->key(), nh_idx_); //nh_idx_ + 1);
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // program NACL for internal mgmt lif to host mgmt lif traffic
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    memset(&tparams, 0, sizeof(tparams));
    key.capri_intrinsic_lif = int_mgmt_lif->key();
    mask.capri_intrinsic_lif_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.nacl_redirect_action.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.nacl_redirect_action.nexthop_id = nh_idx_; //nh_idx_ + 1;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &mask, &data,
                                   NACL_NACL_REDIRECT_ID,
                                   sdk::table::handle_t::null());
    ret = apulu_impl_db()->nacl_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to install NACL entry for internal mgmt "
                      "lif %u to host mgmt lif %u traffic, err %u",
                      int_mgmt_lif->key(), host_mgmt_lif->key(), ret);
        goto error;
    }

    // allocate vnic h/w ids for these lifs
    if ((ret = vnic_impl_db()->vnic_idxr()->alloc(&idx, 2)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate vnic h/w id for lif %u, err %u",
                      key_, ret);
        return ret;
    }
    // program the LIF table entry for host mgmt lif
    ret = program_lif_table_(host_mgmt_lif->key(), idx);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // program the LIF table entry for host mgmt lif
    ret = program_lif_table_(int_mgmt_lif->key(), idx + 1);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    return SDK_RET_OK;

error:

    nexthop_impl_db()->nh_idxr()->free(nh_idx_, 2);
    vnic_impl_db()->vnic_idxr()->free(idx, 2);
    nh_idx_ = 0xFFFFFFFF;
    return ret;
}

sdk_ret_t
lif_impl::create_host_lif_(pds_lif_spec_t *spec) {
    uint32_t idx;
    sdk_ret_t ret;
    lif_actiondata_t lif_data = { 0 };

    // allocate vnic h/w id for this lif
    if ((ret = vnic_impl_db()->vnic_idxr()->alloc(&idx)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate vnic h/w id for lif %u, err %u",
                      key_, ret);
        return ret;
    }
    vnic_hw_id_ = idx;
    // program the LIF table
    ret = program_lif_table_(key_, vnic_hw_id_);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }
    return SDK_RET_OK;

error:

    vnic_impl_db()->vnic_idxr()->free(vnic_hw_id_);
    vnic_hw_id_ = 0xFFFF;
    return ret;
}

sdk_ret_t
lif_impl::create(pds_lif_spec_t *spec) {
    sdk_ret_t ret;

    switch (spec->type) {
    case sdk::platform::LIF_TYPE_MNIC_OOB_MGMT:
        ret = create_oob_mnic_(spec);
        break;
    case sdk::platform::LIF_TYPE_MNIC_INBAND_MGMT:
        ret = create_inb_mnic_(spec);
        break;
    case sdk::platform::LIF_TYPE_MNIC_CPU:
        ret = create_datapath_mnic_(spec);
        ret = SDK_RET_OK;
        break;
    case sdk::platform::LIF_TYPE_HOST_MGMT:
    case sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT:
        ret = create_internal_mgmt_mnic_(spec);
        ret = SDK_RET_OK;
        break;
    case sdk::platform::LIF_TYPE_HOST:
        ret = create_host_lif_(spec);
        break;
    default:
        return SDK_RET_INVALID_ARG;
    }
    return ret;
}

/// \@}

}    // namespace impl
}    // namespace api
