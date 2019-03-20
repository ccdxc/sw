//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif implementation state
///
//----------------------------------------------------------------------------

#ifndef __LIF_IMPL_STATE_HPP__
#define __LIF_IMPL_STATE_HPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_LIF_IMPL_STATE - lif state functionality
/// \ingroup PDS_LIF
/// \@{

// forward declarion
class lif_impl;

///< \brief    state maintained for lifs
class lif_impl_state : public obj_base {
public:
    ///< \brief    constructor
    lif_impl_state(pds_state *state);

    ///< \@brief destructor
    ~lif_impl_state();

private:
    directmap *tx_rate_limiter_tbl(void) { return tx_rate_limiter_tbl_; }
    friend class lif_impl;    // lif_impl class is friend of lif_impl_state

private:
    directmap    *tx_rate_limiter_tbl_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __LIF_IMPL_STATE_HPP__
