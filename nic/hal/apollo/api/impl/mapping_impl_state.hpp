/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl_state.hpp
 *
 * @brief   MAPPING implementation state
 */
#if !defined (__MAPPING_IMPL_STATE_HPP__)
#define __MAPPING_IMPL_STATEHPP__

#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"

namespace impl {

/**
 * @defgroup OCI_MAPPING_IMPL_STATE - mapping state functionality
 * @ingroup OCI_MAPPING
 * @{
 */

 /**< forward declaration */
class mapping_impl;

/**
 * @brief    state maintained for mappings
 */
class mapping_impl_state : public obj_base {
public:
    /**< @brief    constructor */
    mapping_impl_state(oci_state *state);

    /**< @brief    destructor */
    ~mapping_impl_state();

private:
    mem_hash *local_ip_mapping_tbl(void) { return local_ip_mapping_tbl_; }
    mem_hash *remote_vnic_mapping_rx_tbl(void) { return remote_vnic_mapping_rx_tbl_; }
    mem_hash *remote_vnic_mapping_tx_tbl(void) { return remote_vnic_mapping_tx_tbl_; }
    directmap *nat_tbl(void) { return nat_tbl_; }
    /**< mapping_impl class is friend of mapping_impl_state */
    friend class mapping_impl;

private:
    mem_hash *local_ip_mapping_tbl_;
    mem_hash *remote_vnic_mapping_rx_tbl_;
    mem_hash *remote_vnic_mapping_tx_tbl_;
    directmap *nat_tbl_;
};

/** * @} */    // end of OCI_MAPPING_IMPL_STATE

}    // namespace impl

#endif    /** __MAPPING_IMPL_STATE_HPP__ */
