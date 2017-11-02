#ifndef __TENANT_HPP__
#define __TENANT_HPP__

#include "nic/include/base.h"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/tenant.pb.h"
#include "nic/gen/proto/hal/key_handles.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal.hpp"
#include "nic/hal/src/nwsec.hpp"

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;
using key_handles::TenantKeyHandle;

using tenant::TenantSpec;
using tenant::TenantStatus;
using tenant::TenantResponse;
using tenant::TenantRequestMsg;
using tenant::TenantResponseMsg;
using tenant::TenantDeleteRequest;
using tenant::TenantDeleteResponse;
using tenant::TenantDeleteRequestMsg;
using tenant::TenantDeleteResponseMsg;
using tenant::TenantGetRequest;
using tenant::TenantGetRequestMsg;
using tenant::TenantGetResponse;
using tenant::TenantGetResponseMsg;

namespace hal {

typedef uint8_t tenant_hw_id_t;
typedef uint32_t tenant_id_t;

typedef struct tenant_s {
    hal_spinlock_t     slock;                // lock to protect this structure
    types::TenantType  tenant_type;          // type of the tenant
    tenant_id_t        tenant_id;            // app provided tenant id
    hal_handle_t       nwsec_profile_handle; // security profile handle
    ip_prefix_t        gipo_prefix;          // the prefix to terminate gipo
    // tenant?

    // operational state of tenant
    hal_handle_t       hal_handle;           // HAL allocated handle
    uint32_t           num_l2seg;            // no. of L2 segments
    uint32_t           num_sg;               // no. of security groups
    uint32_t           num_l4lb_svc;         // no. of L4 LB services
    uint32_t           num_ep;               // no. of endpoints
    // Back references
    dllist_ctxt_t      l2seg_list_head;      // L2 segment list

    // TODO: Check 
    dllist_ctxt_t      ep_list_head;         // endpoint list
    dllist_ctxt_t      session_list_head;    // session list

    // PD state
    void               *pd;                  // all PD specific state
} __PACK__ tenant_t;

typedef struct tenant_create_app_ctxt_s {
    nwsec_profile_t    *sec_prof;
} __PACK__ tenant_create_app_ctxt_t;

typedef struct tenant_update_app_ctxt_s {
    bool                nwsec_prof_change;

    // valid for nwsec_prof_change
    hal_handle_t        nwsec_profile_handle;   // new profile handle
    nwsec_profile_t    *nwsec_prof;             // new nwsec profile
} __PACK__ tenant_update_app_ctxt_t;


// max. number of VRFs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_VRFS                                 256

static inline void 
tenant_lock(tenant_t *tenant, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking tenant:{} from {}:{}:{}", 
                    __FUNCTION__, tenant->tenant_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&tenant->slock);
}

static inline void 
tenant_unlock(tenant_t *tenant, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking tenant:{} from {}:{}:{}", 
                    __FUNCTION__, tenant->tenant_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&tenant->slock);
}

// allocate a tenant instance
static inline tenant_t *
tenant_alloc (void)
{
    tenant_t    *tenant;

    tenant = (tenant_t *)g_hal_state->tenant_slab()->alloc();
    if (tenant == NULL) {
        return NULL;
    }
    return tenant;
}

// initialize a tenant instance
static inline tenant_t *
tenant_init (tenant_t *tenant)
{
    if (!tenant) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&tenant->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    tenant->hal_handle = HAL_HANDLE_INVALID;
    tenant->num_l2seg = 0;
    tenant->num_sg = 0;
    tenant->num_l4lb_svc = 0;
    tenant->num_ep = 0;
    tenant->pd = NULL;

    // initialize meta information
    // tenant->ht_ctxt.reset();
    utils::dllist_reset(&tenant->l2seg_list_head);
    utils::dllist_reset(&tenant->ep_list_head);
    utils::dllist_reset(&tenant->session_list_head);

    return tenant;
}

// allocate and initialize a tenant instance
static inline tenant_t *
tenant_alloc_init (void)
{
    return tenant_init(tenant_alloc());
}

// free tenant instance
static inline hal_ret_t
tenant_free (tenant_t *tenant)
{
    HAL_SPINLOCK_DESTROY(&tenant->slock);
    g_hal_state->tenant_slab()->free(tenant);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// find a tenant instance by its id
//------------------------------------------------------------------------------
static inline tenant_t *
tenant_lookup_by_id (tenant_id_t tid)
{
    hal_handle_id_ht_entry_t    *entry;
    tenant_t                    *tenant;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->tenant_id_ht()->lookup(&tid);
    if (entry) {

        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                HAL_OBJ_ID_TENANT);

        tenant = (tenant_t *)hal_handle_get_obj(entry->handle_id);
        return tenant;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// find a tenant instance by its handle
//------------------------------------------------------------------------------
static inline tenant_t *
tenant_lookup_by_handle (hal_handle_t handle)
{
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_TENANT) {
        HAL_TRACE_DEBUG("{}:failed to find tenant with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (tenant_t *)hal_handle->get_obj();
    // HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
    //           HAL_OBJ_ID_TENANT);
   // return (tenant_t *)hal_handle_get_obj(handle); 
}

extern void *tenant_id_get_key_func(void *entry);
extern uint32_t tenant_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool tenant_id_compare_key_func(void *key1, void *key2);
tenant_t *tenant_lookup_key_or_handle (const TenantKeyHandle& kh);

hal_ret_t tenant_create(tenant::TenantSpec& spec,
                        tenant::TenantResponse *rsp);
#if 0
hal_ret_t tenant_create_v2(tenant::TenantSpec& spec,
                           tenant::TenantResponse *rsp);
#endif
hal_ret_t tenant_update(tenant::TenantSpec& spec,
                        tenant::TenantResponse *rsp);
hal_ret_t tenant_delete(tenant::TenantDeleteRequest& req,
                        tenant::TenantDeleteResponse *rsp);
hal_ret_t tenant_get(tenant::TenantGetRequest& req,
                     tenant::TenantGetResponse *rsp);
#if 0
hal_ret_t tenant_get_v2(tenant::TenantGetRequest& req,
                        tenant::TenantGetResponse *rsp);
#endif

}    // namespace hal

#endif    // __TENANT_HPP__

