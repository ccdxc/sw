//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline global state implementation
///
//----------------------------------------------------------------------------

#ifndef __ARTEMIS_IMPL_STATE_HPP__
#define __ARTEMIS_IMPL_STATEHPP__

#include "nic/sdk/lib/table/tcam/tcam.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/sdk/lib/table/tcam/tcam.hpp"
#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ARTEMIS_IMPL_STATE - pipeline global state functionality
/// \ingroup PDS_ARTEMIS
/// \@{

// forward declaration
class artemis_impl;

#define MAX_KEY_NATIVE_TBL_ENTRIES          6
#define MAX_KEY_TUNNELED_TBL_ENTRIES        6
#define MAX_KEY_TUNNELED2_TBL_ENTRIES       3

/// \brief    pipeline global state
class artemis_impl_state : public state_base {
public:
    // constructor
    artemis_impl_state(pds_state *state);

    // destructor
    ~artemis_impl_state();

    // accessors
    tcam *key_native_tbl(void) { return key_native_tbl_; };
    tcam *key_tunneled_tbl(void) { return key_tunneled_tbl_; };
    tcam *key_tunneled2_tbl(void) { return key_tunneled2_tbl_; };
    tcam *ingress_drop_stats_tbl(void) { return ingress_drop_stats_tbl_; }
    tcam *egress_drop_stats_tbl(void) { return egress_drop_stats_tbl_; }
    tcam *nacl_tbl(void) { return nacl_tbl_; }
    directmap *nat_tbl(void) { return nat_tbl_; }

    /// \brief    API to initiate transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief    API to end transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    friend class artemis_impl;   // artemisa_impl is friend of artemis_impl_state

private:
    // any tables that are shared across objects are instantiated here
    tcam *key_native_tbl_;          // key table for native packets
    tcam *key_tunneled_tbl_;        // key table for tunneled packets
    tcam *key_tunneled2_tbl_;       // key table for double tunneled packets
    tcam *ingress_drop_stats_tbl_;  // ingress drop stats table
    tcam *egress_drop_stats_tbl_;   // egress drop stats table
    tcam *nacl_tbl_;                // NACL tcam table
    directmap *nat_tbl_;            // NAT table
    uint32_t key_native_tbl_idx_[MAX_KEY_NATIVE_TBL_ENTRIES];
    uint32_t key_tunneled_tbl_idx_[MAX_KEY_TUNNELED_TBL_ENTRIES];
    uint32_t key_tunneled2_tbl_idx_[MAX_KEY_TUNNELED_TBL_ENTRIES];
};

/// \@}    // end of PDS_ARTEMIS_IMPL_STATE

}    // namespace impl
}    // namespace api

#endif    // __ARTEMIS_IMPL_STATE_HPP__
