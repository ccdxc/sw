//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// VNIC datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL_STATE - VNIC database functionality
/// \ingroup PDS_VNIC
/// @{

vnic_impl_state::vnic_impl_state(pds_state *state) {
    sdk_table_factory_params_t    table_params;

    // allocate indexer for vnic hw id allocation and reserve 0th entry
    vnic_idxr_ = rte_indexer::factory(PDS_MAX_VNIC, false, true);
    SDK_ASSERT(vnic_idxr_ != NULL);

    // create a slab for vnic impl entries
    vnic_impl_slab_ =
        slab::factory("vnic-impl", PDS_SLAB_ID_VNIC_IMPL,
                      sizeof(vnic_impl), 16, true, true);
    SDK_ASSERT(vnic_impl_slab_ != NULL);
}

vnic_impl_state::~vnic_impl_state() {
    rte_indexer::destroy(vnic_idxr_);
    slab::destroy(vnic_impl_slab_);
}

sdk_ret_t
vnic_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(vnic_impl_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
