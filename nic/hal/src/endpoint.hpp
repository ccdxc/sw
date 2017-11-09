#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include "nic/include/base.h"
#include "nic/include/list.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/ip.h"
#include "nic/utils/ht/ht.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/include/pd.hpp"
#include <netinet/ether.h>

using endpoint::EndpointKeyHandle;
using endpoint::EndpointGetResponse;
using types::ApiStatus;
using endpoint::EndpointSpec;
using endpoint::EndpointStatus;
using endpoint::EndpointResponse;
using endpoint::EndpointRequestMsg;
using endpoint::EndpointResponseMsg;
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteResponseMsg;
using endpoint::EndpointGetRequest;
using endpoint::EndpointGetRequestMsg;
using endpoint::EndpointGetResponse;
using endpoint::EndpointGetResponseMsg;
using endpoint::EndpointUpdateRequestMsg;
using endpoint::EndpointUpdateResponseMsg;
using endpoint::EndpointUpdateRequest;
using endpoint::EndpointKeyHandle;
using endpoint::EndpointDeleteRequest;
using endpoint::EndpointDeleteResponseMsg;

using hal::utils::ht_ctxt_t;

namespace hal {

#define EP_FLAGS_LOCAL                               0x1
#define EP_FLAGS_REMOTE                              0x2
#define EP_FLAGS_LEARN_SRC_DHCP                      0x4
#define EP_FLAGS_LEARN_SRC_ARP                       0x8
#define EP_FLAGS_LEARN_SRC_RARP                      0x10
#define EP_FLAGS_LEARN_SRC_CFG                       0x20

#define MAX_SG_PER_ARRAY                             0x10 //16 

// L2 key of the endpoint
typedef struct ep_l2_key_s {
    l2seg_id_t    l2_segid;    // L2 segment id
    mac_addr_t    mac_addr;    // MAC address of the endpoint
} __PACK__ ep_l2_key_t;

// L3 key of the endpoint
typedef struct ep_l3_key_s {
    tenant_id_t    tenant_id;    // VRF id
    ip_addr_t      ip_addr;      // IP address of the endpoint
} __PACK__ ep_l3_key_t;

// endpoint's L3/IP information -- an endpoint can have multiple IPs
typedef struct ep_ip_entry_s {
    ip_addr_t            ip_addr;             // IP address of the endponit
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
    hal_handle_t         tenant_handle;        // tenant handle 
    // tenant_id_t          tenant_id;            // VRF this endpoint belongs to
    vlan_id_t            useg_vlan;            // micro-seg vlan allocated for this endpoint
    uint64_t             ep_flags;             // endpoint flags
    ep_sginfo_t          sgs;                  // Holds the security group ids
    dllist_ctxt_t        ip_list_head;         // list of IP addresses for this endpoint

    // operational state of endpoint
    hal_handle_t         hal_handle;           // HAL allocated handle

    // PD state
    pd::pd_ep_t          *pd;                  // all PD specific state

    // meta data maintained for endpoint
    // ht_ctxt_t            l2key_ht_ctxt;        // hal handle based hash table ctxt
    // ht_ctxt_t            hal_handle_ht_ctxt;   // hal handle based hash table ctxt
    // llist_ctxt_t        tenant_ep_lentry;     // links in L2 segment endpoint list
    // dllist_ctxt_t        l2seg_ep_lentry;      // links in L2 segment endpoint list
    // dllist_ctxt_t        if_ep_lentry;         // links in inteface endpoint list
    dllist_ctxt_t        session_list_head;    // session from/to this EP
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

// CB data structures
typedef struct ep_create_app_ctxt_s {
    tenant_t        *tenant;
    l2seg_t         *l2seg;
    if_t            *hal_if;
} __PACK__ ep_create_app_ctxt_t;

typedef struct ep_update_app_ctxt_s {
    bool            iplist_change;
    bool            if_change;

    dllist_ctxt_t   *add_iplist;
    dllist_ctxt_t   *del_iplist;
    hal_handle_t    new_if_handle;    
} __PACK__ ep_update_app_ctxt_t;

const char *ep_l2_key_to_str(ep_t *ep);

static inline void 
ep_lock (ep_t *ep, const char *fname,
          int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking ep:{} from {}:{}:{}", 
                    __FUNCTION__, ep_l2_key_to_str(ep),
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&ep->slock);
}

static inline void 
ep_unlock (ep_t *ep, const char *fname,
            int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking ep:{} from {}:{}:{}", 
                    __FUNCTION__, ep_l2_key_to_str(ep),
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&ep->slock);
}
// allocate a ep instance
static inline ep_t *
ep_alloc (void)
{
    ep_t    *ep;

    ep = (ep_t *)g_hal_state->ep_slab()->alloc();
    if (ep == NULL) {
        return NULL;
    }
    return ep;
}

// initialize a ep instance
static inline ep_t *
ep_init (ep_t *ep)
{
    if (!ep) {
        return NULL;
    }
    memset(ep, 0, sizeof(ep_t));
    HAL_SPINLOCK_INIT(&ep->slock, PTHREAD_PROCESS_PRIVATE);
    
    utils::dllist_reset(&ep->ip_list_head);
    utils::dllist_reset(&ep->session_list_head);
    ep->sgs.sg_id_cnt = 0;
    memset(&ep->sgs.arr_sg_id, 0 , MAX_SG_PER_ARRAY);

    return ep;
}

// allocate and initialize a ep instance
static inline ep_t *
ep_alloc_init (void)
{
    return ep_init(ep_alloc());
}

static inline hal_ret_t
ep_free (ep_t *ep)
{
    HAL_SPINLOCK_DESTROY(&ep->slock);

    // TODO: may have to free list of ip entries
    g_hal_state->ep_slab()->free(ep);
    return HAL_RET_OK;
}

// find EP from l2 key
static inline ep_t *
find_ep_by_l2_key (l2seg_id_t l2seg_id, const mac_addr_t mac_addr)
{
    hal_handle_id_ht_entry_t    *entry;
    ep_l2_key_t                 l2_key = { 0 };
    ep_t                        *ep;

    l2_key.l2_segid = l2seg_id;
    memcpy(&l2_key.mac_addr, mac_addr, ETH_ADDR_LEN);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        ep_l2_ht()->lookup(&l2_key);
    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_ENDPOINT);
        ep = (ep_t *)hal_handle_get_obj(entry->handle_id);
        return ep;
    }
    return NULL;
}

// find EP from hal handle
static inline ep_t *
find_ep_by_handle (hal_handle_t handle)
{
    // check for object type
    HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
               HAL_OBJ_ID_ENDPOINT);
    return (ep_t *)hal_handle_get_obj(handle);
}

// find EP from l3 key
static inline ep_t *
find_ep_by_l3_key (ep_l3_key_t *ep_l3_key)
{
    ep_l3_entry_t    *ep_l3_entry;

    HAL_ASSERT(ep_l3_key != NULL);
    ep_l3_entry =
        (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->lookup(ep_l3_key);
    if (ep_l3_entry == NULL) {
        return NULL;
    }
    return find_ep_by_handle(ep_l3_entry->ep_hal_handle);
}

// find EP from v4 key
static inline ep_t *
find_ep_by_v4_key (tenant_id_t tid, uint32_t v4_addr)
{
    ep_l3_key_t    l3_key = { 0 };

    l3_key.tenant_id = tid;
    l3_key.ip_addr.af = IP_AF_IPV4;
    l3_key.ip_addr.addr.v4_addr = v4_addr;
    return find_ep_by_l3_key(&l3_key);
}

// find EP from v6 key
static inline ep_t *
find_ep_by_v6_key (tenant_id_t tid, const ip_addr_t *ip_addr)
{
    ep_l3_key_t    l3_key = { 0 };

    l3_key.tenant_id = tid;
    memcpy(&l3_key.ip_addr, ip_addr, sizeof(ip_addr_t));
    return find_ep_by_l3_key(&l3_key);
}










#if 0


static inline ep_t *
find_ep_by_handle (hal_handle_t handle)
{
    return (ep_t *)g_hal_state->ep_hal_handle_ht()->lookup(&handle);
}

static inline ep_t *
find_ep_by_l2_key (l2seg_id_t l2seg_id, const mac_addr_t mac_addr)
{
    ep_l2_key_t    l2_key = { 0 };
    ep_t           *ep;

    l2_key.l2_segid = l2seg_id;
    memcpy(&l2_key.mac_addr, mac_addr, ETH_ADDR_LEN);
    ep = (ep_t *)g_hal_state->ep_l2_ht()->lookup(&l2_key);
    return ep;
}

static inline ep_t *
find_ep_by_l3_key (ep_l3_key_t *ep_l3_key)
{
    ep_l3_entry_t    *ep_l3_entry;

    HAL_ASSERT(ep_l3_key != NULL);
    ep_l3_entry =
        (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->lookup(ep_l3_key);
    if (ep_l3_entry == NULL) {
        return NULL;
    }
    return ep_l3_entry->ep;
}

static inline ep_t *
find_ep_by_v4_key (tenant_id_t tid, uint32_t v4_addr)
{
    ep_l3_key_t    l3_key = { 0 };

    l3_key.tenant_id = tid;
    l3_key.ip_addr.af = IP_AF_IPV4;
    l3_key.ip_addr.addr.v4_addr = v4_addr;
    return find_ep_by_l3_key(&l3_key);
}

static inline ep_t *
find_ep_by_v6_key (tenant_id_t tid, const ip_addr_t *ip_addr)
{
    ep_l3_key_t    l3_key = { 0 };

    l3_key.tenant_id = tid;
    memcpy(&l3_key.ip_addr, ip_addr, sizeof(ip_addr_t));
    return find_ep_by_l3_key(&l3_key);
}
#endif

extern void *ep_get_l2_key_func(void *entry);
extern uint32_t ep_compute_l2_hash_func(void *key, uint32_t ht_size);
extern bool ep_compare_l2_key_func(void *key1, void *key2);

extern void *ep_get_l3_key_func(void *entry);
extern uint32_t ep_compute_l3_hash_func(void *key, uint32_t ht_size);
extern bool ep_compare_l3_key_func(void *key1, void *key2);

//extern void *ep_get_handle_key_func(void *entry);
//extern uint32_t ep_compute_handle_hash_func(void *key, uint32_t ht_size);
//extern bool ep_compare_handle_key_func(void *key1, void *key2);
mac_addr_t *ep_get_mac_addr(ep_t *pi_ep);
mac_addr_t *ep_get_rmac(ep_t *pi_ep, l2seg_t *l2seg);
hal_ret_t endpoint_update_pi_with_iplist(ep_t *ep, dllist_ctxt_t *add_iplist,
                                         dllist_ctxt_t *del_iplist);  
hal_ret_t endpoint_free_ip_list(dllist_ctxt_t *iplist);
hal_ret_t endpoint_cleanup_ip_list(dllist_ctxt_t **list);

// Debug APIs
void ep_print_ips(ep_t *ep);
const char *ep_l2_key_to_str(ep_t *ep);



// SVC CRUD APIs
hal_ret_t endpoint_create(EndpointSpec& spec, EndpointResponse *rsp);
hal_ret_t endpoint_update(EndpointUpdateRequest& spec, EndpointResponse *rsp);
hal_ret_t endpoint_delete(EndpointDeleteRequest& spec, 
                          EndpointDeleteResponseMsg *rsp);
hal_ret_t endpoint_get(endpoint::EndpointGetRequest& spec,
                       endpoint::EndpointGetResponseMsg *rsp);




}    // namespace hal

#endif    // __ENDPOINT_HPP__

