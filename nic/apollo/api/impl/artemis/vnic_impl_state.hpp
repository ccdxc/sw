//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vnic implementation state
///
//----------------------------------------------------------------------------

#ifndef __VNIC_IMPL_STATE_HPP__
#define __VNIC_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/sltcam/sltcam.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL_STATE vnic state functionality
/// \ingroup PDS_VNIC
/// @{

// forward declaration
class vnic_impl;

/// \brief  state maintained for vnics
class vnic_impl_state : public state_base {
public:
    /// \brief  constructor
    vnic_impl_state(pds_state *state);

    /// \brief  destructor
    ~vnic_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

private:
    indexer *vnic_idxr(void) { return vnic_idxr_; }
    sltcam *vnic_mapping_tbl(void) { return vnic_mapping_tbl_; }
    directmap *rxdma_vnic_info_tbl(void) { return rxdma_vnic_info_tbl_; }
    directmap *txdma_vnic_info_tbl(void) { return txdma_vnic_info_tbl_; }
    friend class vnic_impl;   // vnic_impl class is friend of vnic_impl_state

private:
    // NOTE: there is no explicit table mgmt for rx and tx stats, we directly
    //       index using hw_id_ of vnic and and bzero out when we create vnic
    indexer     *vnic_idxr_;              ///< indexer to allocate hw vnic id
    sltcam      *vnic_mapping_tbl_;       ///< tcam table for VNIC_MAPPING
    directmap   *rxdma_vnic_info_tbl_;    ///< directmap table for rxdma VNIC_INFO
    directmap   *txdma_vnic_info_tbl_;    ///< directmap table for txdma VNIC_INFO
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __VNIC_IMPL_STATE_HPP__
