#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

namespace hal {

// key to network prefix object
typedef struct nw_key_s {
    tenant_id_t    tenant_id;
    ip_prefix_t    ip_pfx;
} __PACK__ nw_key_t;

// network pefix object
// TODO: capture multiple categories of multiple-labels
typedef struct nw_s {
    hal_spinlock_t    slock;           // lock to protect this structure
    nw_key_t          nw_key;          // key of the network object
    hal_handle_t      gw_ep_handle;    // gateway EP's handle

    // operational state of network
    hal_handle_t      hal_handle;      // HAL allocated handle
} __PACK__ nw_t;

}    // namespace hal

#endif    // __NETWORK_HPP__

