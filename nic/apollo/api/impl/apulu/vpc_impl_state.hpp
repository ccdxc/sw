//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc implementation state
///
//----------------------------------------------------------------------------

#ifndef __VPC_IMPL_STATE_HPP__
#define __VPC_IMPL_STATEHPP__

#include "nic/sdk/lib/table/slhash/slhash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

using sdk::table::slhash;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE vpc state functionality
/// \ingroup PDS_VPC
/// @{

// forward declaration
class vpc_impl;

/// \brief  state maintained for VPCs
class vpc_impl_state : public state_base {
public:
    /// \brief  constructor
    vpc_impl_state(pds_state *state);

    /// \brief  destructor
    ~vpc_impl_state();

    /// \brief  allocate memory required for a vpc impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    vpc_impl *alloc(void);

    /// \brief     free vpc impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(vpc_impl *impl);

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief     API to get table stats
    /// \param[in]  cb    callback to be called on stats
    ///             ctxt    opaque ctxt passed to the callback
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt);

private:
    slhash *vni_tbl(void) { return vni_tbl_; }
    friend class vpc_impl;      // vpc_impl class is friend of vpc_impl_state
    friend class subnet_impl;   // subnet_impl class is friend of vpc_impl_state

private:
    ///< hash table for vpc VxLAN vnids
    slhash *vni_tbl_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __VPC_IMPL_STATE_HPP__
