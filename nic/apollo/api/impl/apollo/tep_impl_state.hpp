//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP implementation state
///
//----------------------------------------------------------------------------

#ifndef __TEP_IMPL_STATE_HPP__
#define __TEP_IMPL_STATE_HPP__

#include "nic/sdk/lib/table/sldirectmap/sldirectmap.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP state functionality
/// \ingroup PDS_TEP
/// \@{

// forward declaration
class tep_impl;
class mapping_impl;

/// \brief state maintained for TEPs
class tep_impl_state : public state_base {
public:
    /// \brief constructor
    tep_impl_state(pds_state *state);

    /// \brief destructor
    ~tep_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief API to end transaction over all the table manamgement
    ///        library instances
    /// return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);
private:
    sldirectmap *tep_tbl(void) { return tep_tbl_; }
    friend class tep_impl;         ///< tep_impl is friend of tep_impl_state
    friend class mapping_impl;     ///< mapping_impl is friend of tep_impl_state
    friend class device_impl;      ///< device_impl is friend of tep_impl_state

private:
    sldirectmap *tep_tbl_;    ///< sldirectmap table for TEP
};

///   \@}

}    // namespace impl
}    // namespace api

#endif    // __TEP_IMPL_STATE_HPP__
