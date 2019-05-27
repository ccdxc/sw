//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc implementation state
///
//----------------------------------------------------------------------------

#ifndef __VPC_IMPL_STATE_HPP__
#define __VPC_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/sltcam/sltcam.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE vpc state functionality
/// \ingroup PDS_VPC
/// @{

// forward declaration
class vpc_impl;

/// \brief  state maintained for VPCs
class vpc_impl_state : public state_base {
public:
    /// \brief  constructor
    vpc_impl_state(pds_state *state);

    /// \brief  destructor
    ~vpc_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

private:
    sltcam *tep1_rx_tbl(void) { return tep1_rx_tbl_; }
    friend class vpc_impl;   // vpc_impl class is friend of vpc_impl_state

private:
    sltcam    *tep1_rx_tbl_;    // tcam table for TEP1_RX
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __VPC_IMPL_STATE_HPP__
