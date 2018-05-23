//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include "nic/include/base.h"
#include "sdk/list.hpp"
#include "nic/include/ip.h"
#include "sdk/ht.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/gen/proto/hal/eplearn.pb.h"
#include "nic/include/pd.hpp"
#include <netinet/ether.h>

using kh::EndpointKeyHandle;
using endpoint::EndpointGetResponse;
using types::ApiStatus;
using endpoint::EndpointSpec;
using endpoint::EndpointStatus;
using endpoint::EndpointResponse;
using endpoint::EndpointRequestMsg;
using endpoint::EndpointResponseMsg;
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteResponseMsg;
using endpoint::EndpointDeleteResponse;
using endpoint::EndpointGetRequest;
using endpoint::EndpointGetRequestMsg;
using endpoint::EndpointGetResponse;
using endpoint::EndpointGetResponseMsg;
using endpoint::EndpointUpdateRequestMsg;
using endpoint::EndpointUpdateResponseMsg;
using endpoint::EndpointUpdateRequest;
using endpoint::EndpointIpAddress;
using eplearn::DhcpTransactionState;
using eplearn::DhcpStatus;
using eplearn::ArpStatus;
using eplearn::EplearnStatus;
using kh::EndpointKeyHandle;
using endpoint::EndpointDeleteRequest;
using kh::EndpointL2Key;
using kh::EndpointKey;

using endpoint::EndpointVMotionState;
using namespace endpoint;
using sdk::lib::ht_ctxt_t;

namespace hal {

#define EP_FLAGS_LOCAL                               0x1
#define EP_FLAGS_REMOTE                              0x2
#define EP_FLAGS_LEARN_SRC_DHCP                      0x4
#define EP_FLAGS_LEARN_SRC_ARP                       0x8
#define EP_FLAGS_LEARN_SRC_RARP                      0x10
#define EP_FLAGS_LEARN_SRC_CFG                       0x20

#define MAX_SG_PER_ARRAY                             0x10 //16

typedef hal_ret_t (*dhcp_status_func_t)(vrf_id_t vrf_id, ip_addr_t *ip_addr,
        DhcpStatus *dhcp_status);
typedef hal_ret_t (*arp_status_func_t)(vrf_id_t vrf_id, ip_addr_t *ip_addr,
        ArpStatus *arp_status);

// L2 key of the endpoint
typedef struct ep_l2_key_s {
    l2seg_id_t    l2_segid;    // L2 segment id
    mac_addr_t    mac_addr;    // MAC address of the endpoint
} __PACK__ ep_l2_key_t;

// L3 key of the endpoint
typedef struct ep_l3_key_s {
    vrf_id_t     vrf_id;    // VRF id
    ip_addr_t    ip_addr;   // IP address of the endpoint
} __PACK__ ep_l3_key_t;

// endpoint's L3/IP information -- an endpoint can have multiple IPs
typedef struct ep_ip_entry_s {
    ip_addr_t            ip_addr;             // IP address of the endpoint
    uint64_t             ip_flags;            // IP flags

    // PD state
    pd::pd_ep_ip_entry_t *pd;                 // all PD specific state

    dllist_ctxt_t        ep_ip_lentry;        // IP entry list context
} __PACK__ ep_ip_entry_t;

// Stores the list of Security_group_ids in this strucutre.
// Expected to store MAX_SG_PER_ARRAY per structure. If it exceeds
// we populate next sg_p structure
typedef struct ep_sg_s {
    uint8_t             sg_id_cnt;                       // Current sec_group id < MAX_SG_PER_ARRAY
    uint32_t            arr_sg_id[MAX_SG_PER_ARRAY];     // Array of security group ids
    struct ep_sg_s      *next_sg_p;                      // Point to the next set of securiy_group_ids
}__PACK__ ep_sginfo_t;


// endpoint data structure
// TODO: capture multiple categories of multiple-labels
typedef struct ep_s {
    hal_spinlock_t       slock;                // lock to protect this structure
    ep_l2_key_t          l2_key;               // MAC, vlan information
    hal_handle_t         nw_handle;            // network this EP belongs to
    hal_handle_t         l2seg_handle;         // L2 segment this endpoint belongs to
    hal_handle_t         if_handle;            // interface endpoint is attached to
    hal_handle_t         gre_if_handle;        // Set if there is a GRE tunnel destined to this EP.
    hal_handle_t         pinned_if_handle;     // interface endpoint is attached to
    hal_handle_t         vrf_handle;           // vrf handle
    vlan_id_t            useg_vlan;            // micro-seg vlan allocated for this endpoint
    uint64_t             ep_flags;             // endpoint flags
    ep_sginfo_t          sgs;                  // Holds the security group ids
    EndpointVMotionState vmotion_state;        // Vmotion state
    dllist_ctxt_t        ip_list_head;         // list of IP addresses for this endpoint

    // operational state of endpoint
    hal_handle_t         hal_handle;           // HAL allocated handle

    // PD state
    pd::pd_ep_t          *pd;                  // all PD specific state

    // meta data maintained for endpoint
    // ht_ctxt_t            l2key_ht_ctxt;        // hal handle based hash table ctxt
    // ht_ctxt_t            hal_handle_ht_ctxt;   // hal handle based hash table ctxt
    // llist_ctxt_t        vrf_ep_lentry;     // links in L2 segment endpoint list
    // dllist_ctxt_t        l2seg_ep_lentry;      // links in L2 segment endpoint list
    // dllist_ctxt_t        if_ep_lentry;         // links in inteface endpoint list
    dllist_ctxt_t        session_list_head;    // session from/to this EP
    block_list           *nh_list;             // next hops back refs

} __PACK__ ep_t;


// Endpoint's intermediate L3 entry object that points to actual endpoint info
typedef struct ep_l3_entry_s {
    ep_l3_key_t             l3_key;           // EP's L3 key
    // ep_t                    *ep;              // pointer to the EP record
    hal_handle_t            ep_hal_handle;       // HAL allocated handle
    ep_ip_entry_t           *ep_ip;           // pointer to IP entry in the EP

    ht_ctxt_t               ht_ctxt;    // EP's l3 key based hash table ctxt
} __PACK__ ep_l3_entry_t;

// max. number of endpoints supported  (TODO: we can take this from cfg file)
#define HAL_MAX_ENDPOINTS                            (1 << 20)

// cb data structures
typedef struct ep_create_app_ctxt_s {
    vrf_t           *vrf;
    l2seg_t         *l2seg;
    if_t            *hal_if;
} __PACK__ ep_create_app_ctxt_t;

typedef struct ep_update_app_ctxt_s {
    uint64_t                iplist_change:1;
    uint64_t                if_change:1;
    uint64_t                vmotion_state_change:1;

    dllist_ctxt_t           *add_iplist;
    dllist_ctxt_t           *del_iplist;
    hal_handle_t            new_if_handle;
    EndpointVMotionState    new_vmotion_state;
} __PACK__ ep_update_app_ctxt_t;

const char *ep_l2_key_to_str(ep_t *ep);

static inline void
ep_lock (ep_t *ep, const char *fname,
          int lineno, const char *fxname)
{
    HAL_SPINLOCK_LOCK(&ep->slock);
}

static inline void
ep_unlock (ep_t *ep, const char *fname,
            int lineno, const char *fxname)
{
    HAL_SPINLOCK_UNLOCK(&ep->slock);
}

extern void *ep_get_l2_key_func(void *entry);
extern uint32_t ep_compute_l2_hash_func(void *key, uint32_t ht_size);
extern bool ep_compare_l2_key_func(void *key1, void *key2);

extern void *ep_get_l3_key_func(void *entry);
extern uint32_t ep_compute_l3_hash_func(void *key, uint32_t ht_size);
extern bool ep_compare_l3_key_func(void *key1, void *key2);

mac_addr_t *ep_get_mac_addr(ep_t *pi_ep);
mac_addr_t *ep_get_rmac(ep_t *pi_ep, l2seg_t *l2seg);
hal_ret_t endpoint_update_pi_with_iplist(ep_t *ep, dllist_ctxt_t *add_iplist,
                                         dllist_ctxt_t *del_iplist);
hal_ret_t endpoint_free_ip_list(dllist_ctxt_t *iplist);
hal_ret_t endpoint_cleanup_ip_list(dllist_ctxt_t **list);
hal_ret_t endpoint_update_ip_add(ep_t *ep, ip_addr_t *ip,
        uint64_t learn_src_flag);
hal_ret_t endpoint_update_ip_delete(ep_t *ep, ip_addr_t *ip,
        uint64_t learn_src_flag);
bool ip_in_ep(ip_addr_t *ip, ep_t *ep, ep_ip_entry_t **ip_entry);
// find EP from l2 key
ep_t *find_ep_by_l2_key(l2seg_id_t l2seg_id, const mac_addr_t mac_addr);
// find EP from hal handle
ep_t *find_ep_by_handle(hal_handle_t handle);
// find EP from l3 key
ep_t *find_ep_by_l3_key(ep_l3_key_t *ep_l3_key);
// find EP from v4 key
ep_t *find_ep_by_v4_key(vrf_id_t tid, uint32_t v4_addr);
// find EP from v6 key
ep_t *find_ep_by_v6_key(vrf_id_t tid, const ip_addr_t *ip_addr);
hal_ret_t find_ep(EndpointKeyHandle kh, ep_t **ep, ::types::ApiStatus *api_status);

// Debug APIs
void ep_print_ips(ep_t *ep);
const char *ep_l2_key_to_str(ep_t *ep);

hal_ret_t endpoint_create(EndpointSpec& spec, EndpointResponse *rsp);
hal_ret_t endpoint_update(EndpointUpdateRequest& spec, EndpointResponse *rsp);
hal_ret_t endpoint_delete(EndpointDeleteRequest& spec,
                          EndpointDeleteResponse *rsp);
hal_ret_t endpoint_get(endpoint::EndpointGetRequest& spec,
                       endpoint::EndpointGetResponseMsg *rsp);

hal_ret_t ep_store_cb(void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen);
uint32_t ep_restore_cb(void *obj, uint32_t len);

void register_dhcp_ep_status_callback(dhcp_status_func_t func);
void register_arp_ep_status_callback(arp_status_func_t func);

}    // namespace hal

#endif    // __ENDPOINT_HPP__

