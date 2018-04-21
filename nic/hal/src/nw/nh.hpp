// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NEXTHOP_HPP__
#define __NEXTHOP_HPP__

#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/nw.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/utils/block_list/block_list.hpp"

using nw::NexthopSpec;
using nw::NexthopResponse;
using nw::NexthopGetResponse;
using nw::NexthopGetResponseMsg;
using nw::NexthopGetRequest;
using nw::NexthopDeleteRequest;
using nw::NexthopDeleteResponse;
using kh::NexthopKeyHandle;
using hal::utils::block_list;

namespace hal {

// nexthop object
typedef struct nexthop_s {
    hal_spinlock_t    slock;                // lock to protect this structure
    nh_id_t           nh_id;                // next hop id
    hal_handle_t      hal_handle;           // HAL allocated handle

    hal_handle_t      if_handle;
    hal_handle_t      ep_handle;

    // back references
    block_list        *route_list;           // routes referring to this

} __PACK__ nexthop_t;

// max. number of networks supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NEXTHOPS                           256

// cb data structures
typedef struct nexthop_create_app_ctxt_s {
} __PACK__ nexthop_create_app_ctxt_t;

typedef struct nexthop_update_app_ctxt_s {
    bool nexthop_changed;
} __PACK__ nexthop_update_app_ctxt_t;

const char *nexthop_to_str (nexthop_t *nh);
static inline void
nexthop_lock (nexthop_t *nexthop, const char *fname, int lineno,
              const char *fxname)
{
    HAL_TRACE_DEBUG("Locking nexthop : {} from {} : {} : {}",
                    nexthop_to_str(nexthop),
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&nexthop->slock);
}

static inline void
nexthop_unlock (nexthop_t *nexthop, const char *fname, int lineno,
                const char *fxname)
{
    HAL_TRACE_DEBUG("Unlocking nexthop : {} from {} : {} : {}",
                    nexthop_to_str(nexthop),
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&nexthop->slock);
}

// allocate a nexthop instance
static inline nexthop_t *
nexthop_alloc (void)
{
    nexthop_t    *nexthop;

    nexthop = (nexthop_t *)g_hal_state->nexthop_slab()->alloc();
    if (nexthop == NULL) {
        return NULL;
    }
    return nexthop;
}

// initialize a nexthop instance
static inline nexthop_t *
nexthop_init (nexthop_t *nexthop)
{
    if (!nexthop) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&nexthop->slock, PTHREAD_PROCESS_SHARED);

    nexthop->nh_id      = 0;
    nexthop->hal_handle = HAL_HANDLE_INVALID;
    nexthop->if_handle  = HAL_HANDLE_INVALID;
    nexthop->ep_handle  = HAL_HANDLE_INVALID;
    nexthop->route_list = block_list::factory(sizeof(hal_handle_t));

    return nexthop;
}

// allocate and initialize a nexthop instance
static inline nexthop_t *
nexthop_alloc_init (void)
{
    return nexthop_init(nexthop_alloc());
}

// free nexthop instance
static inline hal_ret_t
nexthop_free (nexthop_t *nexthop)
{
    HAL_SPINLOCK_DESTROY(&nexthop->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NEXTHOP, nexthop);
    return HAL_RET_OK;
}

static inline hal_ret_t
nexthop_cleanup (nexthop_t *nexthop)
{
    block_list::destroy(nexthop->route_list);
    return nexthop_free(nexthop);
}

// find a nexthop instance by its id
static inline nexthop_t *
nexthop_lookup_by_id (nh_id_t id)
{
    hal_handle_id_ht_entry_t    *entry;
    nexthop_t                   *nh;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->nexthop_id_ht()->lookup(&id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {

        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_NEXTHOP);

        nh = (nexthop_t *)hal_handle_get_obj(entry->handle_id);
        return nh;
    }
    return NULL;
}

// find a nexthop instance by its handle
static inline nexthop_t *
nexthop_lookup_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_NEXTHOP) {
        HAL_TRACE_DEBUG("Failed to find nexthop with handle {}", handle);
        return NULL;
    }
    return (nexthop_t *)hal_handle->obj();
}

nexthop_t *nexthop_lookup_key_or_handle (const NexthopKeyHandle& kh);
const char *nexthop_lookup_key_or_handle_to_str (const NexthopKeyHandle& kh);

void *nexthop_id_get_key_func(void *entry);
uint32_t nexthop_id_compute_hash_func(void *key, uint32_t ht_size);
bool nexthop_id_compare_key_func(void *key1, void *key2);

}    // namespace hal

#endif    // __NEXTHOP_HPP__

