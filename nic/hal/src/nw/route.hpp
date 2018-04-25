//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __ROUTE_HPP__
#define __ROUTE_HPP__

#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/nw.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/utils/block_list/block_list.hpp"

using nw::RouteSpec;
using nw::RouteResponse;
using nw::RouteDeleteRequest;
using nw::RouteDeleteResponse;
using nw::RouteGetRequest;
using nw::RouteGetResponseMsg;
using nw::RouteGetResponse;
using kh::RouteKeyHandle;
using hal::utils::block_list;

namespace hal {

// route key
typedef struct route_key_s {
    vrf_id_t    vrf_id;     // VRF id
    ip_prefix_t pfx;        // IP prefix
} __PACK__ route_key_t;

// route object
typedef struct route_s {
    hal_spinlock_t    slock;                // lock to protect this structure
    route_key_t       key;                  // route key
    hal_handle_t      hal_handle;           // HAL allocated handle

    hal_handle_t      nh_handle;            // nexthop handle
    // TODO: Interface instead of nh??
} __PACK__ route_t;

// max. number of routes supported  (TODO: we can take this from cfg file)
#define HAL_MAX_ROUTES 256

// cb data structures
typedef struct route_create_app_ctxt_s {
} __PACK__ route_create_app_ctxt_t;

typedef struct route_update_app_ctxt_s {
    bool route_changed;
} __PACK__ route_update_app_ctxt_t;

const char *route_to_str (route_t *route);
static inline void
route_lock (route_t *route, const char *fname, int lineno,
              const char *fxname)
{
    HAL_TRACE_DEBUG("Locking route : {} from {} : {} : {}",
                    route_to_str(route),
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&route->slock);
}

static inline void
route_unlock (route_t *route, const char *fname, int lineno,
                const char *fxname)
{
    HAL_TRACE_DEBUG("Unlocking route : {} from {} : {} : {}",
                    route_to_str(route),
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&route->slock);
}

// allocate a route instance
static inline route_t *
route_alloc (void)
{
    route_t    *route;

    route = (route_t *)g_hal_state->route_slab()->alloc();
    if (route == NULL) {
        return NULL;
    }
    return route;
}

// initialize a route instance
static inline route_t *
route_init (route_t *route)
{
    if (!route) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&route->slock, PTHREAD_PROCESS_SHARED);

    route->hal_handle = HAL_HANDLE_INVALID;
    route->nh_handle  = HAL_HANDLE_INVALID;

    return route;
}

// allocate and initialize a route instance
static inline route_t *
route_alloc_init (void)
{
    return route_init(route_alloc());
}

// Prefer to call this only when freeing up original
static inline hal_ret_t
route_free (route_t *route)
{
    HAL_SPINLOCK_DESTROY(&route->slock);
    hal::delay_delete_to_slab(HAL_SLAB_ROUTE, route);
    return HAL_RET_OK;
}

// Complete cleanup
static inline hal_ret_t
route_cleanup (route_t *route)
{
    // destory block lists

    return route_free(route);
}

// find a route instance by its id
static inline route_t *
route_lookup_by_key (route_key_t *key)
{
    hal_handle_id_ht_entry_t   *entry;
    route_t                    *route;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->route_ht()->lookup(key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_ROUTE);

        route = (route_t *)hal_handle_get_obj(entry->handle_id);
        return route;
    }
    return NULL;
}

// find a route instance by its handle
static inline route_t *
route_lookup_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_ROUTE) {
        HAL_TRACE_DEBUG("Failed to find route with handle {}", handle);
        return NULL;
    }
    return (route_t *)hal_handle->obj();
}

void *route_get_key_func(void *entry);
uint32_t route_compute_hash_func(void *key, uint32_t ht_size);
bool route_compare_key_func(void *key1, void *key2);


hal_ret_t route_clean_handle_mapping (hal_handle_t route_handle);
}    // namespace hal

#endif    // __ROUTE_HPP__

