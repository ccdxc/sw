//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __MULTICAST_HPP__
#define __MULTICAST_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "gen/proto/multicast.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/src/utils/utils.hpp"

using multicast::Oif;
using multicast::OifList;
using kh::MulticastEntryKeyHandle;
using multicast::MulticastEntrySpec;
using multicast::MulticastEntryStatus;
using multicast::MulticastEntryResponse;
using multicast::MulticastEntryRequestMsg;
using multicast::MulticastEntryResponseMsg;
using multicast::MulticastEntryDeleteRequest;
using multicast::MulticastEntryDeleteResponse;
using multicast::MulticastEntryGetRequest;
using multicast::MulticastEntryGetRequestMsg;
using multicast::MulticastEntryGetResponse;
using multicast::MulticastEntryGetResponseMsg;

namespace hal {

#define HAL_MAX_OIF_LISTS            65536
#define HAL_MAX_MC_ENTRIES            2048
#define HAL_MAX_OIF_PER_MC_ENTRY       256
#define HAL_MC_ENTRY_OIFL_BLOCK          3 // 0: Mgmt Repls, 1: MSeg/BM Repls, 2: Mgmt + MSeg/BM Repls

typedef enum mc_key_type {
    MC_KEY_TYPE_NONE = 0,
    MC_KEY_TYPE_IP = 1,
    MC_KEY_TYPE_MAC = 2
} mc_key_type_e;

typedef union mc_key_u {
    ip_addr_t ip;
    mac_addr_t mac;
} mc_key_u;

typedef struct mc_key_s {
    mc_key_u      u;
    mc_key_type_e type;
    hal_handle_t  l2seg_handle;
} __PACK__ mc_key_t;

typedef struct mc_entry_s {
    mc_key_t              key;
    oif_list_id_t         oif_list;                // outgoing interface list

    // operational state of L2 segment
    sdk_spinlock_t        slock;                   // lock to protect this structure
    hal_handle_t          hal_handle;              // HAL allocated handle
    dllist_ctxt_t         if_list_head;            // interface list

    void                  *pd;                     // all PD specific state

} __PACK__ mc_entry_t;

// callback data structures
typedef struct mc_entry_create_app_ctxt_s {
    mc_key_t   key;
} __PACK__ mc_entry_create_app_ctxt_t;

static inline void multicast_entry_lock(mc_entry_t *mc_entry, const char *fname,
                                        int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking multicast_entry:{} from {}:{}:{}",
                    __FUNCTION__, mc_entry->hal_handle, fname, lineno, fxname);
    SDK_SPINLOCK_LOCK(&mc_entry->slock);
}

static inline void multicast_entry_unlock(mc_entry_t *mc_entry, const char *fname,
                                          int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking multicast_entry:{} from {}:{}:{}",
                    __FUNCTION__, mc_entry->hal_handle, fname, lineno, fxname);
    SDK_SPINLOCK_UNLOCK(&mc_entry->slock);
}

// allocate a multicast entry instance
static inline mc_entry_t *mc_entry_alloc (void)
{
    return (mc_entry_t *)g_hal_state->mc_entry_slab()->alloc();
}

// initialize a multicast entry instance
static inline mc_entry_t *mc_entry_init (mc_entry_t *mc_entry)
{
    if (!mc_entry) {
        return NULL;
    }

    SDK_SPINLOCK_INIT(&mc_entry->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    mc_entry->pd = NULL;
    mc_entry->hal_handle = 0;

    // initialize meta information
    sdk::lib::dllist_reset(&mc_entry->if_list_head);
    return mc_entry;
}

// allocate and initialize a multicast entry instance
static inline mc_entry_t *mc_entry_alloc_init (void)
{
    return mc_entry_init(mc_entry_alloc());
}

static inline hal_ret_t mc_entry_free (mc_entry_t *mc_entry)
{
    SDK_SPINLOCK_DESTROY(&mc_entry->slock);
    return hal::delay_delete_to_slab(HAL_SLAB_MC_ENTRY, mc_entry);
}

static inline mc_entry_t *
find_mc_entry_by_key (mc_key_t *mc_key)
{
    hal_handle_id_ht_entry_t    *entry;
    mc_entry_t                  *mc_entry;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->mc_key_ht()->lookup(mc_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_MC_ENTRY);
        mc_entry = (mc_entry_t *)hal_handle_get_obj(entry->handle_id);
        return mc_entry;
    }
    return NULL;
}

static inline mc_entry_t *
find_mc_entry_by_handle (hal_handle_t handle)
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
    if (hal_handle->obj_id() != HAL_OBJ_ID_MC_ENTRY) {
        HAL_TRACE_DEBUG("{}:failed to find l2seg with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    // SDK_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() ==
    //           HAL_OBJ_ID_TENANT);
   return (mc_entry_t *)hal_handle_get_obj(handle);
}

void *mc_entry_get_key_func(void *entry);
uint32_t mc_entry_key_size(void);
mc_entry_t *mc_entry_lookup_key_or_handle(const MulticastEntryKeyHandle& kh);
char *mc_key_to_string(mc_key_t *key);
#if 0
static inline oif_list_id_t 
mc_entry_shared_oifl(mc_entry_t *mc_entry) 
{
    return mc_entry->oif_list + 1;
}
#endif

static inline oif_list_id_t
mc_entry_mgmt_repls(oif_list_id_t base_oifl_id)
{
    return base_oifl_id;
}

static inline oif_list_id_t
mc_entry_mseg_bm_repls(oif_list_id_t base_oifl_id)
{
    return base_oifl_id + 1;
}

static inline oif_list_id_t
mc_entry_mgmt_mseg_bm_repls(oif_list_id_t base_oifl_id)
{
    return base_oifl_id + 2;
}

// SVC CRUD APIs
hal_ret_t multicastentry_create(MulticastEntrySpec& spec,
                                MulticastEntryResponse *rsp);
hal_ret_t multicastentry_update(MulticastEntrySpec& spec,
                                 MulticastEntryResponse *rsp);
hal_ret_t multicastentry_delete(MulticastEntryDeleteRequest& req,
                                MulticastEntryDeleteResponse *rsp);
hal_ret_t multicastentry_get(MulticastEntryGetRequest& req,
                             MulticastEntryGetResponseMsg *rsp);

}    // namespace hal

#endif    // __MULTICAST_HPP__

