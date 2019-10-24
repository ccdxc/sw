//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// VPC state handling
///
//----------------------------------------------------------------------------

#ifndef __API_VPC_STATE_HPP__
#define __API_VPC_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/vpc.hpp"

namespace api {

/// \defgroup PDS_VPC_STATE - VPC state functionality
/// \ingroup PDS_VPC
/// @{

/// \brief state maintained for VPCs
class vpc_state : public state_base {
public:
    /// \brief constructor
    vpc_state();

    /// \brief destructor
    ~vpc_state();

    /// \brief  allocate memory required for a VPC instance
    /// \return pointer to the allocated VPC, NULL if no memory
    vpc_entry *alloc(void);

    /// \brief     insert given VPC instance into the VPC db
    /// \param[in] vpc VPC entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(vpc_entry *vpc);

    /// \brief     remove the given instance of VPC object from db
    /// \param[in] vpc VPC entry to be deleted from the db
    /// \return    pointer to the removed VPC instance or NULL, if not found
    vpc_entry *remove(vpc_entry *vpc);

    /// \brief     free VPC instance back to slab
    /// \param[in] vpc pointer to the allocated VPC
    void free(vpc_entry *vpc);

    /// \brief     lookup a vpc in database given the key
    /// \param[in] vpc key for the VPC object
    /// \return    pointer to the VPC instance found or NULL
    vpc_entry *find(pds_vpc_key_t *key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *vpc_ht(void) const { return vpc_ht_; }
    indexer *vpc_idxr(void) const { return vpc_idxr_; }
    slab *vpc_slab(void) const { return vpc_slab_; }
    friend class vpc_entry;    ///< vpc_entry class is friend of vpc_state

private:
    ht      *vpc_ht_;                ///< vpc hash table
    indexer *vpc_idxr_;              ///< indexer to allocate hw vpc id
    slab    *vpc_slab_;              ///< slab for allocating vpc entry
};

/// \@}

}    // namespace api

using api::vpc_state;

#endif    // __API_VPC_STATE_HPP__
