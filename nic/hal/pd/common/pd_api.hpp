#ifndef __HAL_PD_API_HPP__
#define __HAL_PD_API_HPP__

#include "nic/hal/src/tenant.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/src/dos.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/network.hpp"
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
#include "nic/hal/src/port.hpp"

namespace hal {
namespace pd {

using hal::tenant_t;
using hal::l2seg_t;
using hal::network_t;
using hal::nwsec_profile_t;
using hal::dos_policy_t;
using hal::if_t;
using hal::port_t;
using hal::lif_t;
using hal::session_t;
using hal::flow_key_t;
using hal::flow_t;
using hal::tlscb_t;
using hal::tcpcb_t;
using hal::buf_pool_t;
using hal::queue_t;
using hal::policer_t;
using hal::acl_t;
using hal::wring_t;
using hal::mirror_session_t;
using hal::ipseccb_t;
using hal::l4lb_service_entry_t;
using hal::cpucb_t;
using hal::rawrcb_t;
using hal::rawccb_t;

typedef uint32_t    l2seg_hw_id_t;

typedef struct pd_tenant_args_s {
    tenant_t           *tenant;
    nwsec_profile_t    *nwsec_profile;
} __PACK__ pd_tenant_args_t;

typedef struct pd_l2seg_args_s {
    tenant_t           *tenant;
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
} pd_lif_upd_args_t;

typedef struct pd_port_args_s {
    port_t  *pi_p;

    PortType         port_type;               // port type
    PortSpeed        port_speed;              // port speed
    PortAdminState   admin_state;             // admin state of the port
    uint32_t         mac_id;                  // mac id associated with the port
    uint32_t         mac_ch;                  // mac channel associated with the port
    uint32_t         num_lanes;               // number of lanes for the port
} pd_port_args_t;

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
                bool            pinned_uplink_clsc_change;
                hal_handle_t    new_pinned_uplink_clsc;
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
    tenant_t        *tenant;
    l2seg_t         *l2seg;
    if_t            *intf;
    ep_t            *ep;
} __PACK__ pd_ep_args_t;

typedef struct pd_ep_upd_args_s {
    ep_t            *ep;

    // IP list change attrs
    bool            iplist_change;
    dllist_ctxt_t   *add_iplist;
    dllist_ctxt_t   *del_iplist;

    // X change attrs
} pd_ep_upd_args_t;

typedef struct pd_sessoin_args_s {
    tenant_t           *tenant;
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

typedef struct pd_rw_entry_args_s {
    mac_addr_t          mac_sa;
    mac_addr_t          mac_da;
    rewrite_actions_en  rw_act;
} __PACK__ pd_rw_entry_args_t;


typedef struct pd_system_args_s {
    SystemResponse *rsp;
} __PACK__ pd_system_args_t;

static inline void
pd_system_args_init(pd_system_args_t *args)
{
    args->rsp = NULL;
}

static inline void
pd_tenant_args_init (pd_tenant_args_t *args)
{
    args->tenant = NULL;
    return;
}

static inline void
pd_l2seg_args_init (pd_l2seg_args_t *args)
{
    args->tenant = NULL;
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
pd_port_args_init (pd_port_args_t *args)
{
    args->pi_p = NULL;
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
    args->tenant = NULL;
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
    args->tenant = NULL;
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

hal_ret_t pd_tenant_create(pd_tenant_args_t *tenant);
hal_ret_t pd_tenant_update(pd_tenant_args_t *tenant);
hal_ret_t pd_tenant_delete(pd_tenant_args_t *tenant);
hal_ret_t pd_tenant_mem_free(pd_tenant_args_t *tenant);
hal_ret_t pd_tenant_make_clone(tenant_t *ten, tenant_t *clone);

hal_ret_t pd_l2seg_create(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_update(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_delete(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_mem_free(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_make_clone(l2seg_t *l2seg, l2seg_t *clone);
l2seg_t *find_l2seg_by_hwid (l2seg_hw_id_t hwid);
l2seg_hw_id_t pd_l2seg_get_ten_hwid(l2seg_t *l2seg);
bool pd_l2seg_get_fromcpu_id(l2seg_t *l2seg, uint16_t *vid);

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

hal_ret_t pd_if_create(pd_if_args_t *hal_if);
hal_ret_t pd_if_update(pd_if_args_t *hal_if);
hal_ret_t pd_if_delete(pd_if_args_t *hal_if);
hal_ret_t pd_if_nwsec_update(pd_if_nwsec_upd_args_t *args);
hal_ret_t pd_if_lif_update(pd_if_lif_upd_args_t *args);
hal_ret_t pd_if_mem_free(pd_if_args_t *intf);
hal_ret_t pd_if_make_clone(if_t *hal_if, if_t *clone);

hal_ret_t pd_port_create     (pd_port_args_t *hal_port);
hal_ret_t pd_port_update     (pd_port_args_t *hal_port);
hal_ret_t pd_port_delete     (pd_port_args_t *hal_port);
hal_ret_t pd_port_mem_free   (pd_port_args_t *hal_port);
hal_ret_t pd_port_make_clone (port_t *pi_p, port_t *clone);
bool      pd_port_has_speed_changed
                             (pd_port_args_t *hal_port);

uint32_t if_get_hw_lif_id(if_t *pi_if);
uint32_t if_get_lport_id(if_t *pi_if);
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
hal_ret_t pd_rawrcb_create(pd_rawrcb_args_t *rawrcb);
hal_ret_t pd_rawrcb_update(pd_rawrcb_args_t *rawrcb);
hal_ret_t pd_rawrcb_delete(pd_rawrcb_args_t *rawrcb);
hal_ret_t pd_rawrcb_get(pd_rawrcb_args_t *rawrcb);

hal_ret_t pd_rawccb_create(pd_rawccb_args_t *rawccb);
hal_ret_t pd_rawccb_update(pd_rawccb_args_t *rawccb);
hal_ret_t pd_rawccb_delete(pd_rawccb_args_t *rawccb);
hal_ret_t pd_rawccb_get(pd_rawccb_args_t *rawccb);

typedef struct pd_buf_pool_args_s {
    buf_pool_t    *buf_pool;
} __PACK__ pd_buf_pool_args_t;

static inline void
pd_buf_pool_args_init (pd_buf_pool_args_t *args)
{
    args->buf_pool = NULL;
    return;
}

hal_ret_t pd_buf_pool_create(pd_buf_pool_args_t *buf_pool);

typedef struct pd_queue_args_s {
    uint32_t    cnt_l0;
    queue_t    **l0_nodes;
    uint32_t    cnt_l1;
    queue_t    **l1_nodes;
    uint32_t    cnt_l2;
    queue_t    **l2_nodes;
} __PACK__ pd_queue_args_t;

static inline void
pd_queue_args_init (pd_queue_args_t *args)
{
    args->cnt_l0 = 0;
    args->cnt_l1 = 0;
    args->cnt_l2 = 0;
    args->l0_nodes = NULL;
    args->l1_nodes = NULL;
    args->l2_nodes = NULL;
    return;
}

hal_ret_t pd_queue_create(pd_queue_args_t *queue);

typedef struct pd_policer_args_s {
    policer_t    *policer;
} __PACK__ pd_policer_args_t;

static inline void
pd_policer_args_init (pd_policer_args_t *args)
{
    args->policer = NULL;
    return;
}

hal_ret_t pd_policer_create(pd_policer_args_t *policer);

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


typedef struct pd_descr_aol_s {
	uint64_t	scratch[8];
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

hal_ret_t pd_crypto_alloc_key(int32_t *key_idx);
hal_ret_t pd_crypto_free_key(int32_t key_idx);
hal_ret_t pd_crypto_write_key(int32_t key_idx, crypto_key_t *key);
hal_ret_t pd_crypto_read_key(int32_t key_idx, crypto_key_t *key);

hal_ret_t pd_drop_stats_get(pd_system_args_t *pd_sys_args);

// OIF List related PD APIs
hal_ret_t oif_list_create(oif_list_id_t *list); // Creates a new oif_list and returns handle
hal_ret_t oif_list_delete(oif_list_id_t list);  // Takes an oiflis_handle and deletes it
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif); // Adds an oif to list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif); // Removes an oif from list
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif); // Check if an oif is present in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs); // Get the number of oifs in the list
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs); // Get an array of all oifs in the list

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_API_HPP__
