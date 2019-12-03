//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface implementation state
///
//----------------------------------------------------------------------------

#ifndef __IF_IMPL_STATE_HPP__
#define __IF_IMPL_STATEHPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/sdk/lib/table/slhash/slhash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_IF_IMPL_STATE interface state functionality
/// \ingroup PDS_IF
/// @{

// forward declaration
class if_impl;

/// \brief  state maintained for interfaces
class if_impl_state : public state_base {
public:
    /// \brief  constructor
    if_impl_state(pds_state *state);

    /// \brief  destructor
    ~if_impl_state();

    /// \brief  allocate memory required for an interface impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    if_impl *alloc(void);

    /// \brief     free interface impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(if_impl *impl);

private:
    rte_indexer *l3if_idxr(void) { return l3if_idxr_; }
    rte_indexer *lif_idxr(void) { return lif_idxr_; }
    friend class if_impl;      // if_impl class is friend of if_impl_state

private:
    ///< indexer for L3 interfaces and sub-interfaces
    rte_indexer *l3if_idxr_;
    ///< indexer for uplinks and internal svc lifs
    rte_indexer  *lif_idxr_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __IF_IMPL_STATE_HPP__
