/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl_state.cc
 *
 * @brief   This file contains mapping datapath database handling
 */
#include "nic/hal/apollo/include/api/oci_mapping.hpp"
#include "nic/hal/apollo/api/impl/oci_impl_state.hpp"
#include "nic/hal/apollo/api/impl/mapping_impl.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/p4/apollo/include/defines.h"

namespace impl {

/**
 * @defgroup OCI_MAPPING_IMPL_STATE - mapping database functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    constructor
 */
mapping_impl_state::mapping_impl_state() {
    p4pd_table_properties_t    tinfo, ctinfo;

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_LOCAL_IP_MAPPING, &tinfo);
    p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
    local_ip_mapping_tbl_ =
        HbmHash::factory(tinfo.tablename, P4TBL_ID_LOCAL_IP_MAPPING,
                         tinfo.oflow_table_id, tinfo.tabledepth,
                         ctinfo.tabledepth, tinfo.key_struct_size,
                         sizeof(local_ip_mapping_data_t),
                         P4_LOCAL_IP_MAPPING_NUM_HINTS_PER_ENTRY,
                         8,    /**< max recircs */
                         static_cast<HbmHash::HashPoly>(tinfo.hash_type),
                         OCI_MEM_ALLOC_LOCAL_IP_MAPPING_TBL, true,
                         NULL);    // TODO: table_health_monitor
    SDK_ASSERT(local_ip_mapping_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_REMOTE_VNIC_MAPPING_RX, &tinfo);
    p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
    remote_vnic_mapping_rx_tbl_ =
        HbmHash::factory(tinfo.tablename, P4TBL_ID_REMOTE_VNIC_MAPPING_RX,
                         tinfo.oflow_table_id, tinfo.tabledepth,
                         ctinfo.tabledepth, tinfo.key_struct_size,
                         sizeof(remote_vnic_mapping_rx_data_t),
                         P4_REMOTE_VNIC_MAPPING_RX_NUM_HINTS_PER_ENTRY,
                         8,    /**< max recircs */
                         static_cast<HbmHash::HashPoly>(tinfo.hash_type),
                         OCI_MEM_ALLOC_REMOTE_VNIC_MAPPING_RX_TBL, true,
                         NULL);    // TODO: table_health_monitor
    SDK_ASSERT(remote_vnic_mapping_rx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_REMOTE_VNIC_MAPPING_TX, &tinfo);
    p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
    remote_vnic_mapping_tx_tbl_ =
        HbmHash::factory(tinfo.tablename, P4TBL_ID_REMOTE_VNIC_MAPPING_TX,
                         tinfo.oflow_table_id, tinfo.tabledepth,
                         ctinfo.tabledepth, tinfo.key_struct_size,
                         sizeof(remote_vnic_mapping_rx_data_t),
                         P4_REMOTE_VNIC_MAPPING_TX_NUM_HINTS_PER_ENTRY,
                         8,    /**< max recircs */
                         static_cast<HbmHash::HashPoly>(tinfo.hash_type),
                         OCI_MEM_ALLOC_REMOTE_VNIC_MAPPING_TX_TBL, true,
                         NULL);    // TODO: table_health_monitor
    SDK_ASSERT(remote_vnic_mapping_tx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_NAT, &tinfo);
    nat_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_NAT,
                                  tinfo.tabledepth,
                                  tinfo.actiondata_struct_size,
                                  false, true, NULL);
    SDK_ASSERT(nat_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
mapping_impl_state::~mapping_impl_state() {
    HbmHash::destroy(local_ip_mapping_tbl_);
    HbmHash::destroy(remote_vnic_mapping_rx_tbl_);
    HbmHash::destroy(remote_vnic_mapping_tx_tbl_);
}

/** @} */    // end of OCI_MAPPING_IMPL_STATE

}    // namespace impl
