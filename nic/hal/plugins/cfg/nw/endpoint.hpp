//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include "nic/include/base.hpp"
#include "lib/list/list.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/eplearn.pb.h"
#include "gen/proto/session.pb.h"
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
using endpoint::MigrationResponse;
using endpoint::MigrationState;
using endpoint::EndpointIpAddress;
using eplearn::DhcpTransactionState;
using eplearn::DhcpStatus;
using eplearn::ArpStatus;
using eplearn::EplearnStatus;
using kh::EndpointKeyHandle;
using endpoint::EndpointDeleteRequest;
using kh::EndpointL2Key;
using kh::EndpointKey;
using session::SessionGetResponseMsg;

using namespace endpoint;
using sdk::lib::ht_ctxt_t;

namespace hal {

#define EP_FLAGS_LOCAL                               0x1
#define EP_FLAGS_REMOTE                              0x2
#define EP_FLAGS_LEARN_SRC_DHCP                      0x4
#define EP_FLAGS_LEARN_SRC_ARP                       0x8
#define EP_FLAGS_LEARN_SRC_RARP                      0x10
#define EP_FLAGS_LEARN_SRC_CFG                       0x20
#define EP_FLAGS_LEARN_SRC_FLOW_MISS                 0x40

#define MAX_SG_PER_ARRAY                             0x10 //16

#define EP_UPDATE_SESSION_TIMER                      (250) 

typedef enum ep_vmotion_type_s {
    VMOTION_TYPE_MIGRATE_NONE,
    VMOTION_TYPE_MIGRATE_IN,
    VMOTION_TYPE_MIGRATE_OUT
} ep_vmotion_type_t;

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
    uint64_t             cur_ip_flags;        // IP flags which are active now.

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
    sdk_spinlock_t       slock;                // lock to protect this structure
    ep_l2_key_t          l2_key;               // MAC, vlan information
    hal_handle_t         nw_handle;            // network this EP belongs to
    hal_handle_t         l2seg_handle;         // L2 segment this endpoint belongs to
    hal_handle_t         if_handle;            // interface endpoint is attached to
    // hal_handle_t         gre_if_handle;        // Set if there is a GRE tunnel destined to this EP.
    // hal_handle_t         pinned_if_handle;     // interface endpoint is attached to
    hal_handle_t         vrf_handle;           // vrf handle
    vlan_id_t            useg_vlan;            // micro-seg vlan allocated for this endpoint
    uint64_t             ep_flags;             // endpoint flags
    ep_sginfo_t          sgs;                  // Holds the security group ids
    ep_vmotion_type_t    vmotion_type;         // vMotion type - Migrate IN/OUT
    MigrationState       vmotion_state;        // Vmotion state
    ip_addr_t            old_homing_host_ip;   // IP Address of host where the ep was homed - for vMotion
    dllist_ctxt_t        ip_list_head;         // list of IP addresses for this endpoint
    bool                 egress_en;            // based on filter cfg from NIC mgr

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
    dllist_ctxt_t        *session_list_head;    // session from/to this EP
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
    MigrationState   vmotion_state;   
} __PACK__ ep_create_app_ctxt_t;

typedef struct ep_update_app_ctxt_s {
    uint64_t                iplist_change:1;
    uint64_t                if_change:1;
    uint64_t                useg_vlan_change:1;
    uint64_t                vmotion_state_change:1;

    dllist_ctxt_t           *add_iplist;
    dllist_ctxt_t           *del_iplist;
    hal_handle_t            new_if_handle;
    ip_addr_t               new_homing_host_ip;
    ip_addr_t               source_host_ip;
    ip_addr_t               destination_host_ip;
    vlan_id_t               new_useg_vlan;
    MigrationState          new_vmotion_state;
} __PACK__ ep_update_app_ctxt_t;

typedef struct ep_sess_upd_ctxt_s {
    uint8_t     fte_id;
    block_list  *sess_list;
} ep_sess_upd_ctxt_t;

typedef struct ep_sess_walk_s {
    ep_t *ep;
    ep_sess_upd_ctxt_t *ep_upd_ctxt;
} __PACK__ ep_sess_walk_t;

typedef struct ep_create_sess_ctxt_s {
    uint8_t fte_id;
    uint32_t count;
    hal_handle_t *sess_hdl_list;
} __PACK__ ep_create_sess_ctxt_t;

const char *ep_l2_key_to_str(ep_t *ep);

static inline void
ep_lock (ep_t *ep, const char *fname,
          int lineno, const char *fxname)
{
    SDK_SPINLOCK_LOCK(&ep->slock);
}

static inline void
ep_unlock (ep_t *ep, const char *fname,
            int lineno, const char *fxname)
{
    SDK_SPINLOCK_UNLOCK(&ep->slock);
}

typedef void (*sessions_empty_cb_t)(const ep_t *ep);
extern void *ep_get_l2_key_func(void *entry);
extern uint32_t ep_l2_key_size(void);

extern void *ep_get_l3_key_func(void *entry);
extern uint32_t ep_l3_key_size(void);

mac_addr_t *ep_get_mac_addr(ep_t *pi_ep);
mac_addr_t *ep_get_rmac(ep_t *pi_ep, l2seg_t *l2seg);
hal_ret_t endpoint_update_pi_with_iplist(ep_t *ep, dllist_ctxt_t *add_iplist,
                                         dllist_ctxt_t *del_iplist);
hal_ret_t endpoint_free_ip_list(dllist_ctxt_t *iplist);
hal_ret_t endpoint_cleanup_ip_list(dllist_ctxt_t **list);
hal_ret_t endpoint_update_ip_add(ep_t *ep, ip_addr_t *ip,
        uint64_t learn_src_flag);
hal_ret_t endpoint_update_if(ep_t *ep, if_t *new_hal_if);
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
// find EP from v4 key in segment
ep_t* find_ep_by_v4_key_in_l2segment(uint32_t v4_addr,
        const hal::l2seg_t *l2seg);
// find EP from v6 key in segment
ep_t* find_ep_by_v6_key_in_l2segment(const ip_addr_t *ip_addr,
        const hal::l2seg_t *l2seg);
hal_ret_t find_ep(EndpointKeyHandle kh, ep_t **ep);
ep_t * find_ep_by_mac(mac_addr_t mac);
if_t *ep_get_pinned_uplink(ep_t *ep);

bool ep_handle_ipsg_change_cb(void *ht_entry, void *ctxt);

// Debug APIs
void ep_print_ips(ep_t *ep);
const char *ep_l2_key_to_str(ep_t *ep);

hal_ret_t endpoint_create(EndpointSpec& spec, EndpointResponse *rsp);
hal_ret_t endpoint_update(EndpointUpdateRequest& spec, EndpointResponse *rsp);
hal_ret_t endpoint_delete(EndpointDeleteRequest& spec,
                          EndpointDeleteResponse *rsp);
hal_ret_t endpoint_get(endpoint::EndpointGetRequest& spec,
                       endpoint::EndpointGetResponseMsg *rsp);
bool endpoint_is_remote(ep_t *ep);
hal_ret_t endpoint_migration_status_update (ep_t *ep, MigrationState migration_state);
hal_ret_t endpoint_migration_inp_mac_vlan_pgm(ep_t *ep, bool create);
void endpoint_migration_done (ep_t *ep, MigrationState mig_state);
hal_ret_t endpoint_migration_normalization_cfg(ep_t *ep, bool disable);
hal_ret_t endpoint_migration_if_update(ep_t *ep);
void ep_sessions_delete(ep_t *ep);

hal_ret_t ep_store_cb(void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen);
uint32_t ep_restore_cb(void *obj, uint32_t len);
hal_ret_t ep_handle_vmotion(ep_t *ep, MigrationState new_vmotion_state);
hal_ret_t ep_quiesce(ep_t *ep, bool entry_add);

void register_dhcp_ep_status_callback(dhcp_status_func_t func);
void register_arp_ep_status_callback(arp_status_func_t func);
void register_sessions_empty_callback(sessions_empty_cb_t func);

hal_ret_t endpoint_create_process_sessions(ep_t *ep);
void fte_session_update_list(void *data);
void ep_create_session_timer_cb(void *timer, uint32_t timer_id, void *ctxt);
hal_ret_t ep_get_session_info (ep_t *ep, session::SessionGetResponseMsg *rsp, uint64_t ts = 0);

// Filter APIs
hal_ret_t filter_create(FilterSpec& spec, FilterResponse *rsp);
hal_ret_t filter_delete(FilterDeleteRequest& spec,
                        FilterDeleteResponse *rsp);
hal_ret_t filter_get(FilterGetRequest& spec,
                     FilterGetResponseMsg *rsp);

}    // namespace hal

#endif    // __ENDPOINT_HPP__

