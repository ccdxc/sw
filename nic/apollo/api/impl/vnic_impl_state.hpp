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
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/core/oci_state.hpp"

namespace impl {

/**
 * @defgroup OCI_VNIC_IMPL_STATE - vnic state functionality
 * @ingroup OCI_VNIC
 * @{
 */

 /**< forward declaration */
class vnic_impl;

/**
 * @brief    state maintained for vnics
 */
class vnic_impl_state : public obj_base {
public:
    /**< @brief    constructor */
    vnic_impl_state(oci_state *state);

    /**< @brief    destructor */
    ~vnic_impl_state();

private:
    indexer *vnic_idxr(void) { return vnic_idxr_; }
    directmap *local_vnic_by_vlan_tx_tbl(void) { return local_vnic_by_vlan_tx_tbl_; }
    sdk_hash *local_vnic_by_slot_rx_tbl(void) { return local_vnic_by_slot_rx_tbl_; }
    directmap *egress_local_vnic_info_rx_tbl(void) { return egress_local_vnic_info_rx_tbl_; }
    friend class vnic_impl;   /**< vnic_impl class is friend of vnic_impl_state */

private:
    /**< P4 datapath tables for vnic */
    indexer      *vnic_idxr_;                    /**< indexer to allocate hw
                                                      vnic id */
     /**
      * NOTE: there is no explicit table mgmt for rx and tx stats, we directly
      *       index using hw_id_ of vnic and and bzero out when we create vnic
      */
    directmap    *local_vnic_by_vlan_tx_tbl_;        /**< directmap table for LOCAL_VNIC_BY_VLAN_TX */
    sdk_hash     *local_vnic_by_slot_rx_tbl_;        /**< hash table for LOCAL_VNIC_BY_SLOT_RX */
    directmap    *egress_local_vnic_info_rx_tbl_;    /**< directmap table for EGRESS_LOCAL_VNIC_INFO_RX */
};

/** * @} */    // end of OCI_VNIC_IMPL_STATE

}    // namespace impl

#endif    /** __VNIC_IMPL_STATE_HPP__ */
