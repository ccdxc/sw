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
#define __TEP_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

// default drop/blackhole nexthop (dataplane drops pkts when this nh is hit)
#define PDS_SYSTEM_DROP_NEXTHOP_HW_ID        0

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - tep state functionality
/// \ingroup PDS_TEP
/// \@{

 /**< forward declaration */
class tep_impl;
class mapping_impl;

/// \brief    state maintained for teps
class tep_impl_state : public obj_base {
public:
    // constructor
    tep_impl_state(pds_state *state);

    // destructor
    ~tep_impl_state();

    /**
     * @brief    API to initiate transaction over all the table manamgement
     *           library instances
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t table_transaction_begin(void);

    /**
     * @brief    API to end transaction over all the table manamgement
     *           library instances
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t table_transaction_end(void);

private:
    directmap *tep_tx_tbl(void) { return tep_tx_tbl_; }
    directmap *nh_tx_tbl(void) { return nh_tx_tbl_; }
    friend class tep_impl;       // tep_impl class is friend of tep_impl_state
    friend class mapping_impl;   // mapping_impl class is friend of tep_impl_state

private:
    directmap    *tep_tx_tbl_;  // directmap table for TEP_TX
    directmap    *nh_tx_tbl_;   // directmap table for NH_TX
};

///   \@}    // end of PDS_TEP_IMPL_STATE

}    // namespace impl
}    // namespace api

#endif    // __TEP_IMPL_STATE_HPP__
