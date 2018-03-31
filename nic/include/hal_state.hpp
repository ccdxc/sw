// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_STATE_HPP__
#define __HAL_STATE_HPP__

#include "sdk/list.hpp"
#include "sdk/catalog.hpp"
#include "sdk/slab.hpp"
#include "sdk/indexer.hpp"
#include "sdk/ht.hpp"
#include "sdk/timestamp.hpp"
#include "sdk/shmmgr.hpp"
#include "nic/include/eventmgr.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/ip.h"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/periodic/periodic.hpp"

#ifdef SHM
#define slab_ptr_t        offset_ptr<slab>
#define TO_SLAB_PTR(x)    (x).get()
#else
#define slab_ptr_t        slab *
#define TO_SLAB_PTR(x)    x
#endif

namespace hal {

using sdk::lib::slab;
using sdk::lib::indexer;
using sdk::lib::ht;
using hal::utils::bitmap;
using hal::utils::eventmgr;
using sdk::lib::dllist_ctxt_t;

typedef enum hal_timer_id_s {
    HAL_TIMER_ID_CLOCK_SYNC               = 0,
    HAL_TIMER_ID_SESSION_AGEOUT           = 1,
    HAL_TIMER_ID_TCP_CLOSE_WAIT           = 2,
    HAL_TIMER_ID_TCP_HALF_CLOSED_WAIT     = 3,
    HAL_TIMER_ID_TCP_CXNSETUP_WAIT        = 4,
    HAL_TIMER_ID_MAX                      = 5, 
} hal_timer_id_t;

#define HAL_HANDLE_HT_SZ                             (16 << 10)

// TODO: this should be coming from catalogue or platform API
#define HAL_MAX_TM_PORTS                             12

// forward declaration
class hal_handle;

//------------------------------------------------------------------------------
// HAL config database
// This should consist of only the slabs needed to allocate config related
// objects, and the state from this is persisted and restored during upgrades
//------------------------------------------------------------------------------
class hal_cfg_db {
public:
    static hal_cfg_db *factory(shmmgr *mmgr = NULL);
    static void destroy(hal_cfg_db *cfg_db);
    void init_on_restart(void);

    void rlock(void) { rwlock_.rlock(); }
    void runlock(void) { rwlock_.runlock(); }
    void wlock(void) { rwlock_.wlock(); }
    void wunlock(void) { rwlock_.wunlock(); }

    // API to call before processing any packet by FTE, any operation by config
    hal_ret_t db_open(cfg_op_t cfg_op);

    // API to call after processing any packet by FTE, any operation by config
    // thread or periodic thread etc.
    hal_ret_t db_close(void);

    // API to register a config object with HAL infra
    hal_ret_t register_cfg_object(hal_obj_id_t obj_id, uint32_t obj_sz);
    //uint32_t object_size(hal_obj_id_t obj_id) const;

    slab *get_slab(hal_slab_t slab_id);
    slab *hal_handle_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_HANDLE]); }
    slab *hal_handle_ht_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_HANDLE_HT_ENTRY]); }

    // TODO: Is this config or oper ??
    slab *hal_handle_list_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_HANDLE_LIST_ENTRY]); }
    slab *hal_handle_id_ht_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_HANDLE_ID_HT_ENTRY]); }
    slab *hal_handle_id_list_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_HANDLE_ID_LIST_ENTRY]); }

    slab *dos_policy_sg_list_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_DOS_POLICY_SG_LIST]); }
    slab *vrf_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_VRF]); }
    slab *network_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NETWORK]); }
    slab *nwsec_profile_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_SECURITY_PROFILE]); }
    slab *nwsec_group_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_GROUP]); }
    slab *nwsec_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_RULE]); }
    slab *ruledb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_RULE]); }
    slab *nwsec_policy_rules_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_POLICY_RULES]); }
    slab *nwsec_policy_cfg_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_POLICY_CFG]); }
    slab *nwsec_policy_svc_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_POLICY_SVC]); }
    slab *nwsec_policy_appid_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_POLICY_APPID]); }
    slab *dos_policy_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_DOS_POLICY]); }
    slab *l2seg_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_L2SEG]); }
    slab *mc_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_MC_ENTRY]); }
    slab *lif_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_LIF]); }
    slab *if_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IF]); }
    slab *enic_l2seg_entry_slab(void) { return TO_SLAB_PTR(slabs_[HAL_SLAB_ENIC_L2SEG_ENTRY]); }
    slab *ep_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_EP]); }
    slab *ep_ip_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_EP_IP_ENTRY]); }
    slab *ep_l3_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_EP_L3_ENTRY]); }
    slab *l4lb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_L4LB]); }
    slab *flow_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_FLOW]); }
    slab *session_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_SESSION]); }
    slab *tlscb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_TLSCB]); }
    slab *tcpcb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_TCPCB]); }
    slab *qos_class_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_QOS_CLASS]); }
    slab *copp_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_COPP]); }
    slab *acl_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_ACL]); }
    slab *wring_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_WRING]); }
    slab *ipseccb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IPSECCB]); }
    slab *cpucb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_CPUCB]); }
    slab *rawrcb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RAWRCB]); }
    slab *rawccb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RAWCCB]); }
    slab *proxyrcb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXYRCB]); }
    slab *proxyccb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXYCCB]); }
    slab *gft_exact_match_profile_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_GFT_EXACT_MATCH_PROFILE]); }
    slab *gft_hdr_transposition_profile_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE]); }
    slab *gft_exact_match_flow_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY]); }
    slab *proxy_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXY]); }
    slab *proxy_flow_info_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXY_FLOW_INFO]); }

private:
    slab_ptr_t    slabs_[HAL_SLAB_PI_MAX - HAL_SLAB_PI_MIN];

    // following comes from linux process virtual memory
    shmmgr       *mmgr_;
    wp_rwlock    rwlock_;
    typedef struct slab_meta_s {
        hal_obj_id_t       obj_id;           // unique object id
    } slab_meta_t;
    slab_meta_t        slab_meta_[HAL_SLAB_PI_MAX];

private:
    hal_cfg_db();
    ~hal_cfg_db();
    bool init_pss(shmmgr *mmgr);
    bool init_vss(void);
    bool init(shmmgr *mmgr = NULL);
};

//------------------------------------------------------------------------------
// HAL oper db consists of the following:
// 1. caches built for faster lookups in the data path (and hence need not be
//    preserved/restored but can be rebuilt during upgrades)
// 2. any opertional state that is learnt - DHCP/ARP EP learns etc.
//------------------------------------------------------------------------------
class hal_oper_db {
public:
    static hal_oper_db *factory(shmmgr *mmgr = NULL);
    static void destroy(hal_oper_db *oper_db);
    void init_on_restart(void);

    ht *hal_handle_id_ht(void) const { return hal_handle_id_ht_; };
    ht *vrf_id_ht(void) const { return vrf_id_ht_; }
    ht *network_key_ht(void) const { return network_key_ht_; }
    ht *nwsec_profile_id_ht(void) const { return nwsec_profile_id_ht_; }
    ht *nwsec_policy_cfg_ht(void) const { return nwsec_policy_cfg_ht_; }
    ht *nwsec_policy_ht(void) const { return nwsec_policy_ht_; }
    ht *nwsec_group_ht(void) const { return nwsec_group_ht_; }
    ht *l2seg_id_ht(void) const { return l2seg_id_ht_; }
    ht *ep_l2_ht(void) const { return ep_l2_ht_; }
    ht *ep_l3_entry_ht(void) const { return ep_l3_entry_ht_; }
    ht *mc_key_ht(void) const { return mc_key_ht_; }
    ht *lif_id_ht(void) const { return lif_id_ht_; }
    ht *if_id_ht(void) const { return if_id_ht_; }
    ht *l4lb_ht(void) const { return l4lb_ht_; }
    ht *session_id_ht(void) const { return session_id_ht_; }
    ht *session_hal_handle_ht(void) const { return session_hal_handle_ht_; }
    ht *session_hal_iflow_ht(void)  const { return session_hal_iflow_ht_; }
    ht *session_hal_rflow_ht(void)  const { return session_hal_rflow_ht_; }
    ht *qos_class_ht(void) const { return qos_class_ht_; }
    bitmap *qos_cmap_pcp_bmp(void) const { return qos_cmap_pcp_bmp_; }
    bitmap *qos_cmap_dscp_bmp(void) const { return qos_cmap_dscp_bmp_; }
    ht *tlscb_id_ht(void) const { return tlscb_id_ht_; }
    ht *tcpcb_id_ht(void) const { return tcpcb_id_ht_; }
    ht *copp_ht(void) const { return copp_ht_; }
    ht *acl_ht(void) const { return acl_ht_; }
    ht *wring_id_ht(void) const { return wring_id_ht_; }
    ht *proxy_type_ht(void) const { return proxy_type_ht_; }
    ht *ipseccb_id_ht(void) const { return ipseccb_id_ht_; }
    ht *cpucb_id_ht(void) const { return cpucb_id_ht_; }
    ht *rawrcb_id_ht(void) const { return rawrcb_id_ht_; }
    ht *rawccb_id_ht(void) const { return rawccb_id_ht_; }
    ht *proxyrcb_id_ht(void) const { return proxyrcb_id_ht_; }
    ht *proxyccb_id_ht(void) const { return proxyccb_id_ht_; }
    ht *gft_exact_match_profile_id_ht(void) const { return gft_exact_match_profile_id_ht_; }
    ht *gft_hdr_transposition_profile_id_ht(void) const { return gft_hdr_transposition_profile_id_ht_; }
    ht *gft_exact_match_flow_entry_id_ht(void) const { return gft_exact_match_flow_entry_id_ht_; }

    void set_infra_vrf_handle(hal_handle_t infra_vrf_hdl) { infra_vrf_handle_ = infra_vrf_hdl; }
    hal_handle_t infra_vrf_handle(void) const { return infra_vrf_handle_; }
    ip_addr_t *mytep(void) { return &mytep_ip; }
    eventmgr *event_mgr(void) const { return event_mgr_; }
    if_id_t app_redir_if_id(void) const { return app_redir_if_id_; }
    void set_app_redir_if_id(if_id_t id) { 
        app_redir_if_id_ = id;
    }
    void set_forwarding_mode(hal_forwarding_mode_t mode) {
        forwarding_mode_ = mode;
    }
    hal_forwarding_mode_t forwarding_mode(void) const { return forwarding_mode_; }

private:
    // following can come from shared memory or non-linux HBM memory
    // NOTE: strictly shmnot required as we can rebuild this from slab elements,
    //       but restart case will be extremely efficent if we do this
    ht    *hal_handle_id_ht_;
    ht    *vrf_id_ht_;
    ht    *network_key_ht_;
    ht    *nwsec_profile_id_ht_;
    ht    *nwsec_policy_cfg_ht_;
    ht    *nwsec_policy_ht_;
    ht    *nwsec_group_ht_;
    ht    *l2seg_id_ht_;
    ht    *ep_l2_ht_;
    ht    *ep_l3_entry_ht_;
    ht    *mc_key_ht_;
    ht    *lif_id_ht_;
    ht    *if_id_ht_;
    ht    *session_id_ht_;
    ht    *session_hal_handle_ht_;
    ht    *session_hal_iflow_ht_;
    ht    *session_hal_rflow_ht_;
    ht    *l4lb_ht_;
    ht    *tlscb_id_ht_;
    ht    *tcpcb_id_ht_;
    ht    *qos_class_ht_;
    ht    *copp_ht_;
    ht    *wring_id_ht_;
    ht    *proxy_type_ht_;
    ht    *acl_ht_;
    ht    *ipseccb_id_ht_;
    ht    *cpucb_id_ht_;
    ht    *rawrcb_id_ht_;
    ht    *rawccb_id_ht_;
    ht    *proxyrcb_id_ht_;
    ht    *proxyccb_id_ht_;
    ht    *gft_exact_match_profile_id_ht_;
    ht    *gft_hdr_transposition_profile_id_ht_;
    ht    *gft_exact_match_flow_entry_id_ht_;
    bitmap                  *qos_cmap_pcp_bmp_;
    bitmap                  *qos_cmap_dscp_bmp_;

    hal_handle_t            infra_vrf_handle_;    // infra vrf handle
    eventmgr                *event_mgr_;
    ip_addr_t               mytep_ip;
    hal_forwarding_mode_t   forwarding_mode_;
    if_id_t                 app_redir_if_id_;

    // following comes from linux process virtual memory
    shmmgr       *mmgr_;

private:
    bool init_pss(shmmgr *mmgr);
    bool init_vss(void);
    bool init(shmmgr *mmgr = NULL);
    hal_oper_db();
    ~hal_oper_db();
};

//------------------------------------------------------------------------------
// Whole of HAL state including both config & oper - it has all the HAL state
// including all slab memory instances, hash tables, indexers etc. used inside
// HAL and this class is instantiated during init time
// NOTE: any HAL state put outside this is not under warranty
//------------------------------------------------------------------------------
class hal_state {
public:
    hal_state(hal_obj_meta **obj_meta, shmmgr *mmgr = NULL);
    ~hal_state();
    void init_on_restart(void);
    shmmgr *mmgr(void) const { return mmgr_; }

    // get APIs for various DBs
    hal_cfg_db *cfg_db(void) const { return cfg_db_; }
    hal_oper_db *oper_db(void) const { return oper_db_; }

    slab *get_slab(hal_slab_t slab_id) const { return cfg_db_->get_slab(slab_id); }

    // get APIs for HAL handle related state
    slab *hal_handle_slab(void) const { return cfg_db_->hal_handle_slab(); }
    slab *hal_handle_ht_entry_slab(void) const { return cfg_db_->hal_handle_ht_entry_slab(); }
    slab *hal_handle_list_entry_slab(void) const { return cfg_db_->hal_handle_list_entry_slab(); }
    slab *hal_handle_id_ht_entry_slab(void) const { return cfg_db_->hal_handle_id_ht_entry_slab(); }
    slab *hal_handle_id_list_entry_slab(void) const { return cfg_db_->hal_handle_id_list_entry_slab(); }
    slab *dos_policy_sg_list_entry_slab(void) const { return cfg_db_->dos_policy_sg_list_entry_slab(); }
    ht *hal_handle_id_ht(void) const { return oper_db_->hal_handle_id_ht(); }

     hal_ret_t register_cfg_object(hal_obj_id_t obj_id, uint32_t obj_sz) {
         return cfg_db_->register_cfg_object(obj_id, obj_sz);
     }
     //uint32_t object_size(hal_obj_id_t obj_id) { return cfg_db_->object_size(obj_id); }

    // get APIs for vrf related state
    slab *vrf_slab(void) const { return cfg_db_->vrf_slab(); }
    ht *vrf_id_ht(void) const { return oper_db_->vrf_id_ht(); }

    // get APIs for network related state
    slab *network_slab(void) const { return cfg_db_->network_slab(); }
    ht *network_key_ht(void) const { return oper_db_->network_key_ht(); }

    // get APIs for security profile related state
    slab *nwsec_profile_slab(void) const { return cfg_db_->nwsec_profile_slab(); }
    ht *nwsec_profile_id_ht(void) const { return oper_db_->nwsec_profile_id_ht(); }

    // get APIs for security Policy related state
    slab *nwsec_group_slab(void) const { return cfg_db_->nwsec_group_slab(); }
    slab *nwsec_rule_slab(void) const { return cfg_db_->nwsec_rule_slab(); }
    slab *ruledb_slab(void) const    { return cfg_db_->ruledb_slab(); }
    slab *nwsec_policy_rules_slab(void) const { return cfg_db_->nwsec_policy_rules_slab(); }
    slab *nwsec_policy_cfg_slab(void) const { return cfg_db_->nwsec_policy_cfg_slab(); }
    slab *nwsec_policy_svc_slab(void) const { return cfg_db_->nwsec_policy_svc_slab(); }
    slab *nwsec_policy_appid_slab(void) const { return cfg_db_->nwsec_policy_appid_slab(); }
    ht   *nwsec_policy_cfg_ht(void) const { return oper_db_->nwsec_policy_cfg_ht(); }
    ht   *nwsec_policy_ht(void) const { return oper_db_->nwsec_policy_ht(); }
    ht   *nwsec_group_ht(void) const { return oper_db_->nwsec_group_ht(); }

    // get APIs for dos policy related state
    slab *dos_policy_slab(void) const { return cfg_db_->dos_policy_slab(); }

    // get APIs for L2 segment state
    slab *l2seg_slab(void) const { return cfg_db_->l2seg_slab(); }
    ht *l2seg_id_ht(void) const { return oper_db_->l2seg_id_ht(); }

    // get APIs for multicast entry state
    slab *mc_entry_slab(void) const { return cfg_db_->mc_entry_slab(); }
    ht *mc_key_ht(void) const { return oper_db_->mc_key_ht(); }

    // get APIs for LIF state
    slab *lif_slab(void) const { return cfg_db_->lif_slab(); }
    ht *lif_id_ht(void) const { return oper_db_->lif_id_ht(); }

    // get APIs for interface state
    slab *if_slab(void) const { return cfg_db_->if_slab(); }
    ht *if_id_ht(void) const { return oper_db_->if_id_ht(); }
    slab *enic_l2seg_entry_slab(void) const { return cfg_db_->enic_l2seg_entry_slab(); }

    // get APIs for endpoint state
    slab *ep_slab(void) const { return cfg_db_->ep_slab(); }
    slab *ep_ip_entry_slab(void) const { return cfg_db_->ep_ip_entry_slab(); }
    slab *ep_l3_entry_slab(void) const { return cfg_db_->ep_l3_entry_slab(); }
    ht *ep_l2_ht(void) const { return oper_db_->ep_l2_ht(); }
    ht *ep_l3_entry_ht(void) const { return oper_db_->ep_l3_entry_ht(); }

    // get APIs for l4lb state
    slab *l4lb_slab(void) const { return cfg_db_->l4lb_slab(); }
    ht *l4lb_ht(void) const { return oper_db_->l4lb_ht(); }

    // get APIs for flow/session state
    slab *flow_slab(void) const { return cfg_db_->flow_slab(); }
    slab *session_slab(void) const { return cfg_db_->session_slab(); }
    ht *session_id_ht(void) const { return oper_db_->session_id_ht(); }
    ht *session_hal_handle_ht(void) const { return oper_db_->session_hal_handle_ht(); }
    ht *session_hal_iflow_ht(void) const { return oper_db_->session_hal_iflow_ht(); }
    ht *session_hal_rflow_ht(void) const { return oper_db_->session_hal_rflow_ht(); }

    // get APIs for TLS CB state
    slab *tlscb_slab(void) const { return cfg_db_->tlscb_slab(); }
    ht *tlscb_id_ht(void) const { return oper_db_->tlscb_id_ht(); }

    // get APIs for TCP CB state
    slab *tcpcb_slab(void) const { return cfg_db_->tcpcb_slab(); }
    ht *tcpcb_id_ht(void) const { return oper_db_->tcpcb_id_ht(); }

    // get APIs for qos-class state
    slab *qos_class_slab(void) const { return cfg_db_->qos_class_slab(); }
    ht *qos_class_ht(void) const { return oper_db_->qos_class_ht(); }
    bitmap *qos_cmap_pcp_bmp(void) const { return oper_db_->qos_cmap_pcp_bmp(); }
    bitmap *qos_cmap_dscp_bmp(void) const { return oper_db_->qos_cmap_dscp_bmp(); }

    // get APIs for copp state
    slab *copp_slab(void) const { return cfg_db_->copp_slab(); }
    ht *copp_ht(void) const { return oper_db_->copp_ht(); }

    // get APIs for acl state
    slab *acl_slab(void) const { return cfg_db_->acl_slab(); }
    ht *acl_ht(void) const { return oper_db_->acl_ht(); }

    // get APIs for WRing state
    slab *wring_slab(void) const { return cfg_db_->wring_slab(); }
    ht *wring_id_ht(void) const { return oper_db_->wring_id_ht(); }

    // get APIs for Proxy state
    slab *proxy_slab(void) const { return cfg_db_->proxy_slab(); }
    slab *proxy_flow_info_slab(void) const { return cfg_db_->proxy_flow_info_slab(); }
    ht *proxy_type_ht(void) const { return oper_db_->proxy_type_ht(); }

    // get API for infra VRF
    hal_handle_t infra_vrf_handle(void) { return oper_db_->infra_vrf_handle(); }
    void set_infra_vrf_handle(hal_handle_t infra_vrf_handle) { oper_db_->set_infra_vrf_handle(infra_vrf_handle); }

    // get APIs for IPSEC CB state
    slab *ipseccb_slab(void) const { return cfg_db_->ipseccb_slab(); }
    ht *ipseccb_id_ht(void) const { return oper_db_->ipseccb_id_ht(); }

    // get APIs for CPU CB state
    slab *cpucb_slab(void) const { return cfg_db_->cpucb_slab(); }
    ht *cpucb_id_ht(void) const { return oper_db_->cpucb_id_ht(); }

    // get APIs for Raw Redirect CB state
    slab *rawrcb_slab(void) const { return cfg_db_->rawrcb_slab(); }
    ht *rawrcb_id_ht(void) const { return oper_db_->rawrcb_id_ht(); }
    if_id_t app_redir_if_id(void) const { return oper_db_->app_redir_if_id(); }

    void set_app_redir_if_id(if_id_t id) { 
        oper_db_->set_app_redir_if_id(id);
    }

    // get APIs for Raw Chain CB state
    slab *rawccb_slab(void) const { return cfg_db_->rawccb_slab(); }
    ht *rawccb_id_ht(void) const { return oper_db_->rawccb_id_ht(); }

    // get APIs for Raw Redirect CB state
    slab *proxyrcb_slab(void) const { return cfg_db_->proxyrcb_slab(); }
    ht *proxyrcb_id_ht(void) const { return oper_db_->proxyrcb_id_ht(); }

    // get APIs for Raw Chain CB state
    slab *proxyccb_slab(void) const { return cfg_db_->proxyccb_slab(); }
    ht *proxyccb_id_ht(void) const { return oper_db_->proxyccb_id_ht(); }

    // get APIs for GFT state
    slab *gft_exact_match_profile_slab(void) const { return cfg_db_->gft_exact_match_profile_slab(); }
    ht *gft_exact_match_profile_id_ht(void) const { return oper_db_->gft_exact_match_profile_id_ht(); }
    slab *gft_hdr_transposition_profile_slab(void) const { return cfg_db_->gft_hdr_transposition_profile_slab(); }
    ht *gft_hdr_transposition_profile_id_ht(void) const { return oper_db_->gft_hdr_transposition_profile_id_ht(); }
    slab *gft_exact_match_flow_entry_slab(void) const { return cfg_db_->gft_exact_match_flow_entry_slab(); }
    ht *gft_exact_match_flow_entry_id_ht(void) const { return oper_db_->gft_exact_match_flow_entry_id_ht(); }

    // forwarding mode APIs
    void set_forwarding_mode(hal_forwarding_mode_t mode) { 
        oper_db_->set_forwarding_mode(mode);
    }
    hal_forwarding_mode_t forwarding_mode(void) const { return oper_db_->forwarding_mode(); }

    eventmgr *event_mgr(void) const { return oper_db_->event_mgr(); }

    sdk::lib::catalog *catalog(void) const { return catalog_; }
    void set_catalog(sdk::lib::catalog *catalog) { catalog_ = catalog; }

    hal_stats_t api_stats(int idx) const { return api_stats_[idx]; }
    void set_api_stats(int idx, int val) { api_stats_[idx] = val; }
    uint64_t preserve_state(void);
    uint64_t restore_state(void *mem);

private:
    // following come from shared memory or non-linux HBM memory
    hal_cfg_db           *cfg_db_;
    hal_oper_db          *oper_db_;
    hal_stats_t          *api_stats_;

    // following come from linux process virtual memory
    shmmgr               *mmgr_;
    sdk::lib::catalog    *catalog_;
    hal_obj_meta         **obj_meta_;

private:
    void cleanup(void);
};

extern class hal_state    *g_hal_state;
extern bool               g_delay_delete;

static inline bool
is_forwarding_mode_host_pinned (void)
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED;
}

static inline bool
is_forwarding_mode_classic_nic (void)
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC;
}

static inline bool
is_forwarding_mode_smart_nic (void)
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_SWITCH;
}

static inline if_id_t
get_app_redir_if_id (void)
{
    return g_hal_state->app_redir_if_id();
}

//------------------------------------------------------------------------------
// callback invoked by the timerwheel to release an object to its slab
//------------------------------------------------------------------------------
static inline void
slab_delay_delete_cb (void *timer, hal_slab_t slab_id, void *elem)
{
    hal_ret_t    ret;

    if (slab_id < HAL_SLAB_PI_MAX) {
        ret = hal::free_to_slab(slab_id, elem);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("[{}:{}] Failed to release elem {} to slab id {}",
                          __FUNCTION__, __LINE__, elem, slab_id);
        }
    } else {
        HAL_TRACE_ERR("{}: Unexpected slab id {}", __FUNCTION__, slab_id);
    }

    return;
}

//------------------------------------------------------------------------------
// wrapper function to delay delete slab elements
// NOTE: currently delay delete timeout is 2 seconds, it is expected that any
//       other threads using (a pointer to) this object should be done with this
//       object within this timeout or else this memory can be freed and
//       allocated for other objects and can result in corruptions. Hence, tune
//       this timeout, if needed
//------------------------------------------------------------------------------
static inline hal_ret_t
delay_delete_to_slab (hal_slab_t slab_id, void *elem)
{
    void    *timer_ctxt;

    HAL_ASSERT(elem != NULL);

    if (g_delay_delete && hal::periodic::periodic_thread_is_running()) {
        timer_ctxt =
            hal::periodic::timer_schedule(slab_id,
                                          TIME_MSECS_PER_SEC << 1, elem,
                                          (sdk::lib::twheel_cb_t)slab_delay_delete_cb,
                                          false);
        if (!timer_ctxt) {
            return HAL_RET_ERR;
        }
    } else {
        slab_delay_delete_cb(NULL, slab_id, elem);
    }
    return HAL_RET_OK;
}

}    // namespace hal

#endif    // __HAL_STATE_HPP__

