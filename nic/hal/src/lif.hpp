#ifndef __LIF_HPP__
#define __LIF_HPP__

#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/src/qos.hpp"

#define NUM_MAX_COSES 16
// Size of RSS seed in bytes
#define ETH_RSS_KEY_LENGTH          40

using intf::LifSpec;
using intf::LifResponse;
using intf::LifDeleteRequest;
using intf::LifDeleteResponse;
using intf::LifGetRequest;
using intf::LifGetResponse;

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;
using hal::utils::bitmap;

namespace hal {

// typedef struct if_s if_t;

// PKtFilter structure
typedef struct pkt_filter_s {
    bool    receive_broadcast;     // Receive Broadcast
    bool    receive_all_multicast; // Receive all Multicast
    bool    receive_promiscuous;   // Receive Unknown Unicast, Broadcast, Multicast. Not Known Unicast
} __PACK__ pkt_filter_t;

// Lif RSS config structure
typedef struct lif_rss_info_s {
    bool        enable;
    int         type;
    uint8_t     key[ETH_RSS_KEY_LENGTH];
} __PACK__ lif_rss_info_t;

// Lif queue info structure
typedef struct lif_queue_info_s {
    uint8_t     type;        // hardware queue type of queue
    uint16_t    size;        // size of qstate
    uint16_t    num_queues;  // number of queues
} __PACK__ lif_queue_info_t;

// LIF structure
typedef struct lif_s {
    hal_spinlock_t      slock;           // lock to protect this structure
    lif_id_t            lif_id;          // lif id assigned
    intf::IfStatus      admin_status;    // admin status
    bool                vlan_strip_en;   // vlan strip enable
    bool                vlan_insert_en;  // if en, ingress vlan is in p4plus_to_p4 dr
    hal_handle_t        pinned_uplink;   // uplink this LIF is pinned to
    bool                enable_rdma;     // enable rdma on this LIF
    uint32_t            rdma_max_keys;
    uint32_t            rdma_max_pt_entries;
    lif_queue_info_t    qinfo[intf::LifQPurpose_MAX+1]; // purpose to qtype mapping
    uint16_t            cos_bmp;                     // bitmap of COS values supported by this LIF.
    bool                qstate_init_done;            // qstate map init status.
    pkt_filter_t        packet_filters;              // Packet Filter Modes
    lif_rss_info_t      rss;                         // rss enable
    policer_t           rx_policer;      // Rx policer
    policer_t           tx_policer;      // Tx policer

    // operational state of interface
    hal_handle_t        hal_handle;      // HAL allocated handle

    // back references to enic ifs
    dllist_ctxt_t       if_list_head;    // interfaces (enics) behind this lif

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
    LifSpec      *spec;
    LifResponse  *rsp;
    bool         vlan_strip_en;
    bool         qstate_map_init_set;
    uint64_t     vlan_strip_en_changed:1;
    uint64_t     pinned_uplink_changed:1;
    uint64_t     rx_policer_changed:1;
    uint64_t     tx_policer_changed:1;
    hal_handle_t new_pinned_uplink;
    bool         rss_config_changed;
} __PACK__ lif_update_app_ctxt_t;

#define HAL_MAX_LIFS                                 1024

static inline void 
lif_lock (lif_t *lif, const char *fname,
          int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking lif:{} from {}:{}:{}", 
                    __FUNCTION__, lif->lif_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&lif->slock);
}

static inline void 
lif_unlock (lif_t *lif, const char *fname,
            int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking lif:{} from {}:{}:{}", 
                    __FUNCTION__, lif->lif_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&lif->slock);
}

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
    lif->hal_handle    = HAL_HANDLE_INVALID;
    lif->pinned_uplink = HAL_HANDLE_INVALID;

    // initialize meta information
    sdk::lib::dllist_reset(&lif->if_list_head);

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
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_LIF) {
        HAL_TRACE_DEBUG("{}:failed to find lif with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (lif_t *)hal_handle->get_obj();
}

extern void *lif_id_get_key_func(void *entry);
extern uint32_t lif_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool lif_id_compare_key_func(void *key1, void *key2);
hal_ret_t lif_handle_vlan_strip_en_update (lif_t *lif, bool vlan_strip_en);
void lif_print_ifs(lif_t *lif);
void lif_print(lif_t *lif);


void LifGetQState(const intf::QStateGetReq &req, intf::QStateGetResp *resp);
void LifSetQState(const intf::QStateSetReq &req, intf::QStateSetResp *resp);

// SVC APIs
hal_ret_t lif_create(LifSpec& spec, LifResponse *rsp, 
                     lif_hal_info_t *lif_hal_info);
hal_ret_t lif_update(LifSpec& spec, LifResponse *rsp);
hal_ret_t lif_delete(LifDeleteRequest& req,
                     LifDeleteResponse *rsp);
hal_ret_t lif_get(LifGetRequest& req, LifGetResponse *rsp);

}    // namespace hal

#endif    // __LIF_HPP__

