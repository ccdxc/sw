//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// implementation of pds impl state class
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/athena/pds_impl_state.hpp"

namespace api {
namespace impl {

// (singleton) instance of all PDS state in one place
pds_impl_state g_pds_impl_state;

lif_impl_state *
lif_impl_db (void)
{
    return g_pds_impl_state.lif_impl_db();
}

/// \defgroup PDS_IMPL_STATE - internal state
/// \@{

sdk_ret_t
pds_impl_state::init(pds_state *state) {
    athena_impl_db_ = new athena_impl_state(state);
    lif_impl_db_ = new lif_impl_state(state);

    return SDK_RET_OK;
}

void
pds_impl_state::destroy(pds_impl_state *impl_state) {
    delete impl_state->athena_impl_db_;
    delete impl_state->lif_impl_db_;
}

pds_impl_state::pds_impl_state() {
    athena_impl_db_ = NULL;
    lif_impl_db_ = NULL;
}

pds_impl_state::~pds_impl_state() {
}

/// \@}

}    // namespace impl
}    // namespace api
