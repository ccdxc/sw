//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP implementation state
///
//----------------------------------------------------------------------------

#ifndef __TEP_IMPL_STATE_HPP__
#define __TEP_IMPL_STATE_HPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP state functionality
/// \ingroup PDS_TEP
/// \@{

// forward declaration
class tep_impl;
class mapping_impl;

/// \brief state maintained for TEPs
class tep_impl_state : public state_base {
public:
    /// \brief constructor
    tep_impl_state(pds_state *state);

    /// \brief destructor
    ~tep_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief API to end transaction over all the table manamgement
    ///        library instances
    /// return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

private:
    rte_indexer *tunnel_idxr(void) { return tunnel_idxr_; }
    rte_indexer *tunnel2_idxr(void) { return tunnel2_idxr_; }
    friend class tep_impl;    ///< tep_impl, friend of tep_impl_state

private:
    ///< indexer to allocate h/w indices for TEPs
    rte_indexer *tunnel_idxr_;     // TUNNEL table indexer
    rte_indexer *tunnel2_idxr_;    // TUNNEL2 table indexer
};

///   \@}

}    // namespace impl
}    // namespace api

#endif    // __TEP_IMPL_STATE_HPP__
