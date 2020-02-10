//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// IP Learning state handling
///
//----------------------------------------------------------------------------

#ifndef __LEARN_EP_IP_STATE_HPP__
#define __LEARN_EP_IP_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/learn/ep_ip_entry.hpp"
#include "nic/apollo/learn/learn.hpp"

namespace learn {

using api::state_base;
using api::state_walk_cb_t;

/// \defgroup EP_LEARN - Endpoint IP Learning state functionality
/// @{

/// \brief    state maintained for IP Learning
class ep_ip_state : public state_base {
public:
    /// \brief constructor
    ep_ip_state();

    /// \brief destructor
    ~ep_ip_state();

    /// \brief      allocate memory required for an IP entry
    /// \return     pointer to the allocated IP entry, NULL if no memory
    ep_ip_entry *alloc(void);

    /// \brief      free IP entry back to slab
    /// \param[in]  IP   pointer to the allocated IP entry
    void free(ep_ip_entry *ip);

    /// \brief    insert given IP entry into db
    /// \param[in] IP    IP entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(ep_ip_entry *ip);

    /// \brief     remove the IP entry from db
    /// \param[in] IP    IP entry to be deleted from the db
    /// \return    pointer to the removed IP entry or NULL, if not found
    ep_ip_entry *remove(ep_ip_entry *ip);

    /// \brief      lookup an IP entry in database given the key
    /// \param[in]  key   IP entry key
    /// \return     pointer to the IP entry found or NULL
    const ep_ip_entry *find(ep_ip_key_t *key) const;

    /// \brief API to walk the IP entries
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
    ht *ep_ip_ht_;         ///< hash table to store IP entries
    slab *ep_ip_slab_;     ///< slab for memory allocation
};

/// \@}    // end of EP_IP_STATE

}    // namepsace learn

using learn::ep_ip_state;

#endif    // __LEARN_EP_IP_STATE_HPP__
