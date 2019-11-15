//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/if_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_IF_IMPL_STATE - interface database functionality
/// \ingroup PDS_IF
/// \@{

if_impl_state::if_impl_state(pds_state *state) {
    // create indexer for L3 interface h/w ids
    l3if_idxr_ = rte_indexer::factory(16, true, true);
    SDK_ASSERT(l3if_idxr_ != NULL);
    // create indexer for uplinks and internal service lifs
    lif_idxr_ = rte_indexer::factory(64, true, false);
    SDK_ASSERT(lif_idxr_ != NULL);
}

if_impl_state::~if_impl_state() {
    rte_indexer::destroy(l3if_idxr_);
    rte_indexer::destroy(lif_idxr_);
}


/// \@}    // end of PDS_IF_IMPL_STATE

}    // namespace impl
}    // namespace api
