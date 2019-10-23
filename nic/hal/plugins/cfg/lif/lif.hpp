//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __LIF_HPP__
#define __LIF_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/platform/utils/lif_mgr/lif_mgr.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "lib/bitmap/bitmap.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/sdk/platform/utils/program.hpp"
#include "gen/proto/event.pb.h"

#define NUM_MAX_COSES 16
// Size of RSS seed in bytes
#define ETH_RSS_KEY_LENGTH          40
// Number of RSS indirection entries
#define ETH_RSS_INDIR_LENGTH        128

#define LIF_NAME_LEN 32

#define LIF_ID_INVALID      0xFFFFFFFF

using intf::LifSpec;
using intf::LifResponse;
using intf::LifDeleteRequest;
using intf::LifDeleteResponse;
using intf::LifGetRequest;
using intf::LifGetResponse;
using intf::LifGetResponseMsg;

using event::EventResponse;

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

namespace hal {

// typedef struct if_s if_t;

// PKtFilter structure
typedef struct pkt_filter_s {
    bool    receive_broadcast;     // Receive Broadcast
    bool    receive_all_multicast; // Receive all Multicast
    bool    receive_promiscuous;   // Receive Unknown Unicast, Broadcast, Multicast. Not Known Unicast
} __PACK__ pkt_filter_t;

typedef struct lif_cos_info_s {
    uint16_t            cos_bmp;         // bitmap of COS values for Tx traffic supported on this LIF.
    uint8_t             coses;           // cos values supported by LIF. cosA - Bits 0-4, cosB - Bits 4-7.
    hal_handle_t        rx_qos_class_handle; // qos-class-hdl for Rx traffic on this LIF.
    hal_handle_t        tx_qos_class_handle; // qos-class-hdl for Tx traffic on this LIF.
    policer_t           rx_policer;      // Rx policer
    policer_t           tx_policer;      // Tx policer
} __PACK__ lif_qos_info_t;

// Lif RSS config structure
typedef struct lif_rss_info_s {
    uint32_t    type;
    uint8_t     key[ETH_RSS_KEY_LENGTH];
    uint8_t     indir[ETH_RSS_INDIR_LENGTH];
} __PACK__ lif_rss_info_t;

// Lif queue info structure
typedef struct lif_queue_info_s {
    uint8_t     type;        // hardware queue type of queue
    uint16_t    size;        // size of qstate
    uint32_t    num_queues;  // number of queues
} __PACK__ lif_queue_info_t;

// LIF structure
typedef struct lif_s {
    sdk_spinlock_t      slock;           // lock to protect this structure
    lif_id_t            lif_id;          // lif id assigned
    char                name[LIF_NAME_LEN]; // LIF's name
    types::LifType      type;            // lif type
    intf::IfStatus      admin_status;    // admin status
    bool                vlan_strip_en;   // vlan strip enable
    bool                vlan_insert_en;  // if en, ingress vlan is in p4plus_to_p4 dr
    bool                is_management;   // set for OOB MNIC, int_mgmt MNIC and host management NIC
    hal_handle_t        pinned_uplink;   // uplink this LIF is pinned to
    bool                enable_rdma;     // enable rdma on this LIF
    uint32_t            rdma_max_keys;
    uint32_t            rdma_max_ahs;
    uint32_t            rdma_max_pt_entries;
    bool                enable_nvme;     // enable NVME on this LIF
    uint32_t            nvme_max_ns;     // maximum number of namespaces on this LIF
    uint32_t            nvme_max_sess;   // maximum number of sessions on this LIF
    lif_queue_info_t    qinfo[intf::LifQPurpose_MAX+1]; // purpose to qtype mapping
    lif_qos_info_t      qos_info;
    bool                qstate_init_done;// qstate map init status.
    pkt_filter_t        packet_filters;  // Packet Filter Modes
    lif_rss_info_t      rss;             // rss configuration
    uint32_t            qcount;
    bool                qstate_pgm_in_hal;

    // operational state of interface
    hal_handle_t        hal_handle;      // HAL allocated handle

    // back references to enic ifs
    dllist_ctxt_t       if_list_head;    // interfaces (enics) behind this lif
    block_list          *filter_list;    // filter list

    void                *pd_lif;
} __PACK__ lif_t;

typedef struct lif_get_from_hw_lif_id_ctxt_s {
    uint32_t hw_lif_id;
    lif_t *lif;
} lif_get_from_hw_lif_id_ctxt_t;

typedef struct lif_get_from_name_ctxt_s {
    char *name;
    lif_t *lif;
} lif_get_from_name_ctxt_t;

typedef struct lif_hal_info_s {
    bool        with_hw_lif_id;
    uint32_t    hw_lif_id;
    bool        dont_zero_qstate_mem;
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
    bool         vlan_insert_en_changed:1;
    uint64_t     pkt_filter_prom_changed:1;
    uint64_t     pkt_filter_bcast_changed:1;
    uint64_t     pkt_filter_allmc_changed:1;
    uint64_t     receive_promiscous:1;
    uint64_t     receive_broadcast:1;
    uint64_t     receive_all_multicast:1;
    uint64_t     vlan_strip_en_changed:1;
    uint64_t     pinned_uplink_changed:1;
    uint64_t     rx_policer_changed:1;
    uint64_t     tx_policer_changed:1;
    uint64_t     vlan_insert_en;
    hal_handle_t new_pinned_uplink;
    bool         rss_config_changed;
    bool         name_changed:1;
    bool         status_changed:1;
} __PACK__ lif_update_app_ctxt_t;

typedef struct lif_sched_control_cb_ctxt_s {
    bool en;
} __PACK__ lif_sched_control_cb_ctxt_t;

#define HAL_MAX_LIFS                                 1024

static inline void
lif_lock (lif_t *lif, const char *fname,
          int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking lif:{} from {}:{}:{}",
                    __FUNCTION__, lif->lif_id,
                    fname, lineno, fxname);
    SDK_SPINLOCK_LOCK(&lif->slock);
}

static inline void
lif_unlock (lif_t *lif, const char *fname,
            int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking lif:{} from {}:{}:{}",
                    __FUNCTION__, lif->lif_id,
                    fname, lineno, fxname);
    SDK_SPINLOCK_UNLOCK(&lif->slock);
}

extern lif_mgr *lif_manager();
extern program_info *prog_info();

extern void *lif_id_get_key_func(void *entry);
extern uint32_t lif_id_key_size(void);
extern lif_t *find_lif_by_id(lif_id_t lif_id);
extern lif_t *find_lif_by_handle(hal_handle_t handle);
void lif_print_ifs(lif_t *lif);
void lif_print(lif_t *lif);
hal_ret_t lif_update_trigger_if(lif_t *lif,
                                bool vlan_strip_en_changed,
                                bool vlan_strip_en,
                                bool vlan_insert_en_changed,
                                bool vlan_insert_en,
                                bool pinned_uplink_changed,
                                hal_handle_t pinned_uplink);
void LifGetQState(const intf::QStateGetReq &req, intf::QStateGetResp *resp);
void LifSetQState(const intf::QStateSetReq &req, intf::QStateSetResp *resp);
void lif_spec_dump (LifSpec& spec);
const char *lif_spec_keyhandle_to_str(const kh::LifKeyHandle& key_handle);
const char *lif_keyhandle_to_str(lif_t *lif);
lif_t *lif_lookup_key_or_handle(const kh::LifKeyHandle& kh);
lif_t *find_lif_by_name (char *name);

// SVC APIs
hal_ret_t lif_create(LifSpec& spec, LifResponse *rsp,
                     lif_hal_info_t *lif_hal_info);
hal_ret_t lif_update(LifSpec& spec, LifResponse *rsp);
hal_ret_t lif_delete(LifDeleteRequest& req,
                     LifDeleteResponse *rsp);
hal_ret_t lif_get(LifGetRequest& req, LifGetResponseMsg *rsp);

lif_t *find_lif_by_hw_lif_id (uint32_t hw_lif_id);
uint64_t lif_hw_lif_id_get (lif_t *lif);
hal_ret_t lif_disable_tx_scheduler (void);
int32_t hal_get_pc_offset(const char *prog_name, const char *label,
                          uint8_t *offset);
hal_ret_t lif_make_clone (lif_t *lif, lif_t **lif_clone, LifSpec& spec);

}    // namespace hal

#endif    // __LIF_HPP__

