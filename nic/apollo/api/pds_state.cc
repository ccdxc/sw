/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    pds_state.cc
 *
 * @brief   This file contains implementation of pds state class
 */

#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**< (singleton) instance of all PDS state in one place */
pds_state g_pds_state;

/**
 * @defgroup PDS_STATE - Internal state
 * @{
 */

/**< @brief    constructor */
pds_state::pds_state() {
    catalog_ = NULL;
    mpartition_ = NULL;
    memset(state_, 0, sizeof(state_));
    event_cb_ = nullptr;
    memset(&system_mac_, 0, sizeof(system_mac_));
}

/**< @brief    destructor */
pds_state::~pds_state() {
}

sdk_ret_t
pds_state::init(void) {
    state_[PDS_STATE_DEVICE] = new device_state();
    state_[PDS_STATE_LIF] = new lif_state();
    state_[PDS_STATE_IF] = new if_state();
    state_[PDS_STATE_TEP] = new tep_state();
    state_[PDS_STATE_VPC] = new vpc_state();
    state_[PDS_STATE_SUBNET] = new subnet_state();
    state_[PDS_STATE_VNIC] = new vnic_state();
    state_[PDS_STATE_MAPPING] = new mapping_state();
    state_[PDS_STATE_ROUTE_TABLE] = new route_table_state();
    state_[PDS_STATE_POLICY] = new policy_state();
    state_[PDS_STATE_MIRROR] = new mirror_session_state();
    state_[PDS_STATE_METER] = new meter_state();
    state_[PDS_STATE_TAG] = new tag_state();
    state_[PDS_STATE_MAPPING] = new mapping_state();
    state_[PDS_STATE_SVC_MAPPING] = new svc_mapping_state();
    state_[PDS_STATE_VPC_PEER] = new vpc_peer_state();
    state_[PDS_STATE_NEXTHOP] = new nexthop_state();
    state_[PDS_STATE_NEXTHOP_GROUP] = new nexthop_group_state();
    state_[PDS_STATE_POLICER] = new policer_state();
    state_[PDS_STATE_NAT] = new nat_state();
    state_[PDS_STATE_DHCP] = new dhcp_state();
    return SDK_RET_OK;
}

void
pds_state::destroy(pds_state *ps) {
    if (ps->catalog_) {
        catalog::destroy(ps->catalog_);
    }
    if (ps->mpartition_) {
        sdk::platform::utils::mpartition::destroy(ps->mpartition_);
    }
    if (ps->pginfo_) {
        sdk::platform::utils::program_info::destroy(ps->pginfo_);
    }
    for (uint32_t i = PDS_STATE_MIN + 1; i < PDS_STATE_MAX; i++) {
        if (ps->state_[i]) {
            delete ps->state_[i];
        }
    }
}

sdk_ret_t
pds_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    for (uint32_t i = PDS_STATE_MIN + 1; i < PDS_STATE_MAX; i ++) {
        state_[i]->slab_walk(walk_cb, ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_state::state_walk(state_walk_cb_t walk_cb, void *ctxt) {
    state_walk_ctxt_t walk_ctxt;

    for (uint32_t i = PDS_STATE_MIN + 1; i < PDS_STATE_MAX; i ++) {
        if (state_[i]) {
            walk_ctxt.obj_state.assign(PDS_STATE_str(pds_state_t(i)));
            walk_ctxt.state = state_[i];
            walk_cb(&walk_ctxt, ctxt);
        }
    }
    return SDK_RET_OK;
}

/** * @} */    // end of PDS_STATE

}    // namespace api
