#ifndef __HAL_PD_API_HPP__
#define __HAL_PD_API_HPP__

#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/include/nwsec_group_api.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/mcast/multicast.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/internal/tls_proxy_cb.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/src/aclqos/qos.hpp"
#include "nic/hal/src/aclqos/acl.hpp"
#include "nic/hal/src/internal/wring.hpp"
#include "nic/hal/src/telemetry/telemetry.hpp"
#include "nic/hal/src/internal/crypto_keys.hpp"
#include "nic/hal/src/internal/ipseccb.hpp"
#include "nic/hal/src/l4lb/l4lb.hpp"
#include "nic/hal/src/internal/cpucb.hpp"
#include "nic/hal/src/internal/system.hpp"
#include "nic/include/oif_list_api.hpp"
#include "nic/hal/src/internal/rawrcb.hpp"
#include "nic/hal/src/internal/rawccb.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/hal/src/internal/proxyccb.hpp"
#include "nic/hal/src/aclqos/barco_rings.hpp"
#include "nic/hal/src/gft/gft.hpp"
#include "nic/hal/src/lif/lif_manager_base.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/gen/proto/hal/crypto_apis.grpc.pb.h"

namespace hal {
namespace pd {

typedef uint32_t  mc_entry_hw_id_t;
typedef uint32_t  l2seg_hw_id_t;

typedef struct pd_mem_init_args_s {
    const char    *cfg_path;    // HAL config path
} __PACK__ pd_mem_init_args_t;

typedef struct pd_mem_init_phase2_args_s {
    const char    *cfg_path;    // HAL config path
    hal_cfg_t     *hal_cfg;
} __PACK__ pd_mem_init_phase2_args_t;

typedef struct pd_pgm_def_entries_args_s {
    hal_cfg_t    *hal_cfg;
} __PACK__ pd_pgm_def_entries_args_t;

typedef struct pd_pgm_def_p4plus_entries_args_s {
    hal_cfg_t    *hal_cfg;
} __PACK__ pd_pgm_def_p4plus_entries_args_t;

// vrf
typedef struct pd_vrf_create_args_s {
    vrf_t               *vrf;
    bool                gipo_prefix_change;
    ip_prefix_t         *new_gipo_prefix;
} __PACK__ pd_vrf_create_args_t;

typedef struct pd_vrf_restore_args_s {
    vrf_t              *vrf;
    const VrfStatus    *vrf_status;
} __PACK__ pd_vrf_restore_args_t;

typedef struct pd_vrf_delete_args_s {
    vrf_t               *vrf;
    nwsec_profile_t     *nwsec_profile;
    bool                gipo_prefix_change;
    ip_prefix_t         *new_gipo_prefix;
} __PACK__ pd_vrf_delete_args_t;

typedef struct pd_vrf_update_args_s {
    vrf_t               *vrf;
    nwsec_profile_t     *nwsec_profile;
    bool                gipo_prefix_change;
    ip_prefix_t         *new_gipo_prefix;
} __PACK__ pd_vrf_update_args_t;

typedef struct pd_vrf_mem_free_args_s {
    vrf_t               *vrf;
    nwsec_profile_t     *nwsec_profile;
    bool                gipo_prefix_change;
    ip_prefix_t         *new_gipo_prefix;
} __PACK__ pd_vrf_mem_free_args_t;

typedef struct pd_vrf_make_clone_args_s {
    vrf_t *vrf;
    vrf_t *clone;
} __PACK__ pd_vrf_make_clone_args_t;

typedef struct pd_vrf_get_args_s {
    vrf_t *vrf;
    VrfGetResponse *rsp;
} __PACK__ pd_vrf_get_args_t;

static inline void
pd_vrf_create_args_init (pd_vrf_create_args_t *args)
{
    args->vrf = NULL;
    return;
}

static inline void
pd_vrf_restore_args_init (pd_vrf_restore_args_t *args)
{
    args->vrf = NULL;
    args->vrf_status = NULL;
    return;
}

static inline void
pd_vrf_delete_args_init (pd_vrf_delete_args_t *args)
{
    args->vrf = NULL;
    return;
}

static inline void
pd_vrf_update_args_init (pd_vrf_update_args_t *args)
{
    args->vrf = NULL;
    return;
}

static inline void
pd_vrf_mem_free_args_init (pd_vrf_mem_free_args_t *args)
{
    args->vrf = NULL;
    return;
}

static inline void
pd_vrf_make_clone_args_init (pd_vrf_make_clone_args_t *args)
{
    args->vrf = NULL;
    args->clone = NULL;
    return;
}

// l2seg
typedef struct pd_l2seg_create_args_s {
    vrf_t       *vrf;
    l2seg_t     *l2seg;
} __PACK__ pd_l2seg_create_args_t;

typedef struct pd_l2seg_delete_args_s {
    vrf_t       *vrf;
    l2seg_t     *l2seg;
} __PACK__ pd_l2seg_delete_args_t;

typedef struct pd_l2seg_update_args_s {
    vrf_t       *vrf;
    l2seg_t     *l2seg;
} __PACK__ pd_l2seg_update_args_t;

typedef struct pd_l2seg_mem_free_args_s {
    vrf_t       *vrf;
    l2seg_t     *l2seg;
} __PACK__ pd_l2seg_mem_free_args_t;

typedef struct pd_l2seg_make_clone_args_s {
    l2seg_t *l2seg;
    l2seg_t *clone;
} __PACK__ pd_l2seg_make_clone_args_t;

static inline void
pd_l2seg_create_args_init (pd_l2seg_create_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    return;
}

static inline void
pd_l2seg_delete_args_init (pd_l2seg_delete_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    return;
}

static inline void
pd_l2seg_update_args_init (pd_l2seg_update_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    return;
}

static inline void
pd_l2seg_mem_free_args_init (pd_l2seg_mem_free_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    return;
}

// misc. apis of vrf and l2seg
typedef struct pd_get_object_from_flow_lkupid_args_s {
    uint32_t flow_lkupid;   // in
    hal_obj_id_t *obj_id;   // out
    void **pi_obj;          // out
} __PACK__ pd_get_object_from_flow_lkupid_args_t;

typedef struct pd_l2seg_get_flow_lkupid_args_s {
    l2seg_t *l2seg;         // in
    l2seg_hw_id_t hwid;     // out
} __PACK__ pd_l2seg_get_flow_lkupid_args_t;

typedef struct pd_vrf_get_lookup_id_args_s {
    vrf_t *vrf;
    uint32_t lkup_id;
} __PACK__ pd_vrf_get_lookup_id_args_t;

typedef struct pd_l2seg_get_fromcpu_vlanid_args_s {
    l2seg_t *l2seg;         // in
    uint16_t *vid;          // out
} pd_l2seg_get_fromcpu_vlanid_args_t;

typedef struct pd_vrf_get_fromcpu_vlanid_args_s {
    vrf_t *vrf;             // in
    uint16_t *vid;          // out
} __PACK__ pd_vrf_get_fromcpu_vlanid_args_t;

// nwsec profile

typedef struct pd_nwsec_profile_create_args_s {
    nwsec_profile_t    *nwsec_profile;
    nwsec_profile_t    *clone_profile;
} __PACK__ pd_nwsec_profile_create_args_t;

typedef struct pd_nwsec_profile_delete_args_s {
    nwsec_profile_t    *nwsec_profile;
    nwsec_profile_t    *clone_profile;
} __PACK__ pd_nwsec_profile_delete_args_t;

typedef struct pd_nwsec_profile_update_args_s {
    nwsec_profile_t    *nwsec_profile;
    nwsec_profile_t    *clone_profile;
} __PACK__ pd_nwsec_profile_update_args_t;

typedef struct pd_nwsec_profile_mem_free_args_s {
    nwsec_profile_t    *nwsec_profile;
    nwsec_profile_t    *clone_profile;
} __PACK__ pd_nwsec_profile_mem_free_args_t;

typedef struct pd_nwsec_profile_make_clone_args_s {
    nwsec_profile_t    *nwsec_profile;
    nwsec_profile_t    *clone_profile;
} __PACK__ pd_nwsec_profile_make_clone_args_t;

static inline void
pd_nwsec_profile_create_args_init (pd_nwsec_profile_create_args_t *args)
{
    args->nwsec_profile = NULL;
    return;
}

static inline void
pd_nwsec_profile_delete_args_init (pd_nwsec_profile_delete_args_t *args)
{
    args->nwsec_profile = NULL;
    return;
}

static inline void
pd_nwsec_profile_update_args_init (pd_nwsec_profile_update_args_t *args)
{
    args->nwsec_profile = NULL;
    return;
}

static inline void
pd_nwsec_profile_mem_free_args_init (pd_nwsec_profile_mem_free_args_t *args)
{
    args->nwsec_profile = NULL;
    return;
}

static inline void
pd_nwsec_profile_make_clone_args_init (pd_nwsec_profile_make_clone_args_t *args)
{
    args->nwsec_profile = NULL;
    args->clone_profile = NULL;
    return;
}

// dos_policy apis
typedef struct pd_dos_policy_create_args_s {
    dos_policy_t    *dos_policy;
    dos_policy_t    *clone_policy;
} __PACK__ pd_dos_policy_create_args_t;

typedef struct pd_dos_policy_delete_args_s {
    dos_policy_t    *dos_policy;
    dos_policy_t    *clone_policy;
} __PACK__ pd_dos_policy_delete_args_t;

typedef struct pd_dos_policy_update_args_s {
    dos_policy_t    *dos_policy;
    dos_policy_t    *clone_policy;
} __PACK__ pd_dos_policy_update_args_t;

typedef struct pd_dos_policy_mem_free_args_s {
    dos_policy_t    *dos_policy;
    dos_policy_t    *clone_policy;
} __PACK__ pd_dos_policy_mem_free_args_t;

typedef struct pd_dos_policy_make_clone_args_s {
    dos_policy_t    *dos_policy;
    dos_policy_t    *clone_policy;
} __PACK__ pd_dos_policy_make_clone_args_t;

static inline void
pd_dos_policy_create_args_init (pd_dos_policy_create_args_t *args)
{
    args->dos_policy = NULL;
    return;
}

static inline void
pd_dos_policy_delete_args_init (pd_dos_policy_delete_args_t *args)
{
    args->dos_policy = NULL;
    return;
}

static inline void
pd_dos_policy_update_args_init (pd_dos_policy_update_args_t *args)
{
    args->dos_policy = NULL;
    return;
}

static inline void
pd_dos_policy_mem_free_args_init (pd_dos_policy_mem_free_args_t *args)
{
    args->dos_policy = NULL;
    return;
}

static inline void
pd_dos_policy_make_clone_args_init (pd_dos_policy_make_clone_args_t *args)
{
    args->dos_policy = NULL;
    return;
}

// lif updates
typedef struct pd_lif_create_args_s {
    lif_t    *lif;
    bool     with_hw_lif_id;
    uint32_t hw_lif_id;
} __PACK__ pd_lif_create_args_t;

typedef struct pd_lif_update_args_s {
    lif_t    *lif;
    bool    vlan_strip_en_changed;
    bool    vlan_strip_en;
    bool    qstate_map_init_set;
    bool    rss_config_changed;
    bool    rx_policer_changed;
    bool    tx_policer_changed;
} pd_lif_update_args_t;

typedef struct pd_lif_delete_args_s {
    lif_t    *lif;
    bool     with_hw_lif_id;
    uint32_t hw_lif_id;
} __PACK__ pd_lif_delete_args_t;

typedef struct pd_lif_mem_free_args_s {
    lif_t    *lif;
    bool     with_hw_lif_id;
    uint32_t hw_lif_id;
} __PACK__ pd_lif_mem_free_args_t;

typedef struct pd_lif_make_clone_args_s {
    lif_t    *lif;
    lif_t    *clone;
} __PACK__ pd_lif_make_clone_args_t;

typedef struct pd_lif_get_args_s {
    lif_t *lif;          // i/p
    uint32_t hw_lif_id;  // o/p
} __PACK__ pd_lif_get_args_t;

static inline void
pd_lif_create_args_init (pd_lif_create_args_t *args)
{
    args->lif = NULL;
    args->with_hw_lif_id = false;
    args->hw_lif_id = 0;
    return;
}

static inline void
pd_lif_delete_args_init (pd_lif_delete_args_t *args)
{
    args->lif = NULL;
    args->with_hw_lif_id = false;
    args->hw_lif_id = 0;
    return;
}

static inline void
pd_lif_update_args_init (pd_lif_update_args_t *args)
{
    memset(args, 0, sizeof(pd_lif_update_args_t));
    return;
}

static inline void
pd_lif_mem_free_args_init (pd_lif_mem_free_args_t *args)
{
    args->lif = NULL;
    args->with_hw_lif_id = false;
    args->hw_lif_id = 0;
    return;
}

// if apis

typedef struct pd_if_create_args_s {
    if_t    *intf;
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            l2seg_t         *native_l2seg;
            // uplink if specific info
            struct {
            } __PACK__;
            // uplink pc specific info
            struct {
                bool            mbrlist_change;
                dllist_ctxt_t   *add_mbrlist;
                dllist_ctxt_t   *del_mbrlist;
                dllist_ctxt_t   *aggr_mbrlist;
            } __PACK__;
            // enic if specific info
            struct {
                lif_t *lif;

                // Classic mode fields
                bool            pinned_uplink_change;
                hal_handle_t    new_pinned_uplink;
                bool            native_l2seg_clsc_change;
                hal_handle_t    new_native_l2seg_clsc;
                bool            l2seg_clsc_change;
                dllist_ctxt_t   *add_l2seg_clsclist;
                dllist_ctxt_t   *del_l2seg_clsclist;
            } __PACK__;
        } __PACK__;
    } __PACK__;
} __PACK__ pd_if_create_args_t;

typedef struct pd_if_update_args_s {
    if_t    *intf;
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            l2seg_t         *native_l2seg;
            // uplink if specific info
            struct {
            } __PACK__;
            // uplink pc specific info
            struct {
                bool            mbrlist_change;
                block_list      *add_mbrlist;
                block_list      *del_mbrlist;
                block_list      *aggr_mbrlist;
            } __PACK__;
            // enic if specific info
            struct {
                lif_t *lif;

                // Classic mode fields
                bool            pinned_uplink_change;
                hal_handle_t    new_pinned_uplink;
                bool            native_l2seg_clsc_change;
                hal_handle_t    new_native_l2seg_clsc;
                bool            l2seg_clsc_change;
                dllist_ctxt_t   *add_l2seg_clsclist;
                dllist_ctxt_t   *del_l2seg_clsclist;
            } __PACK__;
        } __PACK__;
    } __PACK__;
} __PACK__ pd_if_update_args_t;

typedef struct pd_if_delete_args_s {
    if_t    *intf;
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            l2seg_t         *native_l2seg;
            // uplink if specific info
            struct {
            } __PACK__;
            // uplink pc specific info
            struct {
                bool            mbrlist_change;
                block_list      *add_mbrlist;
                block_list      *del_mbrlist;
                block_list      *aggr_mbrlist;
            } __PACK__;
            // enic if specific info
            struct {
                lif_t *lif;

                // Classic mode fields
                bool            pinned_uplink_change;
                hal_handle_t    new_pinned_uplink;
                bool            native_l2seg_clsc_change;
                hal_handle_t    new_native_l2seg_clsc;
                bool            l2seg_clsc_change;
                block_list      *add_l2seg_clsclist;
                block_list      *del_l2seg_clsclist;
            } __PACK__;
        } __PACK__;
    } __PACK__;
} __PACK__ pd_if_delete_args_t;

typedef struct pd_if_mem_free_args_s {
    if_t    *intf;
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            l2seg_t         *native_l2seg;
            // uplink if specific info
            struct {
            } __PACK__;
            // uplink pc specific info
            struct {
                bool            mbrlist_change;
                block_list      *add_mbrlist;
                block_list      *del_mbrlist;
                block_list      *aggr_mbrlist;
            } __PACK__;
            // enic if specific info
            struct {
                lif_t *lif;

                // Classic mode fields
                bool            pinned_uplink_change;
                hal_handle_t    new_pinned_uplink;
                bool            native_l2seg_clsc_change;
                hal_handle_t    new_native_l2seg_clsc;
                bool            l2seg_clsc_change;
                block_list      *add_l2seg_clsclist;
                block_list      *del_l2seg_clsclist;
            } __PACK__;
        } __PACK__;
    } __PACK__;
} __PACK__ pd_if_mem_free_args_t;

typedef struct pd_if_get_args_s {
    if_t *hal_if;
    InterfaceGetResponse *rsp;
} __PACK__ pd_if_get_args_t;

typedef struct pd_if_make_clone_args_s {
    if_t *hal_if;
    if_t *clone;
} __PACK__ pd_if_make_clone_args_t;

typedef struct pd_if_lif_update_args_s {
    if_t            *intf;
    lif_t           *lif;

    bool            vlan_strip_en_changed;
    bool            vlan_strip_en;
    bool            vlan_insert_en_changed;
    bool            vlan_insert_en;
} __PACK__ pd_if_lif_update_args_t;

static inline void
pd_if_lif_update_args_init (pd_if_lif_update_args_t *args)
{
    memset(args, 0, sizeof(pd_if_lif_update_args_t));
    return;
}

typedef struct pd_if_nwsec_update_args_s {
    l2seg_t         *l2seg;
    if_t            *intf;
    nwsec_profile_t *nwsec_prof;
} __PACK__ pd_if_nwsec_update_args_t;

static inline void
pd_if_nwsec_update_args_init (pd_if_nwsec_update_args_t *args)
{
    args->intf = NULL;
    args->nwsec_prof = NULL;

    return;
}

static inline void
pd_if_create_args_init (pd_if_create_args_t *args)
{
    args->intf = NULL;
    return;
}

static inline void
pd_if_update_args_init (pd_if_update_args_t *args)
{
    args->intf = NULL;
    return;
}

static inline void
pd_if_delete_args_init (pd_if_delete_args_t *args)
{
    args->intf = NULL;
    return;
}

static inline void
pd_if_mem_free_args_init (pd_if_mem_free_args_t *args)
{
    args->intf = NULL;
    return;
}

// ep
typedef struct pd_ep_create_args_s {
    vrf_t        *vrf;
    l2seg_t         *l2seg;
    if_t            *intf;
    ep_t            *ep;
} __PACK__ pd_ep_create_args_t;

static inline void
pd_ep_create_args_init (pd_ep_create_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    args->intf = NULL;
    args->ep = NULL;
    return;
}

typedef struct pd_ep_delete_args_s {
    vrf_t        *vrf;
    l2seg_t         *l2seg;
    if_t            *intf;
    ep_t            *ep;
} __PACK__ pd_ep_delete_args_t;

static inline void
pd_ep_delete_args_init (pd_ep_delete_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    args->intf = NULL;
    args->ep = NULL;
    return;
}

typedef struct pd_ep_update_args_s {
    ep_t                    *ep;

    // IP list change attrs, deprecate and take from app_ctxt
    bool                    iplist_change;
    dllist_ctxt_t           *add_iplist;
    dllist_ctxt_t           *del_iplist;

    ep_update_app_ctxt_t    *app_ctxt;

    // X change attrs
} pd_ep_update_args_t;

static inline void
pd_ep_update_args_init (pd_ep_update_args_t *args)
{
    memset(args, 0, sizeof(pd_ep_update_args_t));
    return;
}

typedef struct pd_ep_mem_free_args_s {
    vrf_t        *vrf;
    l2seg_t         *l2seg;
    if_t            *intf;
    ep_t            *ep;
} __PACK__ pd_ep_mem_free_args_t;

static inline void
pd_ep_mem_free_args_init (pd_ep_mem_free_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    args->intf = NULL;
    args->ep = NULL;
    return;
}

typedef struct pd_ep_make_clone_args_s {
    ep_t *ep;
    ep_t *clone;
} pd_ep_make_clone_args_t;

// session
typedef struct pd_sessoin_create_args_s {
    vrf_t              *vrf;
    nwsec_profile_t    *nwsec_prof;
    bool               update_iflow;
    session_t          *session;
    session_state_t    *session_state;
    SessionResponse    *rsp;
} __PACK__ pd_session_create_args_t;

static inline void
pd_session_create_args_init (pd_session_create_args_t *args)
{
    args->vrf = NULL;
    args->session = NULL;
    args->session_state = NULL;
    args->rsp = NULL;
    args->update_iflow = false;

    return;
}

typedef struct pd_sessoin_delete_args_s {
    vrf_t              *vrf;
    nwsec_profile_t    *nwsec_prof;
    bool               update_iflow;
    session_t          *session;
    session_state_t    *session_state;
    SessionResponse    *rsp;
} __PACK__ pd_session_delete_args_t;

static inline void
pd_session_delete_args_init (pd_session_delete_args_t *args)
{
    args->vrf = NULL;
    args->session = NULL;
    args->session_state = NULL;
    args->rsp = NULL;
    args->update_iflow = false;

    return;
}

typedef struct pd_sessoin_update_args_s {
    vrf_t              *vrf;
    nwsec_profile_t    *nwsec_prof;
    bool               update_iflow;
    session_t          *session;
    session_state_t    *session_state;
    SessionResponse    *rsp;
} __PACK__ pd_session_update_args_t;

static inline void
pd_session_update_args_init (pd_session_update_args_t *args)
{
    args->vrf = NULL;
    args->session = NULL;
    args->session_state = NULL;
    args->rsp = NULL;
    args->update_iflow = false;

    return;
}

typedef struct pd_session_get_args_s {
    session_t          *session;
    session_state_t    *session_state;
    SessionResponse    *rsp;
} __PACK__ pd_session_get_args_t;

static inline void
pd_session_get_args_init (pd_session_get_args_t *args)
{
    args->session = NULL;
    args->session_state = NULL;
    args->rsp = NULL;

    return;
}

// tlscb
typedef struct pd_tlscb_create_args_s {
    tlscb_t            *tlscb;
} __PACK__ pd_tlscb_create_args_t;

static inline void
pd_tlscb_create_args_init (pd_tlscb_create_args_t *args)
{
    args->tlscb = NULL;
    return;
}

typedef struct pd_tlscb_update_args_s {
    tlscb_t            *tlscb;
} __PACK__ pd_tlscb_update_args_t;

static inline void
pd_tlscb_update_args_init (pd_tlscb_update_args_t *args)
{
    args->tlscb = NULL;
    return;
}

typedef struct pd_tlscb_delete_args_s {
    tlscb_t            *tlscb;
} __PACK__ pd_tlscb_delete_args_t;

static inline void
pd_tlscb_delete_args_init (pd_tlscb_delete_args_t *args)
{
    args->tlscb = NULL;
    return;
}

typedef struct pd_tlscb_get_args_s {
    tlscb_t            *tlscb;
} __PACK__ pd_tlscb_get_args_t;

static inline void
pd_tlscb_get_args_init (pd_tlscb_get_args_t *args)
{
    args->tlscb = NULL;
    return;
}

// tcpcb
typedef struct pd_tcpcb_create_args_s {
    tcpcb_t            *tcpcb;
} __PACK__ pd_tcpcb_create_args_t;

static inline void
pd_tcpcb_create_args_init (pd_tcpcb_create_args_t *args)
{
    args->tcpcb = NULL;
    return;
}

typedef struct pd_tcpcb_update_args_s {
    tcpcb_t            *tcpcb;
} __PACK__ pd_tcpcb_update_args_t;

static inline void
pd_tcpcb_update_args_init (pd_tcpcb_update_args_t *args)
{
    args->tcpcb = NULL;
    return;
}
typedef struct pd_tcpcb_delete_args_s {
    tcpcb_t            *tcpcb;
} __PACK__ pd_tcpcb_delete_args_t;

static inline void
pd_tcpcb_delete_args_init (pd_tcpcb_delete_args_t *args)
{
    args->tcpcb = NULL;
    return;
}
typedef struct pd_tcpcb_get_args_s {
    tcpcb_t            *tcpcb;
} __PACK__ pd_tcpcb_get_args_t;

static inline void
pd_tcpcb_get_args_init (pd_tcpcb_get_args_t *args)
{
    args->tcpcb = NULL;
    return;
}

// ipseccb
typedef struct pd_ipseccb_create_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_create_args_t;

static inline void
pd_ipseccb_create_args_init (pd_ipseccb_create_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

typedef struct pd_ipseccb_update_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_update_args_t;

static inline void
pd_ipseccb_update_args_init (pd_ipseccb_update_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

typedef struct pd_ipseccb_delete_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_delete_args_t;

static inline void
pd_ipseccb_delete_args_init (pd_ipseccb_delete_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

typedef struct pd_ipseccb_get_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_get_args_t;

static inline void
pd_ipseccb_get_args_init (pd_ipseccb_get_args_t *args)
{
    args->ipseccb = NULL;
    return;
}


// ipseccb decrypt
typedef struct pd_ipseccb_decrypt_create_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_decrypt_create_args_t;

static inline void
pd_ipseccb_decrypt_create_args_init (pd_ipseccb_decrypt_create_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

typedef struct pd_ipseccb_decrypt_update_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_decrypt_update_args_t;

static inline void
pd_ipseccb_decrypt_update_args_init (pd_ipseccb_decrypt_update_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

typedef struct pd_ipseccb_decrypt_delete_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_decrypt_delete_args_t;

static inline void
pd_ipseccb_decrypt_delete_args_init (pd_ipseccb_decrypt_delete_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

typedef struct pd_ipseccb_decrypt_get_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_decrypt_get_args_t;

static inline void
pd_ipseccb_decrypt_get_args_init (pd_ipseccb_decrypt_get_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

// l4lb
typedef struct pd_l4lb_create_args_s {
    l4lb_service_entry_t    *l4lb;
} __PACK__ pd_l4lb_create_args_t;

static inline void
pd_l4lb_create_args_init (pd_l4lb_create_args_t *args)
{
    args->l4lb = NULL;
    return;
}

// cpucb
typedef struct pd_cpucb_create_args_s {
    cpucb_t            *cpucb;
} __PACK__ pd_cpucb_create_args_t;

static inline void
pd_cpucb_create_args_init (pd_cpucb_create_args_t *args)
{
    args->cpucb = NULL;
    return;
}

typedef struct pd_cpucb_update_args_s {
    cpucb_t            *cpucb;
} __PACK__ pd_cpucb_update_args_t;

static inline void
pd_cpucb_update_args_init (pd_cpucb_update_args_t *args)
{
    args->cpucb = NULL;
    return;
}

typedef struct pd_cpucb_delete_args_s {
    cpucb_t            *cpucb;
} __PACK__ pd_cpucb_delete_args_t;

static inline void
pd_cpucb_delete_args_init (pd_cpucb_delete_args_t *args)
{
    args->cpucb = NULL;
    return;
}

typedef struct pd_cpucb_get_args_s {
    cpucb_t            *cpucb;
} __PACK__ pd_cpucb_get_args_t;

static inline void
pd_cpucb_get_args_init (pd_cpucb_get_args_t *args)
{
    args->cpucb = NULL;
    return;
}

// rawrcb
typedef struct pd_rawrcb_create_args_s {
    rawrcb_t           *rawrcb;
} __PACK__ pd_rawrcb_create_args_t;

static inline void
pd_rawrcb_create_args_init (pd_rawrcb_create_args_t *args)
{
    args->rawrcb = NULL;
    return;
}

typedef struct pd_rawrcb_update_args_s {
    rawrcb_t           *rawrcb;
} __PACK__ pd_rawrcb_update_args_t;

static inline void
pd_rawrcb_update_args_init (pd_rawrcb_update_args_t *args)
{
    args->rawrcb = NULL;
    return;
}

typedef struct pd_rawrcb_args_s {
    rawrcb_t           *rawrcb;
} __PACK__ pd_rawrcb_args_t;

typedef struct pd_rawrcb_delete_args_s {
    pd_rawrcb_args_t *r_args;
    bool retain_in_db;
} __PACK__ pd_rawrcb_delete_args_t;

static inline void
pd_rawrcb_delete_args_init (pd_rawrcb_delete_args_t *args)
{
    args->r_args = NULL;
    return;
}

typedef struct pd_rawrcb_get_args_s {
    rawrcb_t           *rawrcb;
} __PACK__ pd_rawrcb_get_args_t;

static inline void
pd_rawrcb_get_args_init (pd_rawrcb_get_args_t *args)
{
    args->rawrcb = NULL;
    return;
}

// rawccb
typedef struct pd_rawccb_create_args_s {
    rawccb_t           *rawccb;
} __PACK__ pd_rawccb_create_args_t;

static inline void
pd_rawccb_create_args_init (pd_rawccb_create_args_t *args)
{
    args->rawccb = NULL;
    return;
}

typedef struct pd_rawccb_update_args_s {
    rawccb_t           *rawccb;
} __PACK__ pd_rawccb_update_args_t;

static inline void
pd_rawccb_update_args_init (pd_rawccb_update_args_t *args)
{
    args->rawccb = NULL;
    return;
}

typedef struct pd_rawccb_args_s {
    rawccb_t           *rawccb;
} __PACK__ pd_rawccb_args_t;

typedef struct pd_rawccb_delete_args_s {
    pd_rawccb_args_t *r_args;
    bool retain_in_db;
} __PACK__ pd_rawccb_delete_args_t;

static inline void
pd_rawccb_delete_args_init (pd_rawccb_delete_args_t *args)
{
    args->r_args = NULL;
    return;
}

typedef struct pd_rawccb_get_args_s {
    rawccb_t           *rawccb;
} __PACK__ pd_rawccb_get_args_t;

static inline void
pd_rawccb_get_args_init (pd_rawccb_get_args_t *args)
{
    args->rawccb = NULL;
    return;
}

// proxrcb
typedef struct pd_proxyrcb_create_args_s {
    proxyrcb_t         *proxyrcb;
} __PACK__ pd_proxyrcb_create_args_t;

static inline void
pd_proxyrcb_create_args_init (pd_proxyrcb_create_args_t *args)
{
    args->proxyrcb = NULL;
    return;
}

typedef struct pd_proxyrcb_update_args_s {
    proxyrcb_t         *proxyrcb;
} __PACK__ pd_proxyrcb_update_args_t;

static inline void
pd_proxyrcb_update_args_init (pd_proxyrcb_update_args_t *args)
{
    args->proxyrcb = NULL;
    return;
}

typedef struct pd_proxyrcb_args_s {
    proxyrcb_t         *proxyrcb;
} __PACK__ pd_proxyrcb_args_t;

typedef struct pd_proxyrcb_delete_args_s {
    pd_proxyrcb_args_t *r_args;
    bool retain_in_db;
} __PACK__ pd_proxyrcb_delete_args_t;

static inline void
pd_proxyrcb_delete_args_init (pd_proxyrcb_delete_args_t *args)
{
    args->r_args = NULL;
    return;
}

typedef struct pd_proxyrcb_get_args_s {
    proxyrcb_t         *proxyrcb;
} __PACK__ pd_proxyrcb_get_args_t;

static inline void
pd_proxyrcb_get_args_init (pd_proxyrcb_get_args_t *args)
{
    args->proxyrcb = NULL;
    return;
}

// proxy ccb
typedef struct pd_proxyccb_create_args_s {
    proxyccb_t         *proxyccb;
} __PACK__ pd_proxyccb_create_args_t;

static inline void
pd_proxyccb_create_args_init (pd_proxyccb_create_args_t *args)
{
    args->proxyccb = NULL;
    return;
}

typedef struct pd_proxyccb_update_args_s {
    proxyccb_t         *proxyccb;
} __PACK__ pd_proxyccb_update_args_t;

static inline void
pd_proxyccb_update_args_init (pd_proxyccb_update_args_t *args)
{
    args->proxyccb = NULL;
    return;
}

typedef struct pd_proxyccb_args_s {
    proxyccb_t         *proxyccb;
} __PACK__ pd_proxyccb_args_t;
typedef struct pd_proxyccb_delete_args_s {
    pd_proxyccb_args_t *r_args;
    bool retain_in_db;
} __PACK__ pd_proxyccb_delete_args_t;

static inline void
pd_proxyccb_delete_args_init (pd_proxyccb_delete_args_t *args)
{
    args->r_args = NULL;
    return;
}

typedef struct pd_proxyccb_get_args_s {
    proxyccb_t         *proxyccb;
} __PACK__ pd_proxyccb_get_args_t;

static inline void
pd_proxyccb_get_args_init (pd_proxyccb_get_args_t *args)
{
    args->proxyccb = NULL;
    return;
}

// qos class
typedef struct pd_qos_class_create_args_s {
    qos_class_t    *qos_class;
} __PACK__ pd_qos_class_create_args_t;

static inline void
pd_qos_class_create_args_init (pd_qos_class_create_args_t *args)
{
    args->qos_class = NULL;
    return;
}

typedef struct pd_qos_class_restore_args_s {
    qos_class_t              *qos_class;
    const QosClassStatus    *qos_class_status;
} __PACK__ pd_qos_class_restore_args_t;

static inline void
pd_qos_class_restore_args_init (pd_qos_class_restore_args_t *args)
{
    args->qos_class = NULL;
    args->qos_class_status = NULL;
    return;
}

typedef struct pd_qos_class_delete_args_s {
    qos_class_t    *qos_class;
} __PACK__ pd_qos_class_delete_args_t;

static inline void
pd_qos_class_delete_args_init (pd_qos_class_delete_args_t *args)
{
    args->qos_class = NULL;
    return;
}

typedef struct pd_qos_class_update_args_s {
    qos_class_t    *qos_class;
    bool mtu_changed;
    bool threshold_changed;
    bool dot1q_pcp_changed;
    uint32_t dot1q_pcp_src;
    bool ip_dscp_changed;
    bool ip_dscp_remove[HAL_MAX_IP_DSCP_VALS];
    bool pfc_changed;
    bool scheduler_changed;
    bool marking_changed;
} __PACK__ pd_qos_class_update_args_t;

static inline void
pd_qos_class_update_args_init (pd_qos_class_update_args_t *args)
{
    args->qos_class = NULL;
    return;
}

typedef struct pd_qos_class_mem_free_args_s {
    qos_class_t    *qos_class;
} __PACK__ pd_qos_class_mem_free_args_t;

static inline void
pd_qos_class_mem_free_args_init (pd_qos_class_mem_free_args_t *args)
{
    args->qos_class = NULL;
    return;
}

typedef struct pd_qos_class_make_clone_args_s {
    qos_class_t *qos_class;
    qos_class_t *clone;
} __PACK__ pd_qos_class_make_clone_args_t;

typedef struct pd_qos_class_get_args_s {
    qos_class_t *qos_class;
    QosClassGetResponse *rsp;
} __PACK__ pd_qos_class_get_args_t;

// copp
typedef struct pd_copp_create_args_s {
    copp_t    *copp;
} __PACK__ pd_copp_create_args_t;

static inline void
pd_copp_create_args_init (pd_copp_create_args_t *args)
{
    args->copp = NULL;
    return;
}

typedef struct pd_copp_restore_args_s {
    copp_t              *copp;
    const CoppStatus    *copp_status;
} __PACK__ pd_copp_restore_args_t;

static inline void
pd_copp_restore_args_init (pd_copp_restore_args_t *args)
{
    args->copp = NULL;
    args->copp_status = NULL;
    return;
}

typedef struct pd_copp_delete_args_s {
    copp_t    *copp;
} __PACK__ pd_copp_delete_args_t;

static inline void
pd_copp_delete_args_init (pd_copp_delete_args_t *args)
{
    args->copp = NULL;
    return;
}

typedef struct pd_copp_update_args_s {
    copp_t    *copp;
} __PACK__ pd_copp_update_args_t;

static inline void
pd_copp_update_args_init (pd_copp_update_args_t *args)
{
    args->copp = NULL;
    return;
}

typedef struct pd_copp_mem_free_args_s {
    copp_t    *copp;
} __PACK__ pd_copp_mem_free_args_t;

static inline void
pd_copp_mem_free_args_init (pd_copp_mem_free_args_t *args)
{
    args->copp = NULL;
    return;
}

typedef struct pd_copp_make_clone_args_s {
    copp_t *copp;
    copp_t *clone;
} __PACK__ pd_copp_make_clone_args_t;

typedef struct pd_copp_get_args_s {
    copp_t *copp;
    CoppGetResponse *rsp;
} __PACK__ pd_copp_get_args_t;

// acl
typedef struct pd_acl_create_args_s {
    acl_t    *acl;
} __PACK__ pd_acl_create_args_t;

static inline void
pd_acl_create_args_init (pd_acl_create_args_t *args)
{
    args->acl = NULL;
    return;
}

typedef struct pd_acl_restore_args_s {
    acl_t              *acl;
    const AclStatus    *acl_status;
} __PACK__ pd_acl_restore_args_t;

static inline void
pd_acl_restore_args_init (pd_acl_restore_args_t *args)
{
    args->acl = NULL;
    args->acl_status = NULL;
    return;
}

typedef struct pd_acl_delete_args_s {
    acl_t    *acl;
} __PACK__ pd_acl_delete_args_t;

static inline void
pd_acl_delete_args_init (pd_acl_delete_args_t *args)
{
    args->acl = NULL;
    return;
}

typedef struct pd_acl_update_args_s {
    acl_t    *acl;
} __PACK__ pd_acl_update_args_t;

static inline void
pd_acl_update_args_init (pd_acl_update_args_t *args)
{
    args->acl = NULL;
    return;
}

typedef struct pd_acl_mem_free_args_s {
    acl_t    *acl;
} __PACK__ pd_acl_mem_free_args_t;

static inline void
pd_acl_mem_free_args_init (pd_acl_mem_free_args_t *args)
{
    args->acl = NULL;
    return;
}

typedef struct pd_acl_make_clone_args_s {
    acl_t *acl;
    acl_t *clone;
} __PACK__ pd_acl_make_clone_args_t;

typedef struct pd_acl_get_args_s {
    acl_t *acl;
    AclGetResponse *rsp;
} __PACK__ pd_acl_get_args_t;

// wring
typedef struct pd_wring_create_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_create_args_t;

static inline void
pd_wring_create_args_init (pd_wring_create_args_t *args)
{
    args->wring = NULL;
    return;
}

typedef struct pd_wring_update_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_update_args_t;

static inline void
pd_wring_update_args_init (pd_wring_update_args_t *args)
{
    args->wring = NULL;
    return;
}

typedef struct pd_wring_delete_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_delete_args_t;

static inline void
pd_wring_delete_args_init (pd_wring_delete_args_t *args)
{
    args->wring = NULL;
    return;
}

typedef struct pd_wring_get_entry_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_get_entry_args_t;

static inline void
pd_wring_get_entry_args_init (pd_wring_get_entry_args_t *args)
{
    args->wring = NULL;
    return;
}

typedef struct pd_wring_get_meta_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_get_meta_args_t;

static inline void
pd_wring_get_meta_args_init (pd_wring_get_meta_args_t *args)
{
    args->wring = NULL;
    return;
}

typedef struct pd_wring_set_meta_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_set_meta_args_t;

static inline void
pd_wring_set_meta_args_init (pd_wring_set_meta_args_t *args)
{
    args->wring = NULL;
    return;
}

// telemetry module's PD init
typedef struct pd_telemetry_init_args_s {
    hal_cfg_t    *hal_cfg;
} __PACK__ pd_telemetry_init_args_t;

static inline void
pd_telemetry_init_args_init (pd_telemetry_init_args_t *args)
{
    args->hal_cfg = NULL;
}

// telemetry module's PD cleanup
typedef struct pd_telemetry_cleanup_args_s {
} __PACK__ pd_telemetry_cleanup_args_t;

static inline void
pd_telemetry_cleanup_args_init (pd_telemetry_cleanup_args_t *args)
{
}

// mirror session
typedef struct pd_mirror_session_create_args_s {
    mirror_session_t *session;
} __PACK__ pd_mirror_session_create_args_t;

static inline void
pd_mirror_session_create_args_init (pd_mirror_session_create_args_t *args)
{
    args->session = NULL;
}

typedef struct pd_mirror_session_delete_args_s {
    mirror_session_t *session;
} __PACK__ pd_mirror_session_delete_args_t;

static inline void
pd_mirror_session_delete_args_init (pd_mirror_session_delete_args_t *args)
{
    args->session = NULL;
}

typedef struct pd_mirror_session_get_args_s {
    mirror_session_t *session;
} __PACK__ pd_mirror_session_get_args_t;

static inline void
pd_mirror_session_get_args_init(pd_mirror_session_get_args_t *args)
{
    args->session = NULL;
}

// flow monitoring rule
typedef struct pd_flow_monitor_rule_create_args_s {
    flow_monitor_rule_t *rule;
} __PACK__ pd_flow_monitor_rule_create_args_t;

static inline void
pd_flow_monitor_rule_create_args_init (pd_flow_monitor_rule_create_args_t *args)
{
    args->rule = NULL;
}

typedef struct pd_flow_monitor_rule_delete_args_s {
    flow_monitor_rule_t *rule;
} __PACK__ pd_flow_monitor_rule_delete_args_t;

static inline void
pd_flow_monitor_rule_delete_args_init (pd_flow_monitor_rule_delete_args_t *args)
{
    args->rule = NULL;
}

typedef struct pd_flow_monitor_rule_get_args_s {
    flow_monitor_rule_t *rule;
} __PACK__ pd_flow_monitor_rule_get_args_t;

static inline void
pd_flow_monitor_rule_get_args_init (pd_flow_monitor_rule_get_args_t *args)
{
    args->rule = NULL;
}

// drop monitoring rule
typedef struct pd_drop_monitor_rule_create_args_s {
    drop_monitor_rule_t *rule;
} __PACK__ pd_drop_monitor_rule_create_args_t;

static inline void
pd_drop_monitor_rule_create_args_init (pd_drop_monitor_rule_create_args_t *args)
{
    args->rule = NULL;
}

typedef struct pd_drop_monitor_rule_delete_args_s {
    drop_monitor_rule_t *rule;
} __PACK__ pd_drop_monitor_rule_delete_args_t;

static inline void
pd_drop_monitor_rule_delete_args_init (pd_drop_monitor_rule_delete_args_t *args)
{
    args->rule = NULL;
}

typedef struct pd_drop_monitor_rule_get_args_s {
    drop_monitor_rule_t *rule;
} __PACK__ pd_drop_monitor_rule_get_args_t;

static inline void
pd_drop_monitor_rule_get_args_init (pd_drop_monitor_rule_get_args_t *args)
{
    args->rule = NULL;
}

// collector
typedef struct pd_collector_create_args_s {
    collector_config_t *cfg;
} __PACK__ pd_collector_create_args_t;

static inline void
pd_collector_create_args_init (pd_collector_create_args_t *args)
{
    args->cfg = NULL;
}

// mc entry
typedef struct pd_mc_entry_create_args_s {
    mc_entry_t      *mc_entry;
} __PACK__ pd_mc_entry_create_args_t;

static inline void
pd_mc_entry_create_args_init (pd_mc_entry_create_args_t *args)
{
    args->mc_entry = NULL;
    return;
}

typedef struct pd_mc_entry_delete_args_s {
    mc_entry_t      *mc_entry;
} __PACK__ pd_mc_entry_delete_args_t;

static inline void
pd_mc_entry_delete_args_init (pd_mc_entry_delete_args_t *args)
{
    args->mc_entry = NULL;
    return;
}

typedef struct pd_mc_entry_update_args_s {
    mc_entry_t      *mc_entry;
    mc_entry_t      *upd_entry;
} __PACK__ pd_mc_entry_update_args_t;

static inline void
pd_mc_entry_update_args_init (pd_mc_entry_update_args_t *args)
{
    args->mc_entry = NULL;
    args->upd_entry = NULL;
    return;
}

// flow get
typedef struct pd_flow_get_args_s {
    pd_session_t *pd_session;
    flow_role_t   role;
    flow_state_t *flow_state;
} __PACK__ pd_flow_get_args_t;

// l2seg uplink
typedef struct pd_add_l2seg_uplink_args_s {
    l2seg_t     *l2seg;
    if_t        *intf;
} __PACK__ pd_add_l2seg_uplink_args_t;

static inline void
pd_add_l2seg_uplink_args_init (pd_add_l2seg_uplink_args_t *args)
{
    args->l2seg = NULL;
    args->intf = NULL;
    return;
}

typedef struct pd_del_l2seg_uplink_args_s {
    l2seg_t     *l2seg;
    if_t        *intf;
} __PACK__ pd_del_l2seg_uplink_args_t;

static inline void
pd_del_l2seg_uplink_args_init (pd_del_l2seg_uplink_args_t *args)
{
    args->l2seg = NULL;
    args->intf = NULL;
    return;
}

// table properties
typedef struct pd_table_properties_get_args_s {
    uint32_t tableid;
    uint32_t tabledepth;
} __PACK__ pd_table_properties_get_args_t;

// debug cli
typedef struct pd_debug_cli_read_args_s {
    uint32_t tableid;
    uint32_t index;
    void     *swkey;
    void     *swkey_mask;
    void     *actiondata;
} __PACK__ pd_debug_cli_read_args_t;

// pipeline type for MPU tracing
typedef enum pd_mpu_trace_pipeline_type_e {
    MPU_TRACE_PIPELINE_NONE,
    MPU_TRACE_PIPELINE_P4_INGRESS,
    MPU_TRACE_PIPELINE_P4_EGRESS,
} pd_mpu_trace_pipeline_type_t;

// MPU tracing
typedef struct pd_mpu_trace_enable_args_s {
    pd_mpu_trace_pipeline_type_t pipeline_type;
    uint32_t                     stage_id;
    uint32_t                     mpu;
    uint8_t                      enable;
    uint8_t                      trace_enable;
    uint8_t                      phv_debug;
    uint8_t                      phv_error;
    uint64_t                     watch_pc;
    uint64_t                     base_addr;
    uint8_t                      table_key;
    uint8_t                      instructions;
    uint8_t                      wrap;
    uint8_t                      reset;
    uint32_t                     buf_size;
    uint32_t                     mpu_trace_size;
    uint32_t                     max_mpu_per_stage;
} __PACK__ pd_mpu_trace_enable_args_t;

typedef struct pd_debug_cli_write_args_s {
    uint32_t tableid;
    uint32_t index;
    void     *swkey;
    void     *swkey_mask;
    void     *actiondata;
} __PACK__ pd_debug_cli_write_args_t;

// get apis
typedef struct pd_if_get_hw_lif_id_args_s {
    if_t *pi_if;
    uint32_t hw_lif_id;
} __PACK__ pd_if_get_hw_lif_id_args_t;

typedef struct pd_if_get_lport_id_args_s {
    if_t *pi_if;
    uint32_t lport_id;
} __PACK__ pd_if_get_lport_id_args_t;

typedef struct pd_if_get_tm_oport_args_s {
    if_t *pi_if;
    uint32_t tm_oport;
} __PACK__ pd_if_get_tm_oport_args_t;

typedef enum twice_nat_actions_enum {
    TWICE_NAT_NOP_ID = 0,
    TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID = 1,
    TWICE_NAT_MAX_ID = 2
} twice_nat_actions_en;

typedef struct pd_twice_nat_entry_args_s {
    ip_addr_t               nat_ip;
    uint16_t                nat_l4_port;
    twice_nat_actions_en    twice_nat_act;
} __PACK__ pd_twice_nat_entry_args_t;

typedef struct pd_rw_entry_args_s {
    mac_addr_t          mac_sa;
    mac_addr_t          mac_da;
    rewrite_actions_en  rw_act;
} __PACK__ pd_rw_entry_args_t;

enum pd_swphv_type_t {
    PD_SWPHV_TYPE_RXDMA   = 0,     // P4+ Rx DMA
    PD_SWPHV_TYPE_TXDMA   = 1,     // P4+ Tx DMA
    PD_SWPHV_TYPE_INGRESS = 2,     // P4 Ingress
    PD_SWPHV_TYPE_EGRESS  = 3,     // P4 Egress
};

// SW PHV params for injecting software PHV
typedef struct pd_swphv_inject_args_s {
    pd_swphv_type_t    type;
} pd_swphv_inject_args_t;

// state of SW PHV injection
typedef struct pd_swphv_get_state_args_s {
    pd_swphv_type_t    type;
    bool               enabled;
    bool               done;
    uint32_t           current_cntr;
    uint32_t           no_data_cntr;
    uint32_t           drop_no_data_cntr;
} pd_swphv_get_state_args_t;

// TODO: PD-Cleanup Handle
uint32_t qos_class_get_admin_cos (void);

typedef struct pd_rw_entry_find_or_alloc_args_s {
    pd_rw_entry_args_t *args;
    uint32_t *rw_idx;
} pd_rw_entry_find_or_alloc_args_t;

typedef struct pd_twice_nat_add_args_s {
    pd_twice_nat_entry_args_t *args;
    uint32_t *twice_nat_idx;
} pd_twice_nat_add_args_t;

typedef struct pd_twice_nat_del_args_s {
    pd_twice_nat_entry_args_t *args;
} pd_twice_nat_del_args_t;

// Qos
typedef struct pd_qos_class_get_qos_class_id_args_s {
    qos_class_t *qos_class;
    if_t *dest_if;
    uint32_t *qos_class_id;
} __PACK__ pd_qos_class_get_qos_class_id_args_t;

typedef struct pd_qos_class_get_admin_cos_args_s {
    uint32_t cos;
} __PACK__ pd_qos_class_get_admin_cos_args_t;


typedef struct pd_descr_aol_s {
    uint64_t    scratch[8];
    uint64_t    a0;
    uint32_t    o0;
    uint32_t    l0;
    uint64_t    a1;
    uint32_t    o1;
    uint32_t    l1;
    uint64_t    a2;
    uint32_t    o2;
    uint32_t    l2;
    uint64_t    next_addr;
} __attribute__ ((__packed__)) pd_descr_aol_t;

typedef struct pd_descriptor_aol_get_args_s {
    pd_descr_aol_t *src;
    pd_descr_aol_t *dst;
} __PACK__ pd_descriptor_aol_get_args_t;

// crypto
typedef struct pd_crypto_alloc_key_args_s {
    int32_t *key_idx;
} __PACK__ pd_crypto_alloc_key_args_t;

typedef struct pd_crypto_free_key_args_s {
    int32_t key_idx;
} __PACK__ pd_crypto_free_key_args_t;

typedef struct pd_crypto_write_key_args_s {
    int32_t key_idx;
    crypto_key_t *key;
} __PACK__ pd_crypto_write_key_args_t;

typedef struct pd_crypto_read_key_args_s {
    int32_t key_idx;
    crypto_key_t *key;
} pd_crypto_read_key_args_t;

typedef struct pd_crypto_asym_alloc_key_args_s {
    int32_t *key_idx;
} __PACK__ pd_crypto_asym_alloc_key_args_t;

typedef struct pd_crypto_asym_free_key_args_s {
    int32_t key_idx;
} __PACK__ pd_crypto_asym_free_key_args_t;

typedef struct pd_crypto_asym_write_key_args_s {
    int32_t key_idx;
    crypto_asym_key_t *key;
} __PACK__ pd_crypto_asym_write_key_args_t;

typedef struct pd_crypto_asym_read_key_args_s {
    int32_t key_idx;
    crypto_asym_key_t *key;
} pd_crypto_asym_read_key_args_t;

// barco
typedef struct pd_get_opaque_tag_addr_args_s {
    types::BarcoRings ring_type;
    uint64_t* addr;
} __PACK__ pd_get_opaque_tag_addr_args_t;

// drop stats
typedef struct pd_system_args_s {
    SystemResponse *rsp;
} __PACK__ pd_system_args_t;

static inline void
pd_system_args_init (pd_system_args_t *args)
{
    args->rsp = NULL;
}

typedef struct pd_drop_stats_get_args_s {
    pd_system_args_t *pd_sys_args;
} __PACK__ pd_drop_stats_get_args_t;

typedef struct pd_table_stats_get_args_s {
    pd_system_args_t *pd_sys_args;
} __PACK__ pd_table_stats_get_args_t;

typedef struct pd_system_drop_action_args_s {
    SystemConfigSpec *spec;
} __PACK__ pd_system_drop_action_args_t;

static inline void
pd_system_drop_action_args_init (pd_system_drop_action_args_t *args)
{
    args->spec = NULL;
}

// oiflist
typedef struct pd_oif_list_create_args_s {
    oif_list_id_t *list;
} __PACK__ pd_oif_list_create_args_t;

typedef struct pd_oif_list_create_block_args_s {
    oif_list_id_t *list;
    uint32_t size;
} __PACK__ pd_oif_list_create_block_args_t;

typedef struct pd_oif_list_delete_args_s {
    oif_list_id_t list;
} __PACK__ pd_oif_list_delete_args_t;

typedef struct pd_oif_list_delete_block_args_s {
    oif_list_id_t list;
    uint32_t size;
} __PACK__ pd_oif_list_delete_block_args_t;

typedef struct pd_oif_list_attach_args_s {
    oif_list_id_t frm;
    oif_list_id_t to;
} __PACK__ pd_oif_list_attach_args_t;

typedef struct pd_oif_list_detach_args_s {
    oif_list_id_t frm;
} __PACK__ pd_oif_list_detach_args_t;

typedef struct pd_oif_list_add_oif_args_s {
    oif_list_id_t list;
    oif_t *oif;
} __PACK__ pd_oif_list_add_oif_args_t;

typedef struct pd_oif_list_add_qp_oif_args_s {
    oif_list_id_t list;
    oif_t *oif;
} __PACK__ pd_oif_list_add_qp_oif_args_t;

typedef struct pd_oif_list_remove_oif_args_s {
    oif_list_id_t list;
    oif_t *oif;
} __PACK__ pd_oif_list_remove_oif_args_t;

typedef struct pd_oif_list_is_member_args_s {
    oif_list_id_t list;
    oif_t *oif;
} __PACK__ pd_oif_list_is_member_args_t;

typedef struct pd_oif_list_get_num_oifs_args_s {
    oif_list_id_t list;
    uint32_t *num_oifs;
} __PACK__ pd_oif_list_get_num_oifs_args_t;

typedef struct pd_oif_list_get_oif_array_args_s {
    oif_list_id_t list;
    uint32_t *num_oifs;
    oif_t *oifs;
} __PACK__ pd_oif_list_get_oif_array_args_t;

typedef struct pd_oif_list_set_honor_ingress_args_s {
    oif_list_id_t list;
} __PACK__ pd_oif_list_set_honor_ingress_args_t;

typedef struct pd_oif_list_clr_honor_ingress_args_s {
    oif_list_id_t list;
} __PACK__ pd_oif_list_clr_honor_ingress_args_t;

typedef struct pd_tunnelif_get_rw_idx_args_s {
    if_t *hal_if;
    uint32_t tnnl_rw_idx;
} __PACK__ pd_tunnelif_get_rw_idx_args_t;

// cpu pkt apis
typedef struct pd_cpupkt_ctxt_alloc_init_args_s {
    cpupkt_ctxt_t *ctxt;
} __PACK__ pd_cpupkt_ctxt_alloc_init_args_t;

typedef struct pd_cpupkt_register_rx_queue_args_s {
    cpupkt_ctxt_t* ctxt;
    types::WRingType type;
    uint32_t queue_id;
} __PACK__ pd_cpupkt_register_rx_queue_args_t;

typedef struct pd_cpupkt_register_tx_queue_args_s {
    cpupkt_ctxt_t* ctxt;
    types::WRingType type;
    uint32_t queue_id;
} __PACK__ pd_cpupkt_register_tx_queue_args_t;

typedef struct pd_cpupkt_unregister_tx_queue_args_s {
    cpupkt_ctxt_t* ctxt;
    types::WRingType type;
    uint32_t queue_id;
} __PACK__ pd_cpupkt_unregister_tx_queue_args_t;

typedef struct pd_cpupkt_poll_receive_args_s {
    cpupkt_ctxt_t* ctxt;
     p4_to_p4plus_cpu_pkt_t** flow_miss_hdr;
     uint8_t** data;
     size_t* data_len;
} __PACK__ pd_cpupkt_poll_receive_args_t;

typedef struct pd_cpupkt_free_args_s {
    p4_to_p4plus_cpu_pkt_t* flow_miss_hdr;
    uint8_t* data;
} __PACK__ pd_cpupkt_free_args_t;

typedef struct pd_cpupkt_send_args_s {
    cpupkt_ctxt_t* ctxt;
    types::WRingType type;
    uint32_t queue_id;
    cpu_to_p4plus_header_t* cpu_header;
    p4plus_to_p4_header_t* p4_header;
    uint8_t* data;
    size_t data_len;
    uint16_t dest_lif;
    uint8_t  qtype;
    uint32_t qid;
    uint8_t  ring_number;
} __PACK__ pd_cpupkt_send_args_t;

typedef struct pd_cpupkt_page_alloc_args_s {
    cpupkt_hw_id_t* page_addr;
} __PACK__ pd_cpupkt_page_alloc_args_t;

typedef struct pd_cpupkt_descr_alloc_args_s {
    cpupkt_hw_id_t* descr_addr;
} __PACK__ pd_cpupkt_descr_alloc_args_t;

typedef struct pd_cpupkt_program_send_ring_doorbell_args_s {
    uint16_t dest_lif;
    uint8_t  qtype;
    uint32_t qid;
    uint8_t  ring_number;
} pd_cpupkt_program_send_ring_doorbell_args_t;

// rdma
typedef struct pd_rxdma_table_entry_add_args_s {
    uint32_t idx;
    uint8_t rdma_en_qtype_mask;
    uint32_t pt_base_addr_page_id;
    uint8_t log_num_pt_entries;
    uint32_t cqcb_base_addr_hi;
    uint8_t log_num_cq_entries;
    uint32_t prefetch_pool_base_addr_page_id;
    uint8_t log_num_prefetch_pool_entries;
    uint8_t sq_qtype;
    uint8_t rq_qtype;
} __PACK__ pd_rxdma_table_entry_add_args_t;

typedef struct pd_txdma_table_entry_add_args_s {
    uint32_t idx;
    uint8_t rdma_en_qtype_mask;
    uint32_t pt_base_addr_page_id;
    uint8_t log_num_pt_entries;
    uint32_t cqcb_base_addr_hi;
    uint8_t log_num_cq_entries;
    uint32_t prefetch_pool_base_addr_page_id;
    uint8_t log_num_prefetch_pool_entries;
    uint8_t sq_qtype;
    uint8_t rq_qtype;
} __PACK__ pd_txdma_table_entry_add_args_t;

// lif
typedef struct pd_lif_get_lport_id_args_s {
    lif_t *pi_lif;
    uint32_t lport_id;
} __PACK__ pd_lif_get_lport_id_args_t;

// p4pt
typedef struct p4pt_pd_init_args_s {
} __PACK__ p4pt_pd_init_args_t;

// eth
typedef struct pd_rss_params_table_entry_add_args_s {
    uint32_t hw_lif_id;
    uint8_t rss_type;
    uint8_t *rss_key;
} __PACK__ pd_rss_params_table_entry_add_args_t;

typedef struct pd_rss_indir_table_entry_add_args_s {
    uint32_t hw_lif_id;
    uint8_t index;
    uint8_t enable;
    uint8_t qid;
} __PACK__ pd_rss_indir_table_entry_add_args_t;

// capri
typedef struct pd_get_start_offset_args_s {
    const char *reg_name;
    uint64_t offset;
} __PACK__ pd_get_start_offset_args_t;

typedef struct pd_get_size_kb_args_s {
    const char *reg_name;
    uint32_t size;
} __PACK__ pd_get_size_kb_args_t;

typedef struct pd_push_qstate_to_capri_args_s {
    hal::LIFQState *qstate;
    uint32_t       cos;
} __PACK__ pd_push_qstate_to_capri_args_t;

typedef struct pd_clear_qstate_args_s {
    LIFQState *qstate;
} __PACK__ pd_clear_qstate_args_t;

typedef struct pd_read_qstate_args_s {
    uint64_t q_addr;
    uint8_t *buf;
    uint32_t q_size;
} __PACK__ pd_read_qstate_args_t;

typedef struct pd_write_qstate_args_s {
    uint64_t q_addr;
    const uint8_t *buf;
    uint32_t q_size;
} __PACK__ pd_write_qstate_args_t;

typedef struct pd_get_pc_offset_args_s {
    const char *handle;
    const char *prog_name;
    const char *label;
    uint8_t *offset;
} __PACK__ pd_get_pc_offset_args_t;

typedef struct pd_capri_hbm_read_mem_args_s {
    uint64_t addr;
    uint8_t *buf;
    uint32_t size;
} __PACK__ pd_capri_hbm_read_mem_args_t;

typedef struct pd_capri_hbm_write_mem_args_s {
    uint64_t addr;
    uint8_t *buf;
    uint32_t size;
} __PACK__ pd_capri_hbm_write_mem_args_t;

typedef struct pd_capri_program_label_to_offset_args_s {
    const char *handle;
    char *prog_name;
    char *label_name;
    uint64_t *offset;
} __PACK__ pd_capri_program_label_to_offset_args_t;

typedef struct pd_capri_pxb_cfg_lif_bdf_args_s {
    uint32_t lif;
    uint16_t bdf;
} __PACK__ pd_capri_pxb_cfg_lif_bdf_args_t;

typedef struct pd_capri_program_to_base_addr_args_s {
    const char *handle;
    char *prog_name;
    uint64_t *base_addr;
} __PACK__ pd_capri_program_to_base_addr_args_t;

typedef struct pd_capri_barco_asym_req_descr_get_args_s {
    uint32_t slot_index;
    barco_asym_descr_t *asym_req_descr;
} __PACK__ pd_capri_barco_asym_req_descr_get_args_t;

typedef struct pd_capri_barco_symm_req_descr_get_args_s {
    types::BarcoRings ring_type;
    uint32_t slot_index;
    hal::barco_symm_descr_t *symm_req_descr;
} __PACK__ pd_capri_barco_symm_req_descr_get_args_t;

typedef struct pd_capri_barco_ring_meta_get_args_s {
    types::BarcoRings ring_type;
    uint32_t *pi;
    uint32_t *ci;
} __PACK__ pd_capri_barco_ring_meta_get_args_t;

// crypto
typedef struct pd_capri_barco_asym_ecc_point_mul_p256_args_s {
    uint8_t *p;
    uint8_t *n;
    uint8_t *xg;
    uint8_t *yg;
    uint8_t *a;
    uint8_t *b;
    uint8_t *x1;
    uint8_t *y1;
    uint8_t *k;
    uint8_t *x3;
    uint8_t *y3;
} __PACK__ pd_capri_barco_asym_ecc_point_mul_p256_args_t;

typedef struct pd_capri_barco_asym_ecdsa_p256_sig_gen_args_s {
    uint8_t *p;
    uint8_t *n;
    uint8_t *xg;
    uint8_t *yg;
    uint8_t *a;
    uint8_t *b;
    uint8_t *da;
    uint8_t *k;
    uint8_t *h;
    uint8_t *r;
    uint8_t *s;
} __PACK__ pd_capri_barco_asym_ecdsa_p256_sig_gen_args_t;

typedef struct pd_capri_barco_asym_ecdsa_p256_sig_verify_args_s {
    uint8_t *p;
    uint8_t *n;
    uint8_t *xg;
    uint8_t *yg;
    uint8_t *a;
    uint8_t *b;
    uint8_t *xq;
    uint8_t *yq;
    uint8_t *r;
    uint8_t *s;
    uint8_t *h;
} __PACK__ pd_capri_barco_asym_ecdsa_p256_sig_verify_args_t;

typedef struct pd_capri_barco_asym_rsa2k_encrypt_args_s {
    uint8_t *n;
    uint8_t *e;
    uint8_t *m;
    uint8_t *c;
} __PACK__ pd_capri_barco_asym_rsa2k_encrypt_args_t;

typedef struct pd_capri_barco_asym_rsa2k_decrypt_args_s {
    uint8_t *n;
    uint8_t *d;
    uint8_t *c;
    uint8_t *m;
} __PACK__ pd_capri_barco_asym_rsa2k_decrypt_args_t;

typedef struct pd_capri_barco_asym_rsa2k_crt_decrypt_args_s {
    uint8_t *p;
    uint8_t *q;
    uint8_t *dp;
    uint8_t *dq;
    uint8_t *qinv;
    uint8_t *c;
    uint8_t *m;
} __PACK__ pd_capri_barco_asym_rsa2k_crt_decrypt_args_t;

typedef struct pd_capri_barco_asym_rsa2k_setup_private_key_args_s {
    uint8_t *n;
    uint8_t *d;
    int32_t *key_idx;
} __PACK__ pd_capri_barco_asym_rsa2k_setup_private_key_args_t;

typedef struct pd_capri_barco_asym_rsa2k_sig_gen_args_s {
    int32_t key_idx;
    uint8_t *n;
    uint8_t *d;
    uint8_t *h;
    uint8_t *s;
} __PACK__ pd_capri_barco_asym_rsa2k_sig_gen_args_t;

typedef struct pd_capri_barco_asym_rsa2k_sig_verify_args_s {
    uint8_t *n;
    uint8_t *e;
    uint8_t *h;
    uint8_t *s;
} __PACK__ pd_capri_barco_asym_rsa2k_sig_verify_args_t;

typedef struct pd_capri_barco_sym_hash_process_request_args_s {
    cryptoapis::CryptoApiHashType hash_type;
    bool generate;
    unsigned char *key;
    int key_len;
    unsigned char *data;
    int data_len;
    uint8_t *output_digest;
    int digest_len;
} __PACK__ pd_capri_barco_sym_hash_process_request_args_t;

// gft
typedef struct pd_gft_exact_match_profile_args_s {
    gft_exact_match_profile_t       *exact_match_profile;
} __PACK__ pd_gft_exact_match_profile_args_t;

typedef struct pd_gft_exact_match_profile_make_clone_args_s {
    gft_exact_match_profile_t       *exact_match_profile;
    gft_exact_match_profile_t       *clone;
} __PACK__ pd_gft_exact_match_profile_make_clone_args_t;

static inline void
pd_gft_exact_match_profile_args_init (pd_gft_exact_match_profile_args_t *args)
{
    args->exact_match_profile = NULL;
}

static inline void
pd_gft_exact_match_profile_make_clone_args_init (pd_gft_exact_match_profile_make_clone_args_t *args)
{
    args->exact_match_profile = NULL;
    args->clone = NULL;
}

typedef struct pd_gft_hdr_group_xposition_profile_args_s {
    gft_hdr_xposition_profile_t    *profile;
} __PACK__ pd_gft_hdr_group_xposition_profile_args_t;

typedef struct pd_gft_hdr_group_xposition_profile_make_clone_args_s {
    gft_hdr_xposition_profile_t    *profile;
    gft_hdr_xposition_profile_t    *clone;
} __PACK__ pd_gft_hdr_group_xposition_profile_make_clone_args_t;

static inline void
pd_gft_hdr_group_xposition_profile_args_init (pd_gft_hdr_group_xposition_profile_args_t *args)
{
    args->profile = NULL;
}

static inline void
pd_gft_hdr_group_xposition_profile_make_clone_args_init (pd_gft_hdr_group_xposition_profile_make_clone_args_t *args)
{
    args->profile = NULL;
    args->clone = NULL;
}

typedef struct pd_gft_exact_match_flow_entry_args_s {
    gft_exact_match_flow_entry_t    *exact_match_flow_entry;
    GftExactMatchFlowEntryResponse  *rsp;
} __PACK__ pd_gft_exact_match_flow_entry_args_t;

typedef struct pd_gft_exact_match_flow_entry_make_clone_args_s {
    gft_exact_match_flow_entry_t    *exact_match_flow_entry;
    gft_exact_match_flow_entry_t    *clone;
} __PACK__ pd_gft_exact_match_flow_entry_make_clone_args_t;

static inline void
pd_gft_exact_match_flow_entry_args_init (pd_gft_exact_match_flow_entry_args_t *args)
{
    args->exact_match_flow_entry = NULL;
}

static inline void
pd_gft_exact_match_flow_entry_make_clone_args_init (pd_gft_exact_match_flow_entry_make_clone_args_t *args)
{
    args->exact_match_flow_entry = NULL;
    args->clone = NULL;
}

// clock
typedef struct pd_conv_hw_clock_to_sw_clock_args_s {
    uint32_t   hw_tick;
    uint64_t  *sw_ns;
} __PACK__ pd_conv_hw_clock_to_sw_clock_args_t;

typedef struct pd_conv_sw_clock_to_hw_clock_args_s {
    uint64_t   sw_ns;
    uint32_t  *hw_tick;
} __PACK__ pd_conv_sw_clock_to_hw_clock_args_t;

typedef struct pd_clock_delta_comp_args_s {
} __PACK__ pd_clock_delta_comp_args_t;

// slab
typedef struct pd_get_slab_args_s {
    hal_slab_t slab_id;
    sdk::lib::slab *slab;
} __PACK__ pd_get_slab_args_t;

// generic pd call macros
#define PD_FUNC_IDS(ENTRY)                                                              \
    ENTRY(PD_FUNC_ID_MEM_INIT,              0, "pd_func_id_pd_mem_init")                \
    ENTRY(PD_FUNC_ID_MEM_INIT_PHASE2,       1, "pd_func_id_pd_mem_init_phase2")         \
    ENTRY(PD_FUNC_ID_PGM_DEF_ENTRIES,       2, "pd_func_id_pgm_def_entries")            \
    ENTRY(PD_FUNC_ID_PGM_DEF_P4PLUS_ENTRIES,3, "pd_func_id_pgm_def_p4plus_entries")     \
    ENTRY(PD_FUNC_ID_VRF_CREATE,            4, "pd_func_id_vrf_create")                 \
    ENTRY(PD_FUNC_ID_VRF_DELETE,            5, "pd_func_id_vrf_delete")                 \
    ENTRY(PD_FUNC_ID_VRF_UPDATE,            6, "pd_func_id_vrf_update")                 \
    ENTRY(PD_FUNC_ID_VRF_MEM_FREE,          7, "PD_FUNC_ID_VRF_MEM_FREE")               \
    ENTRY(PD_FUNC_ID_VRF_MAKE_CLONE,        8, "PD_FUNC_ID_VRF_MAKE_CLONE")             \
    ENTRY(PD_FUNC_ID_L2SEG_CREATE,          9, "pd_func_id_l2seg_create")               \
    ENTRY(PD_FUNC_ID_L2SEG_DELETE,          10, "PD_FUNC_ID_L2SEG_DELETE")              \
    ENTRY(PD_FUNC_ID_L2SEG_UPDATE,          11, "PD_FUNC_ID_L2SEG_UPDATE")              \
    ENTRY(PD_FUNC_ID_L2SEG_MEM_FREE,        12, "PD_FUNC_ID_L2SEG_MEM_FREE")            \
    ENTRY(PD_FUNC_ID_L2SEG_MAKE_CLONE,      13, "PD_FUNC_ID_L2SEG_MAKE_CLONE")          \
    ENTRY(PD_FUNC_ID_FIND_L2SEG_BY_HWID,    14, "PD_FUNC_ID_FIND_L2SEG_BY_HWID")        \
    ENTRY(PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, 15, "PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID") \
    ENTRY(PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, 16, "PD_FUNC_ID_L2SEG_GET_FLOW_LKPID")       \
    ENTRY(PD_FUNC_ID_VRF_GET_FLOW_LKPID, 17, "PD_FUNC_ID_VRF_GET_FLOW_LKPID")           \
    ENTRY(PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID, 18, "PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID")   \
    ENTRY(PD_FUNC_ID_VRF_GET_FRCPU_VLANID, 19, "PD_FUNC_ID_VRF_GET_FRCPU_VLANID")       \
    ENTRY(PD_FUNC_ID_NWSEC_PROF_CREATE,    20, "PD_FUNC_ID_NWSEC_PROF_CREATE")          \
    ENTRY(PD_FUNC_ID_NWSEC_PROF_DELETE,    21, "PD_FUNC_ID_NWSEC_PROF_DELETE")          \
    ENTRY(PD_FUNC_ID_NWSEC_PROF_UPDATE,    22, "PD_FUNC_ID_NWSEC_PROF_UPDATE")          \
    ENTRY(PD_FUNC_ID_NWSEC_PROF_MEM_FREE,  23, "PD_FUNC_ID_NWSEC_PROF_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_NWSEC_PROF_MAKE_CLONE,24, "PD_FUNC_ID_NWSEC_PROF_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_DOS_POLICY_CREATE,    25, "PD_FUNC_ID_DOS_POLICY_CREATE")          \
    ENTRY(PD_FUNC_ID_DOS_POLICY_DELETE,    26, "PD_FUNC_ID_DOS_POLICY_DELETE")          \
    ENTRY(PD_FUNC_ID_DOS_POLICY_UPDATE,    27, "PD_FUNC_ID_DOS_POLICY_UPDATE")          \
    ENTRY(PD_FUNC_ID_DOS_POLICY_MEM_FREE,  28, "PD_FUNC_ID_DOS_POLICY_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_DOS_POLICY_MAKE_CLONE,29, "PD_FUNC_ID_DOS_POLICY_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_LIF_CREATE,           30, "PD_FUNC_ID_LIF_CREATE")          \
    ENTRY(PD_FUNC_ID_LIF_DELETE,           31, "PD_FUNC_ID_LIF_DELETE")          \
    ENTRY(PD_FUNC_ID_LIF_UPDATE,           32, "PD_FUNC_ID_LIF_UPDATE")          \
    ENTRY(PD_FUNC_ID_LIF_MEM_FREE,         33, "PD_FUNC_ID_LIF_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_LIF_MAKE_CLONE,       34, "PD_FUNC_ID_LIF_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_LIF_GET,              35, "PD_FUNC_ID_LIF_GET")            \
    ENTRY(PD_FUNC_ID_IF_CREATE,            36, "PD_FUNC_ID_IF_CREATE")          \
    ENTRY(PD_FUNC_ID_IF_DELETE,            37, "PD_FUNC_ID_IF_DELETE")          \
    ENTRY(PD_FUNC_ID_IF_UPDATE,            38, "PD_FUNC_ID_IF_UPDATE")          \
    ENTRY(PD_FUNC_ID_IF_MEM_FREE,          39, "PD_FUNC_ID_IF_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_IF_MAKE_CLONE,        40, "PD_FUNC_ID_IF_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_IF_NWSEC_UPDATE,      41, "PD_FUNC_ID_IF_NWSEC_UPDATE")    \
    ENTRY(PD_FUNC_ID_IF_LIF_UPDATE,        42, "PD_FUNC_ID_IF_LIF_UPDATE")      \
    ENTRY(PD_FUNC_ID_EP_CREATE,            43, "PD_FUNC_ID_EP_CREATE")          \
    ENTRY(PD_FUNC_ID_EP_DELETE,            44, "PD_FUNC_ID_EP_DELETE")          \
    ENTRY(PD_FUNC_ID_EP_UPDATE,            45, "PD_FUNC_ID_EP_UPDATE")          \
    ENTRY(PD_FUNC_ID_EP_MEM_FREE,          46, "PD_FUNC_ID_EP_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_EP_MAKE_CLONE,        47, "PD_FUNC_ID_EP_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_SESSION_CREATE,       48, "PD_FUNC_ID_SESSION_CREATE")          \
    ENTRY(PD_FUNC_ID_SESSION_DELETE,       49, "PD_FUNC_ID_SESSION_DELETE")          \
    ENTRY(PD_FUNC_ID_SESSION_UPDATE,       50, "PD_FUNC_ID_SESSION_UPDATE")          \
    ENTRY(PD_FUNC_ID_SESSION_GET,          51, "PD_FUNC_ID_SESSION_GET")        \
    ENTRY(PD_FUNC_ID_TLSCB_CREATE,         52, "PD_FUNC_ID_TLSCB_CREATE")          \
    ENTRY(PD_FUNC_ID_TLSCB_DELETE,         53, "PD_FUNC_ID_TLSCB_DELETE")          \
    ENTRY(PD_FUNC_ID_TLSCB_UPDATE,         54, "PD_FUNC_ID_TLSCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_TLSCB_GET,            55, "PD_FUNC_ID_TLSCB_GET")        \
    ENTRY(PD_FUNC_ID_TCPCB_CREATE,         56, "PD_FUNC_ID_TCPCB_CREATE")          \
    ENTRY(PD_FUNC_ID_TCPCB_DELETE,         57, "PD_FUNC_ID_TCPCB_DELETE")          \
    ENTRY(PD_FUNC_ID_TCPCB_UPDATE,         58, "PD_FUNC_ID_TCPCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_TCPCB_GET,            59, "PD_FUNC_ID_TCPCB_GET")        \
    ENTRY(PD_FUNC_ID_IPSECCB_CREATE,       60, "PD_FUNC_ID_IPSECCB_CREATE")          \
    ENTRY(PD_FUNC_ID_IPSECCB_DELETE,       61, "PD_FUNC_ID_IPSECCB_DELETE")          \
    ENTRY(PD_FUNC_ID_IPSECCB_UPDATE,       62, "PD_FUNC_ID_IPSECCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_IPSECCB_GET,          63, "PD_FUNC_ID_IPSECCB_GET")        \
    ENTRY(PD_FUNC_ID_IPSECCB_DECRYPT_CREATE, 64, "PD_FUNC_ID_IPSECCB_DECRYPT_CREATE") \
    ENTRY(PD_FUNC_ID_IPSECCB_DECRYPT_DELETE, 65, "PD_FUNC_ID_IPSECCB_DECRYPT_DELETE") \
    ENTRY(PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE, 67, "PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE") \
    ENTRY(PD_FUNC_ID_IPSECCB_DECRYPT_GET,    68, "PD_FUNC_ID_IPSECCB_DECRYPT_GET")    \
    ENTRY(PD_FUNC_ID_L4LB_CREATE,            69, "PD_FUNC_ID_L4LB_CREATE") \
    ENTRY(PD_FUNC_ID_CPUCB_CREATE,         70, "PD_FUNC_ID_CPUCB_CREATE")          \
    ENTRY(PD_FUNC_ID_CPUCB_DELETE,         71, "PD_FUNC_ID_CPUCB_DELETE")          \
    ENTRY(PD_FUNC_ID_CPUCB_UPDATE,         72, "PD_FUNC_ID_CPUCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_CPUCB_GET,            73, "PD_FUNC_ID_CPUCB_GET")        \
    ENTRY(PD_FUNC_ID_RAWRCB_CREATE,        74, "PD_FUNC_ID_RAWRCB_CREATE")          \
    ENTRY(PD_FUNC_ID_RAWRCB_DELETE,        75, "PD_FUNC_ID_RAWRCB_DELETE")          \
    ENTRY(PD_FUNC_ID_RAWRCB_UPDATE,        76, "PD_FUNC_ID_RAWRCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_RAWRCB_GET,           77, "PD_FUNC_ID_RAWRCB_GET")        \
    ENTRY(PD_FUNC_ID_RAWCCB_CREATE,        78, "PD_FUNC_ID_RAWCCB_CREATE")          \
    ENTRY(PD_FUNC_ID_RAWCCB_DELETE,        79, "PD_FUNC_ID_RAWCCB_DELETE")          \
    ENTRY(PD_FUNC_ID_RAWCCB_UPDATE,        80, "PD_FUNC_ID_RAWCCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_RAWCCB_GET,           81, "PD_FUNC_ID_RAWCCB_GET")        \
    ENTRY(PD_FUNC_ID_PROXYRCB_CREATE,      82, "PD_FUNC_ID_PROXYRCB_CREATE")          \
    ENTRY(PD_FUNC_ID_PROXYRCB_DELETE,      83, "PD_FUNC_ID_PROXYRCB_DELETE")          \
    ENTRY(PD_FUNC_ID_PROXYRCB_UPDATE,      84, "PD_FUNC_ID_PROXYRCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_PROXYRCB_GET,         85, "PD_FUNC_ID_RAWRCB_GET")        \
    ENTRY(PD_FUNC_ID_PROXYCCB_CREATE,      86, "PD_FUNC_ID_PROXYCCB_CREATE")          \
    ENTRY(PD_FUNC_ID_PROXYCCB_DELETE,      87, "PD_FUNC_ID_PROXYCCB_DELETE")          \
    ENTRY(PD_FUNC_ID_PROXYCCB_UPDATE,      88, "PD_FUNC_ID_PROXYCCB_UPDATE")          \
    ENTRY(PD_FUNC_ID_PROXYCCB_GET,         89, "PD_FUNC_ID_PROXYCCB_GET")        \
    ENTRY(PD_FUNC_ID_QOS_CLASS_CREATE,     90, "PD_FUNC_ID_QOS_CLASS_CREATE")        \
    ENTRY(PD_FUNC_ID_QOS_CLASS_DELETE,     91, "PD_FUNC_ID_QOS_CLASS_DELETE")        \
    ENTRY(PD_FUNC_ID_COPP_CREATE,          92, "PD_FUNC_ID_COPP_CREATE")        \
    ENTRY(PD_FUNC_ID_COPP_DELETE,          93, "PD_FUNC_ID_COPP_DELETE")        \
    ENTRY(PD_FUNC_ID_ACL_CREATE,           94, "PD_FUNC_ID_ACL_CREATE") \
    ENTRY(PD_FUNC_ID_ACL_DELETE,           95, "PD_FUNC_ID_ACL_DELETE")          \
    ENTRY(PD_FUNC_ID_ACL_UPDATE,           96, "PD_FUNC_ID_ACL_UPDATE")          \
    ENTRY(PD_FUNC_ID_ACL_MEM_FREE,         97, "PD_FUNC_ID_ACL_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_ACL_MAKE_CLONE,       98, "PD_FUNC_ID_ACL_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_WRING_CREATE,         99, "pd_func_id_wring_create")                 \
    ENTRY(PD_FUNC_ID_WRING_DELETE,         100, "pd_func_id_wring_delete")                 \
    ENTRY(PD_FUNC_ID_WRING_UPDATE,         101, "pd_func_id_wring_update")                 \
    ENTRY(PD_FUNC_ID_WRING_GET_ENTRY,      102,"PD_FUNC_ID_WRING_GET_ENTRY")               \
    ENTRY(PD_FUNC_ID_WRING_GET_META,       103, "PD_FUNC_ID_WRING_GET_META")               \
    ENTRY(PD_FUNC_ID_WRING_SET_META,       104, "PD_FUNC_ID_WRING_SET_META")               \
    ENTRY(PD_FUNC_ID_MIRROR_SESSION_CREATE, 105, "PD_FUNC_ID_MIRROR_SESSION_CREATE")\
    ENTRY(PD_FUNC_ID_MIRROR_SESSION_DELETE, 106, "PD_FUNC_ID_MIRROR_SESSION_DELETE")\
    ENTRY(PD_FUNC_ID_MIRROR_SESSION_GET,    107, "PD_FUNC_ID_MIRROR_SESSION_GET")\
    ENTRY(PD_FUNC_ID_COLLECTOR_CREATE,      108, "PD_FUNC_ID_COLLECTOR_CREATE")\
    ENTRY(PD_FUNC_ID_MC_ENTRY_CREATE,       109, "PD_FUNC_ID_MC_ENTRY_CREATE")\
    ENTRY(PD_FUNC_ID_MC_ENTRY_DELETE,       110, "PD_FUNC_ID_MC_ENTRY_DELETE")\
    ENTRY(PD_FUNC_ID_MC_ENTRY_UPDATE,       111, "PD_FUNC_ID_MC_ENTRY_UPDATE")\
    ENTRY(PD_FUNC_ID_FLOW_GET,              112, "PD_FUNC_ID_FLOW_GET")\
    ENTRY(PD_FUNC_ID_ADD_L2SEG_UPLINK,      113, "PD_FUNC_ID_ADD_L2SEG_UPLINK")\
    ENTRY(PD_FUNC_ID_DEL_L2SEG_UPLINK,      114, "PD_FUNC_ID_DEL_L2SEG_UPLINK")\
    ENTRY(PD_FUNC_ID_DEBUG_CLI_READ,        115, "PD_FUNC_ID_DEBUG_CLI_READ")\
    ENTRY(PD_FUNC_ID_DEBUG_CLI_WRITE,       116, "PD_FUNC_ID_DEBUG_CLI_WRITE")\
    ENTRY(PD_FUNC_ID_IF_GET_HW_LIF_ID,      117, "PD_FUNC_ID_IF_GET_HW_LIF_ID")\
    ENTRY(PD_FUNC_ID_IF_GET_LPORT_ID,       118, "PD_FUNC_ID_IF_GET_LPORT_ID")\
    ENTRY(PD_FUNC_ID_IF_GET_TM_OPORT,       119, "PD_FUNC_ID_IF_GET_TM_OPORT")\
    ENTRY(PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC, 120, "PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC")\
    ENTRY(PD_FUNC_ID_TWICE_NAT_ADD,         121, "PD_FUNC_ID_TWICE_NAT_ADD")\
    ENTRY(PD_FUNC_ID_TWICE_NAT_DEL,         122, "PD_FUNC_ID_TWICE_NAT_DEL")\
    ENTRY(PD_FUNC_ID_GET_QOS_CLASSID,       123, "PD_FUNC_ID_GET_QOS_CLASSID")\
    ENTRY(PD_FUNC_ID_DESC_AOL_GET,          124, "PD_FUNC_ID_DESC_AOL_GET")\
    ENTRY(PD_FUNC_ID_CRYPTO_ALLOC_KEY,      125, "PD_FUNC_ID_CRYPTO_ALLOC_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_FREE_KEY,       126, "PD_FUNC_ID_CRYPTO_FREE_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_WRITE_KEY,      127, "PD_FUNC_ID_CRYPTO_WRITE_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_READ_KEY,       128, "PD_FUNC_ID_CRYPTO_READ_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_ASYM_ALLOC_KEY, 129, "PD_FUNC_ID_CRYPTO_ASYM_ALLOC_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_ASYM_FREE_KEY,  130, "PD_FUNC_ID_CRYPTO_ASYM_FREE_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_ASYM_WRITE_KEY, 131, "PD_FUNC_ID_CRYPTO_ASYM_WRITE_KEY")\
    ENTRY(PD_FUNC_ID_CRYPTO_ASYM_READ_KEY,  132, "PD_FUNC_ID_CRYPTO_ASYM_READ_KEY")\
    ENTRY(PD_FUNC_ID_OPAQUE_TAG_ADDR,       133, "PD_FUNC_ID_OPAQUE_TAG_ADDR")\
    ENTRY(PD_FUNC_ID_DROP_STATS_GET,        134, "PD_FUNC_ID_DROP_STATS_GET")\
    ENTRY(PD_FUNC_ID_TABLE_STATS_GET,       135, "PD_FUNC_ID_TABLE_STATS_GET")\
    ENTRY(PD_FUNC_ID_OIFL_CREATE,           136, "PD_FUNC_ID_OIFL_CREATE")\
    ENTRY(PD_FUNC_ID_OIFL_CREATE_BLOCK,     137, "PD_FUNC_ID_OIFL_CREATE_BLOCK")\
    ENTRY(PD_FUNC_ID_OIFL_DELETE,           138, "PD_FUNC_ID_OIFL_DELETE")\
    ENTRY(PD_FUNC_ID_OIFL_DELETE_BLOCK,     139, "PD_FUNC_ID_OIFL_DELETE_BLOCK")\
    ENTRY(PD_FUNC_ID_OIFL_ADD_OIF,          140, "PD_FUNC_ID_OIFL_ADD_OIF")\
    ENTRY(PD_FUNC_ID_OIFL_ADD_QP_OIF,       141, "PD_FUNC_ID_OIFL_ADD_QP_OIF")\
    ENTRY(PD_FUNC_ID_OIFL_REM_OIF,          142, "PD_FUNC_ID_OIFL_REM_OIF")\
    ENTRY(PD_FUNC_ID_OIFL_IS_MEMBER,        143, "PD_FUNC_ID_OIFL_IS_MEMBER")\
    ENTRY(PD_FUNC_ID_GET_NUM_OIFS,          144, "PD_FUNC_ID_GET_NUM_OIFS")\
    ENTRY(PD_FUNC_ID_GET_OIF_ARRAY,         145, "PD_FUNC_ID_GET_OIF_ARRAY")\
    ENTRY(PD_FUNC_ID_SET_HONOR_ING,         146, "PD_FUNC_ID_SET_HONOR_ING")\
    ENTRY(PD_FUNC_ID_TNNL_IF_GET_RW_IDX,    147, "PD_FUNC_ID_TNNL_IF_GET_RW_IDX")\
    ENTRY(PD_FUNC_ID_CPU_ALLOC_INIT,        148, "PD_FUNC_ID_CPU_ALLOC_INIT")\
    ENTRY(PD_FUNC_ID_CPU_REG_RXQ,           149, "PD_FUNC_ID_CPU_REG_RXQ")\
    ENTRY(PD_FUNC_ID_CPU_REG_TXQ,           150, "PD_FUNC_ID_CPU_REG_TXQ")\
    ENTRY(PD_FUNC_ID_CPU_UNREG_TXQ,         151, "PD_FUNC_ID_CPU_UNREG_TXQ")\
    ENTRY(PD_FUNC_ID_CPU_POLL_REC,          152, "PD_FUNC_ID_CPU_POLL_REC")\
    ENTRY(PD_FUNC_ID_CPU_FREE,              153, "PD_FUNC_ID_CPU_FREE")\
    ENTRY(PD_FUNC_ID_CPU_SEND,              154, "PD_FUNC_ID_CPU_SEND")\
    ENTRY(PD_FUNC_ID_CPU_PAGE_ALLOC,        155, "PD_FUNC_ID_CPU_PAGE_ALLOC")\
    ENTRY(PD_FUNC_ID_CPU_DESCR_ALLOC,       156, "PD_FUNC_ID_CPU_DESCR_ALLOC")\
    ENTRY(PD_FUNC_ID_PGM_SEND_RING_DBELL,   157, "PD_FUNC_ID_PGM_SEND_RING_DBELL")\
    ENTRY(PD_FUNC_ID_RXDMA_TABLE_ADD,       158, "PD_FUNC_ID_RXDMA_TABLE_ADD")\
    ENTRY(PD_FUNC_ID_TXDMA_TABLE_ADD,       159, "PD_FUNC_ID_TXDMA_TABLE_ADD")\
    ENTRY(PD_FUNC_ID_LIF_GET_LPORTID,       160, "PD_FUNC_ID_LIF_GET_LPORTID")\
    ENTRY(PD_FUNC_ID_P4PT_INIT,             161, "PD_FUNC_ID_P4PT_INIT")\
    ENTRY(PD_FUNC_ID_RSS_PARAMS_TABLE_ADD,  162, "PD_FUNC_ID_RSS_PARAMS_TABLE_ADD")\
    ENTRY(PD_FUNC_ID_RSS_INDIR_TABLE_ADD,   163, "PD_FUNC_ID_RSS_INDIR_TABLE_ADD")\
    ENTRY(PD_FUNC_ID_ASIC_INIT,             164, "PD_FUNC_ID_ASIC_INIT")\
    ENTRY(PD_FUNC_ID_GET_START_OFFSET,      165, "PD_FUNC_ID_GET_START_OFFSET")\
    ENTRY(PD_FUNC_ID_GET_REG_SIZE,          166, "PD_FUNC_ID_GET_REG_SIZE")\
    ENTRY(PD_FUNC_ID_PUSH_QSTATE,           167, "PD_FUNC_ID_PUSH_QSTATE")\
    ENTRY(PD_FUNC_ID_CLEAR_QSTATE,          168, "PD_FUNC_ID_CLEAR_QSTATE")\
    ENTRY(PD_FUNC_ID_READ_QSTATE,           169, "PD_FUNC_ID_READ_QSTATE")\
    ENTRY(PD_FUNC_ID_WRITE_QSTATE,          170, "PD_FUNC_ID_WRITE_QSTATE")\
    ENTRY(PD_FUNC_ID_GET_PC_OFFSET,         171, "PD_FUNC_ID_GET_PC_OFFSET")\
    ENTRY(PD_FUNC_ID_HBM_READ,              172, "PD_FUNC_ID_HBM_READ")\
    ENTRY(PD_FUNC_ID_HBM_WRITE,             173, "PD_FUNC_ID_HBM_WRITE")\
    ENTRY(PD_FUNC_ID_PROG_LBL_TO_OFFSET,    174, "PD_FUNC_ID_PROG_LBL_TO_OFFSET")\
    ENTRY(PD_FUNC_ID_PXB_CFG_LIF_BDF,       175, "PD_FUNC_ID_PXB_CFG_LIF_BDF")\
    ENTRY(PD_FUNC_ID_QOS_GET_ADMIN_COS,     176, "PD_FUNC_ID_QOS_GET_ADMIN_COS")\
    ENTRY(PD_FUNC_ID_PROG_TO_BASE_ADDR,     177, "PD_FUNC_ID_PROG_TO_BASE_ADDR")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_REQ_DSC_GET,178, "PD_FUNC_ID_BARCO_ASYM_REQ_DSC_GET")\
    ENTRY(PD_FUNC_ID_BARCO_SYM_REQ_DSC_GET, 179, "PD_FUNC_ID_BARCO_SYM_REQ_DSC_GET")\
    ENTRY(PD_FUNC_ID_BARCO_RING_META_GET,   180, "PD_FUNC_ID_BARCO_RING_META_GET")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_ECC_MUL_P256, 181, "PD_FUNC_ID_BARCO_ASYM_ECC_MUL_P256")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_GEN, 182, "PD_FUNC_ID_BARCO_ASYM_ECDSA_P256")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_VER, 183, "PD_FUNC_ID_BARCO_ASYM_ECDSA_P256_SIG_VER")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_RSA2K_ENCRYPT, 184, "PD_FUNC_ID_BARCO_ASYM_RSA2K_ENCRYPT")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_RSA2K_DECRYPT, 185, "PD_FUNC_ID_BARCO_ASYM_RSA2K_DECRYPT")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_RSA2K_CRT_DECRYPT, 186, "PD_FUNC_ID_BARCO_ASYM_RSA2K_CRT_DECRYPT")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_GEN, 187, "PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_GEN")\
    ENTRY(PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_VERIFY, 188, "PD_FUNC_ID_BARCO_ASYM_RSA2K_SIG_VERIFY")\
    ENTRY(PD_FUNC_ID_BARCO_SYM_HASH_PROC_REQ, 189, "PD_FUNC_ID_BARCO_SYM_HASH_PROC_REQ")\
    ENTRY(PD_FUNC_ID_GFT_EXACT_MATCH_PROFILE_CREATE, 190, "PD_FUNC_ID_GFT_EXACT_MATCH_PROFILE_CREATE") \
    ENTRY(PD_FUNC_ID_GFT_HDR_TRANSPOSITION_PROFILE_CREATE, 191, "PD_FUNC_ID_GFT_HDR_TRANSPOSITION_PROFILE_CREATE") \
    ENTRY(PD_FUNC_ID_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE, 192, "PD_FUNC_ID_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE") \
    ENTRY(PD_FUNC_ID_GET_SLAB,              193, "PD_FUNC_ID_GET_SLAB")\
    ENTRY(PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK, 194, "PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK")\
    ENTRY(PD_FUNC_ID_CLOCK_DELTA_COMP, 195, "PD_FUNC_ID_CLOCK_DELTA_COMP")\
    ENTRY(PD_FUNC_ID_SYSTEM_DROP_ACTION,    196, "PD_FUNC_ID_SYSTEM_DROP_ACTION") \
    ENTRY(PD_FUNC_ID_MPU_TRACE_ENABLE,      197, "PD_FUNC_ID_MPU_TRACE_ENABLE")\
    ENTRY(PD_FUNC_ID_CONV_SW_CLOCK_TO_HW_CLOCK, 198, "PD_FUNC_ID_CONV_SW_CLOCK_TO_HW_CLOCK")\
    ENTRY(PD_FUNC_ID_QOS_CLASS_UPDATE,      199, "PD_FUNC_ID_QOS_CLASS_UPDATE")     \
    ENTRY(PD_FUNC_ID_QOS_CLASS_MAKE_CLONE,  200, "PD_FUNC_ID_QOS_CLASS_MAKE_CLONE") \
    ENTRY(PD_FUNC_ID_QOS_CLASS_MEM_FREE,    201, "PD_FUNC_ID_QOS_CLASS_MEM_FREE")   \
    ENTRY(PD_FUNC_ID_COPP_UPDATE,           202, "PD_FUNC_ID_COPP_UPDATE")          \
    ENTRY(PD_FUNC_ID_COPP_MAKE_CLONE,       203, "PD_FUNC_ID_COPP_MAKE_CLONE")      \
    ENTRY(PD_FUNC_ID_COPP_MEM_FREE,         204, "PD_FUNC_ID_COPP_MEM_FREE")        \
    ENTRY(PD_FUNC_ID_SWPHV_INJECT,          205, "PD_FUNC_ID_SWPHV_INJECT")\
    ENTRY(PD_FUNC_ID_SWPHV_GET_STATE,       206, "PD_FUNC_ID_SWPHV_GET_STATE")\
    ENTRY(PD_FUNC_ID_CLR_HONOR_ING,         207, "PD_FUNC_ID_CLR_HONOR_ING")\
    ENTRY(PD_FUNC_ID_IF_GET,                208, "PD_FUNC_ID_IF_GET")\
    ENTRY(PD_FUNC_ID_OIFL_ATTACH,           209, "PD_FUNC_ID_OIFL_ATTACH")\
    ENTRY(PD_FUNC_ID_OIFL_DETACH,           210, "PD_FUNC_ID_OIFL_DETACH")\
    ENTRY(PD_FUNC_ID_FLOW_MONITOR_RULE_CREATE, 211, "PD_FUNC_ID_FLOW_MONITOR_RULE_CREATE")\
    ENTRY(PD_FUNC_ID_FLOW_MONITOR_RULE_DELETE, 212, "PD_FUNC_ID_FLOW_MONITOR_RULE_DELETE")\
    ENTRY(PD_FUNC_ID_FLOW_MONITOR_RULE_GET,    213, "PD_FUNC_ID_FLOW_MONITOR_RULE_GET")\
    ENTRY(PD_FUNC_ID_DROP_MONITOR_RULE_CREATE, 214, "PD_FUNC_ID_DROP_MONITOR_RULE_CREATE")\
    ENTRY(PD_FUNC_ID_DROP_MONITOR_RULE_DELETE, 215, "PD_FUNC_ID_DROP_MONITOR_RULE_DELETE")\
    ENTRY(PD_FUNC_ID_DROP_MONITOR_RULE_GET,    216, "PD_FUNC_ID_DROP_MONITOR_RULE_GET")\
    ENTRY(PD_FUNC_ID_VRF_RESTORE,              217, "PD_FUNC_ID_VRF_RESTORE") \
    ENTRY(PD_FUNC_ID_TABLE_PROPERTIES_GET,     218, "PD_FUNC_ID_TABLE_PROPERTIES_GET") \
    ENTRY(PD_FUNC_ID_ASYM_RSA2K_SETUP_PRIV_KEY, 219, "PD_FUNC_ID_ASYM_RSA2K_SETUP_PRIV_KEY")\
    ENTRY(PD_FUNC_ID_VRF_GET,                  220, "PD_FUNC_ID_VRF_GET") \
    ENTRY(PD_FUNC_ID_QOS_CLASS_GET,            221, "PD_FUNC_ID_QOS_CLASS_GET") \
    ENTRY(PD_FUNC_ID_QOS_CLASS_RESTORE,        222, "PD_FUNC_ID_QOS_CLASS_RESTORE") \
    ENTRY(PD_FUNC_ID_COPP_GET,                 223, "PD_FUNC_ID_COPP_GET") \
    ENTRY(PD_FUNC_ID_COPP_RESTORE,             224, "PD_FUNC_ID_COPP_RESTORE") \
    ENTRY(PD_FUNC_ID_ACL_GET,                  225, "PD_FUNC_ID_ACL_GET") \
    ENTRY(PD_FUNC_ID_ACL_RESTORE,              226, "PD_FUNC_ID_ACL_RESTORE") \
    ENTRY(PD_FUNC_ID_MAX,                      227, "pd_func_id_max")
DEFINE_ENUM(pd_func_id_t, PD_FUNC_IDS)
#undef PD_FUNC_IDS

#define PD_FUNCP_TYPEDEF(NAME)                                              \
    extern "C" hal_ret_t NAME(NAME ## _args_t *args);                       \
    typedef hal_ret_t (* NAME ## _t)(NAME ## _args_t *args)

#define PD_FUNCP_ARGS_TYPEDEF(NAME, ARGS)                                   \
    extern "C" hal_ret_t NAME(ARGS ## _args_t *args);                       \
    typedef hal_ret_t (* NAME ## _t)(ARGS ## _args_t *args)

#define PD_UNION_FIELD(NAME) \
    NAME ## _t NAME

//#define  PD_FUNCTION(NAME) {#NAME, NULL}

// init pd calls
PD_FUNCP_TYPEDEF(pd_mem_init);
PD_FUNCP_TYPEDEF(pd_mem_init_phase2);
PD_FUNCP_TYPEDEF(pd_pgm_def_entries);
PD_FUNCP_TYPEDEF(pd_pgm_def_p4plus_entries);

// vrf pd calls
PD_FUNCP_TYPEDEF(pd_vrf_create);
PD_FUNCP_TYPEDEF(pd_vrf_restore);
PD_FUNCP_TYPEDEF(pd_vrf_delete);
PD_FUNCP_TYPEDEF(pd_vrf_update);
PD_FUNCP_TYPEDEF(pd_vrf_mem_free);
PD_FUNCP_TYPEDEF(pd_vrf_make_clone);
PD_FUNCP_TYPEDEF(pd_vrf_get);

// l2seg pd calls
PD_FUNCP_TYPEDEF(pd_l2seg_create);
PD_FUNCP_TYPEDEF(pd_l2seg_update);
PD_FUNCP_TYPEDEF(pd_l2seg_delete);
PD_FUNCP_TYPEDEF(pd_l2seg_mem_free);
PD_FUNCP_TYPEDEF(pd_l2seg_make_clone);

// misc apis for vrf and l2seg
PD_FUNCP_TYPEDEF(pd_get_object_from_flow_lkupid);
PD_FUNCP_TYPEDEF(pd_l2seg_get_flow_lkupid);
PD_FUNCP_TYPEDEF(pd_vrf_get_lookup_id);
PD_FUNCP_TYPEDEF(pd_l2seg_get_fromcpu_vlanid);
PD_FUNCP_TYPEDEF(pd_vrf_get_fromcpu_vlanid);

// nwsec_profile pd calls
PD_FUNCP_TYPEDEF(pd_nwsec_profile_create);
PD_FUNCP_TYPEDEF(pd_nwsec_profile_delete);
PD_FUNCP_TYPEDEF(pd_nwsec_profile_update);
PD_FUNCP_TYPEDEF(pd_nwsec_profile_mem_free);
PD_FUNCP_TYPEDEF(pd_nwsec_profile_make_clone);

// dos_policy pd calls
PD_FUNCP_TYPEDEF(pd_dos_policy_create);
PD_FUNCP_TYPEDEF(pd_dos_policy_delete);
PD_FUNCP_TYPEDEF(pd_dos_policy_update);
PD_FUNCP_TYPEDEF(pd_dos_policy_mem_free);
PD_FUNCP_TYPEDEF(pd_dos_policy_make_clone);

// lif pd calls
PD_FUNCP_TYPEDEF(pd_lif_create);
PD_FUNCP_TYPEDEF(pd_lif_delete);
PD_FUNCP_TYPEDEF(pd_lif_update);
PD_FUNCP_TYPEDEF(pd_lif_mem_free);
PD_FUNCP_TYPEDEF(pd_lif_make_clone);
PD_FUNCP_TYPEDEF(pd_lif_get);

// if calls
PD_FUNCP_TYPEDEF(pd_if_create);
PD_FUNCP_TYPEDEF(pd_if_delete);
PD_FUNCP_TYPEDEF(pd_if_update);
PD_FUNCP_TYPEDEF(pd_if_mem_free);
PD_FUNCP_TYPEDEF(pd_if_make_clone);
PD_FUNCP_TYPEDEF(pd_if_nwsec_update);
PD_FUNCP_TYPEDEF(pd_if_lif_update);
PD_FUNCP_TYPEDEF(pd_if_get);

// ep calls
PD_FUNCP_TYPEDEF(pd_ep_create);
PD_FUNCP_TYPEDEF(pd_ep_update);
PD_FUNCP_TYPEDEF(pd_ep_delete);
PD_FUNCP_TYPEDEF(pd_ep_mem_free);
PD_FUNCP_TYPEDEF(pd_ep_make_clone);

// session calls
PD_FUNCP_TYPEDEF(pd_session_create);
PD_FUNCP_TYPEDEF(pd_session_update);
PD_FUNCP_TYPEDEF(pd_session_delete);
PD_FUNCP_TYPEDEF(pd_session_get);

// tlscb calls
PD_FUNCP_TYPEDEF(pd_tlscb_create);
PD_FUNCP_TYPEDEF(pd_tlscb_update);
PD_FUNCP_TYPEDEF(pd_tlscb_delete);
PD_FUNCP_TYPEDEF(pd_tlscb_get);

// tcpcb calls
PD_FUNCP_TYPEDEF(pd_tcpcb_create);
PD_FUNCP_TYPEDEF(pd_tcpcb_update);
PD_FUNCP_TYPEDEF(pd_tcpcb_delete);
PD_FUNCP_TYPEDEF(pd_tcpcb_get);

// ipseccb calls
PD_FUNCP_TYPEDEF(pd_ipseccb_create);
PD_FUNCP_TYPEDEF(pd_ipseccb_update);
PD_FUNCP_TYPEDEF(pd_ipseccb_delete);
PD_FUNCP_TYPEDEF(pd_ipseccb_get);

// ipseccb_decrypt calls
PD_FUNCP_TYPEDEF(pd_ipseccb_decrypt_create);
PD_FUNCP_TYPEDEF(pd_ipseccb_decrypt_update);
PD_FUNCP_TYPEDEF(pd_ipseccb_decrypt_delete);
PD_FUNCP_TYPEDEF(pd_ipseccb_decrypt_get);

// l4lb
PD_FUNCP_TYPEDEF(pd_l4lb_create);

// cpucb
PD_FUNCP_TYPEDEF(pd_cpucb_create);
PD_FUNCP_TYPEDEF(pd_cpucb_update);
PD_FUNCP_TYPEDEF(pd_cpucb_delete);
PD_FUNCP_TYPEDEF(pd_cpucb_get);

// rawrcb
PD_FUNCP_TYPEDEF(pd_rawrcb_create);
PD_FUNCP_TYPEDEF(pd_rawrcb_update);
PD_FUNCP_TYPEDEF(pd_rawrcb_delete);
PD_FUNCP_TYPEDEF(pd_rawrcb_get);

// rawccb
PD_FUNCP_TYPEDEF(pd_rawccb_create);
PD_FUNCP_TYPEDEF(pd_rawccb_update);
PD_FUNCP_TYPEDEF(pd_rawccb_delete);
PD_FUNCP_TYPEDEF(pd_rawccb_get);

// proxyrcb
PD_FUNCP_TYPEDEF(pd_proxyrcb_create);
PD_FUNCP_TYPEDEF(pd_proxyrcb_update);
PD_FUNCP_TYPEDEF(pd_proxyrcb_delete);
PD_FUNCP_TYPEDEF(pd_proxyrcb_get);

// proxyccb
PD_FUNCP_TYPEDEF(pd_proxyccb_create);
PD_FUNCP_TYPEDEF(pd_proxyccb_update);
PD_FUNCP_TYPEDEF(pd_proxyccb_delete);
PD_FUNCP_TYPEDEF(pd_proxyccb_get);

// qos clas
PD_FUNCP_TYPEDEF(pd_qos_class_create);
PD_FUNCP_TYPEDEF(pd_qos_class_restore);
PD_FUNCP_TYPEDEF(pd_qos_class_delete);
PD_FUNCP_TYPEDEF(pd_qos_class_update);
PD_FUNCP_TYPEDEF(pd_qos_class_mem_free);
PD_FUNCP_TYPEDEF(pd_qos_class_make_clone);
PD_FUNCP_TYPEDEF(pd_qos_class_get);

// copp
PD_FUNCP_TYPEDEF(pd_copp_create);
PD_FUNCP_TYPEDEF(pd_copp_restore);
PD_FUNCP_TYPEDEF(pd_copp_delete);
PD_FUNCP_TYPEDEF(pd_copp_update);
PD_FUNCP_TYPEDEF(pd_copp_mem_free);
PD_FUNCP_TYPEDEF(pd_copp_make_clone);
PD_FUNCP_TYPEDEF(pd_copp_get);

// acl pd calls
PD_FUNCP_TYPEDEF(pd_acl_create);
PD_FUNCP_TYPEDEF(pd_acl_restore);
PD_FUNCP_TYPEDEF(pd_acl_delete);
PD_FUNCP_TYPEDEF(pd_acl_update);
PD_FUNCP_TYPEDEF(pd_acl_mem_free);
PD_FUNCP_TYPEDEF(pd_acl_make_clone);
PD_FUNCP_TYPEDEF(pd_acl_get);

// wring
PD_FUNCP_TYPEDEF(pd_wring_create);
PD_FUNCP_TYPEDEF(pd_wring_delete);
PD_FUNCP_TYPEDEF(pd_wring_update);
PD_FUNCP_TYPEDEF(pd_wring_get_entry);
PD_FUNCP_TYPEDEF(pd_wring_get_meta);
PD_FUNCP_TYPEDEF(pd_wring_set_meta);

// mirror session
PD_FUNCP_TYPEDEF(pd_mirror_session_create);
PD_FUNCP_TYPEDEF(pd_mirror_session_delete);
PD_FUNCP_TYPEDEF(pd_mirror_session_get);

// flow monitor rule
PD_FUNCP_TYPEDEF(pd_flow_monitor_rule_create);
PD_FUNCP_TYPEDEF(pd_flow_monitor_rule_delete);
PD_FUNCP_TYPEDEF(pd_flow_monitor_rule_get);

// drop monitor rule
PD_FUNCP_TYPEDEF(pd_drop_monitor_rule_create);
PD_FUNCP_TYPEDEF(pd_drop_monitor_rule_delete);
PD_FUNCP_TYPEDEF(pd_drop_monitor_rule_get);

// collector
PD_FUNCP_TYPEDEF(pd_collector_create);

// mc entry
PD_FUNCP_TYPEDEF(pd_mc_entry_create);
PD_FUNCP_TYPEDEF(pd_mc_entry_delete);
PD_FUNCP_TYPEDEF(pd_mc_entry_update);

// flow get
PD_FUNCP_TYPEDEF(pd_flow_get);

// l2seg-uplink
PD_FUNCP_TYPEDEF(pd_add_l2seg_uplink);
PD_FUNCP_TYPEDEF(pd_del_l2seg_uplink);

// debug cli
PD_FUNCP_TYPEDEF(pd_debug_cli_read);
PD_FUNCP_TYPEDEF(pd_debug_cli_write);
PD_FUNCP_TYPEDEF(pd_mpu_trace_enable);

// get apis
PD_FUNCP_TYPEDEF(pd_if_get_hw_lif_id);
PD_FUNCP_TYPEDEF(pd_if_get_lport_id);
PD_FUNCP_TYPEDEF(pd_if_get_tm_oport);

// twice nat
PD_FUNCP_TYPEDEF(pd_rw_entry_find_or_alloc);
PD_FUNCP_TYPEDEF(pd_twice_nat_add);
PD_FUNCP_TYPEDEF(pd_twice_nat_del);

// qos
PD_FUNCP_TYPEDEF(pd_qos_class_get_qos_class_id);
PD_FUNCP_TYPEDEF(pd_qos_class_get_admin_cos);

// aol
PD_FUNCP_TYPEDEF(pd_descriptor_aol_get);

// crypto
PD_FUNCP_TYPEDEF(pd_crypto_alloc_key);
PD_FUNCP_TYPEDEF(pd_crypto_free_key);
PD_FUNCP_TYPEDEF(pd_crypto_write_key);
PD_FUNCP_TYPEDEF(pd_crypto_read_key);
PD_FUNCP_TYPEDEF(pd_crypto_asym_alloc_key);
PD_FUNCP_TYPEDEF(pd_crypto_asym_free_key);
PD_FUNCP_TYPEDEF(pd_crypto_asym_write_key);
PD_FUNCP_TYPEDEF(pd_crypto_asym_read_key);

// barco
PD_FUNCP_TYPEDEF(pd_get_opaque_tag_addr);

// stats
PD_FUNCP_TYPEDEF(pd_drop_stats_get);
PD_FUNCP_TYPEDEF(pd_table_stats_get);
PD_FUNCP_TYPEDEF(pd_system_drop_action);

// oifl
PD_FUNCP_TYPEDEF(pd_oif_list_create);
PD_FUNCP_TYPEDEF(pd_oif_list_create_block);
PD_FUNCP_TYPEDEF(pd_oif_list_delete);
PD_FUNCP_TYPEDEF(pd_oif_list_delete_block);
PD_FUNCP_TYPEDEF(pd_oif_list_attach);
PD_FUNCP_TYPEDEF(pd_oif_list_detach);
PD_FUNCP_TYPEDEF(pd_oif_list_add_oif);
PD_FUNCP_TYPEDEF(pd_oif_list_add_qp_oif);
PD_FUNCP_TYPEDEF(pd_oif_list_remove_oif);
PD_FUNCP_TYPEDEF(pd_oif_list_is_member);
PD_FUNCP_TYPEDEF(pd_oif_list_get_num_oifs);
PD_FUNCP_TYPEDEF(pd_oif_list_get_oif_array);
PD_FUNCP_TYPEDEF(pd_oif_list_set_honor_ingress);
PD_FUNCP_TYPEDEF(pd_oif_list_clr_honor_ingress);

// if
PD_FUNCP_TYPEDEF(pd_tunnelif_get_rw_idx);

// cpu
PD_FUNCP_TYPEDEF(pd_cpupkt_ctxt_alloc_init);
PD_FUNCP_TYPEDEF(pd_cpupkt_register_rx_queue);
PD_FUNCP_TYPEDEF(pd_cpupkt_register_tx_queue);
PD_FUNCP_TYPEDEF(pd_cpupkt_unregister_tx_queue);
PD_FUNCP_TYPEDEF(pd_cpupkt_poll_receive);
PD_FUNCP_TYPEDEF(pd_cpupkt_free);
PD_FUNCP_TYPEDEF(pd_cpupkt_send);
PD_FUNCP_TYPEDEF(pd_cpupkt_page_alloc);
PD_FUNCP_TYPEDEF(pd_cpupkt_descr_alloc);
PD_FUNCP_TYPEDEF(pd_cpupkt_program_send_ring_doorbell);

// rdma
PD_FUNCP_TYPEDEF(pd_rxdma_table_entry_add);
PD_FUNCP_TYPEDEF(pd_txdma_table_entry_add);

// lif
PD_FUNCP_TYPEDEF(pd_lif_get_lport_id);

// p4pt
PD_FUNCP_TYPEDEF(p4pt_pd_init);

// eth
PD_FUNCP_TYPEDEF(pd_rss_params_table_entry_add);
PD_FUNCP_TYPEDEF(pd_rss_indir_table_entry_add);

// asic init
PD_FUNCP_TYPEDEF(pd_asic_init);

PD_FUNCP_TYPEDEF(pd_table_properties_get);

// capri
PD_FUNCP_TYPEDEF(pd_get_start_offset);
PD_FUNCP_TYPEDEF(pd_get_size_kb);
PD_FUNCP_TYPEDEF(pd_push_qstate_to_capri);
PD_FUNCP_TYPEDEF(pd_clear_qstate);
PD_FUNCP_TYPEDEF(pd_read_qstate);
PD_FUNCP_TYPEDEF(pd_write_qstate);
PD_FUNCP_TYPEDEF(pd_get_pc_offset);
PD_FUNCP_TYPEDEF(pd_capri_hbm_read_mem);
PD_FUNCP_TYPEDEF(pd_capri_hbm_write_mem);
PD_FUNCP_TYPEDEF(pd_capri_program_label_to_offset);
PD_FUNCP_TYPEDEF(pd_capri_pxb_cfg_lif_bdf);
PD_FUNCP_TYPEDEF(pd_capri_program_to_base_addr);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_req_descr_get);
PD_FUNCP_TYPEDEF(pd_capri_barco_symm_req_descr_get);
PD_FUNCP_TYPEDEF(pd_capri_barco_ring_meta_get);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_ecc_point_mul_p256);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_ecdsa_p256_sig_gen);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_ecdsa_p256_sig_verify);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_rsa2k_encrypt);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_rsa2k_decrypt);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_rsa2k_crt_decrypt);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_rsa2k_setup_private_key);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_rsa2k_sig_gen);
PD_FUNCP_TYPEDEF(pd_capri_barco_asym_rsa2k_sig_verify);
PD_FUNCP_TYPEDEF(pd_capri_barco_sym_hash_process_request);

// gft exact match profile apis
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_profile_create, pd_gft_exact_match_profile);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_profile_delete, pd_gft_exact_match_profile);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_profile_update, pd_gft_exact_match_profile);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_profile_mem_free, pd_gft_exact_match_profile);
PD_FUNCP_TYPEDEF(pd_gft_exact_match_profile_make_clone);

// gft group xposition profile
PD_FUNCP_ARGS_TYPEDEF(pd_gft_hdr_group_xposition_profile_create, pd_gft_hdr_group_xposition_profile);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_hdr_group_xposition_profile_delete, pd_gft_hdr_group_xposition_profile);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_hdr_group_xposition_profile_update, pd_gft_hdr_group_xposition_profile);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_hdr_group_xposition_profile_mem_free, pd_gft_hdr_group_xposition_profile);
PD_FUNCP_TYPEDEF(pd_gft_hdr_group_xposition_profile_make_clone);

// gft match flow entry
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_flow_entry_create, pd_gft_exact_match_flow_entry);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_flow_entry_delete, pd_gft_exact_match_flow_entry);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_flow_entry_update, pd_gft_exact_match_flow_entry);
PD_FUNCP_ARGS_TYPEDEF(pd_gft_exact_match_flow_entry_mem_free, pd_gft_exact_match_flow_entry);
PD_FUNCP_TYPEDEF(pd_gft_exact_match_flow_entry_make_clone);

// clock
PD_FUNCP_TYPEDEF(pd_conv_hw_clock_to_sw_clock);
PD_FUNCP_TYPEDEF(pd_conv_sw_clock_to_hw_clock);
PD_FUNCP_TYPEDEF(pd_clock_delta_comp);

// slab
PD_FUNCP_TYPEDEF(pd_get_slab);

// swphv
PD_FUNCP_TYPEDEF(pd_swphv_inject);
PD_FUNCP_TYPEDEF(pd_swphv_get_state);

typedef struct pd_call_s {
    union {
        // init pd calls
        PD_UNION_FIELD(pd_mem_init);
        PD_UNION_FIELD(pd_mem_init_phase2);
        PD_UNION_FIELD(pd_pgm_def_entries);
        PD_UNION_FIELD(pd_pgm_def_p4plus_entries);

        // vrf pd calls
        PD_UNION_FIELD(pd_vrf_create);
        PD_UNION_FIELD(pd_vrf_restore);
        PD_UNION_FIELD(pd_vrf_delete);
        PD_UNION_FIELD(pd_vrf_update);
        PD_UNION_FIELD(pd_vrf_mem_free);
        PD_UNION_FIELD(pd_vrf_make_clone);
        PD_UNION_FIELD(pd_vrf_get);

        // l2seg pd calls
        PD_UNION_FIELD(pd_l2seg_create);
        PD_UNION_FIELD(pd_l2seg_update);
        PD_UNION_FIELD(pd_l2seg_delete);
        PD_UNION_FIELD(pd_l2seg_mem_free);
        PD_UNION_FIELD(pd_l2seg_make_clone);

        // misc apis for vrf and l2seg
        PD_UNION_FIELD(pd_get_object_from_flow_lkupid);
        PD_UNION_FIELD(pd_l2seg_get_flow_lkupid);
        PD_UNION_FIELD(pd_vrf_get_lookup_id);
        PD_UNION_FIELD(pd_l2seg_get_fromcpu_vlanid);
        PD_UNION_FIELD(pd_vrf_get_fromcpu_vlanid);

        // nwsec_profile pd calls
        PD_UNION_FIELD(pd_nwsec_profile_create);
        PD_UNION_FIELD(pd_nwsec_profile_delete);
        PD_UNION_FIELD(pd_nwsec_profile_update);
        PD_UNION_FIELD(pd_nwsec_profile_mem_free);
        PD_UNION_FIELD(pd_nwsec_profile_make_clone);

        // dos_policy pd calls
        PD_UNION_FIELD(pd_dos_policy_create);
        PD_UNION_FIELD(pd_dos_policy_delete);
        PD_UNION_FIELD(pd_dos_policy_update);
        PD_UNION_FIELD(pd_dos_policy_mem_free);
        PD_UNION_FIELD(pd_dos_policy_make_clone);

        // lif pd calls
        PD_UNION_FIELD(pd_lif_create);
        PD_UNION_FIELD(pd_lif_delete);
        PD_UNION_FIELD(pd_lif_update);
        PD_UNION_FIELD(pd_lif_mem_free);
        PD_UNION_FIELD(pd_lif_make_clone);
        PD_UNION_FIELD(pd_lif_get);

        // if pd calls
        PD_UNION_FIELD(pd_if_create);
        PD_UNION_FIELD(pd_if_delete);
        PD_UNION_FIELD(pd_if_update);
        PD_UNION_FIELD(pd_if_mem_free);
        PD_UNION_FIELD(pd_if_make_clone);
        PD_UNION_FIELD(pd_if_nwsec_update);
        PD_UNION_FIELD(pd_if_lif_update);
        PD_UNION_FIELD(pd_if_get);

        // ep calls
        PD_UNION_FIELD(pd_ep_create);
        PD_UNION_FIELD(pd_ep_update);
        PD_UNION_FIELD(pd_ep_delete);
        PD_UNION_FIELD(pd_ep_mem_free);
        PD_UNION_FIELD(pd_ep_make_clone);

        // session calls
        PD_UNION_FIELD(pd_session_create);
        PD_UNION_FIELD(pd_session_update);
        PD_UNION_FIELD(pd_session_delete);
        PD_UNION_FIELD(pd_session_get);

        // tlscb calls
        PD_UNION_FIELD(pd_tlscb_create);
        PD_UNION_FIELD(pd_tlscb_update);
        PD_UNION_FIELD(pd_tlscb_delete);
        PD_UNION_FIELD(pd_tlscb_get);

        // tcpcb calls
        PD_UNION_FIELD(pd_tcpcb_create);
        PD_UNION_FIELD(pd_tcpcb_update);
        PD_UNION_FIELD(pd_tcpcb_delete);
        PD_UNION_FIELD(pd_tcpcb_get);

        // ipseccb calls
        PD_UNION_FIELD(pd_ipseccb_create);
        PD_UNION_FIELD(pd_ipseccb_update);
        PD_UNION_FIELD(pd_ipseccb_delete);
        PD_UNION_FIELD(pd_ipseccb_get);

        // ipseccb_decrypt calls
        PD_UNION_FIELD(pd_ipseccb_decrypt_create);
        PD_UNION_FIELD(pd_ipseccb_decrypt_update);
        PD_UNION_FIELD(pd_ipseccb_decrypt_delete);
        PD_UNION_FIELD(pd_ipseccb_decrypt_get);

        // l4lb
        PD_UNION_FIELD(pd_l4lb_create);

        // cpucb
        PD_UNION_FIELD(pd_cpucb_create);
        PD_UNION_FIELD(pd_cpucb_update);
        PD_UNION_FIELD(pd_cpucb_delete);
        PD_UNION_FIELD(pd_cpucb_get);

        // rawrcb
        PD_UNION_FIELD(pd_rawrcb_create);
        PD_UNION_FIELD(pd_rawrcb_update);
        PD_UNION_FIELD(pd_rawrcb_delete);
        PD_UNION_FIELD(pd_rawrcb_get);

        // rawccb
        PD_UNION_FIELD(pd_rawccb_create);
        PD_UNION_FIELD(pd_rawccb_update);
        PD_UNION_FIELD(pd_rawccb_delete);
        PD_UNION_FIELD(pd_rawccb_get);

        // proxyrcb
        PD_UNION_FIELD(pd_proxyrcb_create);
        PD_UNION_FIELD(pd_proxyrcb_update);
        PD_UNION_FIELD(pd_proxyrcb_delete);
        PD_UNION_FIELD(pd_proxyrcb_get);

        // proxyccb
        PD_UNION_FIELD(pd_proxyccb_create);
        PD_UNION_FIELD(pd_proxyccb_update);
        PD_UNION_FIELD(pd_proxyccb_delete);
        PD_UNION_FIELD(pd_proxyccb_get);

        // qos clas
        PD_UNION_FIELD(pd_qos_class_create);
        PD_UNION_FIELD(pd_qos_class_restore);
        PD_UNION_FIELD(pd_qos_class_delete);
        PD_UNION_FIELD(pd_qos_class_update);
        PD_UNION_FIELD(pd_qos_class_make_clone);
        PD_UNION_FIELD(pd_qos_class_mem_free);
        PD_UNION_FIELD(pd_qos_class_get);

        // copp
        PD_UNION_FIELD(pd_copp_create);
        PD_UNION_FIELD(pd_copp_restore);
        PD_UNION_FIELD(pd_copp_delete);
        PD_UNION_FIELD(pd_copp_update);
        PD_UNION_FIELD(pd_copp_make_clone);
        PD_UNION_FIELD(pd_copp_mem_free);
        PD_UNION_FIELD(pd_copp_get);

        // acl
        PD_UNION_FIELD(pd_acl_create);
        PD_UNION_FIELD(pd_acl_restore);
        PD_UNION_FIELD(pd_acl_delete);
        PD_UNION_FIELD(pd_acl_update);
        PD_UNION_FIELD(pd_acl_mem_free);
        PD_UNION_FIELD(pd_acl_make_clone);
        PD_UNION_FIELD(pd_acl_get);

        // wring
        PD_UNION_FIELD(pd_wring_create);
        PD_UNION_FIELD(pd_wring_delete);
        PD_UNION_FIELD(pd_wring_update);
        PD_UNION_FIELD(pd_wring_get_entry);
        PD_UNION_FIELD(pd_wring_get_meta);
        PD_UNION_FIELD(pd_wring_set_meta);

        // mirror session
        PD_UNION_FIELD(pd_mirror_session_create);
        PD_UNION_FIELD(pd_mirror_session_delete);
        PD_UNION_FIELD(pd_mirror_session_get);

        // flow monitoring rule
        PD_UNION_FIELD(pd_flow_monitor_rule_create);
        PD_UNION_FIELD(pd_flow_monitor_rule_delete);
        PD_UNION_FIELD(pd_flow_monitor_rule_get);

        // drop monitoring rule
        PD_UNION_FIELD(pd_drop_monitor_rule_create);
        PD_UNION_FIELD(pd_drop_monitor_rule_delete);
        PD_UNION_FIELD(pd_drop_monitor_rule_get);

        // collector
        PD_UNION_FIELD(pd_collector_create);

        // mc entry
        PD_UNION_FIELD(pd_mc_entry_create);
        PD_UNION_FIELD(pd_mc_entry_delete);
        PD_UNION_FIELD(pd_mc_entry_update);

        // flow get
        PD_UNION_FIELD(pd_flow_get);

        // l2seg-uplink
        PD_UNION_FIELD(pd_add_l2seg_uplink);
        PD_UNION_FIELD(pd_del_l2seg_uplink);

        // debug cli
        PD_UNION_FIELD(pd_debug_cli_read);
        PD_UNION_FIELD(pd_debug_cli_write);
        PD_UNION_FIELD(pd_mpu_trace_enable);

        // get apis
        PD_UNION_FIELD(pd_if_get_hw_lif_id);
        PD_UNION_FIELD(pd_if_get_lport_id);
        PD_UNION_FIELD(pd_if_get_tm_oport);

        // twice nat
        PD_UNION_FIELD(pd_rw_entry_find_or_alloc);
        PD_UNION_FIELD(pd_twice_nat_add);
        PD_UNION_FIELD(pd_twice_nat_del);

        // qos
        PD_UNION_FIELD(pd_qos_class_get_qos_class_id);
        PD_UNION_FIELD(pd_qos_class_get_admin_cos);

        // aol
        PD_UNION_FIELD(pd_descriptor_aol_get);

        // crypto
        PD_UNION_FIELD(pd_crypto_alloc_key);
        PD_UNION_FIELD(pd_crypto_free_key);
        PD_UNION_FIELD(pd_crypto_write_key);
        PD_UNION_FIELD(pd_crypto_read_key);
        PD_UNION_FIELD(pd_crypto_asym_alloc_key);
        PD_UNION_FIELD(pd_crypto_asym_free_key);
        PD_UNION_FIELD(pd_crypto_asym_write_key);
        PD_UNION_FIELD(pd_crypto_asym_read_key);

        // barco
        PD_UNION_FIELD(pd_get_opaque_tag_addr);

        // stats
        PD_UNION_FIELD(pd_drop_stats_get);
        PD_UNION_FIELD(pd_table_stats_get);
        PD_UNION_FIELD(pd_system_drop_action);

        // oifl
        PD_UNION_FIELD(pd_oif_list_create);
        PD_UNION_FIELD(pd_oif_list_create_block);
        PD_UNION_FIELD(pd_oif_list_delete);
        PD_UNION_FIELD(pd_oif_list_delete_block);
        PD_UNION_FIELD(pd_oif_list_attach);
        PD_UNION_FIELD(pd_oif_list_detach);
        PD_UNION_FIELD(pd_oif_list_add_oif);
        PD_UNION_FIELD(pd_oif_list_add_qp_oif);
        PD_UNION_FIELD(pd_oif_list_remove_oif);
        PD_UNION_FIELD(pd_oif_list_is_member);
        PD_UNION_FIELD(pd_oif_list_get_num_oifs);
        PD_UNION_FIELD(pd_oif_list_get_oif_array);
        PD_UNION_FIELD(pd_oif_list_set_honor_ingress);
        PD_UNION_FIELD(pd_oif_list_clr_honor_ingress);

        PD_UNION_FIELD(pd_tunnelif_get_rw_idx);

        // cpu
        PD_UNION_FIELD(pd_cpupkt_ctxt_alloc_init);
        PD_UNION_FIELD(pd_cpupkt_register_rx_queue);
        PD_UNION_FIELD(pd_cpupkt_register_tx_queue);
        PD_UNION_FIELD(pd_cpupkt_unregister_tx_queue);
        PD_UNION_FIELD(pd_cpupkt_poll_receive);
        PD_UNION_FIELD(pd_cpupkt_free);
        PD_UNION_FIELD(pd_cpupkt_send);
        PD_UNION_FIELD(pd_cpupkt_page_alloc);
        PD_UNION_FIELD(pd_cpupkt_descr_alloc);
        PD_UNION_FIELD(pd_cpupkt_program_send_ring_doorbell);

        // rdma
        PD_UNION_FIELD(pd_rxdma_table_entry_add);
        PD_UNION_FIELD(pd_txdma_table_entry_add);

        // lif
        PD_UNION_FIELD(pd_lif_get_lport_id);

        // p4pt
        PD_UNION_FIELD(p4pt_pd_init);

        // eth
        PD_UNION_FIELD(pd_rss_params_table_entry_add);
        PD_UNION_FIELD(pd_rss_indir_table_entry_add);

        // asic init
        PD_UNION_FIELD(pd_asic_init);

        PD_UNION_FIELD(pd_table_properties_get);

        // capri
        PD_UNION_FIELD(pd_get_start_offset);
        PD_UNION_FIELD(pd_get_size_kb);
        PD_UNION_FIELD(pd_push_qstate_to_capri);
        PD_UNION_FIELD(pd_clear_qstate);
        PD_UNION_FIELD(pd_read_qstate);
        PD_UNION_FIELD(pd_write_qstate);
        PD_UNION_FIELD(pd_get_pc_offset);
        PD_UNION_FIELD(pd_capri_hbm_read_mem);
        PD_UNION_FIELD(pd_capri_hbm_write_mem);
        PD_UNION_FIELD(pd_capri_program_label_to_offset);
        PD_UNION_FIELD(pd_capri_pxb_cfg_lif_bdf);
        PD_UNION_FIELD(pd_capri_program_to_base_addr);
        PD_UNION_FIELD(pd_capri_barco_asym_req_descr_get);
        PD_UNION_FIELD(pd_capri_barco_symm_req_descr_get);
        PD_UNION_FIELD(pd_capri_barco_ring_meta_get);
        PD_UNION_FIELD(pd_capri_barco_asym_ecc_point_mul_p256);
        PD_UNION_FIELD(pd_capri_barco_asym_ecdsa_p256_sig_gen);
        PD_UNION_FIELD(pd_capri_barco_asym_ecdsa_p256_sig_verify);
        PD_UNION_FIELD(pd_capri_barco_asym_rsa2k_encrypt);
        PD_UNION_FIELD(pd_capri_barco_asym_rsa2k_decrypt);
        PD_UNION_FIELD(pd_capri_barco_asym_rsa2k_crt_decrypt);
        PD_UNION_FIELD(pd_capri_barco_asym_rsa2k_setup_private_key);
        PD_UNION_FIELD(pd_capri_barco_asym_rsa2k_sig_gen);
        PD_UNION_FIELD(pd_capri_barco_asym_rsa2k_sig_verify);
        PD_UNION_FIELD(pd_capri_barco_sym_hash_process_request);

        // gft APIs
        PD_UNION_FIELD(pd_gft_exact_match_profile_create);
        PD_UNION_FIELD(pd_gft_hdr_group_xposition_profile_create);
        PD_UNION_FIELD(pd_gft_exact_match_flow_entry_create);

        // clock
        PD_UNION_FIELD(pd_conv_hw_clock_to_sw_clock);
        PD_UNION_FIELD(pd_conv_sw_clock_to_hw_clock);
        PD_UNION_FIELD(pd_clock_delta_comp);

        // slab
        PD_UNION_FIELD(pd_get_slab);

        // swphv
        PD_UNION_FIELD(pd_swphv_inject);
        PD_UNION_FIELD(pd_swphv_get_state);
    };

} pd_call_t;

hal_ret_t hal_pd_call(pd_func_id_t pd_func_id, void *args);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_API_HPP__

