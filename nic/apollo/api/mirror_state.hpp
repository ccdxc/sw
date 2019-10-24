//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// @file
/// @brief   mirror session database maintenance
///
//----------------------------------------------------------------------------

#ifndef __MIRROR_SESSION_STATE_HPP__
#define __MIRROR_SESSION_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/mirror.hpp"

namespace api {

/// \defgroup PDS_MIRROR_SESSION_STATE - mirror session state functionality
/// \ingroup PDS_MIRROR
/// @{

/// \brief    state maintained for mirror sessions
class mirror_session_state : public state_base {
public:

    /// \brief    constructor
    mirror_session_state();

    /// \brief    destructor
    ~mirror_session_state();

     /// \brief    allocate memory required for a mirror session
     /// \return pointer to the allocated mirror session, NULL if no memory
    mirror_session *alloc(void);

#if 0
    /// \brief    insert given mirror session instance into the db
    /// \param[in] ms    mirror session to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(mirror_session *ms);

    /// \brief     remove given instance of mirror session object from db
    /// \param[in] ms mirror session to be deleted from the db
    /// \return    pointer to the removed mirror session  instance or NULL,
    ///            if not found
    mirror_session *remove(mirror_session *ms);
#endif

    /// \brief      free mirror session  instance back to slab
    /// \param[in]  mirror session pointer to the allocated mirror session
    void free(mirror_session *ms);

#if 0
    /// \brief     lookup a mirror session in database given the key
    /// \param[in] key mirror session key
    mirror_session *find(pds_mirror_session_key_t *key) const;
#endif

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    slab *mirror_session_slab(void) const { return mirror_session_slab_; }
    ///< mirror_session class is friend of mirror_session_state
    friend class mirror_session;

private:
    slab    *mirror_session_slab_;    ///< slab to allocate mirror session entry
};

/// \@}    // end of PDS_MIRROR_SESSION_STATE

}    // end namespace api

using api::mirror_session_state;

#endif    // __MIRROR_STATE_HPP__
