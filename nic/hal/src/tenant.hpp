#ifndef __TENANT_HPP__
#define __TENANT_HPP__

#include <base.h>
#include <list.hpp>
#include <ht.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <tenant.pb.h>
#include <pd.hpp>

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

namespace hal {

typedef uint8_t tenant_hw_id_t;
typedef uint32_t tenant_id_t;

typedef struct tenant_s {
    hal_spinlock_t     slock;                // lock to protect this structure
    tenant_id_t        tenant_id;            // app provided tenant id
    hal_handle_t       nwsec_profile_handle; // security profile handle

    // operational state of tenant
    hal_handle_t       hal_handle;           // HAL allocated handle
    uint32_t           num_l2seg;            // no. of L2 segments
    uint32_t           num_sg;               // no. of security groups
    uint32_t           num_l4lb_svc;         // no. of L4 LB services
    uint32_t           num_ep;               // no. of endpoints

    // PD state
    void               *pd;                  // all PD specific state

    // meta data maintained for tenant
    ht_ctxt_t          ht_ctxt;              // tenant id based hash table ctxt
    ht_ctxt_t          hal_handle_ht_ctxt;   // hal handle based hash table ctxt
    dllist_ctxt_t      l2seg_list_head;      // L2 segment list
    dllist_ctxt_t      ep_list_head;         // endpoint list
    dllist_ctxt_t      session_list_head;    // session list
} __PACK__ tenant_t;

// max. number of VRFs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_VRFS                                 256

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
    tenant->num_l2seg = 0;
    tenant->num_sg = 0;
    tenant->num_l4lb_svc = 0;
    tenant->num_ep = 0;
    tenant->pd = NULL;

    // initialize meta information
    tenant->ht_ctxt.reset();
    tenant->hal_handle_ht_ctxt.reset();
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

// insert this tenant in all meta data structures
static inline hal_ret_t
add_tenant_to_db (tenant_t *tenant)
{
    g_hal_state->tenant_hal_handle_ht()->insert(tenant,
                                                &tenant->hal_handle_ht_ctxt);
    g_hal_state->tenant_id_ht()->insert(tenant, &tenant->ht_ctxt);

    return HAL_RET_OK;
}
 
// find a tenant instance by its id
static inline tenant_t *
find_tenant_by_id (tenant_id_t tid)
{
    return (tenant_t *)g_hal_state->tenant_id_ht()->lookup(&tid);
}

// find a tenant instance by its handle
static inline tenant_t *
find_tenant_by_handle (hal_handle_t handle)
{
    return (tenant_t *)g_hal_state->tenant_hal_handle_ht()->lookup(&handle);
}

extern void *tenant_get_key_func(void *entry);
extern uint32_t tenant_compute_hash_func(void *key, uint32_t ht_size);
extern bool tenant_compare_key_func(void *key1, void *key2);

extern void *tenant_get_handle_key_func(void *entry);
extern uint32_t tenant_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tenant_compare_handle_key_func(void *key1, void *key2);

hal_ret_t tenant_create(tenant::TenantSpec& spec,
                        tenant::TenantResponse *rsp);
hal_ret_t tenant_update(tenant::TenantSpec& spec,
                        tenant::TenantResponse *rsp);
hal_ret_t tenant_delete(tenant::TenantDeleteRequest& req,
                        tenant::TenantDeleteResponseMsg *rsp);
hal_ret_t tenant_get(tenant::TenantGetRequest& req,
                     tenant::TenantGetResponse *rsp);

}    // namespace hal

#endif    // __TENANT_HPP__

