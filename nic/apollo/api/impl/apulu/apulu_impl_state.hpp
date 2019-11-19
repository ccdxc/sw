//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline global state implementation
///
//----------------------------------------------------------------------------

#ifndef __APULU_IMPL_STATE_HPP__
#define __APULU_IMPL_STATEHPP__

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/table/sltcam/sltcam.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"

using sdk::table::handle_t;

namespace api {
namespace impl {

/// \defgroup PDS_APULU_IMPL_STATE - pipeline global state functionality
/// \ingroup PDS_APULU
/// \@{

// forward declaration
class apulu_impl;

/// \brief pipeline global state
class apulu_impl_state : public state_base {
public:
    /// \brief constructor
    apulu_impl_state(pds_state *state);

    /// \brief destructor
    ~apulu_impl_state();

    /// \brief accessors
    sltcam *ingress_drop_stats_tbl(void) { return ingress_drop_stats_tbl_; }
    sltcam *egress_drop_stats_tbl(void) { return egress_drop_stats_tbl_; }
    sltcam *nacl_tbl(void) { return nacl_tbl_; }
    rte_indexer *copp_idxr(void) { return copp_idxr_; }
    rte_indexer *nat_idxr(void) { return nat_idxr_; }

    friend class apulu_impl;            ///< friend of apulu_impl_state

private:
    sltcam *ingress_drop_stats_tbl_;    ///< ingress drop stats table
    sltcam *egress_drop_stats_tbl_;     ///< egress drop stats table
    sltcam *nacl_tbl_;                  ///< NACL tcam table
    rte_indexer *copp_idxr_;            ///< indexer for CoPP table
    rte_indexer *nat_idxr_;             ///< indexer for NAT table
    handle_t ing_drop_stats_tbl_hdls_[P4I_DROP_REASON_MAX + 1];
    handle_t egr_drop_stats_tbl_hdls_[P4E_DROP_REASON_MAX + 1];
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __APULU_IMPL_STATE_HPP__
