//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// MAC Learning state handling
///
//----------------------------------------------------------------------------

#ifndef __LEARN_EP_MAC_STATE_HPP__
#define __LEARN_EP_MAC_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/learn/ep_mac_entry.hpp"
#include "nic/apollo/learn/learn.hpp"

namespace learn {

using api::state_base;
using api::state_walk_cb_t;

/// \defgroup EP_LEARN - Endpoint MAC Learning state functionality
/// @{

/// \brief    state maintained for MAC Learning
class ep_mac_state : public state_base {
public:
    /// \brief constructor
    ep_mac_state();

    /// \brief destructor
    ~ep_mac_state();

    /// \brief      allocate memory required for a mac  entry
    /// \return     pointer to the allocated mac entry, NULL if no memory
    ep_mac_entry *alloc(void);

    /// \brief      free mac entry back to slab
    /// \param[in]  mac   pointer to the allocated mac entry
    void free(ep_mac_entry *mac);

    /// \brief    insert given mac entry into db
    /// \param[in] mac    mac entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(ep_mac_entry *mac);

    /// \brief     remove the mac entry from db
    /// \param[in] mac    mac entry to be deleted from the db
    /// \return    pointer to the removed mac entry or NULL, if not found
    ep_mac_entry *remove(ep_mac_entry *mac);

    /// \brief      lookup a mac entry in database given the key
    /// \param[in]  key   mac entry key
    /// \return     pointer to the mac entry found or NULL
    const ep_mac_entry *find(ep_mac_key_t *key) const;

    /// \brief API to walk learnt mac entries
    /// \param[in] walk_cb    callback to be invoked for every entry
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief API to walk the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *ep_mac_ht_;        ///< hash table to store mac entries
    slab *ep_mac_slab_;    ///< slab for memory allocation
};

/// \@}    // end of EP_MAC_STATE

}    // namepsace learn

using learn::ep_mac_state;

#endif    // __LEARN_EP_MAC_STATE_HPP__
