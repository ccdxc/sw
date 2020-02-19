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
#include "nic/sdk/lib/event_thread/event_thread.hpp"
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
    /// \param[in]      key            key info for MAC learnt
    /// \param[in]      vnic_obj_id    vnic object id associated with this MAC
    /// \return         new instance of MAC entry or NULL, in case of error
    static ep_mac_entry *factory(ep_mac_key_t *key, uint32_t vnic_obj_id);

    /// \brief          free memory allocated to MAC entry
    /// \param[in]      ep_mac    pointer to MAC entry
    static void destroy(ep_mac_entry *ep_mac);

    /// \brief          add this entry to EP database
    /// \return         SDK_RET_OK on sucess, failure status code on error
    sdk_ret_t add_to_db(void);

    /// \brief          del this entry from EP database
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

    /// \brief          helper function to get key given EP MAC entry
    /// \param[in]      entry    pointer to EP MAC instance
    /// \return         pointer to the EP MAC instance's key
    static void *ep_mac_key_func_get(void *entry) {
        ep_mac_entry *ep_mac = (ep_mac_entry *)entry;
        return (void *)&(ep_mac->key_);
    }

    /// \brief          link IP entry with this MAC endpoint
    /// \param[in]      IP    IP entry learnt with this mac
    /// \return         SDK_RET_OK on success, error code on failure
    void add_ip(ep_ip_entry *ip);

    /// \brief          unlink IP entry from this MAC endpoint
    /// \param[in]      IP    IP entry to be unlinked from this mac
    /// \return         SDK_RET_OK on success, error code on failure
    void del_ip(ep_ip_entry *ip);

    // safely walk all IP entries associated with this ep
    void walk_ip_list(state_walk_cb_t walk_cb, void *ctxt);

    /// \brief          get vnic object id of this entry
    /// \return         vnic object id
    uint32_t vnic_obj_id(void) const { return vnic_obj_id_; }

    /// \brief          set vnic object id of this entry
    /// \param[in]      idx vnic object id for this entry
    void set_vnic_obj_id(uint32_t idx) { vnic_obj_id_ = idx; }

    /// \brief          get aging timer of this entry
    /// \return         pointer to event timer
    sdk::event_thread::timer_t *timer(void) { return &aging_timer_; }

    /// \brief          get number of associated IP mappings
    /// \return         count of IP addresses
    uint16_t ip_count(void) const { return (uint16_t) ip_list_.size(); }

    /// \brief          get key to this entry
    /// \return         pointer to key
    const ep_mac_key_t *key(void) const { return &key_; }

    /// \brief          return stringified key of the object (for debugging)
    string key2str(void) const {
        return "MAC entry-(" + std::string(key_.subnet.str()) + ", " +
            std::string(macaddr2str(key_.mac_addr)) + ")";
    }

private:
    /// \brief          constructor
    ep_mac_entry();

    /// \brief          destructor
    ~ep_mac_entry();

private:
    ep_mac_key_t                key_;             ///< MAC learning entry key
    uint32_t                    vnic_obj_id_;     ///< key for vnic associated
    ep_state_t                  state_;           ///< state of this entry
    sdk::event_thread::timer_t  aging_timer_;     ///< timer to ageout MAC entry
    ht_ctxt_t                   ht_ctxt_;         ///< hash table context
    ip_entry_list_t             ip_list_;         ///< list of linked IP entries

    friend class ep_mac_state;
};
/// \@}    // end of EP_MAC_ENTRY

}    // namespace learn

using learn::ep_mac_entry;

#endif    // __LEARN_EP_MAC_ENTRY_HPP__
