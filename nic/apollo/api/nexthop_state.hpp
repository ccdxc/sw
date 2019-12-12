//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop state handling
///
//----------------------------------------------------------------------------

#ifndef __API_NEXTHOP_STATE_HPP__
#define __API_NEXTHOP_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/nexthop.hpp"

namespace api {

/// \defgroup PDS_NEXTHOP_STATE - nexthop state functionality
/// \ingroup PDS_NEXTHOP
/// @{

/// \brief state maintained for nexthops
class nexthop_state : public state_base {
public:
    /// \brief constructor
    nexthop_state();

    /// \brief destructor
    ~nexthop_state();

    /// \brief  allocate memory required for a nexthop instance
    /// \return pointer to the allocated nexthop, NULL if no memory
    nexthop *alloc(void);

    /// \brief     insert given nexthop instance into the nexthop db
    /// \param[in] nh   nexthop entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(nexthop *nh);

    /// \brief     remove the given instance of nexthop object from db
    /// \param[in] nh    nexthop entry to be deleted from the db
    /// \return    pointer to the removed nexthop instance or NULL, if not found
    nexthop *remove(nexthop *nh);

    /// \brief     free nexthop instance back to slab
    /// \param[in] nh    pointer to the allocated nexthop
    void free(nexthop *nh);

    /// \brief     lookup a nexthop in database given the key
    /// \param[in] key key for the nexthop object
    /// \return    pointer to the nexthop instance found or NULL
    nexthop *find(pds_nexthop_key_t *key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *nh_ht(void) const { return nexthop_ht_; }
    slab *nh_slab(void) const { return nexthop_slab_; }
    friend class nexthop;    ///< nexthop class is friend of nexthop_state

private:
    ht      *nexthop_ht_;      ///< nexthop hash table
    slab    *nexthop_slab_;    ///< slab for allocating nexthop entry
};
static inline nexthop *
nexthop_find (void) {
    return (nexthop *)api_base::find_obj(OBJ_ID_NEXTHOP, NULL);
}


/// \@}

}    // namespace api

using api::nexthop_state;

#endif    // __API_NEXTHOP_STATE_HPP__
