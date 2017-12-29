#ifndef __HAL_PD_API_HPP__
#define __HAL_PD_API_HPP__

#include "nic/hal/src/vrf.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/src/nwsec_group.hpp"
#include "nic/hal/src/dos.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/multicast.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/tlscb.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/hal/src/qos.hpp"
#include "nic/hal/src/dos.hpp"
#include "nic/hal/src/acl.hpp"
#include "nic/hal/src/wring.hpp"
#include "nic/hal/src/telemetry.hpp"
#include "nic/hal/src/crypto_keys.hpp"
#include "nic/hal/src/ipseccb.hpp"
#include "nic/hal/src/l4lb.hpp"
#include "nic/hal/src/cpucb.hpp"
#include "nic/hal/src/system.hpp"
#include "nic/include/oif_list_api.hpp"
#include "nic/hal/src/rawrcb.hpp"
#include "nic/hal/src/rawccb.hpp"
#include "nic/hal/src/proxyrcb.hpp"
#include "nic/hal/src/proxyccb.hpp"
#include "nic/hal/src/barco_rings.hpp"

namespace hal {
namespace pd {

using hal::vrf_t;
using hal::l2seg_t;
using hal::network_t;
using hal::nwsec_profile_t;
using hal::dos_policy_t;
using hal::if_t;
using hal::lif_t;
using hal::session_t;
using hal::flow_key_t;
using hal::flow_t;
using hal::tlscb_t;
using hal::tcpcb_t;
using hal::qos_class_t;
using hal::acl_t;
using hal::wring_t;
using hal::mirror_session_t;
using hal::ipseccb_t;
using hal::l4lb_service_entry_t;
using hal::cpucb_t;
using hal::rawrcb_t;
using hal::rawccb_t;
using hal::proxyrcb_t;
using hal::proxyccb_t;
using hal::copp_t;

typedef uint32_t    mc_entry_hw_id_t;
typedef uint32_t    l2seg_hw_id_t;

typedef struct pd_vrf_args_s {
    vrf_t           *vrf;
    nwsec_profile_t    *nwsec_profile;
} __PACK__ pd_vrf_args_t;

typedef struct pd_l2seg_args_s {
    vrf_t           *vrf;
    l2seg_t            *l2seg;
} __PACK__ pd_l2seg_args_t;

typedef struct pd_nwsec_profile_args_s {
    nwsec_profile_t    *nwsec_profile;
    nwsec_profile_t    *clone_profile;
} __PACK__ pd_nwsec_profile_args_t;

typedef struct pd_dos_policy_args_s {
    dos_policy_t    *dos_policy;
    dos_policy_t    *clone_policy;
} __PACK__ pd_dos_policy_args_t;

typedef struct pd_lif_args_s {
    lif_t    *lif;
    bool     with_hw_lif_id;
    uint32_t hw_lif_id;
} __PACK__ pd_lif_args_t;

typedef struct pd_lif_upd_args_s {
    lif_t    *lif;

    bool    vlan_strip_en_changed;
    bool    vlan_strip_en;
    bool    qstate_map_init_set;
    bool    rss_config_changed;
    bool    rx_policer_changed;
    bool    tx_policer_changed;
} pd_lif_upd_args_t;

typedef struct pd_if_args_s {
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
} __PACK__ pd_if_args_t;

typedef struct pd_if_nwsec_upd_args_s {
    l2seg_t         *l2seg;
    if_t            *intf;
    nwsec_profile_t *nwsec_prof;
} __PACK__ pd_if_nwsec_upd_args_t;

typedef struct pd_if_lif_upd_args_s {
    if_t            *intf;
    lif_t           *lif;

    bool            vlan_strip_en_changed;
    bool            vlan_strip_en;
} __PACK__ pd_if_lif_upd_args_t;

typedef struct pd_ep_args_s {
    vrf_t        *vrf;
    l2seg_t         *l2seg;
    if_t            *intf;
    ep_t            *ep;
} __PACK__ pd_ep_args_t;

typedef struct pd_ep_upd_args_s {
    ep_t                    *ep;

    // IP list change attrs, deprecate and take from app_ctxt
    bool                    iplist_change;
    dllist_ctxt_t           *add_iplist;
    dllist_ctxt_t           *del_iplist;

    ep_update_app_ctxt_t    *app_ctxt;

    // X change attrs
} pd_ep_upd_args_t;

typedef struct pd_sessoin_args_s {
    vrf_t           *vrf;
    nwsec_profile_t    *nwsec_prof;
    bool               update_iflow;

#if 0
    l2seg_t            *l2seg_s;
    l2seg_t            *l2seg_d;
    if_t               *sif;
    if_t               *dif;
    ep_t               *sep;
    ep_t               *dep;
#endif
    session_t          *session;
    session_state_t    *session_state;
    SessionResponse    *rsp;
} __PACK__ pd_session_args_t;

typedef struct pd_l2seg_uplink_args_s {
    l2seg_t     *l2seg;
    if_t        *intf;
} __PACK__ pd_l2seg_uplink_args_t;

typedef struct pd_tlscb_args_s {
    tlscb_t            *tlscb;
} __PACK__ pd_tlscb_args_t;

typedef struct pd_tcpcb_args_s {
    tcpcb_t            *tcpcb;
} __PACK__ pd_tcpcb_args_t;

typedef struct pd_wring_args_s {
    wring_t            *wring;
} __PACK__ pd_wring_args_t;

typedef struct pd_mirror_session_args_s {
    mirror_session_t *session;
} __PACK__ pd_mirror_session_args_t;

typedef struct pd_ipseccb_args_s {
    ipseccb_t            *ipseccb;
} __PACK__ pd_ipseccb_args_t;

typedef struct pd_l4lb_args_s {
    l4lb_service_entry_t    *l4lb;
} __PACK__ pd_l4lb_args_t;

typedef struct pd_cpucb_args_s {
    cpucb_t            *cpucb;
} __PACK__ pd_cpucb_args_t;

typedef struct pd_rawrcb_args_s {
    rawrcb_t           *rawrcb;
} __PACK__ pd_rawrcb_args_t;

typedef struct pd_rawccb_args_s {
    rawccb_t           *rawccb;
} __PACK__ pd_rawccb_args_t;

typedef struct pd_proxyrcb_args_s {
    proxyrcb_t         *proxyrcb;
} __PACK__ pd_proxyrcb_args_t;

typedef struct pd_proxyccb_args_s {
    proxyccb_t         *proxyccb;
} __PACK__ pd_proxyccb_args_t;

typedef struct pd_rw_entry_args_s {
    mac_addr_t          mac_sa;
    mac_addr_t          mac_da;
    rewrite_actions_en  rw_act;
} __PACK__ pd_rw_entry_args_t;

typedef struct pd_twice_nat_entry_args_s {
    ip_addr_t               nat_ip;
    uint16_t                nat_l4_port;
    twice_nat_actions_en    twice_nat_act;
} __PACK__ pd_twice_nat_entry_args_t;

typedef struct pd_system_args_s {
    SystemResponse *rsp;
} __PACK__ pd_system_args_t;

static inline void
pd_system_args_init(pd_system_args_t *args)
{
    args->rsp = NULL;
}

static inline void
pd_vrf_args_init (pd_vrf_args_t *args)
{
    args->vrf = NULL;
    return;
}

static inline void
pd_l2seg_args_init (pd_l2seg_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    return;
}

static inline void
pd_nwsec_profile_args_init (pd_nwsec_profile_args_t *args)
{
    args->nwsec_profile = NULL;
    return;
}

static inline void
pd_dos_policy_args_init (pd_dos_policy_args_t *args)
{
    args->dos_policy = NULL;
    return;
}

static inline void
pd_lif_args_init (pd_lif_args_t *args)
{
    args->lif = NULL;
    args->with_hw_lif_id = false;
    args->hw_lif_id = 0;
    return;
}

static inline void
pd_lif_upd_args_init (pd_lif_upd_args_t *args)
{
    memset(args, 0, sizeof(pd_lif_upd_args_t));
    return;
}

static inline void
pd_if_args_init (pd_if_args_t *args)
{
    args->intf = NULL;
    return;
}

static inline void
pd_if_nwsec_upd_args_init (pd_if_nwsec_upd_args_t *args)
{
    args->intf = NULL;
    args->nwsec_prof = NULL;
    return;
}

static inline void
pd_if_lif_upd_args_init (pd_if_lif_upd_args_t *args)
{
    memset(args, 0, sizeof(pd_if_lif_upd_args_t));
    return;
}

static inline void
pd_ep_args_init (pd_ep_args_t *args)
{
    args->vrf = NULL;
    args->l2seg = NULL;
    args->intf = NULL;
    args->ep = NULL;
    return;
}

static inline void
pd_ep_upd_args_init (pd_ep_upd_args_t *args)
{
    memset(args, 0, sizeof(pd_ep_upd_args_t));
    return;
}
static inline void
pd_session_args_init (pd_session_args_t *args)
{
    args->vrf = NULL;
    args->session = NULL;
    args->session_state = NULL;
    args->rsp = NULL;
    args->update_iflow = false;

    return;
}

static inline void
pd_l2seg_uplinkif_args_init (pd_l2seg_uplink_args_t *args)
{
    args->l2seg = NULL;
    args->intf = NULL;
    return;
}

static inline void
pd_tlscb_args_init (pd_tlscb_args_t *args)
{
    args->tlscb = NULL;
    return;
}

static inline void
pd_tcpcb_args_init (pd_tcpcb_args_t *args)
{
    args->tcpcb = NULL;
    return;
}

static inline void
pd_wring_args_init (pd_wring_args_t *args)
{
    args->wring = NULL;
    return;
}

static inline void
pd_mirror_session_args_init(pd_mirror_session_args_t *args)
{
    args->session = NULL;
}

static inline void
pd_ipseccb_args_init (pd_ipseccb_args_t *args)
{
    args->ipseccb = NULL;
    return;
}

static inline void
pd_l4lb_args_init (pd_l4lb_args_t *args)
{
    args->l4lb = NULL;
    return;
}

static inline void
pd_cpucb_args_init (pd_cpucb_args_t *args)
{
    args->cpucb = NULL;
    return;
}

static inline void
pd_rawrcb_args_init (pd_rawrcb_args_t *args)
{
    args->rawrcb = NULL;
    return;
}

static inline void
pd_rawccb_args_init (pd_rawccb_args_t *args)
{
    args->rawccb = NULL;
    return;
}

static inline void
pd_proxyrcb_args_init (pd_proxyrcb_args_t *args)
{
    args->proxyrcb = NULL;
    return;
}

static inline void
pd_proxyccb_args_init (pd_proxyccb_args_t *args)
{
    args->proxyccb = NULL;
    return;
}

hal_ret_t pd_vrf_create(pd_vrf_args_t *vrf);
hal_ret_t pd_vrf_update(pd_vrf_args_t *vrf);
hal_ret_t pd_vrf_delete(pd_vrf_args_t *vrf);
hal_ret_t pd_vrf_mem_free(pd_vrf_args_t *vrf);
hal_ret_t pd_vrf_make_clone(vrf_t *ten, vrf_t *clone);

hal_ret_t pd_l2seg_create(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_update(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_delete(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_mem_free(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_make_clone(l2seg_t *l2seg, l2seg_t *clone);
l2seg_t *find_l2seg_by_hwid (l2seg_hw_id_t hwid);
hal_ret_t pd_get_object_from_flow_lkupid(uint32_t flow_lkupid, 
                                         hal_obj_id_t *obj_id,
                                         void **pi_obj);
l2seg_hw_id_t pd_l2seg_get_flow_lkupid(l2seg_t *l2seg);
uint32_t pd_vrf_get_lookup_id(vrf_t *vrf);
hal_ret_t pd_l2seg_get_fromcpu_vlanid(l2seg_t *l2seg, uint16_t *vid);
hal_ret_t pd_vrf_get_fromcpu_vlanid(vrf_t *vrf, uint16_t *vid);

hal_ret_t pd_nwsec_profile_create(pd_nwsec_profile_args_t *nwsec_profile);
hal_ret_t pd_nwsec_profile_update(pd_nwsec_profile_args_t *nwsec_profile);
hal_ret_t pd_nwsec_profile_delete(pd_nwsec_profile_args_t *nwsec_profile);
hal_ret_t pd_nwsec_profile_mem_free(pd_nwsec_profile_args_t *args);
hal_ret_t pd_nwsec_profile_make_clone(nwsec_profile_t *nwsec, 
                                      nwsec_profile_t *clone);

hal_ret_t pd_dos_policy_create(pd_dos_policy_args_t *dos_policy);
hal_ret_t pd_dos_policy_update(pd_dos_policy_args_t *dos_policy);
hal_ret_t pd_dos_policy_delete(pd_dos_policy_args_t *dos_policy);
hal_ret_t pd_dos_policy_mem_free(pd_dos_policy_args_t *args);
hal_ret_t pd_dos_policy_make_clone(dos_policy_t *nwsec, 
                                      dos_policy_t *clone);

hal_ret_t pd_lif_create(pd_lif_args_t *lif);
hal_ret_t pd_lif_update(pd_lif_upd_args_t *lif);
hal_ret_t pd_lif_delete(pd_lif_args_t *lif);
hal_ret_t pd_lif_mem_free(pd_lif_args_t *lif);
hal_ret_t pd_lif_make_clone(lif_t *lif, lif_t *clone);
uint32_t  pd_get_hw_lif_id(lif_t *lif);

hal_ret_t pd_if_create(pd_if_args_t *hal_if);
hal_ret_t pd_if_update(pd_if_args_t *hal_if);
hal_ret_t pd_if_delete(pd_if_args_t *hal_if);
hal_ret_t pd_if_nwsec_update(pd_if_nwsec_upd_args_t *args);
hal_ret_t pd_if_lif_update(pd_if_lif_upd_args_t *args);
hal_ret_t pd_if_mem_free(pd_if_args_t *intf);
hal_ret_t pd_if_make_clone(if_t *hal_if, if_t *clone);

hal_ret_t
pd_debug_cli_read(uint32_t tableid,
                  uint32_t index,
                  void     *swkey,
                  void     *swkey_mask,
                  void     *actiondata);

hal_ret_t
pd_debug_cli_write(uint32_t tableid,
                   uint32_t index,
                   void     *swkey,
                   void     *swkey_mask,
                   void     *actiondata);

uint32_t if_get_hw_lif_id(if_t *pi_if);
uint32_t if_get_lport_id(if_t *pi_if);
uint32_t if_get_tm_oport(if_t *pi_if);
uint32_t ep_pd_get_rw_tbl_idx_from_pi_ep(ep_t *pi_ep, 
        rewrite_actions_en rw_act);
uint32_t ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(ep_t *pi_ep,
        tunnel_rewrite_actions_en tnnl_rw_act);
uint32_t l4lb_pd_get_rw_tbl_idx_from_pi_l4lb(l4lb_service_entry_t *pi_l4lb, 
                                             rewrite_actions_en rw_act);

hal_ret_t pd_ep_create(pd_ep_args_t *pd_ep_args);
hal_ret_t pd_ep_update(pd_ep_upd_args_t *pd_ep_upd_args);
hal_ret_t pd_ep_delete(pd_ep_args_t *pd_ep_args);
hal_ret_t pd_ep_mem_free(pd_ep_args_t *args);
hal_ret_t pd_ep_make_clone(ep_t *ep, ep_t *clone);

hal_ret_t pd_session_create(pd_session_args_t *session);
hal_ret_t pd_session_update(pd_session_args_t *session);
hal_ret_t pd_session_delete(pd_session_args_t *session);

hal_ret_t pd_add_l2seg_uplink(pd_l2seg_uplink_args_t *args);
hal_ret_t pd_del_l2seg_uplink(pd_l2seg_uplink_args_t *args);

hal_ret_t pd_tlscb_create(pd_tlscb_args_t *tlscb);
hal_ret_t pd_tlscb_update(pd_tlscb_args_t *tlscb);
hal_ret_t pd_tlscb_delete(pd_tlscb_args_t *tlscb);
hal_ret_t pd_tlscb_get(pd_tlscb_args_t *tlscb);

hal_ret_t pd_tcpcb_create(pd_tcpcb_args_t *tcpcb);
hal_ret_t pd_tcpcb_update(pd_tcpcb_args_t *tcpcb);
hal_ret_t pd_tcpcb_delete(pd_tcpcb_args_t *tcpcb);
hal_ret_t pd_tcpcb_get(pd_tcpcb_args_t *tcpcb);

hal_ret_t pd_ipseccb_create(pd_ipseccb_args_t *ipseccb);
hal_ret_t pd_ipseccb_update(pd_ipseccb_args_t *ipseccb);
hal_ret_t pd_ipseccb_delete(pd_ipseccb_args_t *ipseccb);
hal_ret_t pd_ipseccb_get(pd_ipseccb_args_t *ipseccb);

hal_ret_t pd_ipseccb_decrypt_create(pd_ipseccb_args_t *ipseccb);
hal_ret_t pd_ipseccb_decrypt_update(pd_ipseccb_args_t *ipseccb);
hal_ret_t pd_ipseccb_decrypt_delete(pd_ipseccb_args_t *ipseccb);
hal_ret_t pd_ipseccb_decrypt_get(pd_ipseccb_args_t *ipseccb);

hal_ret_t pd_l4lb_create(pd_l4lb_args_t *pd_l4lb_args);

hal_ret_t pd_cpucb_create(pd_cpucb_args_t *cpucb);
hal_ret_t pd_cpucb_update(pd_cpucb_args_t *cpucb);
hal_ret_t pd_cpucb_delete(pd_cpucb_args_t *cpucb);
hal_ret_t pd_cpucb_get(pd_cpucb_args_t *cpucb);
hal_ret_t pd_rw_entry_find_or_alloc(pd_rw_entry_args_t *rw_key, 
                                    uint32_t *rw_idx);

hal_ret_t pd_twice_nat_add(pd_twice_nat_entry_args_t *args,
                           uint32_t *twice_nat_idx);
hal_ret_t pd_twice_nat_del(pd_twice_nat_entry_args_t *args);
hal_ret_t pd_rawrcb_create(pd_rawrcb_args_t *rawrcb);
hal_ret_t pd_rawrcb_update(pd_rawrcb_args_t *rawrcb);
hal_ret_t pd_rawrcb_delete(pd_rawrcb_args_t *rawrcb,
                           bool retain_in_db = true);
hal_ret_t pd_rawrcb_get(pd_rawrcb_args_t *rawrcb);

hal_ret_t pd_rawccb_create(pd_rawccb_args_t *rawccb);
hal_ret_t pd_rawccb_update(pd_rawccb_args_t *rawccb);
hal_ret_t pd_rawccb_delete(pd_rawccb_args_t *rawccb,
                           bool retain_in_db = true);
hal_ret_t pd_rawccb_get(pd_rawccb_args_t *rawccb);
hal_ret_t pd_proxyrcb_create(pd_proxyrcb_args_t *proxyrcb);
hal_ret_t pd_proxyrcb_update(pd_proxyrcb_args_t *proxyrcb);
hal_ret_t pd_proxyrcb_delete(pd_proxyrcb_args_t *proxyrcb,
                             bool retain_in_db = true);
hal_ret_t pd_proxyrcb_get(pd_proxyrcb_args_t *proxyrcb);

hal_ret_t pd_proxyccb_create(pd_proxyccb_args_t *proxyccb);
hal_ret_t pd_proxyccb_update(pd_proxyccb_args_t *proxyccb);
hal_ret_t pd_proxyccb_delete(pd_proxyccb_args_t *proxyccb,
                             bool retain_in_db = true);
hal_ret_t pd_proxyccb_get(pd_proxyccb_args_t *proxyccb);

typedef struct pd_qos_class_args_s {
    qos_class_t    *qos_class;
} __PACK__ pd_qos_class_args_t;

static inline void
pd_qos_class_args_init (pd_qos_class_args_t *args)
{
    args->qos_class = NULL;
    return;
}

hal_ret_t pd_qos_class_create(pd_qos_class_args_t *qos_class);
hal_ret_t pd_qos_class_delete(pd_qos_class_args_t *qos_class);

hal_ret_t qos_class_get_qos_class_id(qos_class_t *qos_class, 
                                     if_t *dest_if, 
                                     uint32_t *qos_class_id);

hal_ret_t qos_class_get_qos_class_id(qos_class_t *qos_class, 
                                     if_t *dest_if, 
                                     uint32_t *qos_class_id);

typedef struct pd_copp_args_s {
    copp_t    *copp;
} __PACK__ pd_copp_args_t;

static inline void
pd_copp_args_init (pd_copp_args_t *args)
{
    args->copp = NULL;
    return;
}

hal_ret_t pd_copp_create(pd_copp_args_t *copp);
hal_ret_t pd_copp_delete(pd_copp_args_t *copp);

typedef struct pd_acl_args_s {
    acl_t    *acl;
} __PACK__ pd_acl_args_t;

static inline void
pd_acl_args_init (pd_acl_args_t *args)
{
    args->acl = NULL;
    return;
}

hal_ret_t pd_acl_create(pd_acl_args_t *acl);
hal_ret_t pd_acl_delete(pd_acl_args_t *acl);

hal_ret_t pd_wring_create(pd_wring_args_t *wring);
hal_ret_t pd_wring_update(pd_wring_args_t *wring);
hal_ret_t pd_wring_delete(pd_wring_args_t *wring);
hal_ret_t pd_wring_get_entry(pd_wring_args_t *wring);
hal_ret_t pd_wring_get_meta (pd_wring_args_t *wring);
hal_ret_t pd_wring_set_meta (pd_wring_args_t *wring);


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

hal_ret_t
pd_descriptor_aol_get(pd_descr_aol_t *src, pd_descr_aol_t *dst);

hal_ret_t pd_mirror_session_create(pd_mirror_session_args_t *args);
hal_ret_t pd_mirror_session_get(pd_mirror_session_args_t *args);
hal_ret_t pd_mirror_session_delete(pd_mirror_session_args_t *args);
hal_ret_t pd_collector_create(collector_config_t *cfg);

hal_ret_t pd_crypto_alloc_key(int32_t *key_idx);
hal_ret_t pd_crypto_free_key(int32_t key_idx);
hal_ret_t pd_crypto_write_key(int32_t key_idx, crypto_key_t *key);
hal_ret_t pd_crypto_read_key(int32_t key_idx, crypto_key_t *key);

hal_ret_t pd_crypto_asym_alloc_key(int32_t *key_idx);
hal_ret_t pd_crypto_asym_free_key(int32_t key_idx);
hal_ret_t pd_crypto_asym_write_key(int32_t key_idx, crypto_asym_key_t *key);
hal_ret_t pd_crypto_asym_read_key(int32_t key_idx, crypto_asym_key_t *key);
hal_ret_t get_opaque_tag_addr(types::BarcoRings ring_type, uint64_t* addr);

hal_ret_t pd_drop_stats_get(pd_system_args_t *pd_sys_args);
hal_ret_t pd_table_stats_get(pd_system_args_t *pd_sys_args);

// OIF List related PD APIs
hal_ret_t oif_list_create(oif_list_id_t *list);
hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size);
hal_ret_t oif_list_delete(oif_list_id_t list);
hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size);
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif);
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif);
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif);
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif);
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs);
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs);
hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list);

typedef struct pd_mc_entry_args_s {
    mc_entry_t      *mc_entry;
} __PACK__ pd_mc_entry_args_t;

static inline void
pd_mc_entry_args_init (pd_mc_entry_args_t *args)
{
    args->mc_entry = NULL;
    return;
}

hal_ret_t pd_mc_entry_create(pd_mc_entry_args_t *mc_entry_args);
hal_ret_t pd_mc_entry_update(pd_mc_entry_args_t *mc_entry_args);
hal_ret_t pd_mc_entry_delete(pd_mc_entry_args_t *mc_entry_args);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_API_HPP__
