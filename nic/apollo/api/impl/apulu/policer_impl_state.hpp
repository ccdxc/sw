//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// policer implementation state
///
//----------------------------------------------------------------------------

#ifndef __POLICER_IMPL_STATE_HPP__
#define __POLICER_IMPL_STATEHPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_POLICER_IMPL_STATE    policer state functionality
/// \ingroup PDS_POLICER
/// @{

// forward declaration
class policer_impl;

/// \brief  state maintained for policers
class policer_impl_state : public state_base {
public:
    /// \brief  constructor
    policer_impl_state(pds_state *state);

    /// \brief  destructor
    ~policer_impl_state();

    /// \brief  allocate memory required for a policer impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    policer_impl *alloc(void);

    /// \brief     free policer impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(policer_impl *impl);

    /// \brief  return the Tx policer table indexer
    /// \return pointer to the Tx policer table indexer
    rte_indexer *tx_idxr(void) { return tx_idxr_; }

    /// \brief  return the Rx policer table indexer
    /// \return pointer to the Rx policer table indexer
    rte_indexer *rx_idxr(void) { return rx_idxr_; }

private:
    friend class policer_impl;   // policer_impl is friend of policer_impl_state

private:
    // indexer for Tx policers
    rte_indexer *tx_idxr_;
    // indexer for Tx policers
    rte_indexer *rx_idxr_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __POLICER_IMPL_STATE_HPP__
