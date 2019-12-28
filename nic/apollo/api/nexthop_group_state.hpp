//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group state handling
///
//----------------------------------------------------------------------------

#ifndef __API_NEXTHOP_GROUP_STATE_HPP__
#define __API_NEXTHOP_GROUP_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/nexthop_group.hpp"

namespace api {

/// \defgroup PDS_NEXTHOP_GROUP_STATE - nexthop group state functionality
/// \ingroup PDS_NEXTHOP
/// @{

/// \brief state maintained for nexthop groups
class nexthop_group_state : public state_base {
public:
    /// \brief constructor
    nexthop_group_state();

    /// \brief destructor
    ~nexthop_group_state();

    /// \brief  allocate memory required for a nexthop group instance
    /// \return pointer to the allocated nexthop group, NULL if no memory
    nexthop_group *alloc(void);

    /// \brief     insert given nexthop group instance into the nexthop group db
    /// \param[in] nh_group   nexthop group entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(nexthop_group *nh_group);

    /// \brief     remove the given instance of nexthop group object from db
    /// \param[in] nh_group    nexthop group entry to be deleted from the db
    /// \return    pointer to the removed nexthop group instance or NULL,
    ///            if not found
    nexthop_group *remove(nexthop_group *nh_group);

    /// \brief     free nexthop group instance back to slab
    /// \param[in] nh_group    pointer to the allocated nexthop group
    void free(nexthop_group *nh_group);

    /// \brief     lookup a nexthop group in database given the key
    /// \param[in] key key for the nexthop group object
    /// \return    pointer to the nexthop group instance found or NULL
    nexthop_group *find(pds_nexthop_group_key_t *key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *nh_group_ht(void) const { return nexthop_group_ht_; }
    slab *nh_group_slab(void) const { return nexthop_group_slab_; }
    // nexthop_group class is friend of nexthop_group_state
    friend class nexthop_group;

private:
    ht *nexthop_group_ht_;      ///< nexthop group hash table
    slab *nexthop_group_slab_;  ///< slab for allocating nexthop group entry
};

static inline nexthop_group *
nexthop_group_find (pds_nexthop_group_key_t *key)
{
    return (nexthop_group *)api_base::find_obj(OBJ_ID_NEXTHOP_GROUP, key);
}

/// \@}

}    // namespace api

using api::nexthop_group_state;

#endif    // __API_NEXTHOP_GROUP_STATE_HPP__
