// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_STATE_HPP__
#define __HAL_STATE_HPP__

#include "sdk/list.hpp"
#include "sdk/catalog.hpp"
#include "nic/include/eventmgr.hpp"
#include "sdk/slab.hpp"
#include "sdk/indexer.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/ip.h"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "sdk/timestamp.hpp"

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

typedef enum hal_forwarding_mode_s {
    HAL_FORWARDING_MODE_NONE              = 0,
    HAL_FORWARDING_MODE_SMART_SWITCH      = 1,
    HAL_FORWARDING_MODE_SMART_HOST_PINNED = 2,
    HAL_FORWARDING_MODE_CLASSIC           = 3,
} hal_forwarding_mode_t;

#define HAL_HANDLE_HT_SZ                             (16 << 10)

// TODO: this should be coming from catalogue or platform API
#define HAL_MAX_TM_PORTS                             12

// forward declaration
class hal_handle;

// HAL config database
class hal_cfg_db {
public:
    static hal_cfg_db *factory(void);
    ~hal_cfg_db();

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
    uint32_t object_size(hal_obj_id_t obj_id) const;

    ht *vrf_id_ht(void) const { return vrf_id_ht_; }
    ht *network_key_ht(void) const { return network_key_ht_; }
    ht *nwsec_profile_id_ht(void) const { return nwsec_profile_id_ht_; }
    ht *nwsec_policy_cfg_ht(void) const { return nwsec_policy_cfg_ht_; }
    ht *nwsec_policy_ht(void) const { return nwsec_policy_ht_; }
    ht *nwsec_group_ht(void) const { return nwsec_group_ht_; }
    ht *l2seg_id_ht(void) const { return l2seg_id_ht_; }
    ht *mc_key_ht(void) const { return mc_key_ht_; }
    ht *lif_id_ht(void) const { return lif_id_ht_; }
    ht *if_id_ht(void) const { return if_id_ht_; }
    ht *l4lb_ht(void) const { return l4lb_ht_; }
    ht *l4lb_hal_handle_ht(void) const { return l4lb_hal_handle_ht_; }
    ht *session_id_ht(void) const { return session_id_ht_; }
    ht *session_hal_handle_ht(void) const { return session_hal_handle_ht_; }
    ht *session_hal_iflow_ht(void)  const { return session_hal_iflow_ht_; }
    ht *session_hal_rflow_ht(void)  const { return session_hal_rflow_ht_; }

    ht *tlscb_id_ht(void) const { return tlscb_id_ht_; }
    ht *tlscb_hal_handle_ht(void) const { return tlscb_hal_handle_ht_; }

    ht *tcpcb_id_ht(void) const { return tcpcb_id_ht_; }
    ht *tcpcb_hal_handle_ht(void) const { return tcpcb_hal_handle_ht_; }

    ht *qos_class_ht(void) const { return qos_class_ht_; }
    bitmap *qos_cmap_pcp_bmp(void) const { return qos_cmap_pcp_bmp_; }
    bitmap *qos_cmap_dscp_bmp(void) const { return qos_cmap_dscp_bmp_; }

    ht *copp_ht(void) const { return copp_ht_; }

    ht *acl_ht(void) const { return acl_ht_; }

    ht *wring_id_ht(void) const { return wring_id_ht_; }
    ht *wring_hal_handle_ht(void) const { return wring_hal_handle_ht_; }

    // get APIs for Proxy state
    slab *proxy_slab(void) const { return proxy_slab_; }
    slab *proxy_flow_info_slab(void) const { return proxy_flow_info_slab_; }
    ht *proxy_type_ht(void) const { return proxy_type_ht_; }
    ht *proxy_hal_handle_ht(void) const { return proxy_hal_handle_ht_; }

    // get APIs for IPSEC CB state
    ht *ipseccb_id_ht(void) const { return ipseccb_id_ht_; }
    ht *ipseccb_hal_handle_ht(void) const { return ipseccb_hal_handle_ht_; }

    ht *cpucb_id_ht(void) const { return cpucb_id_ht_; }
    ht *cpucb_hal_handle_ht(void) const { return cpucb_hal_handle_ht_; }

    // get APIs for Raw Redirect CB state
    ht *rawrcb_id_ht(void) const { return rawrcb_id_ht_; }
    ht *rawrcb_hal_handle_ht(void) const { return rawrcb_hal_handle_ht_; }
    if_id_t app_redir_if_id(void) const { return app_redir_if_id_; }

    void set_app_redir_if_id(if_id_t id) { 
        app_redir_if_id_ = id;
    }

    // get APIs for Raw Chain CB state
    ht *rawccb_id_ht(void) const { return rawccb_id_ht_; }
    ht *rawccb_hal_handle_ht(void) const { return rawccb_hal_handle_ht_; }

    // get APIs for Proxy Redirect CB state
    ht *proxyrcb_id_ht(void) const { return proxyrcb_id_ht_; }
    ht *proxyrcb_hal_handle_ht(void) const { return proxyrcb_hal_handle_ht_; }

    // get APIs for Proxy Chain CB state
    ht *proxyccb_id_ht(void) const { return proxyccb_id_ht_; }
    ht *proxyccb_hal_handle_ht(void) const { return proxyccb_hal_handle_ht_; }

    // get APIs for GFT state
    ht *gft_exact_match_profile_id_ht(void) const { return gft_exact_match_profile_id_ht_; }
    ht *gft_hdr_transposition_profile_id_ht(void) const { return gft_hdr_transposition_profile_id_ht_; }
    ht *gft_exact_match_flow_entry_id_ht(void) const { return gft_exact_match_flow_entry_id_ht_; }

    void set_forwarding_mode(std::string modestr);
    hal_forwarding_mode_t forwarding_mode() { return forwarding_mode_; }

private:
    bool init(void);
    hal_cfg_db();

private:
    // vrf/vrf related config
    struct {
        ht         *vrf_id_ht_;
    } __PACK__;

    // network related config
    struct {
        ht         *network_key_ht_;
    } __PACK__;

    // security profile related config
    struct {
        ht         *nwsec_profile_id_ht_;
    } __PACK__;

    // security group policy related config
    struct {
        ht         *nwsec_policy_cfg_ht_;
    }__PACK__;

    // security policy related config
    struct {
        ht         *nwsec_policy_ht_;
    }__PACK__;

    // security group related config
    struct {
        ht         *nwsec_group_ht_;
    }__PACK__;

    // l2segment related config
    struct {
        ht         *l2seg_id_ht_;
    } __PACK__;

    // mc_entry related config
    struct {
        ht         *mc_key_ht_;
    } __PACK__;

    // LIF related config
    struct {
        ht         *lif_id_ht_;
    } __PACK__;

    // interface related config
    struct {
        ht         *if_id_ht_;
    } __PACK__;

    // flow/session related config
    struct {
        ht         *session_id_ht_;
        ht         *session_hal_handle_ht_;
        ht         *session_hal_iflow_ht_;
        ht         *session_hal_rflow_ht_;
    } __PACK__;

    // l4lb related config
    struct {
        ht         *l4lb_ht_;
        ht         *l4lb_hal_handle_ht_;
    } __PACK__;

    // TLS CB related config
    struct {
        ht         *tlscb_id_ht_;
        ht         *tlscb_hal_handle_ht_;
    } __PACK__;

    // TCP CB related config
    struct {
        ht         *tcpcb_id_ht_;
        ht         *tcpcb_hal_handle_ht_;
    } __PACK__;

    // qos-class related config
    struct {
        ht         *qos_class_ht_;
        bitmap     *qos_cmap_pcp_bmp_;
        bitmap     *qos_cmap_dscp_bmp_;
    } __PACK__;

    // copp related config
    struct {
        ht         *copp_ht_;
    } __PACK__;

    // WRing related config
    struct {
        ht         *wring_id_ht_;
        ht         *wring_hal_handle_ht_;
    } __PACK__;

    // Proxy related config
    struct {
        slab       *proxy_slab_;
        slab       *proxy_flow_info_slab_; 
        ht         *proxy_type_ht_;
        ht         *proxy_hal_handle_ht_;
    } __PACK__;

    // acl related config
    struct {
        ht         *acl_ht_;
    } __PACK__;

    // IPSec CB related config
    struct {
        ht         *ipseccb_id_ht_;
        ht         *ipseccb_hal_handle_ht_;
    } __PACK__;

    // CPU CB related config
    struct {
        ht         *cpucb_id_ht_;
        ht         *cpucb_hal_handle_ht_;
    } __PACK__;

    // Raw Redirect CB related state
    struct {
        ht         *rawrcb_id_ht_;
        ht         *rawrcb_hal_handle_ht_;
        if_id_t    app_redir_if_id_;
    } __PACK__;

    // Raw Chain CB related state
    struct {
        ht         *rawccb_id_ht_;
        ht         *rawccb_hal_handle_ht_;
    } __PACK__;

    // Raw Redirect CB related state
    struct {
        ht         *proxyrcb_id_ht_;
        ht         *proxyrcb_hal_handle_ht_;
    } __PACK__;

    // Raw Chain CB related state
    struct {
        ht         *proxyccb_id_ht_;
        ht         *proxyccb_hal_handle_ht_;
    } __PACK__;

    // CFG parameters from JSON file.
    struct {
        hal_forwarding_mode_t   forwarding_mode_;
    } __PACK__;

    // GFT related config
    struct {
        ht         *gft_exact_match_profile_id_ht_;
        ht         *gft_hdr_transposition_profile_id_ht_;
        ht         *gft_exact_match_flow_entry_id_ht_;
    } __PACK__;

    wp_rwlock    rwlock_;
    typedef struct obj_meta_s {
        uint32_t        obj_sz;
    } __PACK__ obj_meta_t;
    obj_meta_t    obj_meta_[HAL_OBJ_ID_MAX];
};

// HAL operational database
class hal_oper_db {
public:
    static hal_oper_db *factory(void);
    ~hal_oper_db();

    ht *hal_handle_id_ht(void) const { return hal_handle_id_ht_; };
    void *infra_l2seg(void) { return infra_l2seg_; }
    void set_infra_l2seg(void *infra_l2seg) { infra_l2seg_ = infra_l2seg; }
    void *infra_vrf(void) { return infra_vrf_; }
    void set_infra_vrf(void *infra_vrf) { infra_vrf_ = infra_vrf; }
    ht *ep_l2_ht(void) const { return ep_l2_ht_; }
    ht *ep_l3_entry_ht(void) const { return ep_l3_entry_ht_; }
    ht *flow_ht(void) const { return flow_ht_; }
    ip_addr_t *mytep(void) { return &mytep_ip; }
    eventmgr *event_mgr(void) const { return event_mgr_; }

private:
    bool init(void);
    hal_oper_db();

private:
    void                   *infra_vrf_;      // Infra VRF *
    void                   *infra_l2seg_;    // l2seg_t *
    eventmgr               *event_mgr_;
    ht                     *hal_handle_id_ht_;
    ht                     *ep_l2_ht_;
    ht                     *ep_l3_entry_ht_;
    ht                     *flow_ht_;
    ip_addr_t               mytep_ip;
};

// HAL memory slabs and any other memory manager state
class hal_mem_db {
public:
    static hal_mem_db *factory(void);
    ~hal_mem_db();

    slab *get_slab(hal_slab_t slab_id);
    slab *hal_handle_slab(void) const { return hal_handle_slab_; }
    slab *hal_handle_ht_entry_slab(void) const { return hal_handle_ht_entry_slab_; }
    slab *hal_handle_list_entry_slab(void) const { return hal_handle_list_entry_slab_; }
    slab *hal_handle_id_ht_entry_slab(void) const { return hal_handle_id_ht_entry_slab_; }
    slab *hal_handle_id_list_entry_slab(void) const { return hal_handle_id_list_entry_slab_; }
    slab *dos_policy_sg_list_entry_slab(void) const { return dos_policy_sg_list_entry_slab_; }
    slab *vrf_slab(void) const { return vrf_slab_; }
    slab *network_slab(void) const { return network_slab_; }
    slab *nwsec_profile_slab(void) const { return nwsec_profile_slab_; }
    slab *nwsec_group_slab(void) const { return nwsec_group_slab_; }
    slab *nwsec_rule_slab(void) const { return nwsec_rule_slab_; }
    slab *ruledb_slab(void) const { return ruledb_slab_; }
    slab *nwsec_policy_rules_slab(void) const { return nwsec_policy_rules_slab_; }
    slab *nwsec_policy_cfg_slab(void) const { return nwsec_policy_cfg_slab_; }
    slab *nwsec_policy_svc_slab(void) const { return nwsec_policy_svc_slab_; }
    slab *nwsec_policy_appid_slab(void) const { return nwsec_policy_appid_slab_; }
    slab *dos_policy_slab(void) const { return dos_policy_slab_; }
    slab *l2seg_slab(void) const { return l2seg_slab_; }
    slab *mc_entry_slab(void) const { return mc_entry_slab_; }
    slab *lif_slab(void) const { return lif_slab_; }
    slab *if_slab(void) const { return if_slab_; }
    slab *enic_l2seg_entry_slab(void) { return enic_l2seg_entry_slab_; }
    slab *ep_slab(void) const { return ep_slab_; }
    slab *ep_ip_entry_slab(void) const { return ep_ip_entry_slab_; }
    slab *ep_l3_entry_slab(void) const { return ep_l3_entry_slab_; }
    slab *l4lb_slab(void) const { return l4lb_slab_; }
    slab *flow_slab(void) const { return flow_slab_; }
    slab *session_slab(void) const { return session_slab_; }
    slab *tlscb_slab(void) const { return tlscb_slab_; }
    slab *tcpcb_slab(void) const { return tcpcb_slab_; }
    slab *qos_class_slab(void) const { return qos_class_slab_; }
    slab *copp_slab(void) const { return copp_slab_; }
    slab *acl_slab(void) const { return acl_slab_; }
    slab *wring_slab(void) const { return wring_slab_; }
    slab *ipseccb_slab(void) const { return ipseccb_slab_; }
    slab *cpucb_slab(void) const { return cpucb_slab_; }
    slab *rawrcb_slab(void) const { return rawrcb_slab_; }
    slab *rawccb_slab(void) const { return rawccb_slab_; }
    slab *proxyrcb_slab(void) const { return proxyrcb_slab_; }
    slab *proxyccb_slab(void) const { return proxyccb_slab_; }
    slab *gft_exact_match_profile_slab(void) const { return gft_exact_match_profile_slab_; }
    slab *gft_hdr_transposition_profile_slab(void) const { return gft_hdr_transposition_profile_slab_; }
    slab *gft_exact_match_flow_entry_slab(void) const { return gft_exact_match_flow_entry_slab_; }

private:
    bool init(void);
    hal_mem_db();

private:
    slab    *hal_handle_slab_;
    slab    *hal_handle_ht_entry_slab_;
    slab    *hal_handle_list_entry_slab_;
    slab    *hal_handle_id_ht_entry_slab_;
    slab    *hal_handle_id_list_entry_slab_;
    slab    *dos_policy_sg_list_entry_slab_;
    slab    *vrf_slab_;
    slab    *network_slab_;
    slab    *nwsec_profile_slab_;
    slab    *nwsec_group_slab_;
    slab    *nwsec_rule_slab_;
    slab    *ruledb_slab_;
    slab    *nwsec_policy_rules_slab_;
    slab    *nwsec_policy_cfg_slab_;
    slab    *nwsec_policy_svc_slab_;
    slab    *nwsec_policy_appid_slab_;
    slab    *dos_policy_slab_;
    slab    *l2seg_slab_;
    slab    *mc_entry_slab_;
    slab    *lif_slab_;
    slab    *if_slab_;
    slab    *enic_l2seg_entry_slab_;
    slab    *ep_slab_;
    slab    *ep_ip_entry_slab_;
    slab    *ep_l3_entry_slab_;
    slab    *flow_slab_;
    slab    *session_slab_;
    slab    *l4lb_slab_;
    slab    *tlscb_slab_;
    slab    *tcpcb_slab_;
    slab    *qos_class_slab_;
    slab    *copp_slab_;
    slab    *wring_slab_;
    slab    *acl_slab_;
    slab    *ipseccb_slab_;
    slab    *cpucb_slab_;
    slab    *rawrcb_slab_;
    slab    *rawccb_slab_;
    slab    *proxyrcb_slab_;
    slab    *proxyccb_slab_;
    slab    *gft_exact_match_profile_slab_;
    slab    *gft_hdr_transposition_profile_slab_;
    slab    *gft_exact_match_flow_entry_slab_;
};

//------------------------------------------------------------------------------
// Whole of HAL state including both config & oper - it has all the HAL state
// including all slab memory instances, hash tables, indexers etc. used inside
// HAL and this class is instantiated during init time
// NOTE: any HAL state put outside this is not under warranty
//------------------------------------------------------------------------------
class hal_state {
public:
    static hal_state *factory(void);
    ~hal_state();

    // get APIs for various DBs
    hal_cfg_db *cfg_db(void) const { return cfg_db_; }
    hal_oper_db *oper_db(void) const { return oper_db_; }
    hal_mem_db *mem_db(void) const { return mem_db_; }

    slab *get_slab(hal_slab_t slab_id) const { return mem_db_->get_slab(slab_id); }

    // get APIs for HAL handle related state
    slab *hal_handle_slab(void) const { return mem_db_->hal_handle_slab(); }
    slab *hal_handle_ht_entry_slab(void) const { return mem_db_->hal_handle_ht_entry_slab(); }
    slab *hal_handle_list_entry_slab(void) const { return mem_db_->hal_handle_list_entry_slab(); }
    slab *hal_handle_id_ht_entry_slab(void) const { return mem_db_->hal_handle_id_ht_entry_slab(); }
    slab *hal_handle_id_list_entry_slab(void) const { return mem_db_->hal_handle_id_list_entry_slab(); }
    slab *dos_policy_sg_list_entry_slab(void) const { return mem_db_->dos_policy_sg_list_entry_slab(); }
    ht *hal_handle_id_ht(void) const { return oper_db_->hal_handle_id_ht(); }

     hal_ret_t register_cfg_object(hal_obj_id_t obj_id, uint32_t obj_sz) {
         return cfg_db_->register_cfg_object(obj_id, obj_sz);
     }
     uint32_t object_size(hal_obj_id_t obj_id) { return cfg_db_->object_size(obj_id); }

    // get APIs for vrf related state
    slab *vrf_slab(void) const { return mem_db_->vrf_slab(); }
    ht *vrf_id_ht(void) const { return cfg_db_->vrf_id_ht(); }

    // get APIs for network related state
    slab *network_slab(void) const { return mem_db_->network_slab(); }
    ht *network_key_ht(void) const { return cfg_db_->network_key_ht(); }

    // get APIs for security profile related state
    slab *nwsec_profile_slab(void) const { return mem_db_->nwsec_profile_slab(); }
    ht *nwsec_profile_id_ht(void) const { return cfg_db_->nwsec_profile_id_ht(); }

    // get APIs for security Policy related state
    slab *nwsec_group_slab(void) const { return mem_db_->nwsec_group_slab(); }
    slab *nwsec_rule_slab(void) const { return mem_db_->nwsec_rule_slab(); }
    slab *ruledb_slab(void) const    { return mem_db_->ruledb_slab(); }
    slab *nwsec_policy_rules_slab(void) const { return mem_db_->nwsec_policy_rules_slab(); }
    slab *nwsec_policy_cfg_slab(void) const { return mem_db_->nwsec_policy_cfg_slab(); }
    slab *nwsec_policy_svc_slab(void) const { return mem_db_->nwsec_policy_svc_slab(); }
    slab *nwsec_policy_appid_slab(void) const { return mem_db_->nwsec_policy_appid_slab(); }
    ht   *nwsec_policy_cfg_ht(void) const { return cfg_db_->nwsec_policy_cfg_ht(); }
    ht   *nwsec_policy_ht(void) const { return cfg_db_->nwsec_policy_ht(); }
    ht   *nwsec_group_ht(void) const { return cfg_db_->nwsec_group_ht(); }

    // get APIs for dos policy related state
    slab *dos_policy_slab(void) const { return mem_db_->dos_policy_slab(); }

    // get APIs for L2 segment state
    slab *l2seg_slab(void) const { return mem_db_->l2seg_slab(); }
    ht *l2seg_id_ht(void) const { return cfg_db_->l2seg_id_ht(); }

    // get APIs for multicast entry state
    slab *mc_entry_slab(void) const { return mem_db_->mc_entry_slab(); }
    ht *mc_key_ht(void) const { return cfg_db_->mc_key_ht(); }

    // get APIs for LIF state
    slab *lif_slab(void) const { return mem_db_->lif_slab(); }
    ht *lif_id_ht(void) const { return cfg_db_->lif_id_ht(); }

    // get APIs for interface state
    slab *if_slab(void) const { return mem_db_->if_slab(); }
    ht *if_id_ht(void) const { return cfg_db_->if_id_ht(); }
    slab *enic_l2seg_entry_slab(void) const { return mem_db_->enic_l2seg_entry_slab(); }

    // get APIs for endpoint state
    slab *ep_slab(void) const { return mem_db_->ep_slab(); }
    slab *ep_ip_entry_slab(void) const { return mem_db_->ep_ip_entry_slab(); }
    slab *ep_l3_entry_slab(void) const { return mem_db_->ep_l3_entry_slab(); }
    ht *ep_l2_ht(void) const { return oper_db_->ep_l2_ht(); }
    ht *ep_l3_entry_ht(void) const { return oper_db_->ep_l3_entry_ht(); }

    // get APIs for l4lb state
    slab *l4lb_slab(void) const { return mem_db_->l4lb_slab(); }
    ht *l4lb_ht(void) const { return cfg_db_->l4lb_ht(); }
    ht *l4lb_hal_handle_ht(void) const { return cfg_db_->l4lb_hal_handle_ht(); }

    // get APIs for flow/session state
    slab *flow_slab(void) const { return mem_db_->flow_slab(); }
    slab *session_slab(void) const { return mem_db_->session_slab(); }
    ht *session_id_ht(void) const { return cfg_db_->session_id_ht(); }
    ht *session_hal_handle_ht(void) const { return cfg_db_->session_hal_handle_ht(); }
    ht *session_hal_iflow_ht(void) const { return cfg_db_->session_hal_iflow_ht(); }
    ht *session_hal_rflow_ht(void) const { return cfg_db_->session_hal_rflow_ht(); }

    // get APIs for TLS CB state
    slab *tlscb_slab(void) const { return mem_db_->tlscb_slab(); }
    ht *tlscb_id_ht(void) const { return cfg_db_->tlscb_id_ht(); }
    ht *tlscb_hal_handle_ht(void) const { return cfg_db_->tlscb_hal_handle_ht(); }

    // get APIs for TCP CB state
    slab *tcpcb_slab(void) const { return mem_db_->tcpcb_slab(); }
    ht *tcpcb_id_ht(void) const { return cfg_db_->tcpcb_id_ht(); }
    ht *tcpcb_hal_handle_ht(void) const { return cfg_db_->tcpcb_hal_handle_ht(); }

    // get APIs for qos-class state
    slab *qos_class_slab(void) const { return mem_db_->qos_class_slab(); }
    ht *qos_class_ht(void) const { return cfg_db_->qos_class_ht(); }
    bitmap *qos_cmap_pcp_bmp(void) const { return cfg_db_->qos_cmap_pcp_bmp(); }
    bitmap *qos_cmap_dscp_bmp(void) const { return cfg_db_->qos_cmap_dscp_bmp(); }

    // get APIs for copp state
    slab *copp_slab(void) const { return mem_db_->copp_slab(); }
    ht *copp_ht(void) const { return cfg_db_->copp_ht(); }

    // get APIs for acl state
    slab *acl_slab(void) const { return mem_db_->acl_slab(); }
    ht *acl_ht(void) const { return cfg_db_->acl_ht(); }

    // get APIs for WRing state
    slab *wring_slab(void) const { return mem_db_->wring_slab(); }
    ht *wring_id_ht(void) const { return cfg_db_->wring_id_ht(); }
    ht *wring_hal_handle_ht(void) const { return cfg_db_->wring_hal_handle_ht(); }

    // get APIs for Proxy state
    slab *proxy_slab(void) const { return cfg_db_->proxy_slab(); }
    slab *proxy_flow_info_slab(void) const { return cfg_db_->proxy_flow_info_slab(); }
    ht *proxy_type_ht(void) const { return cfg_db_->proxy_type_ht(); }
    ht *proxy_hal_handle_ht(void) const { return cfg_db_->proxy_hal_handle_ht(); }

    // get API for infra l2seg
    void *infra_l2seg(void) { return oper_db_->infra_l2seg(); }
    void set_infra_l2seg(void *infra_l2seg) { oper_db_->set_infra_l2seg(infra_l2seg); }

    // get API for infra VRF
    void *infra_vrf(void) { return oper_db_->infra_vrf(); }
    void set_infra_vrf(void *infra_vrf) { oper_db_->set_infra_vrf(infra_vrf); }

    // get APIs for IPSEC CB state
    slab *ipseccb_slab(void) const { return mem_db_->ipseccb_slab(); }
    ht *ipseccb_id_ht(void) const { return cfg_db_->ipseccb_id_ht(); }
    ht *ipseccb_hal_handle_ht(void) const { return cfg_db_->ipseccb_hal_handle_ht(); }

    // get APIs for CPU CB state
    slab *cpucb_slab(void) const { return mem_db_->cpucb_slab(); }
    ht *cpucb_id_ht(void) const { return cfg_db_->cpucb_id_ht(); }
    ht *cpucb_hal_handle_ht(void) const { return cfg_db_->cpucb_hal_handle_ht(); }

    // get APIs for Raw Redirect CB state
    slab *rawrcb_slab(void) const { return mem_db_->rawrcb_slab(); }
    ht *rawrcb_id_ht(void) const { return cfg_db_->rawrcb_id_ht(); }
    ht *rawrcb_hal_handle_ht(void) const { return cfg_db_->rawrcb_hal_handle_ht(); }
    if_id_t app_redir_if_id(void) const { return cfg_db_->app_redir_if_id(); }

    void set_app_redir_if_id(if_id_t id) { 
        cfg_db_->set_app_redir_if_id(id);
    }

    // get APIs for Raw Chain CB state
    slab *rawccb_slab(void) const { return mem_db_->rawccb_slab(); }
    ht *rawccb_id_ht(void) const { return cfg_db_->rawccb_id_ht(); }
    ht *rawccb_hal_handle_ht(void) const { return cfg_db_->rawccb_hal_handle_ht(); }

    // get APIs for Raw Redirect CB state
    slab *proxyrcb_slab(void) const { return mem_db_->proxyrcb_slab(); }
    ht *proxyrcb_id_ht(void) const { return cfg_db_->proxyrcb_id_ht(); }
    ht *proxyrcb_hal_handle_ht(void) const { return cfg_db_->proxyrcb_hal_handle_ht(); }

    // get APIs for Raw Chain CB state
    slab *proxyccb_slab(void) const { return mem_db_->proxyccb_slab(); }
    ht *proxyccb_id_ht(void) const { return cfg_db_->proxyccb_id_ht(); }
    ht *proxyccb_hal_handle_ht(void) const { return cfg_db_->proxyccb_hal_handle_ht(); }

    // get APIs for GFT state
    slab *gft_exact_match_profile_slab(void) const { return mem_db_->gft_exact_match_profile_slab(); }
    ht *gft_exact_match_profile_id_ht(void) const { return cfg_db_->gft_exact_match_profile_id_ht(); }
    slab *gft_hdr_transposition_profile_slab(void) const { return mem_db_->gft_hdr_transposition_profile_slab(); }
    ht *gft_hdr_transposition_profile_id_ht(void) const { return cfg_db_->gft_hdr_transposition_profile_id_ht(); }
    slab *gft_exact_match_flow_entry_slab(void) const { return mem_db_->gft_exact_match_flow_entry_slab(); }
    ht *gft_exact_match_flow_entry_id_ht(void) const { return cfg_db_->gft_exact_match_flow_entry_id_ht(); }

    // forwarding mode APIs
    void set_forwarding_mode(std::string modestr) { 
        cfg_db_->set_forwarding_mode(modestr);
    }
    hal_forwarding_mode_t forwarding_mode(void) const { return cfg_db_->forwarding_mode(); }

    eventmgr *event_mgr(void) const { return oper_db_->event_mgr(); }

    sdk::lib::catalog* catalog(void) const { return catalog_; }
    void set_catalog(sdk::lib::catalog *catalog) { catalog_ = catalog; }

    void* pd_so(void) const { return pd_so_; }
    void set_pd_so(void *so) { pd_so_ = so; }

    void* pd_stub_so(void) const { return pd_stub_so_; }
    void set_pd_stub_so(void *so) { pd_stub_so_ = so; }

private:
    bool init(void);
    hal_state();

private:
    hal_cfg_db           *cfg_db_;
    hal_oper_db          *oper_db_;
    hal_mem_db           *mem_db_;
    sdk::lib::catalog    *catalog_;
    void                 *pd_so_;
    void                 *pd_stub_so_;
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

