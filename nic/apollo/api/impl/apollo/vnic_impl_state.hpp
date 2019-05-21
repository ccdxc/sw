/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic_impl_state.hpp
 *
 * @brief   vnic implementation state
 */

#if !defined (__VNIC_IMPL_STATE_HPP__)
#define __VNIC_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"
#include "nic/sdk/lib/table/sltcam/sltcam.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/**
 * @defgroup PDS_VNIC_IMPL_STATE - vnic state functionality
 * @ingroup PDS_VNIC
 * @{
 */

 /**< forward declaration */
class vnic_impl;

/**
 * @brief    state maintained for vnics
 */
class vnic_impl_state : public state_base {
public:
    /**< @brief    constructor */
    vnic_impl_state(pds_state *state);

    /**< @brief    destructor */
    ~vnic_impl_state();

    /**
     * @brief    API to initiate transaction over all the table manamgement
     *           library instances
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t table_transaction_begin(void);

    /**
     * @brief    API to end transaction over all the table manamgement
     *           library instances
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t table_transaction_end(void);

private:
    indexer *vnic_idxr(void) { return vnic_idxr_; }
    sltcam *local_vnic_by_vlan_tx_tbl(void) { return local_vnic_by_vlan_tx_tbl_; }
    sdk_hash *local_vnic_by_slot_rx_tbl(void) { return local_vnic_by_slot_rx_tbl_; }
    directmap *egress_local_vnic_info_tbl(void) { return egress_local_vnic_info_tbl_; }
    friend class vnic_impl;   /**< vnic_impl class is friend of vnic_impl_state */
    friend class mapping_impl; /**< mapping_impl class is friend of vnic_impl_state */

private:
    /**< P4 datapath tables for vnic */
    indexer      *vnic_idxr_;                    /**< indexer to allocate hw
                                                      vnic id */
     /**
      * NOTE: there is no explicit table mgmt for rx and tx stats, we directly
      *       index using hw_id_ of vnic and and bzero out when we create vnic
      */
    sltcam       *local_vnic_by_vlan_tx_tbl_;        /**< tcam table for LOCAL_VNIC_BY_VLAN_TX */
    sdk_hash     *local_vnic_by_slot_rx_tbl_;        /**< hash table for LOCAL_VNIC_BY_SLOT_RX */
    directmap    *egress_local_vnic_info_tbl_;       /**< directmap table for EGRESS_LOCAL_VNIC_INFO */
};

/** * @} */    // end of PDS_VNIC_IMPL_STATE

}    // namespace impl
}    // namespace api

#endif    /** __VNIC_IMPL_STATE_HPP__ */
