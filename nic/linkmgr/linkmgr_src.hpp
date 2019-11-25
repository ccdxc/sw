// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_SRC_HPP__
#define __LINKMGR_SRC_HPP__

#include "lib/ht/ht.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "lib/thread/thread.hpp"
#include "lib/list/list.hpp"
#include "linkmgr.hpp"
#include "linkmgr_svc.hpp"
#include "linkmgr_debug_svc.hpp"
#include "linkmgr_state.hpp"
#include "gen/proto/linkmgr/mac_metrics.delphi.hpp"

using hal::hal_handle_id_ht_entry_t;
using hal::hal_handle_get_from_handle_id;
using hal::hal_handle_get_obj;
using hal::HAL_OBJ_ID_PORT;
using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

namespace linkmgr {

extern linkmgr_state *g_linkmgr_state;

typedef uint32_t port_num_t;

typedef struct port_ht_cb_ctxt_s {
    void          *ctxt;
    port_get_cb_t cb;
} port_ht_cb_ctxt_t;

typedef struct xcvr_event_port_ctxt_s {
    uint32_t     port_num;
    xcvr_state_t state;
} xcvr_event_port_ctxt_t;

typedef struct linkmgr_cfg_s {
    std::string        cfg_file;
    std::string        catalog_file;
    std::string        grpc_port;
    platform_type_t    platform_type;
} linkmgr_cfg_t;

typedef struct port_s {
    sdk_spinlock_t                 slock;            // lock to protect this structure

    port_num_t                     port_num;         // uplink port number

                                                     // operational state of port
    hal_handle_t                   hal_handle_id;    // HAL allocated handle

    delphi::objects::MacMetricsPtr mac_metrics;      // Delphi mac metrics

                                                     // PD state
    void                           *pd_p;            // all PD specific state
} port_t;

static inline void
port_lock(port_t *pi_p, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("operlock:locking port:{} from {}:{}:{}", 
                     pi_p-> port_num,
                    fname, lineno, fxname);
    SDK_SPINLOCK_LOCK(&pi_p->slock);
}

static inline void
port_unlock(port_t *pi_p, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("operlock:unlocking port:{} from {}:{}:{}", 
                     pi_p->port_num,
                    fname, lineno, fxname);
    SDK_SPINLOCK_UNLOCK(&pi_p->slock);
}

// allocate a port instance
static inline port_t *
port_alloc (void)
{
    return (port_t *)g_linkmgr_state->port_slab()->alloc();
}

// initialize a port instance
static inline port_t *
port_init (port_t *pi_p)
{
    if (!pi_p) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&pi_p->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    pi_p->pd_p = NULL;

    return pi_p;
}

// allocate and initialize a port instance
static inline port_t *
port_alloc_init (void)
{
    return port_init(port_alloc());
}

static inline hal_ret_t
port_free (port_t *pi_p)
{
    SDK_SPINLOCK_DESTROY(&pi_p->slock);
    g_linkmgr_state->port_slab()->free(pi_p);
    return HAL_RET_OK;
}

static inline port_t *
find_port_by_id (port_num_t port_num)
{
    hal_handle_id_ht_entry_t   *entry;
    port_t                     *pi_p= NULL;

    if (g_linkmgr_state == NULL || g_linkmgr_state->port_id_ht() == NULL) {
        return pi_p;
    }
    entry = (hal_handle_id_ht_entry_t *)
            g_linkmgr_state->port_id_ht()->lookup(&port_num);

    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(
                entry->handle_id)->obj_id() == HAL_OBJ_ID_PORT);
        pi_p = (port_t *)hal_handle_get_obj(entry->handle_id);
        return pi_p;
    }

    return NULL;
}

static inline port_t *
find_port_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("failed to find object with handle: {}",
                         handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_PORT) {
        HAL_TRACE_DEBUG("failed to find port with handle: {}",
                         handle);
        return NULL;
    }
    return (port_t *)hal_handle->obj();
}

void     *port_id_get_key_func(void *entry);
uint32_t port_id_key_size(void);

hal_ret_t linkmgr_global_init(linkmgr_cfg_t*);
sdk::lib::thread *current_thread(void);

}    // namespace linkmgr

#endif    // __LINKMGR_SRC_HPP__

