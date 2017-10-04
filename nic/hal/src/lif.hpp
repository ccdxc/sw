#ifndef __LIF_HPP__
#define __LIF_HPP__

#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/proto/hal/interface.pb.h"

using intf::LifSpec;
using intf::LifResponse;
using intf::LifDeleteRequest;
using intf::LifDeleteResponseMsg;
using intf::LifGetRequest;
using intf::LifGetResponse;

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;
using hal::utils::bitmap;

namespace hal {

typedef uint32_t lif_id_t;

// LIF structure
// TODO: capture Q information etc.
typedef struct lif_s {
    hal_spinlock_t      slock;                       // lock to protect this structure
    lif_id_t            lif_id;                      // lif id assigned
    mac_addr_t          mac_addr;                    // LIF's MAC address, if any
    uint32_t            port_num;                    // LIF's port number
    intf::IfStatus      admin_status;                // admin status
    bool                vlan_strip_en;               // vlan strip enable
    //bool                allmulti;                    // All multicast enable
    bool                enable_rdma;                 // enable rdma on this LIF
    uint8_t             qtypes[intf::LifQPurpose_MAX+1]; // purpose to qtype mapping

    // operational state of interface
    hal_handle_t        hal_handle;                  // HAL allocated handle

    // dllist_ctxt_t       if_list_head;                // interfaces behind this lif

    void                *pd_lif;
} __PACK__ lif_t;

typedef struct lif_hal_info_s {
    bool        with_hw_lif_id;
    uint32_t    hw_lif_id;
} lif_hal_info_t;

typedef struct lif_create_app_ctxt_s {
    lif_hal_info_t      *lif_info;
    LifSpec             *spec;
    LifResponse         *rsp;
    uint32_t            hw_lif_id;
} __PACK__ lif_create_app_ctxt_t;

typedef struct lif_update_app_ctxt_s {
    lif_t               *lif;
    LifSpec             *spec;
    LifResponse         *rsp;
} __PACK__ lif_update_app_ctxt_t;

#define HAL_MAX_LIFS                                 1024

// allocate a LIF instance
static inline lif_t *
lif_alloc (void)
{
    lif_t    *lif;

    lif = (lif_t *)g_hal_state->lif_slab()->alloc();
    if (lif == NULL) {
        return NULL;
    }
    return lif;
}

// initialize a LIF instance
static inline lif_t *
lif_init (lif_t *lif)
{
    if (!lif) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&lif->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    lif->hal_handle = HAL_HANDLE_INVALID;

    // initialize meta information
    // utils::dllist_reset(&lif->if_list_head);

    return lif;
}

// allocate and initialize a interface instance
static inline lif_t *
lif_alloc_init (void)
{
    return lif_init(lif_alloc());
}

static inline hal_ret_t
lif_free (lif_t *lif)
{
    HAL_SPINLOCK_DESTROY(&lif->slock);
    g_hal_state->lif_slab()->free(lif);
    return HAL_RET_OK;
}

static inline lif_t *
find_lif_by_id (lif_id_t lif_id)
{
    hal_handle_id_ht_entry_t    *entry;
    lif_t                       *lif;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->lif_id_ht()->lookup(&lif_id);
    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_LIF);
        lif = (lif_t *)hal_handle_get_obj(entry->handle_id);
        return lif;
    }
    return NULL;
}

static inline lif_t *
find_lif_by_handle (hal_handle_t handle)
{
    // check for object type
    HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
               HAL_OBJ_ID_LIF);
    return (lif_t *)hal_handle_get_obj(handle);
}

extern void *lif_id_get_key_func(void *entry);
extern uint32_t lif_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool lif_id_compare_key_func(void *key1, void *key2);

void LifGetQState(const intf::QStateGetReq &req, intf::QStateGetResp *resp);
void LifSetQState(const intf::QStateSetReq &req, intf::QStateSetResp *resp);

hal_ret_t lif_create(LifSpec& spec, LifResponse *rsp, 
                     lif_hal_info_t *lif_hal_info);
hal_ret_t lif_update(LifSpec& spec, LifResponse *rsp);
hal_ret_t lif_delete(LifDeleteRequest& req,
                     LifDeleteResponseMsg *rsp);
hal_ret_t lif_get(LifGetRequest& req,
                  LifGetResponse *rsp);
}    // namespace hal

#endif    // __LIF_HPP__

