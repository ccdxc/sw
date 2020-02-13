//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// IP Learning entry handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/learn/ep_aging.hpp"
#include "nic/apollo/learn/ep_ip_entry.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"

namespace learn {

#define ep_mac_db learn_db()->ep_mac_db
#define ep_ip_db learn_db()->ep_ip_db

ep_ip_entry::ep_ip_entry() {
    ht_ctxt_.reset();
    this->state_ = EP_STATE_LEARNING;
}

ep_ip_entry::~ep_ip_entry() {
}

ep_ip_entry *
ep_ip_entry::factory(ep_ip_key_t *key, uint32_t vnic_obj_id) {
    ep_ip_entry *ep_ip;

    ep_ip = ep_ip_db()->alloc();
    if (ep_ip) {
        new (ep_ip) ep_ip_entry();
        ep_ip->key_ = *key;
        ep_ip->vnic_obj_id_ = vnic_obj_id;
        learn_ep_aging_timer_init(&ep_ip->aging_timer_, (void *)ep_ip,
                                  PDS_MAPPING_TYPE_L3);
    }
    return ep_ip;
}

void
ep_ip_entry::destroy(ep_ip_entry *ep_ip) {
    ep_ip->~ep_ip_entry();
    ep_ip_db()->free(ep_ip);
}

sdk_ret_t
ep_ip_entry::delay_delete(void) {
    return api::delay_delete_to_slab(api::PDS_SLAB_ID_IP_ENTRY, this);
}

sdk_ret_t
ep_ip_entry::add_to_db(void) {
    return ep_ip_db()->insert(this);
}

sdk_ret_t
ep_ip_entry::del_from_db(void) {
    if (ep_ip_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

void
ep_ip_entry::set_state(ep_state_t state) {
    this->state_ = state;
}

ep_mac_entry *
ep_ip_entry::mac_entry(void) {
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;
    ep_mac_key_t mac_key;

    vnic_key = api::uuid_from_objid(vnic_obj_id_);
    vnic = vnic_db()->find(&vnic_key);
    mac_key.subnet = vnic->subnet();
    MAC_ADDR_COPY(&mac_key.mac_addr, &vnic->mac());
    return (ep_mac_db()->find(&mac_key));
}

bool
ep_ip_entry::vnic_compare(uint32_t vnic_obj_id) const {
    return (vnic_obj_id == this->vnic_obj_id_);
}

}    // namespace learn
