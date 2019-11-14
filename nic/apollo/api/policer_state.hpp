//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// policer state handling
///
//----------------------------------------------------------------------------

#ifndef __API_POLICER_STATE_HPP__
#define __API_POLICER_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/policer.hpp"

namespace api {

/// \defgroup PDS_POLICER_STATE - policer state functionality
/// \ingroup PDS_POLICER
/// @{

/// \brief    state maintained for policers
class policer_state : public state_base {
public:
    /// \brief constructor
    policer_state();

    /// \brief destructor
    ~policer_state();

    /// \brief      allocate memory required for a policer
    /// \return     pointer to the allocated policer, NULL if no memory
    policer *alloc(void);

    /// \brief    insert given policer instance into the policer db
    /// \param[in] pol    policer entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(policer *pol);

    /// \brief     remove the (singleton) instance of device object from db
    /// \param[in] pol    policer entry to be deleted from the db
    /// \return    pointer to the removed policer instance or NULL, if not found
    policer *remove(policer *pol);

    /// \brief      free policer instance back to slab
    /// \param[in]  policer   pointer to the allocated policer
    void free(policer *pol);

    /// \brief      lookup a policer in database given the key
    /// \param[in]  key    policer key
    /// \return     pointer to the policer instance found or NULL
    policer *find(pds_policer_key_t *key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *policer_ht(void) const { return policer_ht_; }
    slab *policer_slab(void) const { return policer_slab_; }
    friend class policer;    ///< policer class is friend of policer_state

private:
    ht *policer_ht_;        ///< hash table root
    slab *policer_slab_;    ///< slab for allocating policer entry
};

/// \@}    // end of PDS_POLICER_ENTRY

}    // namespace api

using api::policer_state;

#endif    // __API_POLICER_STATE_HPP__
