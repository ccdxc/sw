//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP state handling
///
//----------------------------------------------------------------------------

#ifndef __API__TEP_STATE_HPP__
#define __API__TEP_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/tep.hpp"

namespace api {

/// \defgroup PDS_TEP_STATE - TEP state functionality
/// \ingroup PDS_TEP
/// @{

/// \brief state maintained for TEPs
class tep_state : public state_base {
public:
    /// \brief constructor
    tep_state();

    /// \brief destructor
    ~tep_state();

    /// \brief  allocate memory required for a TEP
    /// \return pointer to the allocated tep, NULL if no memory
    tep_entry *alloc(void);

    /// \brief     insert given TEP instance into the TEP db
    /// \param[in] tep TEP entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(tep_entry *tep);

    /// \brief     remove given instance of TEP object from db
    /// \param[in] tep TEP entry to be deleted from the db
    /// \return    pointer to the removed TEP instance or NULL, if not found
    tep_entry *remove(tep_entry *tep);

    /// \brief     free TEP instance back to slab
    /// \param[in] tep pointer to the allocated TEP
    void free(tep_entry *tep);

    /// \brief     lookup a TEP in database given the key
    /// \param[in] tep_key TEP key
    tep_entry *find(pds_tep_key_t *tep_key) const;

    /// \brief API to walk all the db elements
    /// \param[in] walk_cb    callback to be invoked for every node
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id,
                                     void *elem);

private:
    ht *tep_ht(void) const { return tep_ht_; }
    slab *tep_slab(void) const { return tep_slab_; }
    friend class tep_entry;    ///< tep_entry class is friend of tep_state

private:
    ht      *tep_ht_;      ///< Hash table root
    slab    *tep_slab_;    ///< slab for allocating tep entry
};

/// \@}

}  // namespace api

using api::tep_state;

#endif    // __API__TEP_STATE_HPP__
