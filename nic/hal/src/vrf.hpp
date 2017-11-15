#ifndef __vrf_HPP__
#define __vrf_HPP__

#include "nic/include/base.h"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal.hpp"
#include "nic/hal/src/nwsec.hpp"

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;
using kh::VrfKeyHandle;

using vrf::VrfSpec;
using vrf::VrfStatus;
using vrf::VrfResponse;
using vrf::VrfRequestMsg;
using vrf::VrfResponseMsg;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponse;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteResponseMsg;
using vrf::VrfGetRequest;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetResponse;
using vrf::VrfGetResponseMsg;

namespace hal {

typedef uint8_t vrf_hw_id_t;
typedef uint32_t vrf_id_t;

typedef struct vrf_s {
    hal_spinlock_t     slock;                // lock to protect this structure
    types::VrfType  vrf_type;          // type of the vrf
    vrf_id_t        vrf_id;            // app provided vrf id
    hal_handle_t       nwsec_profile_handle; // security profile handle
    ip_prefix_t        gipo_prefix;          // the prefix to terminate gipo
    // vrf?

    // operational state of vrf
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
} __PACK__ vrf_t;

typedef struct vrf_create_app_ctxt_s {
    nwsec_profile_t    *sec_prof;
} __PACK__ vrf_create_app_ctxt_t;

typedef struct vrf_update_app_ctxt_s {
    bool                nwsec_prof_change;

    // valid for nwsec_prof_change
    hal_handle_t        nwsec_profile_handle;   // new profile handle
    nwsec_profile_t    *nwsec_prof;             // new nwsec profile
} __PACK__ vrf_update_app_ctxt_t;


// max. number of VRFs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_VRFS                                 256

static inline void 
vrf_lock(vrf_t *vrf, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking vrf:{} from {}:{}:{}", 
                    __FUNCTION__, vrf->vrf_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&vrf->slock);
}

static inline void 
vrf_unlock(vrf_t *vrf, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking vrf:{} from {}:{}:{}", 
                    __FUNCTION__, vrf->vrf_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&vrf->slock);
}

// allocate a vrf instance
static inline vrf_t *
vrf_alloc (void)
{
    vrf_t    *vrf;

    vrf = (vrf_t *)g_hal_state->vrf_slab()->alloc();
    if (vrf == NULL) {
        return NULL;
    }
    return vrf;
}

// initialize a vrf instance
static inline vrf_t *
vrf_init (vrf_t *vrf)
{
    if (!vrf) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&vrf->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    vrf->hal_handle = HAL_HANDLE_INVALID;
    vrf->num_l2seg = 0;
    vrf->num_sg = 0;
    vrf->num_l4lb_svc = 0;
    vrf->num_ep = 0;
    vrf->pd = NULL;

    // initialize meta information
    // vrf->ht_ctxt.reset();
    utils::dllist_reset(&vrf->l2seg_list_head);
    utils::dllist_reset(&vrf->ep_list_head);
    utils::dllist_reset(&vrf->session_list_head);

    return vrf;
}

// allocate and initialize a vrf instance
static inline vrf_t *
vrf_alloc_init (void)
{
    return vrf_init(vrf_alloc());
}

// free vrf instance
static inline hal_ret_t
vrf_free (vrf_t *vrf)
{
    HAL_SPINLOCK_DESTROY(&vrf->slock);
    g_hal_state->vrf_slab()->free(vrf);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// find a vrf instance by its id
//------------------------------------------------------------------------------
static inline vrf_t *
vrf_lookup_by_id (vrf_id_t tid)
{
    hal_handle_id_ht_entry_t    *entry;
    vrf_t                    *vrf;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->vrf_id_ht()->lookup(&tid);
    if (entry) {

        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                HAL_OBJ_ID_VRF);

        vrf = (vrf_t *)hal_handle_get_obj(entry->handle_id);
        return vrf;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// find a vrf instance by its handle
//------------------------------------------------------------------------------
static inline vrf_t *
vrf_lookup_by_handle (hal_handle_t handle)
{
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_VRF) {
        HAL_TRACE_DEBUG("{}:failed to find vrf with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (vrf_t *)hal_handle->get_obj();
}

extern void *vrf_id_get_key_func(void *entry);
extern uint32_t vrf_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool vrf_id_compare_key_func(void *key1, void *key2);
vrf_t *vrf_lookup_key_or_handle (const VrfKeyHandle& kh);

hal_ret_t vrf_create(vrf::VrfSpec& spec,
                        vrf::VrfResponse *rsp);
hal_ret_t vrf_update(vrf::VrfSpec& spec,
                        vrf::VrfResponse *rsp);
hal_ret_t vrf_delete(vrf::VrfDeleteRequest& req,
                        vrf::VrfDeleteResponse *rsp);
hal_ret_t vrf_get(vrf::VrfGetRequest& req,
                     vrf::VrfGetResponse *rsp);
}    // namespace hal

#endif    // __vrf_HPP__

