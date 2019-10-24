//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// VPC peer state handling
///
//----------------------------------------------------------------------------

#ifndef __API_VPC_PEER_STATE_HPP__
#define __API_VPC_PEER_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/vpc_peer.hpp"

namespace api {

/// \defgroup PDS_VPC_PEER_STATE - VPC peer state functionality
/// \ingroup PDS_VPC
/// @{

/// \brief state maintained for VPC peering relationships
class vpc_peer_state : public state_base {
public:
    /// \brief constructor
    vpc_peer_state();

    /// \brief destructor
    ~vpc_peer_state();

    /// \brief  allocate memory required for a VPC peer instance
    /// \return pointer to the allocated VPC peer instance, NULL if no memory
    vpc_peer_entry *alloc(void);

    /// \brief     free VPC peer instance back to slab
    /// \param[in] vpc_peer    vpc peer instance pointer to the allocated VPC
    void free(vpc_peer_entry *vpc_peer);

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    slab *vpc_peer_slab(void) const { return vpc_peer_slab_; }

private:
    slab    *vpc_peer_slab_;    ///< slab for allocating vpc peer entry
};

/// \@}

}    // namespace api

using api::vpc_peer_state;

#endif    // __API_VPC_PEER_STATE_HPP__
