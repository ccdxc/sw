// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_STATE_HPP__
#define __HAL_STATE_HPP__

#include "nic/utils/list/list.hpp"
#include "nic/utils/slab/slab.hpp"
#include "nic/utils/indexer/indexer.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/utils/catalog/catalog.hpp"
#include "nic/include/eventmgr.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/ip.h"

namespace hal {

using hal::utils::slab;
using hal::utils::indexer;
using hal::utils::ht;
using hal::utils::bitmap;
using hal::utils::eventmgr;
using hal::utils::dllist_ctxt_t;

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
    ht *network_hal_handle_ht(void) const { return network_hal_handle_ht_; }

    ht *nwsec_profile_id_ht(void) const { return nwsec_profile_id_ht_; }
    ht *nwsec_profile_hal_handle_ht(void) const { return nwsec_profile_hal_handle_ht_ ; }

    ht *nwsec_policy_cfg_ht(void) const { return nwsec_policy_cfg_ht_; }
    ht *nwsec_group_ht(void) const { return nwsec_group_ht_; }

    ht *dos_policy_hal_handle_ht(void) const { return dos_policy_hal_handle_ht_ ; }

    ht *l2seg_id_ht(void) const { return l2seg_id_ht_; }

    ht *mc_key_ht(void) const { return mc_key_ht_; }

    ht *lif_id_ht(void) const { return lif_id_ht_; }

    ht *if_id_ht(void) const { return if_id_ht_; }

    ht *port_id_ht(void) const { return port_id_ht_; }

    // TODO: may have to create L2 and L3 HTs here !!
    ht *ep_hal_handle_ht(void) const { return ep_hal_handle_ht_; }

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

    ht *buf_pool_id_ht(void) const { return buf_pool_id_ht_; }
    ht *buf_pool_hal_handle_ht(void) const { return buf_pool_hal_handle_ht_; }

    ht *queue_id_ht(void) const { return queue_id_ht_; }
    ht *queue_hal_handle_ht(void) const { return queue_hal_handle_ht_; }

    ht *ingress_policer_id_ht(void) const { return ingress_policer_id_ht_; }
    ht *ingress_policer_hal_handle_ht(void) const { return ingress_policer_hal_handle_ht_; }
    ht *egress_policer_id_ht(void) const { return egress_policer_id_ht_; }
    ht *egress_policer_hal_handle_ht(void) const { return egress_policer_hal_handle_ht_; }

    ht *acl_id_ht(void) const { return acl_id_ht_; }
    ht *acl_hal_handle_ht(void) const { return acl_hal_handle_ht_; }

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

    // get APIs for Raw Chain CB state
    ht *rawccb_id_ht(void) const { return rawccb_id_ht_; }
    ht *rawccb_hal_handle_ht(void) const { return rawccb_hal_handle_ht_; }

    // get APIs for Proxy Redirect CB state
    ht *proxyrcb_id_ht(void) const { return proxyrcb_id_ht_; }
    ht *proxyrcb_hal_handle_ht(void) const { return proxyrcb_hal_handle_ht_; }

    // get APIs for Proxy Chain CB state
    ht *proxyccb_id_ht(void) const { return proxyccb_id_ht_; }
    ht *proxyccb_hal_handle_ht(void) const { return proxyccb_hal_handle_ht_; }

    void set_forwarding_mode(std::string modestr);
    hal_forwarding_mode_t forwarding_mode() { return forwarding_mode_; }

private:
    bool init(void);
    hal_cfg_db();

private:
    // vrf/vrf related config
    struct {
        ht         *vrf_id_ht_;
        // ht         *vrf_hal_handle_ht_;
    } __PACK__;

    // network related config
    struct {
        ht         *network_key_ht_;
        ht         *network_hal_handle_ht_;
    } __PACK__;

    // security profile related config
    struct {
        ht         *nwsec_profile_id_ht_;
        ht         *nwsec_profile_hal_handle_ht_;
    } __PACK__;
    // security group policy related config
    struct {
        ht         *nwsec_policy_cfg_ht_;
    }__PACK__;
    
    // security group related config
    struct {
        ht         *nwsec_group_ht_;
    }__PACK__;

    // dos policy related config
    struct {
        ht         *dos_policy_hal_handle_ht_;
    } __PACK__;

    // l2segment related config
    struct {
        ht         *l2seg_id_ht_;
        // ht         *l2seg_hal_handle_ht_;
    } __PACK__;

    // mc_entry related config
    struct {
        ht         *mc_key_ht_;
        // ht         *mc_entry_hal_handle_ht_;
    } __PACK__;

    // LIF related config
    struct {
        ht         *lif_id_ht_;
        // ht         *lif_hal_handle_ht_;
    } __PACK__;

    // interface related config
    struct {
        ht         *if_id_ht_;
        // ht         *if_hal_handle_ht_;
    } __PACK__;

    // port related config
    struct {
        ht         *port_id_ht_;
        // ht         *port_hal_handle_ht_;
    } __PACK__;

    // endpoint related config
    struct {
        ht         *ep_hal_handle_ht_;
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

    // buf-pool related config
    struct {
        ht         *buf_pool_id_ht_;
        ht         *buf_pool_hal_handle_ht_;
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

    // queue related config
    struct {
        ht         *queue_id_ht_;
        ht         *queue_hal_handle_ht_;
    } __PACK__;

    // policer related config
    struct {
        ht         *ingress_policer_id_ht_;
        ht         *ingress_policer_hal_handle_ht_;
        ht         *egress_policer_id_ht_;
        ht         *egress_policer_hal_handle_ht_;
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
        ht         *acl_id_ht_;
        ht         *acl_hal_handle_ht_;
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
    ht *ep_l2_ht(void) const { return ep_l2_ht_; }
    ht *ep_l3_entry_ht(void) const { return ep_l3_entry_ht_; }
    ht *flow_ht(void) const { return flow_ht_; }
    bitmap *buf_pool_cos_usage_bmp(uint32_t port_num) const {
        return cos_in_use_bmp_[port_num];
    }
    ip_addr_t *mytep(void) { return &mytep_ip; }
    eventmgr *event_mgr(void) const { return event_mgr_; }

private:
    bool init(void);
    hal_oper_db();

private:
    void        *infra_l2seg_;  // l2seg_t *
    eventmgr    *event_mgr_;
    ht          *hal_handle_id_ht_;
    ht          *ep_l2_ht_;
    ht          *ep_l3_entry_ht_;
    ht          *flow_ht_;
    // bitmap indicating if the cos is already assigned to a buffer pool
    bitmap    *cos_in_use_bmp_[HAL_MAX_TM_PORTS];
    ip_addr_t mytep_ip;
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
    slab *nwsec_policy_rules_slab(void) const { return nwsec_policy_rules_slab_; }
    slab *nwsec_policy_cfg_slab(void) const { return nwsec_policy_cfg_slab_; }
    slab *nwsec_policy_svc_slab(void) const { return nwsec_policy_svc_slab_; }
    slab *dos_policy_slab(void) const { return dos_policy_slab_; }
    slab *l2seg_slab(void) const { return l2seg_slab_; }
    slab *mc_entry_slab(void) const { return mc_entry_slab_; }
    slab *lif_slab(void) const { return lif_slab_; }
    slab *if_slab(void) const { return if_slab_; }
    slab *enic_l2seg_entry_slab(void) { return enic_l2seg_entry_slab_; }
    slab *port_slab(void) const { return port_slab_; }
    slab *ep_slab(void) const { return ep_slab_; }
    slab *ep_ip_entry_slab(void) const { return ep_ip_entry_slab_; }
    slab *ep_l3_entry_slab(void) const { return ep_l3_entry_slab_; }
    slab *l4lb_slab(void) const { return l4lb_slab_; }
    slab *flow_slab(void) const { return flow_slab_; }
    slab *session_slab(void) const { return session_slab_; }
    slab *tlscb_slab(void) const { return tlscb_slab_; }
    slab *tcpcb_slab(void) const { return tcpcb_slab_; }
    slab *buf_pool_slab(void) const { return buf_pool_slab_; }
    slab *queue_slab(void) const { return queue_slab_; }
    slab *policer_slab(void) const { return policer_slab_; }
    slab *acl_slab(void) const { return acl_slab_; }
    slab *wring_slab(void) const { return wring_slab_; }
    slab *ipseccb_slab(void) const { return ipseccb_slab_; }
    slab *cpucb_slab(void) const { return cpucb_slab_; }
    slab *rawrcb_slab(void) const { return rawrcb_slab_; }
    slab *rawccb_slab(void) const { return rawccb_slab_; }
    slab *proxyrcb_slab(void) const { return proxyrcb_slab_; }
    slab *proxyccb_slab(void) const { return proxyccb_slab_; }

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
    slab    *nwsec_policy_rules_slab_;
    slab    *nwsec_policy_cfg_slab_;
    slab    *nwsec_policy_svc_slab_;
    slab    *dos_policy_slab_;
    slab    *l2seg_slab_;
    slab    *mc_entry_slab_;
    slab    *lif_slab_;
    slab    *if_slab_;
    slab    *enic_l2seg_entry_slab_;
    slab    *port_slab_;
    slab    *ep_slab_;
    slab    *ep_ip_entry_slab_;
    slab    *ep_l3_entry_slab_;
    slab    *flow_slab_;
    slab    *session_slab_;
    slab    *l4lb_slab_;
    slab    *tlscb_slab_;
    slab    *tcpcb_slab_;
    slab    *buf_pool_slab_;
    slab    *queue_slab_;
    slab    *policer_slab_;
    slab    *wring_slab_;
    slab    *acl_slab_;
    slab    *ipseccb_slab_;
    slab    *cpucb_slab_;
    slab    *rawrcb_slab_;
    slab    *rawccb_slab_;
    slab    *proxyrcb_slab_;
    slab    *proxyccb_slab_;
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
    ht *network_hal_handle_ht(void) const { return cfg_db_->network_hal_handle_ht(); }

    // get APIs for security profile related state
    slab *nwsec_profile_slab(void) const { return mem_db_->nwsec_profile_slab(); }
    ht *nwsec_profile_id_ht(void) const { return cfg_db_->nwsec_profile_id_ht(); }
    ht *nwsec_profile_hal_handle_ht(void) const { return cfg_db_->nwsec_profile_hal_handle_ht(); }

    // get APIs for security Policy related state
    slab *nwsec_group_slab(void) const { return mem_db_->nwsec_group_slab(); }
    slab *nwsec_policy_rules_slab(void) const { return mem_db_->nwsec_policy_rules_slab(); }
    slab *nwsec_policy_cfg_slab(void) const { return mem_db_->nwsec_policy_cfg_slab(); }
    slab *nwsec_policy_svc_slab(void) const { return mem_db_->nwsec_policy_svc_slab(); }
    ht   *nwsec_policy_cfg_ht(void) const { return cfg_db_->nwsec_policy_cfg_ht(); }
    ht   *nwsec_group_ht(void) const { return cfg_db_->nwsec_group_ht(); }
    
    // get APIs for dos policy related state
    slab *dos_policy_slab(void) const { return mem_db_->dos_policy_slab(); }
    ht *dos_policy_hal_handle_ht(void) const { return cfg_db_->dos_policy_hal_handle_ht(); }

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

    // get APIs for port state
    slab *port_slab(void) const { return mem_db_->port_slab(); }
    ht *port_id_ht(void) const { return cfg_db_->port_id_ht(); }

    // get APIs for endpoint state
    slab *ep_slab(void) const { return mem_db_->ep_slab(); }
    slab *ep_ip_entry_slab(void) const { return mem_db_->ep_ip_entry_slab(); }
    slab *ep_l3_entry_slab(void) const { return mem_db_->ep_l3_entry_slab(); }
    ht *ep_l2_ht(void) const { return oper_db_->ep_l2_ht(); }
    ht *ep_l3_entry_ht(void) const { return oper_db_->ep_l3_entry_ht(); }
    ht *ep_hal_handle_ht(void) const { return cfg_db_->ep_hal_handle_ht(); }

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

    // get APIs for buf-pool state
    slab *buf_pool_slab(void) const { return mem_db_->buf_pool_slab(); }
    ht *buf_pool_id_ht(void) const { return cfg_db_->buf_pool_id_ht(); }
    ht *buf_pool_hal_handle_ht(void) const { return cfg_db_->buf_pool_hal_handle_ht(); }
    bitmap *buf_pool_cos_usage_bmp(uint32_t port_num) const {
        return oper_db_->buf_pool_cos_usage_bmp(port_num);
    }

    // get APIs for queue state
    slab *queue_slab(void) const { return mem_db_->queue_slab(); }
    ht *queue_id_ht(void) const { return cfg_db_->queue_id_ht(); }
    ht *queue_hal_handle_ht(void) const { return cfg_db_->queue_hal_handle_ht(); }

    // get APIs for policer state
    slab *policer_slab(void) const { return mem_db_->policer_slab(); }
    ht *ingress_policer_id_ht(void) const { return cfg_db_->ingress_policer_id_ht(); }
    ht *ingress_policer_hal_handle_ht(void) const { return cfg_db_->ingress_policer_hal_handle_ht(); }
    ht *egress_policer_id_ht(void) const { return cfg_db_->egress_policer_id_ht(); }
    ht *egress_policer_hal_handle_ht(void) const { return cfg_db_->egress_policer_hal_handle_ht(); }

    // get APIs for acl state
    slab *acl_slab(void) const { return mem_db_->acl_slab(); }
    ht *acl_id_ht(void) const { return cfg_db_->acl_id_ht(); }
    ht *acl_hal_handle_ht(void) const { return cfg_db_->acl_hal_handle_ht(); }

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

    // forwarding mode APIs
    void set_forwarding_mode(std::string modestr) { 
        cfg_db_->set_forwarding_mode(modestr);
    }
    hal_forwarding_mode_t forwarding_mode(void) const { return cfg_db_->forwarding_mode(); }

    eventmgr *event_mgr(void) const { return oper_db_->event_mgr(); }

    hal::utils::catalog* catalog() { return catalog_; }

    void set_catalog(hal::utils::catalog *catalog) { catalog_ = catalog; }

private:
    bool init(void);
    hal_state();

private:
    hal_cfg_db             *cfg_db_;
    hal_oper_db            *oper_db_;
    hal_mem_db             *mem_db_;
    hal::utils::catalog    *catalog_;
};

extern class hal_state    *g_hal_state;

static inline bool
is_forwarding_mode_host_pinned()
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED;
}

static inline bool
is_forwarding_mode_classic_nic()
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC;
}

static inline bool
is_forwarding_mode_smart_nic()
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_SWITCH;
}

}    // namespace hal

#endif    // __HAL_STATE_HPP__

