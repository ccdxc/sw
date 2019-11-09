//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet implementation state
///
//----------------------------------------------------------------------------

#ifndef __SUBNET_IMPL_STATE_HPP__
#define __SUBNET_IMPL_STATEHPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SUBNET_IMPL_STATE subnet state functionality
/// \ingroup PDS_SUBNET
/// @{

// forward declaration
class subnet_impl;

/// \brief  state maintained for SUBNETs
class subnet_impl_state : public state_base {
public:
    /// \brief  constructor
    subnet_impl_state(pds_state *state);

    /// \brief  destructor
    ~subnet_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief     API to get table stats
    /// \param[in]  cb    callback to be called on stats
    ///             ctxt    opaque ctxt passed to the callback
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt);

private:
    rte_indexer *subnet_idxr(void) const { return subnet_idxr_; }
    // subnet_impl class is friend of subnet_impl_state
    friend class subnet_impl;

private:
    rte_indexer *subnet_idxr_;    ///< indexer to allocate hw subnet id
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __SUBNET_IMPL_STATE_HPP__
