#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

#include "nic/hal/src/vrf.hpp"
#include "nic/gen/proto/hal/nw.pb.h"
#include "nic/include/ip.h"

using nw::NetworkSpec;
using nw::NetworkStatus;
using nw::NetworkResponse;
using kh::NetworkKeyHandle;
using nw::NetworkRequestMsg;
using nw::NetworkResponseMsg;
using nw::NetworkDeleteRequest;
using nw::NetworkDeleteResponse;
using nw::NetworkDeleteRequestMsg;
using nw::NetworkDeleteResponseMsg;
using nw::NetworkGetRequest;
using nw::NetworkGetRequestMsg;
using nw::NetworkGetResponse;
using nw::NetworkGetResponseMsg;

namespace hal {

// key to network prefix object
typedef struct network_key_s {
    vrf_id_t    vrf_id;
    ip_prefix_t    ip_pfx;
} __PACK__ network_key_t;

// network pefix object
// TODO: capture multiple categories of multiple-labels
typedef struct network_s {
    hal_spinlock_t    slock;                // lock to protect this structure
    network_key_t     nw_key;               // key of the network object
    hal_handle_t      gw_ep_handle;         // gateway EP's handle
    mac_addr_t        rmac_addr;            // RMAC address of the network

    // operational state of network
    hal_handle_t      hal_handle;           // HAL allocated handle

    // forward references
    dllist_ctxt_t     sg_list_head;         // security group list   
    // back references
    dllist_ctxt_t     l2seg_list_head;      // l2segs referring to this
    dllist_ctxt_t     session_list_head;    // sessions referring to this

    // meta data maintained for network     
    // ht_ctxt_t         nwkey_ht_ctxt;        // network key based hash table ctxt
    // ht_ctxt_t         hal_handle_ht_ctxt;   // hal handle based hash table ctxt
    // Clean up 
    // dllist_ctxt_t     l2seg_nw_lentry;      // L2 segment's nw list entry
} __PACK__ network_t;

// max. number of networks supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NETWORKS                           4096

// CB data structures
typedef struct network_create_app_ctxt_s {
} __PACK__ network_create_app_ctxt_t;

typedef struct network_update_app_ctxt_s {
    bool            network_changed;
    bool            gw_ep_changed;
    bool            sglist_changed;

    // new gateway EP handle
    hal_handle_t    new_gw_ep_handle;
    // sg list change
    dllist_ctxt_t   *add_sglist;
    dllist_ctxt_t   *del_sglist;
    dllist_ctxt_t   *aggr_sglist;
} __PACK__ network_update_app_ctxt_t;

const char *network_to_str (network_t *nw);
static inline void 
network_lock(network_t *network, const char *fname, int lineno, 
             const char *fxname)
{
    HAL_TRACE_DEBUG("{}:locking network:{} from {}:{}:{}", 
                    __FUNCTION__, 
                    network_to_str(network), 
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&network->slock);
}

static inline void 
network_unlock(network_t *network, const char *fname, int lineno, 
               const char *fxname)
{
    HAL_TRACE_DEBUG("{}:unlocking network:{} from {}:{}:{}", 
                    __FUNCTION__, 
                    network_to_str(network), 
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&network->slock);
}

// allocate a network instance
static inline network_t *
network_alloc (void)
{
    network_t    *network;

    network = (network_t *)g_hal_state->network_slab()->alloc();
    if (network == NULL) {
        return NULL;
    }
    return network;
}

// initialize a network instance
static inline network_t *
network_init (network_t *network)
{
    if (!network) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&network->slock, PTHREAD_PROCESS_PRIVATE);

    sdk::lib::dllist_reset(&network->sg_list_head);
    sdk::lib::dllist_reset(&network->l2seg_list_head);
    sdk::lib::dllist_reset(&network->session_list_head);


    // initialize the operational state
    // network->rmac_addr = 0;

    // initialize meta information
    // network->nwkey_ht_ctxt.reset();
    // network->hal_handle_ht_ctxt.reset();
    // sdk::lib::dllist_reset(&network->l2seg_nw_lentry);

    return network;
}

// allocate and initialize a network instance
static inline network_t *
network_alloc_init (void)
{
    return network_init(network_alloc());
}

// free network instance
static inline hal_ret_t
network_free (network_t *network)
{
    HAL_SPINLOCK_DESTROY(&network->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NETWORK, network);
    return HAL_RET_OK;
}

// find a network instance by its id
static inline network_t *
find_network_by_key (vrf_id_t tid, const ip_prefix_t *ip_pfx)
{
    network_key_t               nw_key = { 0 };
    hal_handle_id_ht_entry_t    *entry;
    network_t                   *nw = NULL;

    nw_key.vrf_id = tid;
    memcpy(&nw_key.ip_pfx, ip_pfx, sizeof(ip_prefix_t));


    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        network_key_ht()->lookup(&nw_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                HAL_OBJ_ID_NETWORK);
        nw = (network_t *)hal_handle_get_obj(entry->handle_id);
        return nw;
    }
    return NULL;
}

// find a network instance by its handle
static inline network_t *
find_network_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_NETWORK) {
        HAL_TRACE_DEBUG("{}:failed to find network with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (network_t *)hal_handle->get_obj();
#if 0
    // check for object type
    HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
               HAL_OBJ_ID_NETWORK);
    return (network_t *)hal_handle_get_obj(handle);
    // return (network_t *)g_hal_state->network_hal_handle_ht()->lookup(&handle);
#endif
}

extern void *network_get_key_func(void *entry);
extern uint32_t network_compute_hash_func(void *key, uint32_t ht_size);
extern bool network_compare_key_func(void *key1, void *key2);

hal_ret_t network_update_sg_relation (dllist_ctxt_t *sg_list, 
                                      network_t *nw, bool add);

hal_ret_t network_create(nw::NetworkSpec& spec,
                         nw::NetworkResponse *rsp);
hal_ret_t network_update(nw::NetworkSpec& spec,
                         nw::NetworkResponse *rsp);
hal_ret_t network_delete(nw::NetworkDeleteRequest& req,
                         nw::NetworkDeleteResponse *rsp);
hal_ret_t network_get(nw::NetworkGetRequest& req,
                      nw::NetworkGetResponse *rsp);
network_t *
network_lookup_key_or_handle (NetworkKeyHandle& kh, vrf_id_t tid);
}    // namespace hal

#endif    // __NETWORK_HPP__

