//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// implementation of pds impl state class
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"

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
    apollo_impl_db_ = new apollo_impl_state(state);
    lif_impl_db_ = new lif_impl_state(state);
    tep_impl_db_ = new tep_impl_state(state);
    vnic_impl_db_ = new vnic_impl_state(state);
    mapping_impl_db_ = new mapping_impl_state(state);
    route_table_impl_db_ = new route_table_impl_state(state);
    security_policy_impl_db_ = new security_policy_impl_state(state);
    mirror_impl_db_ = new mirror_impl_state(state);
    nexthop_impl_db_ = new nexthop_impl_state(state);
    nexthop_group_impl_db_ = new nexthop_group_impl_state(state);

    return SDK_RET_OK;
}

void
pds_impl_state::destroy(pds_impl_state *impl_state) {
    delete impl_state->apollo_impl_db_;
    delete impl_state->lif_impl_db_;
    delete impl_state->tep_impl_db_;
    delete impl_state->vnic_impl_db_;
    delete impl_state->mapping_impl_db_;
    delete impl_state->route_table_impl_db_;
    delete impl_state->security_policy_impl_db_;
    delete impl_state->mirror_impl_db_;
    delete impl_state->nexthop_impl_db_;
    delete impl_state->nexthop_group_impl_db_;
}

pds_impl_state::pds_impl_state() {
    apollo_impl_db_ = NULL;
    lif_impl_db_ = NULL;
    tep_impl_db_ = NULL;
    vnic_impl_db_ = NULL;
    mapping_impl_db_ = NULL;
    route_table_impl_db_ = NULL;
    security_policy_impl_db_ = NULL;
    mirror_impl_db_ = NULL;
    nexthop_impl_db_ = NULL;
}

pds_impl_state::~pds_impl_state() {
}

/// \@}

}    // namespace impl
}    // namespace api
