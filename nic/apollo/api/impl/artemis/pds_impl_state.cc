//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// implementation of pds impl state class
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"

namespace api {
namespace impl {

// (singleton) instance of all PDS state in one place
pds_impl_state g_pds_impl_state;

lif_impl_state *
lif_impl_db (void)
{
    return g_pds_impl_state.lif_impl_db();
}

sdk_ret_t
pds_impl_state::init(pds_state *state) {
    artemis_impl_db_ = new artemis_impl_state(state);
    lif_impl_db_ = new lif_impl_state(state);
    vpc_impl_db_ = new vpc_impl_state(state);
    vnic_impl_db_ = new vnic_impl_state(state);
    mapping_impl_db_ = new mapping_impl_state(state);
    route_table_impl_db_ = new route_table_impl_state(state);
    security_policy_impl_db_ = new security_policy_impl_state(state);
    // mirror_impl_db_ = new mirror_impl_state(state);
    meter_impl_db_ = new meter_impl_state(state);
    tag_impl_db_ = new tag_impl_state(state);
    svc_mapping_impl_db_ = new svc_mapping_impl_state(state);
    vpc_peer_impl_db_ = new vpc_peer_impl_state(state);
    nexthop_impl_db_ = new nexthop_impl_state(state);
    tep_impl_db_ = new tep_impl_state(state);
    return SDK_RET_OK;
}

void
pds_impl_state::destroy(pds_impl_state *impl_state) {
    delete impl_state->artemis_impl_db_;
    delete impl_state->lif_impl_db_;
    delete impl_state->vpc_impl_db_;
    delete impl_state->vnic_impl_db_;
    delete impl_state->mapping_impl_db_;
    delete impl_state->route_table_impl_db_;
    delete impl_state->security_policy_impl_db_;
    // delete impl_state->mirror_impl_db_;
    delete impl_state->meter_impl_db_;
    delete impl_state->tag_impl_db_;
    delete impl_state->svc_mapping_impl_db_;
    delete impl_state->vpc_peer_impl_db_;
    delete impl_state->nexthop_impl_db_;
    delete impl_state->tep_impl_db_;
}

pds_impl_state::pds_impl_state() {
    artemis_impl_db_ = NULL;
    lif_impl_db_ = NULL;
    vpc_impl_db_ = NULL;
    vnic_impl_db_ = NULL;
    mapping_impl_db_ = NULL;
    route_table_impl_db_ = NULL;
    security_policy_impl_db_ = NULL;
    // mirror_impl_db_ = NULL;
    meter_impl_db_ = NULL;
    tag_impl_db_ = NULL;
    svc_mapping_impl_db_ = NULL;
    vpc_peer_impl_db_ = NULL;
    nexthop_impl_db_ = NULL;
    tep_impl_db_ = NULL;
}

pds_impl_state::~pds_impl_state() {
}

}    // namespace impl
}    // namespace api
