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
#include "nic/apollo/api/impl/athena/athena_impl.hpp"
#include "nic/apollo/api/impl/athena/if_impl.hpp"
#include "nic/apollo/api/impl/athena/if_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_IF_IMPL_STATE - interface database functionality
/// \ingroup PDS_IF
/// \@{

if_impl_state::if_impl_state(pds_state *state) {
    // create indexer for uplinks and internal service lifs
    lif_idxr_ = rte_indexer::factory(64, true, false);
    SDK_ASSERT(lif_idxr_ != NULL);
}

if_impl_state::~if_impl_state() {
    rte_indexer::destroy(lif_idxr_);
}

if_impl *
if_impl_state::alloc(void) {
    return (if_impl *)SDK_CALLOC(SDK_MEM_ALLOC_IF_IMPL_IMPL, sizeof(if_impl));
}

void
if_impl_state::free(if_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_IF_IMPL_IMPL, impl);
}

/// \@}    // end of PDS_IF_IMPL_STATE

}    // namespace impl
}    // namespace api
