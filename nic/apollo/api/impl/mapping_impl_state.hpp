//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mappping implementation state
///
//----------------------------------------------------------------------------

#ifndef __MAPPING_IMPL_STATE_HPP__
#define __MAPPING_IMPL_STATEHPP__

#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

// entry reserved for no translation
#define NAT_TX_TBL_RSVD_ENTRY_IDX        0

/// \defgroup PDS_MAPPING_IMPL_STATE - mapping state functionality
/// \ingroup PDS_MAPPING
/// \@{

// forward declaration
class mapping_impl;

///< state maintained for mappings
class mapping_impl_state : public obj_base {
public:
    ///< \brief    constructor
    mapping_impl_state(pds_state *state);

    ///< \brief    destructor
    ~mapping_impl_state();

    /// \brief      allocate memory required for a mapping impl instance
    /// \return     pointer to the allocated instance, NULL if no memory
    mapping_impl *alloc(void);

    /// \brief      free mapping impl instance back to slab
    /// \param[in]  impl    pointer to the allocated impl instance
    void free(mapping_impl *impl);

    /// \brief    API to initiate transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief    API to end transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

private:
    mem_hash *local_ip_mapping_tbl(void) { return local_ip_mapping_tbl_; }
    mem_hash *remote_vnic_mapping_rx_tbl(void) {
        return remote_vnic_mapping_rx_tbl_;
    }
    mem_hash *remote_vnic_mapping_tx_tbl(void) {
        return remote_vnic_mapping_tx_tbl_;
    }
    directmap *nat_tbl(void) { return nat_tbl_; }
    slab *mapping_impl_slab(void) { return mapping_impl_slab_; }
    // mapping_impl class is friend of mapping_impl_state
    friend class mapping_impl;

private:
    mem_hash *local_ip_mapping_tbl_;
    mem_hash *remote_vnic_mapping_rx_tbl_;
    mem_hash *remote_vnic_mapping_tx_tbl_;
    directmap *nat_tbl_;
    slab *mapping_impl_slab_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __MAPPING_IMPL_STATE_HPP__
