/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   This file deals with OCI VNIC API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vnic.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {

/**
 * @defgroup OCI_VNIC_DB - VNIC database functionality
 * @ingroup OCI_VNIC
 * @{
 */

// TODO: these can't be global variables -- have to move to some global class
vnic_state g_vnic_state;

/**
 * constructor
 */
vnic_state::vnic_state() {
    p4pd_table_properties_t    tinfo, oflow_tinfo;

    // TODO: need to tune multi-threading related params later
    ht_ = ht::factory(OCI_MAX_VNIC >> 2,
                      vnic_key_func_get,
                      vnic_hash_func_compute,
                      vnic_key_func_compare);
    idxr_ = indexer::factory(OCI_MAX_VNIC);
    slab_ = slab::factory("vnic", OCI_SLAB_VNIC, sizeof(vnic_t), 16,
                          true, true, true, NULL);

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX, &tinfo);
    local_vnic_by_vlan_tx_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_vlan_tx_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, &tinfo);
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                              &oflow_tinfo);
    local_vnic_by_slot_rx_ =
        sdk_hash::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                          P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                          tinfo.tabledepth,
                          oflow_tinfo.tabledepth,
                          tinfo.key_struct_size,
                          tinfo.actiondata_struct_size,
                          static_cast<sdk_hash::HashPoly>(tinfo.hash_type),
                          true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_slot_rx_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_EGRESS_LOCAL_VNIC_INFO_RX, &tinfo);
    egress_local_vnic_info_rx_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_EGRESS_LOCAL_VNIC_INFO_RX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(egress_local_vnic_info_rx_ != NULL);
}

/**
 * destructor
 */
vnic_state::~vnic_state() {
    ht::destroy(ht_);
    slab::destroy(slab_);
}

/**
 * @brief Handle VNIC create message
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_state::vnic_create(_In_ oci_vnic_t *oci_vnic) {
    vnic_t       *vnic;
    sdk_ret_t    rv;

    if ((vnic = vnic_alloc_init(oci_vnic)) == NULL) {
        return sdk::SDK_RET_OOM;
    }

    // TODO: program_datapath() must be called during batch_commit(), not here
    rv = program_datapath(oci_vnic, vnic);
    return rv;
}

/**
 * @brief Handle VNIC delete API
 *
 * @param[in] vnic_key VNIC key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_state::vnic_delete(_In_ oci_vnic_key_t *vnic_key) {
    vnic_t *vnic;

    if ((vnic = vnic_del_from_db(vnic_key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    // TODO: clear all mappings and delete all p4pd table entires etc.
    vnic_delete(vnic);

    return sdk::SDK_RET_OK;
}

/**
 * @brief Add VNIC to database
 *
 * @param[in] vnic VNIC
 */
sdk_ret_t
vnic_state::vnic_add_to_db(vnic_t *vnic) {
    return ht_->insert_with_key(&vnic->key, vnic,
                                &vnic->ht_ctxt);
}

/**
 * @brief Delete VNIC from database
 *
 * @param[in] vnic_key VNIC key
 */
vnic_t *
vnic_state::vnic_del_from_db(oci_vnic_key_t *vnic_key) {
    return (vnic_t *)(ht_->remove(vnic_key));
}

/**
 * @brief Lookup VNIC in database
 *
 * @param[in] vnic_key VNIC key
 */
vnic_t *
vnic_state::vnic_find(oci_vnic_key_t *vnic_key) const {
    return (vnic_t *)(ht_->lookup(vnic_key));
}

/**
 * @brief Allocate VNIC structure
 *
 * @return Pointer to the allocated internal VNIC, NULL if no memory
 */
vnic_t *
vnic_state::vnic_alloc(void) {
    return ((vnic_t *)slab_->alloc());
}

/**
 * @brief Initialize internal VNIC structure
 *
 * @param[in] vnic VNIC structure to store the state
 * @param[in] oci_vnic VNIC specific information
 */
sdk_ret_t
vnic_state::vnic_init(vnic_t *vnic, oci_vnic_t *oci_vnic) {
    //SDK_SPINLOCK_INIT(&vnic->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&vnic->key, &oci_vnic->key, sizeof(oci_vnic_key_t));
    vnic->ht_ctxt.reset();
    // allocate hw id for this vnic, vnic specific index tables in the p4
    // datapath are indexed by this
    if (idxr_->alloc((uint32_t *)&vnic->hw_id) != sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief Allocate and initialize internal VNIC structure
 *
 * @return Pointer to the allocated and initialized internal VNIC,
 *         NULL if no memory
 */
vnic_t *
vnic_state::vnic_alloc_init(oci_vnic_t *oci_vnic) {
    vnic_t *vnic;

    if ((vnic = vnic_alloc()) == NULL) {
        return NULL;
    }
    vnic_init(vnic, oci_vnic);
    return vnic;
}

/**
 * @brief Cleanup state maintained for given VNIC
 *
 * @param[in] vnic VNIC
 */
void
vnic_state::vnic_cleanup(vnic_t *vnic) {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&vnic->slock);
    idxr_->free(vnic->hw_id);
}

/**
 * @brief Free VNIC structure
 *
 * @param[in] vnic VNIC
 */
void
vnic_state::vnic_free(vnic_t *vnic) {
    api::delay_delete_to_slab(OCI_SLAB_VNIC, vnic);
}

/**
 * @brief Uninitialize and free internal VNIC structure
 *
 * @param[in] vnic VNIC
 */
void
vnic_state::vnic_delete(_In_ vnic_t *vnic) {
    if (vnic) {
        vnic_cleanup(vnic);
        vnic_free(vnic);
    }
}

/**
 * @brief program the P4/P4+ datapath for given vnic
 *
 * @param[in] oci_vnic VNIC configuration
 * @param[in] vnic     VNIC information to be stored in the db
 * @return #SDK_RET_OK on success, failure status code on error
 */
// TODO: these should be generated by NCC
#define local_vnic_by_vlan_tx_info local_vnic_by_vlan_tx_action_u.local_vnic_by_vlan_tx_local_vnic_info_tx
#define local_vnic_by_slot_rx_info local_vnic_by_slot_rx_action_u.local_vnic_by_slot_rx_local_vnic_info_rx
#define egress_local_vnic_info_rx egress_local_vnic_info_rx_action_u.egress_local_vnic_info_rx_egress_local_vnic_info_rx
sdk_ret_t
vnic_state::program_datapath(In_ oci_vnic_t *oci_vnic, In_ vnic_t *vnic) {
    sdk_ret_t                                   rv;
    subnet_t                                    *subnet;
    vcn_t                                       *vcn;
    local_vnic_by_vlan_tx_actiondata            vnic_by_vlan_data = { 0 };
    local_vnic_by_slot_rx_swkey_t               vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_otcam_swkey_mask_t    vnic_by_slot_key_mask = { 0 };
    local_vnic_by_slot_rx_actiondata            vnic_by_slot_data = { 0 };
    egress_local_vnic_info_rx_actiondata        egress_vnic_data = { 0 };
    vnic_rx_stats_actiondata                    vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata                    vnic_tx_stats_data = { 0 };

    /**< find subnet state for this VNIC */
    subnet = g_subnet_state.subnet_find(oci_vnic->subnet_id);
    if (subnet == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< find VCN state for this VNIC */
    vcn = g_vcn_db.vcn_find(oic_vnic->vcn_id);
    if (vcn == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    // TODO: not doing RMW operation yet (will do that when we come to update,
    //       right now table lib APIs have an issue with this)
    //
    /**< initialize rx & tx stats tables for this vnic */
    vnic_rx_stats_data.actionid = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    pd_err = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                     vnic->hw_id, NULL, NULL,
                                     &vnic_rx_stats_data);
    if (pd_err != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    vnic_tx_stats_data.actionid = VNIC_TX_STATS_VNIC_TX_STATS_ID;
    pd_err = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                     vnic->hw_id, NULL, NULL,
                                     &vnic_tx_stats_data);
    if (pd_err != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    /**< initialize mapping table for this vnic */

    /**< initialize NAT table for this vnic */

    /**< initialize egress_local_vnic_info_rx table entry */
    egress_vnic_data.actionid =
        EGRESS_LOCAL_VNIC_INFO_RX_EGRESS_LOCAL_VNIC_INFO_RX_ID;
    egress_vnic_data.egress_local_vnic_info_rx.vr_mac = subnet->vr_mac;
    egress_vnic_data.egress_local_vnic_info_rx.overlay_mac = oci_vnic->mac_addr;
    egress_vnic_data.egress_local_vnic_info_rx.overlay_vlan_id =
        oci_vnic->vlan_id;
    egress_vnic_data.egress_local_vnic_info_rx.subnet_id = subnet->hw_id;
    rv = egress_local_vnic_info_rx_->insert_with_id(&egress_vnic_data,
                                                    vnic->hw_id);
    SDK_ASSERT_GOTO((rv == SDK_RET_OK), error);

    /**< initialize local_vnic_by_slot_rx table entry */
    vnic_by_slot_key.mpls_dst_label = oci_vnic->slot_id;
    vnic_by_slot_data.actionid = LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag =
        oci_vnic->hw_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.vcn_id = vcn->hw_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check =
        oci_vnic->skip_src_dst_check;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group_1 =
        oci_vnic->rsc_pool_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.slacl_addr_1 =
        subnet->policy_base_addr;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = 0;  // TODO: later !!
    rv = local_vnic_by_slot_rx_->insert(&key, &vnic_by_slot_data,
                                        &vnic->vnic_by_slot_hash_idx,
                                        &key_mask);  // TODO: fill in key_mask
    SDK_ASSERT_GOTO((rv == SDK_RET_OK), error);

    /**< initialize local_vnic_by_vlan_tx table entry */
    vnic_by_vlan_data.actionid = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.local_vnic_tag =
        oci_vnic->hw_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vcn_id = vcn->hw_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check =
        oci_vnic->skip_src_dst_check;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group_1 =
        oci_vnic->rsc_pool_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_addr_1 =
        subnet->lpm_base_addr;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.slacl_addr_1 =
        subnet->policy_base_addr;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = 0;  // TODO: later !!
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.overlay_mac =
        oci_vnic->mac_addr;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.src_slot_id = oci_vnic->slot;
    rv = local_vnic_by_vlan_tx_->insert_with_id(&vnic_by_vlan_data,
                                                oci_vnic->vlan_id);
    SDK_ASSERT_GOTO((rv == SDK_RET_OK), error);

error:

    cleanup_datapath(vcn);
    return rv;
}

/** @} */ // end of OCI_VNIC_DB

/**
 * @brief Create VNIC
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_create (_In_ oci_vnic_t *vnic)
{
    sdk_ret_t rv;

    rv = g_vnic_state.vnic_create(vnic);
    return rv;
}

/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_key VNIC key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_delete (_In_ oci_vnic_key_t *vnic_key)
{
    sdk_ret_t rv;

    rv = g_vnic_state.vnic_delete(vnic_key);
    return rv;
}

/** @} */ // end of OCI_VNIC_API

}    // namespace api
