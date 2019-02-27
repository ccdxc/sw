/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    switchport_impl.cc
 *
 * @brief   datapath implementation of switchport
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/switchport.hpp"
#include "nic/apollo/api/impl/switchport_impl.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/utils/utils.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_SWITCHPORT_IMPL - switchport entry datapath implementation
 * @ingroup PDS_SWITCHPORT
 * @{
 */

/**< as there is no state in this impl, single instance is good enough */
switchport_impl    g_switchport_impl;

/**
 * @brief    factory method to allocate & initialize switchport impl instance
 * @param[in] pds_switchport    switchport information
 * @return    new instance of switchport or NULL, in case of error
 */
switchport_impl *
switchport_impl::factory(pds_switchport_spec_t *pds_switchport) {
    return &g_switchport_impl;
}

/**
 * @brief    release all the s/w state associated with the given switchport,
 *           if any, and free the memory
 * @param[in] switchport     switchport to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
switchport_impl::destroy(switchport_impl *impl) {
    return;
}

void
switchport_impl::fill_spec_(pds_switchport_spec_t *spec) {
    uint64_t val;

    //TODO read from HW does NOT work
    sdk::asic::pd::asicpd_read_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                                              &val);
    spec->switch_ip_addr = be64toh(val);
    sdk::asic::pd::asicpd_read_table_constant(P4TBL_ID_TEP_TX, &val);
    val = be64toh(val);
    MAC_UINT64_TO_ADDR(spec->switch_mac_addr, val);
}

void
switchport_impl::fill_idrop_stats_(pds_switchport_idrop_stats_t *idrop_stats) {
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_swkey_t key = {0};
    p4i_drop_stats_swkey_mask_t key_mask = {0};
    p4i_drop_stats_actiondata_t data = {0};

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; ++i) {
        if (p4pd_entry_read(P4TBL_ID_P4I_DROP_STATS, i,
                            &key, &key_mask, &data) == P4PD_SUCCESS) {
            memcpy(&pkts,
                   data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4i_drop_stats_p4i_drop_stats
                              .drop_stats_pkts));
            idrop_stats->drop_stats_pkts[key.control_metadata_p4i_drop_reason] =
                pkts;
        }
    }

    return;
}

sdk_ret_t
switchport_impl::read_hw(pds_switchport_info_t *info) {
    fill_spec_(&info->spec);
    fill_idrop_stats_(&info->stats.idrop_stats);
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    mac_addr_t          tmp;
    switchport_entry    *switchport = (switchport_entry *)api_obj;

    sdk::lib::memrev(tmp, switchport->mac_addr(), ETH_ADDR_LEN);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                                                 switchport->ip_addr());
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP_TX,
                                                 switchport->ip_addr());
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_TEP_TX,
                                                 MAC_TO_UINT64(switchport->mac_addr()));
    PDS_TRACE_DEBUG("Programmed switchport IP %s, MAC %s as table constants",
                    ipv4addr2str(switchport->ip_addr()),
                    macaddr2str(switchport->mac_addr()));
    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, 0);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP_TX, 0);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_TEP_TX, 0);
    return SDK_RET_OK;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] curr_obj    cloned and updated version of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
switchport_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                           obj_ctxt_t *obj_ctxt) {
    switchport_entry *switchport = (switchport_entry *)curr_obj;
    mac_addr_t tmp;
    sdk::lib::memrev(tmp, switchport->mac_addr(), ETH_ADDR_LEN);

    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                                                 switchport->ip_addr());
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP_TX,
                                                 switchport->ip_addr());
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_TEP_TX,
                                                 MAC_TO_UINT64(tmp));
    return SDK_RET_OK;
}

/** @} */    // end of PDS_SWITCHPORT_IMPL

}    // namespace impl
}    // namespace api
