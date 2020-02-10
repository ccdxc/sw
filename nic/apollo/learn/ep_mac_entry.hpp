//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// MAC Learning entry handling
///
//----------------------------------------------------------------------------

#ifndef __LEARN_EP_MAC_ENTRY_HPP__
#define __LEARN_EP_MAC_ENTRY_HPP__

#include <list>
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/learn/learn.hpp"

using std::list;
using api::state_walk_cb_t;

namespace learn {

/// \defgroup EP_LEARN - Endpoint MAC Learning Entry functionality
/// @{

class ep_ip_entry;
typedef list<ep_ip_entry *> ip_entry_list_t;

/// \brief    MAC Learning entry
class ep_mac_entry {
public:
    /// \brief          factory method to create an EP MAC entry
    /// \param[in]      key            key info for mac learnt
    /// \param[in]      vnic_obj_id    vnic object id associated with this MAC
    /// \return         new instance of mac entry or NULL, in case of error
    static ep_mac_entry *factory(ep_mac_key_t *key, uint32_t vnic_obj_id);

    /// \brief          free memory allocated to mac entry
    /// \param[in]      ep_mac    pointer to mac entry
    static void destroy(ep_mac_entry *ep_mac);

    /// \brief          add this entry to ep database
    /// \return         SDK_RET_OK on sucess, failure status code on error
    sdk_ret_t add_to_db(void);

    /// \brief          del this entry from ep database
    /// \return         SDK_RET_OK on success, SDK_RET_ENTRY_NOT_FOUND
    ///                 if entry not found in db
    sdk_ret_t del_from_db(void);

    /// \brief          initiate delay deletion of this object
    sdk_ret_t delay_delete(void);

    /// \brief          get the state of this entry
    /// \return         state of this entry
    ep_state_t state(void) const { return state_; }

    /// \brief          set the state of this entry
    /// \param[in]      state    state to be set on this entry
    void set_state(ep_state_t state);

    /// \brief          helper function to get key given ep mac entry
    /// \param[in]      entry    pointer to ep mac instance
    /// \return         pointer to the ep mac instance's key
    static void *ep_mac_key_func_get(void *entry) {
        ep_mac_entry *ep_mac = (ep_mac_entry *)entry;
        return (void *)&(ep_mac->key_);
    }

    /// \brief          link IP entry with this mac endpoint
    /// \param[in]      IP    IP entry learnt with this mac
    /// \return         SDK_RET_OK on success, error code on failure
    void add_ip(ep_ip_entry *ip);

    /// \brief          unlink IP entry from this mac endpoint
    /// \param[in]      IP    IP entry to be unlinked from this mac
    /// \return         SDK_RET_OK on success, error code on failure
    void del_ip(ep_ip_entry *ip);

    // safely walk all IP entries associated with this ep
    void walk_ip_list(state_walk_cb_t walk_cb, void *ctxt);

    /// \brief          get vnic index of this entry
    /// \return         vnic index
    uint32_t vnic(void) const { return vnic_obj_id_; }

    /// \brief          set vnic index of this entry
    /// \param[in]      idx vnic index for this entry
    void set_vnic(uint32_t idx) { vnic_obj_id_ = idx; }

    /// \brief          test if ep is busy with active state transition
    /// \return         true if ep is in transition, false otherwise
    bool active(void) const { return (state_ != EP_STATE_CREATED &&
                                      state_ != EP_STATE_PROBING);
    }

private:
    /// \brief          constructor
    ep_mac_entry();

    /// \brief          destructor
    ~ep_mac_entry();

private:
    ep_mac_key_t    key_;              ///< mac learning entry key
    uint32_t        vnic_obj_id_;      ///< key for vnic associated
    ep_state_t      state_;            ///< state of this entry
    ht_ctxt_t       ht_ctxt_;          ///< hash table context
    ip_entry_list_t ip_list_;          ///< list of linked IP entries

    friend class ep_mac_state;
};
/// \@}    // end of EP_MAC_ENTRY

}    // namespace learn

using learn::ep_mac_entry;

#endif    // __LEARN_EP_MAC_ENTRY_HPP__
