//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline global state implementation
///
//----------------------------------------------------------------------------

#ifndef __APOLLO_IMPL_STATE_HPP__
#define __APOLLO_IMPL_STATEHPP__

#include "nic/sdk/lib/table/tcam/tcam.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/sdk/lib/table/tcam/tcam.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_APOLLO_IMPL_STATE - pipeline global state functionality
/// \ingroup PDS_APOLLO
/// \@{

// forward declaration
class apollo_impl;

#define MAX_KEY_NATIVE_TBL_ENTRIES   3
#define MAX_KEY_TUNNELED_TBL_ENTRIES 3

/// \brief    pipeline global state
class apollo_impl_state : public state_base {
public:
    // constructor
    apollo_impl_state(pds_state *state);

    // destructor
    ~apollo_impl_state();

    // accessors
    tcam *key_native_tbl(void) { return key_native_tbl_; };
    tcam *key_tunneled_tbl(void) { return key_tunneled_tbl_; };
    tcam *ingress_drop_stats_tbl(void) { return ingress_drop_stats_tbl_; }
    tcam *egress_drop_stats_tbl(void) { return egress_drop_stats_tbl_; }
    tcam *nacl_tbl(void) { return nacl_tbl_; }

    friend class apollo_impl;   // apollo_impl is friend of apollo_impl_state

private:
    tcam *key_native_tbl_;          // key table for native packets
    tcam *key_tunneled_tbl_;        // key table for tunneled packets
    tcam *ingress_drop_stats_tbl_;  // ingress drop stats table
    tcam *egress_drop_stats_tbl_;   // egress drop stats table
    tcam *nacl_tbl_;                // NACL tcam table
    uint32_t key_native_tbl_idx_[MAX_KEY_NATIVE_TBL_ENTRIES];
    uint32_t key_tunneled_tbl_idx_[MAX_KEY_TUNNELED_TBL_ENTRIES];
};

/// \@}    // end of PDS_APOLLO_IMPL_STATE

}    // namespace impl
}    // namespace api

#endif    // __APOLLO_IMPL_STATE_HPP__
