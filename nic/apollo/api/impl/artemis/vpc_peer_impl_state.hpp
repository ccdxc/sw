//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc peering implementation state
///
//----------------------------------------------------------------------------

#ifndef __VPC_PEER_IMPL_STATE_HPP__
#define __VPC_PEER_IMPL_STATEHPP__

#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VPC_PEER_IMPL_STATE - vpc peering state functionality
/// \ingroup PDS_MAPPING
/// \@{

// forward declaration
class vpc_peer_impl;

///< state maintained for vpc peerings
class vpc_peer_impl_state : public state_base {
public:
    ///< \brief    constructor
    vpc_peer_impl_state(pds_state *state);

    ///< \brief    destructor
    ~vpc_peer_impl_state();

    /// \brief      allocate memory required for a vpc peering impl instance
    /// \return     pointer to the allocated instance, NULL if no memory
    vpc_peer_impl *alloc(void);

    /// \brief      free vpc peering impl instance back to slab
    /// \param[in]  impl    pointer to the allocated impl instance
    void free(vpc_peer_impl *impl);

    /// \brief    API to initiate transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief    API to end transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief     API to get table stats
    /// \param[in]  cb    callback to be called on stats
    ///             ctxt    opaque ctxt passed to the callback
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt);

private:
    mem_hash *vpc_peer_tbl(void) { return vpc_peer_tbl_; }
    slab *vpc_peer_impl_slab(void) { return vpc_peer_impl_slab_; }
    // vpc_peer_impl class is friend of vpc_peer_impl_state
    friend class vpc_peer_impl;

private:
    mem_hash *vpc_peer_tbl_;
    slab *vpc_peer_impl_slab_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __VPC_PEER_IMPL_STATE_HPP__
