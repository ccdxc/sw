// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef  __HAL_STATE_HPP__
#define  __HAL_STATE_HPP__

#include "lib/list/list.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "lib/slab/slab.hpp"
#include "lib/indexer/indexer.hpp"
#include "lib/ht/ht.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "lib/bitmap/bitmap.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/include/eventmgr.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/include/hal_mem.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "nic/hal/src/debug/snake.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/vmotion/vmotion.hpp"

#ifdef SHM
#define slab_ptr_t        offset_ptr<slab>
#define TO_SLAB_PTR(x)    (x).get()
#else
#define slab_ptr_t        slab *
#define TO_SLAB_PTR(x)    x
#endif

#define HAL_HT_CREATE(name, htable, table_sz, ...)                             \
do {                                                                           \
    /* HAL_TRACE_DEBUG("Creating {} hash table with size {}", (name), (table_sz)); */\
    (htable) = sdk::lib::ht::factory((table_sz), __VA_ARGS__);                 \
} while (0)

namespace hal {

using sdk::lib::slab;
using sdk::lib::indexer;
using sdk::lib::ht;
using hal::utils::eventmgr;
using sdk::lib::dllist_ctxt_t;
using acl::acl_ctx_t;
using acl::acl_config_t;
using acl::ref_t;

enum {
    HAL_TIMER_ID_CLOCK_SYNC               = HAL_TIMER_ID_INFRA_MAX + 1,
    HAL_TIMER_ID_SESSION_AGEOUT,
    HAL_TIMER_ID_TCP_CLOSE_WAIT,
    HAL_TIMER_ID_TCP_HALF_CLOSED_WAIT,
    HAL_TIMER_ID_TCP_CXNSETUP_WAIT,
    HAL_TIMER_ID_TCP_TICKLE_WAIT,
    HAL_TIMER_ID_EP_SESSION_UPD,
    HAL_TIMER_ID_FTE_STATS,
    HAL_TIMER_ID_EP_SESSION_DELETE,
    HAL_TIMER_ID_IPFIX_MIN,
    HAL_TIMER_ID_IPFIX_MAX = HAL_TIMER_ID_IPFIX_MIN + 32,
    HAL_TIMER_ID_CLOCK_ROLLOVER,
    HAL_TIMER_ID_ALG_MIN,
    HAL_TIMER_ID_ALG_MAX = HAL_TIMER_ID_ALG_MIN + 32,
    HAL_TIMER_ID_ACCEL_RGROUP,
    HAL_TIMER_ID_MAX,
};

#define HAL_HANDLE_HT_SZ                             (16 << 10)

// TODO: this should be coming from catalogue or platform API
#define HAL_MAX_TM_PORTS                             12

typedef struct hal_slab_args_
{
    const char *name;
    uint32_t    size;
    uint32_t    num_elements;
    bool        thread_safe;
    bool        grow_on_demand;
    bool        zero_on_alloc;
} hal_slab_args_t;

// forward declaration
class hal_handle;

//------------------------------------------------------------------------------
// HAL config database
// This should consist of only the slabs needed to allocate config related
// objects, and the state from this is persisted and restored during upgrades
//------------------------------------------------------------------------------
class hal_cfg_db {
public:
    static hal_cfg_db *factory(hal_cfg_t *hal_cfg, shmmgr *mmgr = NULL);
    static void destroy(hal_cfg_db *cfg_db);
    void init_on_restart(hal_cfg_t *hal_cfg);

    void rlock(void) { rwlock_.rlock(); }
    void runlock(void) { rwlock_.runlock(); }
    void wlock(void) { rwlock_.wlock(); }
    void wunlock(void) { rwlock_.wunlock(); }

    // API to call before processing any packet by FTE, any operation by config
    hal_ret_t db_open(cfg_op_t cfg_op);

    // API to call after processing any packet by FTE, any operation by config
    // thread or periodic thread etc.
    hal_ret_t db_close(void);

    slab_ptr_t register_slab(hal_slab_t slab_id, hal_slab_args_t& slab_args);

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

    //slab *dos_policy_sg_list_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_DOS_POLICY_SG_LIST]); }
    slab *vrf_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_VRF]); }
    slab *network_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NETWORK]); }
    slab *nwsec_profile_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_SECURITY_PROFILE]); }
    slab *cpu_pkt_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_CPU_PKT]); }
    slab *nwsec_group_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_GROUP]); }
    slab *nwsec_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_RULE]); }
    slab *nwsec_rulelist_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_RULE_LIST]); }
    slab *ipv4_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IPV4_RULE]); }
    slab *rule_cfg_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RULE_CFG]); }
    slab *rule_data_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RULE_DATA]); }
    slab *rule_ctr_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RULE_CTR]); }
    slab *rule_ctr_data_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RULE_CTR_DATA]); }
    slab *nwsec_policy_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_POLICY]); }
    slab *nwsec_policy_appid_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NWSEC_POLICY_APPID]); }
    //slab *dos_policy_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_DOS_POLICY]); }
    slab *l2seg_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_L2SEG]); }
    slab *l2seg_uplink_oif_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_L2SEG_UPLINK_OIF_LIST]); }
    slab *mc_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_MC_ENTRY]); }
    slab *oif_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_OIF]); }
    slab *oif_list_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_OIF_LIST]); }
    slab *lif_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_LIF]); }
    slab *if_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IF]); }
    slab *enic_l2seg_entry_slab(void) { return TO_SLAB_PTR(slabs_[HAL_SLAB_ENIC_L2SEG_ENTRY]); }
    slab *ep_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_EP]); }
    slab *ep_ip_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_EP_IP_ENTRY]); }
    slab *ep_l3_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_EP_L3_ENTRY]); }
    slab *l4lb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_L4LB]); }
    slab *flow_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_FLOW]); }
    slab *session_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_SESSION]); }
    slab *crypto_cert_store_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_CRYPTO_CERT_STORE]); }
    slab *tlscb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_TLSCB]); }
    slab *tcpcb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_TCPCB]); }
    slab *nvme_global_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NVME_GLOBAL]); }
    slab *nvme_sesscb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NVME_SESSCB]); }
    slab *nvme_ns_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NVME_NS]); }
    slab *nvme_sq_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NVME_SQ]); }
    slab *nvme_cq_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NVME_CQ]); }
    slab *qos_class_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_QOS_CLASS]); }
    slab *copp_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_COPP]); }
    slab *acl_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_ACL]); }
    slab *wring_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_WRING]); }
    slab *ipseccb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IPSECCB]); }
#ifdef __x86_64__
    slab *ipsec_sa_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IPSEC_SA]); }
#endif
    slab *cpucb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_CPUCB]); }
    slab *rawrcb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RAWRCB]); }
    slab *rawccb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_RAWCCB]); }
    slab *proxyrcb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXYRCB]); }
    slab *proxyccb_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXYCCB]); }
    slab *filter_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_FILTER]); }
    slab *gft_exact_match_profile_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_GFT_EXACT_MATCH_PROFILE]); }
    slab *gft_hdr_transposition_profile_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE]); }
    slab *gft_exact_match_flow_entry_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY]); }
    slab *proxy_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXY]); }
    slab *proxy_flow_info_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PROXY_FLOW_INFO]); }
    slab *fte_span_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_FTE_SPAN]); }
    slab *snake_test_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_SNAKE_TEST]); }
    slab *port_timer_ctxt_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PORT_TIMER_CTXT]); }

    slab *v4addr_list_elem_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_V4ADDR_LIST_ELEM]); }
    slab *v6addr_list_elem_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_V6ADDR_LIST_ELEM]); }
    slab *port_list_elem_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_PORT_LIST_ELEM]); }
    slab *mac_addr_list_elem_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_MACADDR_LIST_ELEM]); }
    slab *sg_list_elem_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_SG_LIST_ELEM]); }
    slab *icmp_list_elem_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_ICMP_LIST_ELEM]); }
    slab *nat_pool_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NAT_POOL]); }
    slab *nat_cfg_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NAT_CFG_RULE]); }
    slab *nat_cfg_pol_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NAT_CFG_POL]); }
    slab *nexthop_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_NEXTHOP]); }
    slab *route_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_ROUTE]); }
    slab *route_acl_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_ROUTE_ACL_RULE]); }
    slab *flowmon_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_FLOWMON_RULE]); }
    slab *route_acl_userdata_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_ROUTE_ACL_USERDATA]); }
    slab *ipsec_cfg_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IPSEC_CFG_RULE]); }
    slab *ipsec_cfg_pol_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_IPSEC_CFG_POL]); }
    slab *tcp_proxy_cfg_rule_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_TCP_PROXY_CFG_RULE]); }
    slab *tcp_proxy_cfg_pol_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_TCP_PROXY_CFG_POL]); }
    slab *vmotion_ep_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_VMOTION_EP]); }
    slab *vmotion_thread_ctx_slab(void) const { return TO_SLAB_PTR(slabs_[HAL_SLAB_VMOTION_THREAD_CTX]); }


private:
    slab_ptr_t    slabs_[HAL_SLAB_PI_MAX - HAL_SLAB_PI_MIN + 1];

    // following comes from linux process virtual memory
    shmmgr       *mmgr_;
    sdk::wp_rwlock rwlock_;
    typedef struct slab_meta_s {
        hal_obj_id_t       obj_id;           // unique object id
    } slab_meta_t;
    slab_meta_t        slab_meta_[HAL_SLAB_PI_MAX];

private:
    hal_cfg_db();
    ~hal_cfg_db();
    bool init_pss(hal_cfg_t *hal_cfg, shmmgr *mmgr);
    bool init_vss(hal_cfg_t *hal_cfg);
    bool init(hal_cfg_t *hal_cfg, shmmgr *mmgr = NULL);
};

//------------------------------------------------------------------------------
// HAL oper db consists of the following:
// 1. caches built for faster lookups in the data path (and hence need not be
//    preserved/restored but can be rebuilt during upgrades)
// 2. any opertional state that is learnt - DHCP/ARP EP learns etc.
//------------------------------------------------------------------------------
class hal_oper_db {
public:
    static hal_oper_db *factory(hal_cfg_t *hal_cfg, shmmgr *mmgr = NULL);
    static void destroy(hal_oper_db *oper_db);
    void init_on_restart(hal_cfg_t *hal_cfg);

    ht *hal_handle_id_ht(void) const { return hal_handle_id_ht_; };
    ht *vrf_id_ht(void) const { return vrf_id_ht_; }
    ht *network_key_ht(void) const { return network_key_ht_; }
    ht *nwsec_profile_id_ht(void) const { return nwsec_profile_id_ht_; }
    ht *nwsec_policy_ht(void) const { return nwsec_policy_ht_; }
    ht *nat_policy_ht(void) const { return nat_policy_ht_; }
    ht *nwsec_group_ht(void) const { return nwsec_group_ht_; }
    ht *rule_cfg_ht(void) const { return rule_cfg_ht_; }
    ht *l2seg_id_ht(void) const { return l2seg_id_ht_; }
    ht *l2seg_uplink_oif_ht(void) const { return l2seg_uplink_oif_ht_; }
    ht *ep_l2_ht(void) const { return ep_l2_ht_; }
    ht *ep_l3_entry_ht(void) const { return ep_l3_entry_ht_; }
    ht *mc_key_ht(void) const { return mc_key_ht_; }
    ht *oif_list_id_ht(void) const { return oif_list_id_ht_; }
    ht *lif_id_ht(void) const { return lif_id_ht_; }
    ht *if_id_ht(void) const { return if_id_ht_; }
    ht *l4lb_ht(void) const { return l4lb_ht_; }
    ht *session_hal_telemetry_ht(void) const { return session_hal_telemetry_ht_; }
    ht *session_hal_handle_ht(void) const { return session_hal_handle_ht_; }
    ht *session_hal_iflow_ht(void)  const { return session_hal_iflow_ht_; }
    ht *session_hal_rflow_ht(void)  const { return session_hal_rflow_ht_; }
    ht *qos_class_ht(void) const { return qos_class_ht_; }
    bitmap *telemetry_collectors_bmp(void) const { return telemetry_collectors_bmp_; }
    bitmap *telemetry_flowmon_bmp(void) const { return telemetry_flowmon_bmp_; }
    bitmap *qos_cmap_pcp_bmp(void) const { return qos_cmap_pcp_bmp_; }
    bitmap *qos_cmap_dscp_bmp(void) const { return qos_cmap_dscp_bmp_; }
    ht *crypto_cert_store_id_ht(void) const { return crypto_cert_store_id_ht_; }
    ht *tlscb_id_ht(void) const { return tlscb_id_ht_; }

    ht *tcpcb_id_ht(void) const { return tcpcb_id_ht_; }
    ht *nvme_sesscb_id_ht(void) const { return nvme_sesscb_id_ht_; }
    ht *nvme_ns_id_ht(void) const { return nvme_ns_id_ht_; }
    ht *nvme_sq_id_ht(void) const { return nvme_sq_id_ht_; }
    ht *nvme_cq_id_ht(void) const { return nvme_cq_id_ht_; }
    ht *copp_ht(void) const { return copp_ht_; }
    ht *acl_ht(void) const { return acl_ht_; }
    ht *wring_id_ht(void) const { return wring_id_ht_; }
    ht *proxy_type_ht(void) const { return proxy_type_ht_; }
    ht *ipseccb_id_ht(void) const { return ipseccb_id_ht_; }
#ifdef __x86_64__
    ht *ipsec_sa_id_ht(void) const { return ipsec_sa_id_ht_; }
    ht *ipsec_sa_hal_hdl_ht(void) const { return ipsec_sa_hal_hdl_ht_; }
    ht *ipsec_policy_ht(void) const { return ipsec_policy_ht_; }
#endif
    ht *cpucb_id_ht(void) const { return cpucb_id_ht_; }
    ht *rawrcb_id_ht(void) const { return rawrcb_id_ht_; }
    ht *rawccb_id_ht(void) const { return rawccb_id_ht_; }
    ht *proxyrcb_id_ht(void) const { return proxyrcb_id_ht_; }
    ht *proxyccb_id_ht(void) const { return proxyccb_id_ht_; }
    ht *gft_exact_match_profile_id_ht(void) const { return gft_exact_match_profile_id_ht_; }
    ht *gft_hdr_transposition_profile_id_ht(void) const { return gft_hdr_transposition_profile_id_ht_; }
    ht *gft_exact_match_flow_entry_id_ht(void) const { return gft_exact_match_flow_entry_id_ht_; }
    ht *nat_pool_ht(void) const { return nat_pool_ht_; }
    ht *nat_mapping_ht(void) const { return nat_mapping_ht_; }
    ht *nexthop_id_ht(void) const { return nexthop_id_ht_; }
    ht *route_ht(void) const { return route_ht_; }
    ht *tcp_proxy_policy_ht(void) const { return tcp_proxy_policy_ht_; }
    ht *filter_ht(void) const { return filter_ht_; }

    void set_infra_vrf_handle(hal_handle_t infra_vrf_hdl) { infra_vrf_handle_ = infra_vrf_hdl; }
    hal_handle_t infra_vrf_handle(void) const { return infra_vrf_handle_; }
    ip_addr_t *mytep(void) { return &mytep_ip_; }
    void set_mytep_ip(ip_addr_t *mytep_ip) {
        memcpy(&mytep_ip_, mytep_ip, sizeof(mytep_ip_));
    }
    eventmgr *event_mgr(void) const { return event_mgr_; }
    if_id_t app_redir_if_id(void) const { return app_redir_if_id_; }
    void set_app_redir_if_id(if_id_t id) {
        app_redir_if_id_ = id;
    }

    lif_id_t mnic_internal_mgmt_lif_id(void) const { return mnic_internal_mgmt_lif_id_; }
    void set_mnic_internal_mgmt_lif_id(lif_id_t id) {
        mnic_internal_mgmt_lif_id_ = id;
    }
    void set_forwarding_mode(hal_forwarding_mode_t mode) {
        forwarding_mode_ = mode;
    }
    hal_forwarding_mode_t forwarding_mode(void) const { return forwarding_mode_; }

    void set_mgmt_vlan(uint32_t vlan) {
        mgmt_vlan_ = vlan;
    }
    uint32_t mgmt_vlan(void) const { return mgmt_vlan_; }

    void set_swm_vlan(uint32_t vlan) {
        swm_vlan_ = vlan;
    }
    uint32_t swm_vlan(void) const { return swm_vlan_; }

    void set_uplink_flood_mode(hal_uplink_flood_mode_t mode) {
        uplink_flood_mode_ = mode;
    }
    hal_uplink_flood_mode_t uplink_flood_mode(void) const { return uplink_flood_mode_; }

    void set_allow_local_switch_for_promiscuous(bool allow) {
        allow_local_switch_for_promiscuous_ = allow;
    }

    bool allow_local_switch_for_promiscuous(void) const {
        return allow_local_switch_for_promiscuous_;
    }

    void set_allow_dynamic_pinning(bool allow) {
        allow_dynamic_pinning_ = allow;
    }

    bool allow_dynamic_pinning(void) const {
        return allow_dynamic_pinning_;
    }

    void set_local_mac_address(mac_addr_t local_mac_address) {
        memcpy(this->local_mac_address, local_mac_address, sizeof(mac_addr_t));
    }

    mac_addr_t* get_local_mac_address() {
        return &this->local_mac_address;
    }

    void set_max_data_threads (uint8_t val) { max_data_threads_ = val; }
    uint8_t max_data_threads (void) const { return max_data_threads_; }

    void set_default_security_profile (hal_handle_t hdl) { default_securityprof_hdl_ = hdl; }
    hal_handle_t default_security_profile_hdl (void) const { return default_securityprof_hdl_; }

    void *fte_stats(void) const { return fte_stats_; }
    void set_fte_stats(void *fte_stats) { fte_stats_ = fte_stats; }

    void set_fte_span(fte_span_t *span) { fte_span_ = span; }
    fte_span_t *fte_span(void) const { return fte_span_; }

    void set_snake_test(snake_test_t *snake) { snake_test_ = snake; }
    snake_test_t *snake_test(void) const { return snake_test_; }

    vmotion *get_vmotion(void) const { return vmotion_; }
    void set_vmotion(vmotion *vmn) { vmotion_ = vmn; }

    std::map<std::string, uint32_t> *lif_name_id_map() { return &lif_name_id_map_; }

private:
    // following can come from shared memory or non-linux HBM memory
    // NOTE: strictly shmnot required as we can rebuild this from slab elements,
    //       but restart case will be extremely efficent if we do this
    ht    *hal_handle_id_ht_;
    ht    *vrf_id_ht_;
    ht    *network_key_ht_;
    ht    *nwsec_profile_id_ht_;
    ht    *nwsec_policy_ht_;
    ht    *nat_policy_ht_;
    ht    *nwsec_group_ht_;
    ht    *rule_cfg_ht_;
    ht    *l2seg_id_ht_;
    ht    *l2seg_uplink_oif_ht_;
    ht    *ep_l2_ht_;
    ht    *ep_l3_entry_ht_;
    ht    *mc_key_ht_;
    ht    *oif_list_id_ht_;
    ht    *lif_id_ht_;
    ht    *if_id_ht_;
    ht    *session_hal_telemetry_ht_;
    ht    *session_hal_handle_ht_;
    ht    *session_hal_iflow_ht_;
    ht    *session_hal_rflow_ht_;
    ht    *l4lb_ht_;
    ht    *tlscb_id_ht_;
    ht    *tcpcb_id_ht_;
    ht    *nvme_sesscb_id_ht_;
    ht    *nvme_ns_id_ht_;
    ht    *nvme_sq_id_ht_;
    ht    *nvme_cq_id_ht_;
    ht    *qos_class_ht_;
    ht    *copp_ht_;
    ht    *wring_id_ht_;
    ht    *proxy_type_ht_;
    ht    *acl_ht_;
    ht    *ipseccb_id_ht_;
#ifdef __x86_64__
    ht    *ipsec_sa_id_ht_;
    ht    *ipsec_sa_hal_hdl_ht_;
    ht    *ipsec_policy_ht_;
#endif
    ht    *cpucb_id_ht_;
    ht    *rawrcb_id_ht_;
    ht    *rawccb_id_ht_;
    ht    *proxyrcb_id_ht_;
    ht    *proxyccb_id_ht_;
    ht    *gft_exact_match_profile_id_ht_;
    ht    *gft_hdr_transposition_profile_id_ht_;
    ht    *gft_exact_match_flow_entry_id_ht_;
    ht    *crypto_cert_store_id_ht_;
    ht    *nat_pool_ht_;
    ht    *nat_mapping_ht_;
    ht    *nexthop_id_ht_;
    ht    *route_ht_;
    ht    *tcp_proxy_policy_ht_;
    ht    *filter_ht_;
    bitmap                  *telemetry_collectors_bmp_;
    bitmap                  *telemetry_flowmon_bmp_;
    bitmap                  *qos_cmap_pcp_bmp_;
    bitmap                  *qos_cmap_dscp_bmp_;

    std::map<std::string, uint32_t> lif_name_id_map_;               // lif name to lif id map
    hal_handle_t            infra_vrf_handle_;                      // infra vrf handle
    eventmgr                *event_mgr_;
    ip_addr_t               mytep_ip_;
    hal_forwarding_mode_t   forwarding_mode_;
    uint32_t                mgmt_vlan_;
    uint32_t                swm_vlan_;
    hal_uplink_flood_mode_t uplink_flood_mode_;
    if_id_t                 app_redir_if_id_;
    lif_id_t                mnic_internal_mgmt_lif_id_;
    uint8_t                 max_data_threads_;
    hal_handle_t            default_securityprof_hdl_;
    fte_span_t              *fte_span_;
    snake_test_t            *snake_test_;

    // Classic Mode:
    //  - Ucast packet from host, registered mac will be a MISS.
    //    0: Don't send this packet to promiscous lifs.
    //    1: Send this packet to promiscous lifs.
    bool                    allow_local_switch_for_promiscuous_;

    // Host-pin Mode:
    // - Allows Hal to dynamically pin Enics
    //   0: Pinning is always taken through config in lif or enic.
    //   1: If there is no pin config in lif or enic, HAL will
    //      dynamically pin enic to one of the uplinks. Only being used in HAL.
    bool                    allow_dynamic_pinning_;

    // Mac address used for local probes to be sent out by hal.
    mac_addr_t              local_mac_address;

    // following comes from linux process virtual memory
    shmmgr       *mmgr_;
    void         *fte_stats_;

    // vmotion object
    vmotion *vmotion_;

private:
    bool init_pss(hal_cfg_t *hal_cfg, shmmgr *mmgr);
    bool init_vss(hal_cfg_t *hal_cfg);
    bool init(hal_cfg_t *hal_cfg, shmmgr *mmgr = NULL);
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
    hal_state(hal_obj_meta **obj_meta, hal_cfg_t *hal_cfg, shmmgr *mmgr = NULL);
    ~hal_state();
    void init_on_restart(hal_cfg_t *hal_cfg);
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
    //slab *dos_policy_sg_list_entry_slab(void) const { return cfg_db_->dos_policy_sg_list_entry_slab(); }
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


    slab *cpu_pkt_slab(void) const { return cfg_db_->cpu_pkt_slab(); }
    // get APIs for security Policy related state
    slab *nwsec_group_slab(void) const { return cfg_db_->nwsec_group_slab(); }
    slab *nwsec_rule_slab(void) const { return cfg_db_->nwsec_rule_slab(); }
    slab *ipv4_rule_slab(void) const { return cfg_db_->ipv4_rule_slab(); }
    slab *rule_ctr_slab(void) const { return cfg_db_->rule_ctr_slab(); }
    slab *rule_ctr_data_slab(void) const { return cfg_db_->rule_ctr_data_slab(); }
    slab *rule_data_slab(void) const { return cfg_db_->rule_data_slab(); }
    slab *rule_cfg_slab(void) const { return cfg_db_->rule_cfg_slab(); }
    slab *nwsec_rulelist_slab(void) const { return cfg_db_->nwsec_rulelist_slab(); }
    slab *nwsec_policy_slab(void) const { return cfg_db_->nwsec_policy_slab(); }
    slab *nwsec_policy_appid_slab(void) const { return cfg_db_->nwsec_policy_appid_slab(); }
    ht   *nwsec_policy_ht(void) const { return oper_db_->nwsec_policy_ht(); }
    ht   *nat_policy_ht(void) const { return oper_db_->nat_policy_ht(); }
    ht   *nwsec_group_ht(void) const { return oper_db_->nwsec_group_ht(); }
    ht   *tcp_proxy_policy_ht(void) const { return oper_db_->tcp_proxy_policy_ht(); }

    // get APIs for dos policy related state
    //slab *dos_policy_slab(void) const { return cfg_db_->dos_policy_slab(); }

    // get APIs for L2 segment state
    slab *l2seg_slab(void) const { return cfg_db_->l2seg_slab(); }
    ht *l2seg_id_ht(void) const { return oper_db_->l2seg_id_ht(); }
    slab *l2seg_uplink_oif_slab(void) const { return cfg_db_->l2seg_uplink_oif_slab(); }
    ht *l2seg_uplink_oif_ht(void) const { return oper_db_->l2seg_uplink_oif_ht(); }

    // get APIs for multicast entry state
    slab *mc_entry_slab(void) const { return cfg_db_->mc_entry_slab(); }
    ht *mc_key_ht(void) const { return oper_db_->mc_key_ht(); }

    // get APIs for OIF List state
    slab *oif_slab(void) const { return cfg_db_->oif_slab(); }
    slab *oif_list_slab(void) const { return cfg_db_->oif_list_slab(); }
    ht *oif_list_id_ht(void) const { return oper_db_->oif_list_id_ht(); }

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
    //ht *session_id_ht(void) const { return oper_db_->session_id_ht(); }
    ht *session_hal_telemetry_ht(void) const { return oper_db_->session_hal_telemetry_ht(); }
    ht *session_hal_handle_ht(void) const { return oper_db_->session_hal_handle_ht(); }
    ht *session_hal_iflow_ht(void) const { return oper_db_->session_hal_iflow_ht(); }
    ht *session_hal_rflow_ht(void) const { return oper_db_->session_hal_rflow_ht(); }

    // get APIs for Crypto Cert Store State
    slab *crypto_cert_store_slab(void) const { return cfg_db_->crypto_cert_store_slab(); }
    ht *crypto_cert_store_id_ht(void) const { return oper_db_->crypto_cert_store_id_ht(); }

    // get APIs for TLS CB state
    slab *tlscb_slab(void) const { return cfg_db_->tlscb_slab(); }
    ht *tlscb_id_ht(void) const { return oper_db_->tlscb_id_ht(); }

    // get APIs for TCP CB state
    slab *tcpcb_slab(void) const { return cfg_db_->tcpcb_slab(); }
    ht *tcpcb_id_ht(void) const { return oper_db_->tcpcb_id_ht(); }

    // get APIs for NVME GLOBAL state
    slab *nvme_global_slab(void) const { return cfg_db_->nvme_global_slab(); }

    // get APIs for NVME SESS CB state
    slab *nvme_sesscb_slab(void) const { return cfg_db_->nvme_sesscb_slab(); }
    ht *nvme_sesscb_id_ht(void) const { return oper_db_->nvme_sesscb_id_ht(); }

    // get APIs for NVME NS state
    slab *nvme_ns_slab(void) const { return cfg_db_->nvme_ns_slab(); }
    ht *nvme_ns_id_ht(void) const { return oper_db_->nvme_ns_id_ht(); }

    // get APIs for NVME SQ state
    slab *nvme_sq_slab(void) const { return cfg_db_->nvme_sq_slab(); }
    ht *nvme_sq_id_ht(void) const { return oper_db_->nvme_sq_id_ht(); }

    // get APIs for NVME CQ state
    slab *nvme_cq_slab(void) const { return cfg_db_->nvme_cq_slab(); }
    ht *nvme_cq_id_ht(void) const { return oper_db_->nvme_cq_id_ht(); }

    // get APIs for qos-class state
    slab *qos_class_slab(void) const { return cfg_db_->qos_class_slab(); }
    ht *qos_class_ht(void) const { return oper_db_->qos_class_ht(); }
    bitmap *telemetry_collectors_bmp(void) const { return oper_db_->telemetry_collectors_bmp(); }
    bitmap *telemetry_flowmon_bmp(void) const { return oper_db_->telemetry_flowmon_bmp(); }
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
    slab *fte_span_slab(void) const { return cfg_db_->fte_span_slab(); }

    slab *snake_test_slab(void) const { return cfg_db_->snake_test_slab(); }

    slab *port_timer_ctxt_slab(void) const { return cfg_db_->port_timer_ctxt_slab(); }

    ht *proxy_type_ht(void) const { return oper_db_->proxy_type_ht(); }

    // get API for infra VRF
    hal_handle_t infra_vrf_handle(void) { return oper_db_->infra_vrf_handle(); }
    void set_infra_vrf_handle(hal_handle_t infra_vrf_handle) { oper_db_->set_infra_vrf_handle(infra_vrf_handle); }

    // get APIs for IPSEC CB state
    slab *ipseccb_slab(void) const { return cfg_db_->ipseccb_slab(); }
    ht *ipseccb_id_ht(void) const { return oper_db_->ipseccb_id_ht(); }
#ifdef __x86_64__
    ht   *ipsec_policy_ht(void) const { return oper_db_->ipsec_policy_ht(); }
    slab *ipsec_sa_slab(void) const { return cfg_db_->ipsec_sa_slab(); }
    ht *ipsec_sa_id_ht(void) const { return oper_db_->ipsec_sa_id_ht(); }
#endif

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
    lif_id_t mnic_internal_mgmt_lif_id(void) const { return oper_db_->mnic_internal_mgmt_lif_id(); }
    void set_mnic_internal_mgmt_lif_id(lif_id_t id) {
        oper_db_->set_mnic_internal_mgmt_lif_id(id);
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

    // get APIs for NAT state
    slab *nat_cfg_rule_slab(void) const { return cfg_db_->nat_cfg_rule_slab(); }
    slab *nat_cfg_pol_slab(void) const { return cfg_db_->nat_cfg_pol_slab(); }
    slab *nat_pool_slab(void) const { return cfg_db_->nat_pool_slab(); }
    ht *nat_pool_ht(void) const { return oper_db_->nat_pool_ht(); }
    ht *nat_mapping_ht(void) const { return oper_db_->nat_mapping_ht(); }

    slab *v4addr_list_elem_slab(void) const { return cfg_db_->v4addr_list_elem_slab(); }
    slab *v6addr_list_elem_slab(void) const { return cfg_db_->v6addr_list_elem_slab(); }
    slab *port_list_elem_slab(void) const { return cfg_db_->port_list_elem_slab(); }
    slab *mac_addr_list_elem_slab(void) const { return cfg_db_->mac_addr_list_elem_slab(); }
    slab *sg_list_elem_slab(void) const { return cfg_db_->sg_list_elem_slab(); }
    slab *icmp_list_elem_slab(void) const { return cfg_db_->icmp_list_elem_slab(); }
    // get APIs for nexthop related state
    slab *nexthop_slab(void) const { return cfg_db_->nexthop_slab(); }
    ht *nexthop_id_ht(void) const { return oper_db_->nexthop_id_ht(); }

    // get APIs for route related state
    slab *route_slab(void) const { return cfg_db_->route_slab(); }
    slab *route_acl_rule_slab(void) const { return cfg_db_->route_acl_rule_slab(); }
    slab *flowmon_rule_slab(void) const { return cfg_db_->flowmon_rule_slab(); }
    ht *route_ht(void) const { return oper_db_->route_ht(); }
    slab *route_acl_userdata_slab(void) const { return cfg_db_->route_acl_userdata_slab(); }
    ht *filter_ht(void) const { return oper_db_->filter_ht(); }

    // get APIs for IPSec policy related state
    slab *ipsec_cfg_rule_slab(void) const { return cfg_db_->ipsec_cfg_rule_slab(); }
    slab *ipsec_cfg_pol_slab(void) const { return cfg_db_->ipsec_cfg_pol_slab(); }

    slab *tcp_proxy_cfg_rule_slab(void) const { return cfg_db_->tcp_proxy_cfg_rule_slab(); }
    slab *tcp_proxy_cfg_pol_slab(void) const { return cfg_db_->tcp_proxy_cfg_pol_slab(); }

    // get APIs for filter
    slab *filter_slab(void) const { return cfg_db_->filter_slab(); }

    // get APIs for vMotion
    slab *vmotion_ep_slab(void) const { return cfg_db_->vmotion_ep_slab(); }
    slab *vmotion_thread_ctx_slab(void) const { return cfg_db_->vmotion_thread_ctx_slab(); }

    // forwarding mode APIs
    void set_forwarding_mode(hal_forwarding_mode_t mode) {
        oper_db_->set_forwarding_mode(mode);
    }
    hal_forwarding_mode_t forwarding_mode(void) const { return oper_db_->forwarding_mode(); }

    // mgmt vlan APIs
    void set_mgmt_vlan(uint32_t vlan) {
        oper_db_->set_mgmt_vlan(vlan);
    }
    uint32_t mgmt_vlan(void) const { return oper_db_->mgmt_vlan(); }

    // swm vlan APIs
    void set_swm_vlan(uint32_t vlan) {
        oper_db_->set_swm_vlan(vlan);
    }
    uint32_t swm_vlan(void) const { return oper_db_->swm_vlan(); }

    // multicast uplink mode APIs
    void set_uplink_flood_mode(hal_uplink_flood_mode_t mode) {
        oper_db_->set_uplink_flood_mode(mode);
    }
    hal_uplink_flood_mode_t uplink_flood_mode(void) const { return oper_db_->uplink_flood_mode(); }

    void set_allow_local_switch_for_promiscuous(bool allow) {
        oper_db_->set_allow_local_switch_for_promiscuous(allow);
    }

    bool allow_local_switch_for_promiscuous(void) const {
        return oper_db_->allow_local_switch_for_promiscuous();
    }

    void set_allow_dynamic_pinning(bool allow) {
        oper_db_->set_allow_dynamic_pinning(allow);
    }


    bool allow_dynamic_pinning(void) const {
        return oper_db_->allow_dynamic_pinning();
    }

    void set_local_mac_address(mac_addr_t local_mac_address) {
        oper_db_->set_local_mac_address(local_mac_address);
    }

    mac_addr_t* get_local_mac_address() {
        return oper_db_->get_local_mac_address();
    }

    eventmgr *event_mgr(void) const { return oper_db_->event_mgr(); }

    sdk::lib::catalog *catalog(void) const { return catalog_; }
    void set_catalog(sdk::lib::catalog *catalog) { catalog_ = catalog; }

    hal_stats_t api_stats(int idx) const { return api_stats_[idx]; }
    void set_api_stats(int idx, int val) { api_stats_[idx] = val; }
    uint64_t preserve_state(void);
    hal_ret_t restore_state(void);

    void *fte_stats(void) const { return oper_db_->fte_stats(); }
    void set_fte_stats(void *fte_stats) { oper_db_->set_fte_stats(fte_stats); }

    void set_fte_span(fte_span_t *span) { oper_db_->set_fte_span(span); }
    fte_span_t *fte_span(void) const { return oper_db_->fte_span(); }

    void set_snake_test(snake_test_t *snake) { oper_db_->set_snake_test(snake); }
    snake_test_t *snake_test(void) const { return oper_db_->snake_test(); }

    void set_vmotion(vmotion *vmn) { oper_db_->set_vmotion(vmn); }
    vmotion *get_vmotion(void) const { return oper_db_->get_vmotion(); }

    slab_ptr_t register_slab(hal_slab_t slab_id, hal_slab_args_t& slab_args) {
        return (cfg_db_->register_slab(slab_id, slab_args));
    }

    platform_type_t platform_type() { return platform_; }

    bool lif_name_id_map_insert(std::string lif_name, uint32_t lif_id) {
        auto ptr = oper_db_->lif_name_id_map()->insert(std::make_pair(lif_name, lif_id));
        return ptr.second;
    }
    bool lif_name_id_map_delete(std::string lif_name) {
        auto size = oper_db_->lif_name_id_map()->erase(lif_name);
        return (size == 1);
    }
    uint32_t lif_name_id_map_find(std::string lif_name) {
        auto find = oper_db_->lif_name_id_map()->find(lif_name);
        if (find != oper_db_->lif_name_id_map()->end()) {
            return find->second;
        } else {
            return LIF_ID_INVALID;
        }
    }

private:
    // following come from shared memory or non-linux HBM memory
    hal_cfg_db           *cfg_db_;
    hal_oper_db          *oper_db_;
    hal_stats_t          *api_stats_;

    // following come from linux process virtual memory
    shmmgr               *mmgr_;
    sdk::lib::catalog    *catalog_;
    hal_obj_meta         **obj_meta_;
    platform_type_t platform_;

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
is_forwarding_mode_smart_switch (void)
{
    return g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_SWITCH;
}

static inline bool
is_uplink_flood_mode_host_pinned (void)
{
    return g_hal_state->uplink_flood_mode() == HAL_UPLINK_FLOOD_MODE_PINNED;
}

static inline bool
is_uplink_flood_mode_rpf (void)
{
    return g_hal_state->uplink_flood_mode() == HAL_UPLINK_FLOOD_MODE_RPF;
}

static inline if_id_t
get_app_redir_if_id (void)
{
    return g_hal_state->app_redir_if_id();
}

// Platform type check functions
static inline bool
is_platform_type_sim(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_SIM;
}

static inline bool
is_platform_type_haps(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_HAPS;
}

static inline bool
is_platform_type_hw(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_HW;
}

static inline bool
is_platform_type_mock(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_MOCK;
}

static inline bool
is_platform_type_zebu(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_ZEBU;
}

static inline bool
is_platform_type_rtl(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_RTL;
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

    SDK_ASSERT(elem != NULL);

    if (g_delay_delete && sdk::lib::periodic_thread_is_running()) {
        timer_ctxt =
            sdk::lib::timer_schedule(slab_id,
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

