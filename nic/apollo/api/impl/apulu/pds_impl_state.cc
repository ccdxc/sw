//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// implementation of pds impl state class
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

// (singleton) instance of all PDS state in one place
pds_impl_state g_pds_impl_state;

/// \defgroup PDS_IMPL_STATE - internal state
/// \@{

pds_impl_state::pds_impl_state() {
    memset(impl_state_, 0, sizeof(impl_state_));
}

pds_impl_state::~pds_impl_state() {
}

sdk_ret_t
pds_impl_state::init(pds_state *state) {
    impl_state_[PDS_IMPL_STATE_APULU] = new apulu_impl_state(state);
    impl_state_[PDS_IMPL_STATE_LIF] = new lif_impl_state(state);
    impl_state_[PDS_IMPL_STATE_IF] = new if_impl_state(state);
    impl_state_[PDS_IMPL_STATE_TEP] = new tep_impl_state(state);
    impl_state_[PDS_IMPL_STATE_VPC] = new vpc_impl_state(state);
    impl_state_[PDS_IMPL_STATE_VNIC] = new vnic_impl_state(state);
    impl_state_[PDS_IMPL_STATE_MAPPING] = new mapping_impl_state(state);
    impl_state_[PDS_IMPL_STATE_ROUTE_TABLE] = new route_table_impl_state(state);
    impl_state_[PDS_IMPL_STATE_SECURITY_POLICY] =
        new security_policy_impl_state(state);
    impl_state_[PDS_IMPL_STATE_MIRROR] = new mirror_impl_state(state);
    impl_state_[PDS_IMPL_STATE_NEXTHOP] = new nexthop_impl_state(state);
    impl_state_[PDS_IMPL_STATE_NEXTHOP_GROUP] =
        new nexthop_group_impl_state(state);
    return SDK_RET_OK;
}

void
pds_impl_state::destroy(pds_impl_state *impl_state) {
    for (uint32_t i = PDS_IMPL_STATE_MIN; i < PDS_IMPL_STATE_MAX; i++) {
        if (impl_state->impl_state_[i]) {
            delete impl_state->impl_state_[i];
        }
    }
}

/// \@}

}    // namespace impl
}    // namespace api
