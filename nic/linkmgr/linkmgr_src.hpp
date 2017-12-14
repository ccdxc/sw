// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_SRC_HPP__
#define __LINKMGR_SRC_HPP__

#include "nic/sdk/include/ht.hpp"

#include "nic/include/base.h"
#include "nic/include/list.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_cfg.hpp"

#include "nic/gen/proto/hal/port.pb.h"

#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/src/utils.hpp"
#include "linkmgr_state.hpp"

using port::PortDeleteRequest;
using port::PortDeleteResponseMsg;
using port::PortSpec;
using port::PortOperStatus;
using port::PortType;
using port::PortSpeed;
using port::PortAdminState;
using port::PortResponse;
using port::PortRequestMsg;
using port::PortResponseMsg;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;
using port::PortGetRequest;
using port::PortGetRequestMsg;
using port::PortGetResponse;
using port::PortGetResponseMsg;

using hal::hal_handle_id_ht_entry_t;
using hal::hal_handle_get_from_handle_id;
using hal::hal_handle_get_obj;
using sdk::lib::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

namespace linkmgr {

extern linkmgr_state *g_linkmgr_state;

typedef uint32_t port_num_t;

typedef struct linkmgr_cfg_s {
    std::string                 grpc_port;
    hal::utils::platform_type_t platform_type;
} linkmgr_cfg_t;

typedef struct port_s {
    hal_spinlock_t   slock;           // lock to protect this structure

    port_num_t       port_num;        // port number

    // operational state of port
    hal_handle_t     hal_handle_id;   // HAL allocated handle

    // PD state
    void             *pd_p;           // all PD specific state
} __PACK__ port_t;

// CB data structures
typedef struct port_create_app_ctxt_s {
    PortType         port_type;       // port type
    PortSpeed        port_speed;      // port speed
    PortAdminState   admin_state;     // admin state of the port
    uint32_t         mac_id;          // mac id associated with the port
    uint32_t         mac_ch;          // mac channel associated with the port
    uint32_t         num_lanes;       // number of lanes for the port
} __PACK__ port_create_app_ctxt_t;

typedef port_create_app_ctxt_t port_update_app_ctxt_t;

static inline void
port_lock(port_t *pi_p, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking port:{} from {}:{}:{}", 
                    __FUNCTION__, pi_p-> port_num,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&pi_p->slock);
}

static inline void
port_unlock(port_t *pi_p, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking port:{} from {}:{}:{}", 
                    __FUNCTION__, pi_p->port_num,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&pi_p->slock);
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
    HAL_SPINLOCK_INIT(&pi_p->slock, PTHREAD_PROCESS_PRIVATE);

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
    HAL_SPINLOCK_DESTROY(&pi_p->slock);
    g_linkmgr_state->port_slab()->free(pi_p);
    return HAL_RET_OK;
}

static inline port_t *
find_port_by_id (port_num_t port_num)
{
    hal_handle_id_ht_entry_t   *entry;
    port_t                     *pi_p;

    entry = (hal_handle_id_ht_entry_t *)
            g_linkmgr_state->port_id_ht()->lookup(&port_num);

    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(
                entry->handle_id)->obj_id() == hal::HAL_OBJ_ID_PORT);
        pi_p = (port_t *)hal_handle_get_obj(entry->handle_id);
        return pi_p;
    }

    return NULL;
}

static inline port_t *
find_port_by_handle (hal_handle_t handle)
{
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != hal::HAL_OBJ_ID_PORT) {
        HAL_TRACE_DEBUG("{}:failed to find port with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (port_t *)hal_handle->get_obj();
}

extern void *port_id_get_key_func(void *entry);
extern uint32_t port_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool port_id_compare_key_func(void *key1, void *key2);

// SVC CRUD APIs
hal_ret_t port_create(PortSpec& spec,
                           PortResponse *rsp);
hal_ret_t port_update(PortSpec& spec,
                           PortResponse *rsp);
hal_ret_t port_delete(PortDeleteRequest& req,
                           PortDeleteResponseMsg *rsp);
hal_ret_t port_get(port::PortGetRequest& req,
                        port::PortGetResponse *rsp);

hal_ret_t linkmgr_init();

hal::utils::thread* current_thread();

hal_ret_t port_event_timer(void *ctxt);
hal_ret_t port_event_enable(void *ctxt);
hal_ret_t port_event_disable(void *ctxt);

}    // namespace linkmgr

#endif    // __LINKMGR_SRC_HPP__

