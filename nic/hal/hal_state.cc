//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <boost/interprocess/managed_shared_memory.hpp>
#include "nic/hal/hal.hpp"
#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/nw/nh.hpp"
#include "nic/hal/src/nw/route.hpp"
#include "nic/hal/src/nw/route_acl.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/mcast/multicast.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/src/dos/dos.hpp"
#include "nic/hal/src/internal/event.hpp"
#include "nic/hal/src/internal/tls_proxy_cb.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/include/nwsec_group_api.hpp"
#include "nic/hal/src/firewall/nwsec_group.hpp"
#include "nic/hal/src/aclqos/qos.hpp"
#include "nic/hal/src/aclqos/acl.hpp"
#include "nic/hal/src/internal/wring.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/internal/ipseccb.hpp"
#include "nic/hal/src/l4lb/l4lb.hpp"
#include "nic/hal/src/internal/cpucb.hpp"
#include "nic/hal/src/internal/rawrcb.hpp"
#include "nic/hal/src/internal/rawccb.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/hal/src/internal/proxyccb.hpp"
#include "nic/hal/src/internal/crypto_cert_store.hpp"
#include "nic/hal/src/gft/gft.hpp"
#include "nic/hal/src/utils/addr_list.hpp"
#include "nic/hal/src/utils/port_list.hpp"
#include "nic/hal/src/utils/rule_match.hpp"
#include "nic/hal/src/nat/nat.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "sdk/twheel.hpp"
#include "sdk/shmmgr.hpp"

// name of the HAL state store segment
#define HAL_STATE_STORE                         "h2s"
#define HAL_STATE_STORE_SIZE                    0x20000000     // 500 MB
#define HAL_SERIALIZED_STATE_STORE              "h3s"
#define HAL_SERIALIZED_STATE_STORE_SIZE         0x20000000     // 500 MB
#define HAL_STATE_STORE_VADDR                   0x400000000    // starting from 16G
#define HAL_SERIALIZED_STATE_STORE_VADDR        0x480000000    // starting from 18G
#define HAL_STATE_OBJ                           "halstate"

using namespace boost::interprocess;

namespace hal {

// global instance of all HAL state including config, operational states
class hal_state    *g_hal_state;
class shmmgr       *g_h2s_shmmgr, *g_h3s_shmmgr;

typedef struct tlv_s {
    uint32_t    type;
    uint32_t    len;
    uint8_t     val[0];
} __PACK__ tlv_t;

typedef struct hal_state_mctxt_s {
    uint32_t         obj_id;         // object id (i.e., HAL_OBJ_ID_XXX)
    uint8_t          *mem;           // buffer to marshall the state into
    uint32_t         len;            // length of the buffer available
    marshall_cb_t    marshall_cb;    // marshall callback pointer
} __PACK__ hal_state_mctxt_t;

typedef struct hal_state_umctxt_s {
    uint32_t           obj_id;           // object id (i.e., HAL_OBJ_ID_XXX)
    void               *obj;             // buffer to unmarshall the state from
    uint32_t           len;              // length of the marshalled obj
    unmarshall_cb_t    unmarshall_cb;    // unmarshall callback pointer
} __PACK__ hal_state_umctxt_t;

// HAL serialized state store hints used while restoring the state
class hal_state_hints {
public:
    hal_state_hints() {
        pi_state_ = NULL;
        pi_state_len_ = 0;
        //pd_state_ = NULL;
        //pd_state_len_ = 0;
        //asic_pd_state_ = NULL ;
        //asic_pd_state_len_ = 0;
    }
    ~hal_state_hints() {}

    void set_pi_state(void *state) { pi_state_ = state; }
    void *pi_state(void) { return pi_state_; }
    void set_pi_state_len(uint32_t len) { pi_state_len_ = len; }
    uint32_t pi_state_len(void) const { return pi_state_len_; }

    //void set_pd_state(void *state) { pd_state_ = state; }
    //void *pd_state(void) { return pd_state_; }
    //void set_pd_state_len(uint32_t len) { pd_state_len_ = len; }
    //uint32_t pd_state_len(void) const { return pd_state_len_; }

    //void set_asic_pd_state(void *state) { asic_pd_state_ = state; }
    //void *asic_pd_state(void) { return asic_pd_state_; }
    //void set_asic_pd_state_len(uint32_t len) { asic_pd_state_len_ = len; }
    //uint32_t asic_pd_state_len(void) const { return asic_pd_state_len_; }

private:
    // NOTE: rename pi_state_ to obj_state_ as this is including all cfg, oper,
    //       stats fully now
    void        *pi_state_;
    uint32_t    pi_state_len_;
    //void        *pd_state_;
    //uint32_t    pd_state_len_;
    //void        *asic_pd_state_;
    //uint32_t    asic_pd_state_len_;
};
#define HAL_STATE_HINTS        "h3s-hints"

//------------------------------------------------------------------------------
// initialize DBs and caches that needs to be persisted across restarts/upgrades
//------------------------------------------------------------------------------
bool
hal_cfg_db::init_pss(hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    // initialize slab for HAL handles
    slabs_[HAL_SLAB_HANDLE] =
        slab::factory("hal-handle",
                      HAL_SLAB_HANDLE, sizeof(hal_handle),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_HANDLE] != NULL), false);

    slabs_[HAL_SLAB_HANDLE_HT_ENTRY] =
        slab::factory("hal-handle-ht-entry",
                      HAL_SLAB_HANDLE_HT_ENTRY,
                      sizeof(hal_handle_ht_entry_t),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_HANDLE_HT_ENTRY] != NULL), false);

    slabs_[HAL_SLAB_HANDLE_LIST_ENTRY] =
        slab::factory("hal-handle-list-entry",
                      HAL_SLAB_HANDLE_LIST_ENTRY,
                      sizeof(hal_handle_list_entry_t),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_HANDLE_LIST_ENTRY] != NULL), false);

	slabs_[HAL_SLAB_HANDLE_ID_HT_ENTRY] =
        slab::factory("hal-handle-id-ht-entry",
                      HAL_SLAB_HANDLE_ID_HT_ENTRY,
                      sizeof(hal_handle_id_ht_entry_t),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_HANDLE_ID_HT_ENTRY] != NULL), false);

    slabs_[HAL_SLAB_HANDLE_ID_LIST_ENTRY] =
        slab::factory("hal-handle-id-list-entry",
                      HAL_SLAB_HANDLE_ID_LIST_ENTRY,
                      sizeof(hal_handle_id_list_entry_t),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_HANDLE_ID_LIST_ENTRY] != NULL), false);

	// initialize vrf related data structures
	slabs_[HAL_SLAB_VRF] =
        slab::factory("vrf", HAL_SLAB_VRF,
                      sizeof(hal::vrf_t), 16,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_VRF] != NULL), false);

    // initialize network related data structures
    slabs_[HAL_SLAB_NETWORK] =
        slab::factory("network", HAL_SLAB_NETWORK,
                      sizeof(hal::network_t), 16,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NETWORK] != NULL), false);

    // initialize security profile related data structures
    slabs_[HAL_SLAB_SECURITY_PROFILE] =
        slab::factory("nwsec-profile",
                      HAL_SLAB_SECURITY_PROFILE,
                      sizeof(hal::nwsec_profile_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_SECURITY_PROFILE] != NULL), false);

    // initialize dos policy related data structures
    slabs_[HAL_SLAB_DOS_POLICY] =
        slab::factory("dos-policy",
                      HAL_SLAB_DOS_POLICY,
                      sizeof(hal::dos_policy_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_DOS_POLICY] != NULL), false);

    // initialize dos policy sg list related data structures
    slabs_[HAL_SLAB_DOS_POLICY_SG_LIST] =
        slab::factory("dos-policy-sg-list",
                      HAL_SLAB_DOS_POLICY_SG_LIST,
                      sizeof(hal::dos_policy_sg_list_entry_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_DOS_POLICY_SG_LIST] != NULL), false);

    // initialize L2 segment related data structures
    slabs_[HAL_SLAB_L2SEG] =
        slab::factory("l2seg", HAL_SLAB_L2SEG,
                      sizeof(hal::l2seg_t), 16,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_L2SEG] != NULL), false);

    // initialize multicast related data structures
    slabs_[HAL_SLAB_MC_ENTRY] =
        slab::factory("mc_entry", HAL_SLAB_MC_ENTRY,
                      sizeof(hal::mc_entry_t), 16,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_MC_ENTRY] != NULL), false);

    // initialize LIF related data structures
    slabs_[HAL_SLAB_LIF] =
        slab::factory("LIF", HAL_SLAB_LIF,
                      sizeof(hal::lif_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_LIF] != NULL), false);

    // initialize interface related data structures
    slabs_[HAL_SLAB_IF] =
        slab::factory("interface", HAL_SLAB_IF,
                      sizeof(hal::if_t), 16,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_IF] != NULL), false);

    // initialize enic l2seg entry related data structures
    slabs_[HAL_SLAB_ENIC_L2SEG_ENTRY] =
        slab::factory("if_l2seg_entry", HAL_SLAB_ENIC_L2SEG_ENTRY,
                      sizeof(hal::if_l2seg_entry_t), 16,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_ENIC_L2SEG_ENTRY] != NULL), false);

    // initialize endpoint related data structures
    slabs_[HAL_SLAB_EP] =
        slab::factory("EP", HAL_SLAB_EP, sizeof(hal::ep_t), 128,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_EP] != NULL), false);

    slabs_[HAL_SLAB_EP_IP_ENTRY] = slab::factory("EP IP entry",
                                                 HAL_SLAB_EP_IP_ENTRY,
                                                 sizeof(hal::ep_ip_entry_t),
                                                 64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_EP_IP_ENTRY] != NULL), false);

    slabs_[HAL_SLAB_EP_L3_ENTRY] =
        slab::factory("EP L3 entry", HAL_SLAB_EP_L3_ENTRY,
                      sizeof(hal::ep_l3_entry_t),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_EP_L3_ENTRY] != NULL), false);

    // initialize flow/session related data structures
    slabs_[HAL_SLAB_FLOW] =
        slab::factory("Flow", HAL_SLAB_FLOW, sizeof(hal::flow_t), 128,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_FLOW] != NULL), false);

    slabs_[HAL_SLAB_SESSION] =
        slab::factory("Session", HAL_SLAB_SESSION,
                      sizeof(hal::session_t), 128,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_SESSION] != NULL), false);

    // initialize l4lb related data structures
    slabs_[HAL_SLAB_L4LB] =
        slab::factory("L4LB", HAL_SLAB_L4LB,
                      sizeof(hal::l4lb_service_entry_t), 16,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_L4LB] != NULL), false);

    // initialize Qos-class related data structures
    slabs_[HAL_SLAB_QOS_CLASS] =
        slab::factory("QosClass", HAL_SLAB_QOS_CLASS,
                      sizeof(hal::qos_class_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_QOS_CLASS] != NULL), false);

    // initialize Copp related data structures
    slabs_[HAL_SLAB_COPP] =
        slab::factory("Copp", HAL_SLAB_COPP,
                      sizeof(hal::copp_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_COPP] != NULL), false);

    // initialize Acl related data structures
    slabs_[HAL_SLAB_ACL] =
        slab::factory("Acl", HAL_SLAB_ACL,
                      sizeof(hal::acl_t), 8,
                      false, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_ACL] != NULL), false);

    slabs_[HAL_SLAB_NWSEC_GROUP] =
        slab::factory("nwsec_group", HAL_SLAB_NWSEC_GROUP,
                      sizeof(hal::nwsec_group_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_GROUP] != NULL), false);

    slabs_[HAL_SLAB_V4ADDR_LIST_ELEM] =
        slab::factory("v4addr-list-elem", HAL_SLAB_V4ADDR_LIST_ELEM,
                      sizeof(hal::addr_list_elem_t) +
                          sizeof(ipv4_range_t),
                      8, true, true, true, mmgr);
    HAL_ASSERT_RETURN(slabs_[HAL_SLAB_V4ADDR_LIST_ELEM] != NULL, false);

    slabs_[HAL_SLAB_V6ADDR_LIST_ELEM] =
        slab::factory("v6addr-list-elem", HAL_SLAB_V6ADDR_LIST_ELEM,
                      sizeof(hal::addr_list_elem_t) +
                          sizeof(ipv6_range_t),
                      8, true, true, true, mmgr);
    HAL_ASSERT_RETURN(slabs_[HAL_SLAB_V6ADDR_LIST_ELEM] != NULL, false);

    slabs_[HAL_SLAB_PORT_LIST_ELEM] =
        slab::factory("port-list-elem", HAL_SLAB_PORT_LIST_ELEM,
                      sizeof(hal::port_list_elem_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_PORT_LIST_ELEM] != NULL), false);

    slabs_[HAL_SLAB_NAT_POOL] =
        slab::factory("natpool", HAL_SLAB_NAT_POOL,
                      sizeof(hal::nat_pool_t), 8,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NAT_POOL] != NULL), false);

    slabs_[HAL_SLAB_NAT_CFG_RULE] =
        slab::factory("nat_cfg_rule", HAL_SLAB_NAT_CFG_RULE,
                      sizeof(hal::nat_cfg_rule_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NAT_CFG_RULE] != NULL), false);

    slabs_[HAL_SLAB_NAT_CFG_POL] =
        slab::factory("nat_cfg_policy", HAL_SLAB_NAT_CFG_POL,
                      sizeof(hal::nat_cfg_pol_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NAT_CFG_POL] != NULL), false);

    slabs_[HAL_SLAB_NEXTHOP] =
        slab::factory("nexthop", HAL_SLAB_NEXTHOP,
                      sizeof(hal::nexthop_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NEXTHOP] != NULL), false);

    slabs_[HAL_SLAB_ROUTE] =
        slab::factory("route", HAL_SLAB_ROUTE,
                      sizeof(hal::route_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_ROUTE] != NULL), false);

    slabs_[HAL_SLAB_ROUTE_ACL_RULE] =
        slab::factory("route_acl_rule", HAL_SLAB_ROUTE_ACL_RULE,
                      sizeof(hal::route_acl_rule_t), 64,
                      true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_ROUTE_ACL_RULE] != NULL), false);

    slabs_[HAL_SLAB_ROUTE_ACL_USERDATA] =
        slab::factory("route_acl_userdata",
                      HAL_SLAB_ROUTE_ACL_USERDATA, sizeof(route_acl_user_data_t),
                      64, true, true, true, mmgr);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_ROUTE_ACL_USERDATA] != NULL), false);

    if (hal_cfg->features == HAL_FEATURE_SET_GFT) {
        // initialize GFT related slabs
        slabs_[HAL_SLAB_GFT_EXACT_MATCH_PROFILE] =
            slab::factory("gft_exact_match_profile",
                          HAL_SLAB_GFT_EXACT_MATCH_PROFILE,
                          sizeof(hal::gft_exact_match_profile_t),
                          16, false, true, true, mmgr);
        HAL_ASSERT_RETURN((slabs_[HAL_SLAB_GFT_EXACT_MATCH_PROFILE] != NULL), false);

        slabs_[HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE] =
            slab::factory("gft_hdr_xposition_profile",
                          HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE,
                          sizeof(hal::gft_hdr_xposition_profile_t),
                          16, false, true, true, mmgr);
        HAL_ASSERT_RETURN((slabs_[HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE] != NULL), false);

        slabs_[HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY] =
             slab::factory("gft_exact_match_flow_entry",
                           HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY,
                           sizeof(hal::gft_exact_match_flow_entry_t),
                           16, false, true, true, mmgr);
        HAL_ASSERT_RETURN((slabs_[HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY] != NULL), false);
    }

    return true;
}

//------------------------------------------------------------------------------
// initialize DBs and caches that don't to be persisted (i.e., volatile) across
// restarts/upgrades and these will have to be rebuilt after restart/upgrade
//------------------------------------------------------------------------------
bool
hal_cfg_db::init_vss(hal_cfg_t *hal_cfg)
{
    // initialize Crypto Cert Store related data structures
    slabs_[HAL_SLAB_CRYPTO_CERT_STORE] = slab::factory("crypto_cert_store",
                                                       HAL_SLAB_CRYPTO_CERT_STORE,
                                                       sizeof(hal::crypto_cert_t),
                                                       16,
                                                       false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_CRYPTO_CERT_STORE] != NULL), false);

    // initialize TLS CB related data structures
    slabs_[HAL_SLAB_TLSCB] = slab::factory("tlscb", HAL_SLAB_TLSCB,
                                sizeof(hal::tlscb_t), 16,
                                false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_TLSCB] != NULL), false);

    // initialize TCB CB related data structures
    slabs_[HAL_SLAB_TCPCB] = slab::factory("tcpcb", HAL_SLAB_TCPCB,
                                sizeof(hal::tcpcb_t), 16,
                                false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_TCPCB] != NULL), false);

    // initialize WRing related data structures
    slabs_[HAL_SLAB_WRING] = slab::factory("wring", HAL_SLAB_WRING,
                                sizeof(hal::wring_t), 16,
                                false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_WRING] != NULL), false);

    // initialize IPSEC CB related data structures
    slabs_[HAL_SLAB_IPSECCB] = slab::factory("ipseccb", HAL_SLAB_IPSECCB,
                                  sizeof(hal::ipseccb_t), 16,
                                  false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_IPSECCB] != NULL), false);

    // initialize CPU CB related data structures
    slabs_[HAL_SLAB_CPUCB] = slab::factory("cpucb", HAL_SLAB_CPUCB,
                                sizeof(hal::cpucb_t), 16,
                                false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_CPUCB] != NULL), false);

    // initialize Raw Redirect CB related data structures
    slabs_[HAL_SLAB_RAWRCB] = slab::factory("rawrcb", HAL_SLAB_RAWRCB,
                                 sizeof(hal::rawrcb_t), 16,
                                 false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_RAWRCB] != NULL), false);

    // initialize Raw Chain CB related data structures
    slabs_[HAL_SLAB_RAWCCB] = slab::factory("rawccb", HAL_SLAB_RAWCCB,
                                 sizeof(hal::rawccb_t), 16,
                                 false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_RAWCCB] != NULL), false);

    // initialize Raw Redirect CB related data structures
    slabs_[HAL_SLAB_PROXYRCB] = slab::factory("proxyrcb", HAL_SLAB_PROXYRCB,
                                   sizeof(hal::proxyrcb_t), 16,
                                   false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_PROXYRCB] != NULL), false);

    // initialize Raw Chain CB related data structures
    slabs_[HAL_SLAB_PROXYCCB] = slab::factory("proxyccb", HAL_SLAB_PROXYCCB,
                                   sizeof(hal::proxyccb_t), 16,
                                   false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_PROXYCCB] != NULL), false);

    slabs_[HAL_SLAB_NWSEC_POLICY_RULES] =
        slab::factory("nwsec_policy_rules",
                      HAL_SLAB_NWSEC_POLICY_RULES,
                      sizeof(hal::nwsec_policy_rules_t), 64,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_POLICY_RULES] != NULL), false);

    slabs_[HAL_SLAB_NWSEC_POLICY_CFG] =
        slab::factory("nwsec_policy_cfg", HAL_SLAB_NWSEC_POLICY_CFG,
                      sizeof(hal::nwsec_policy_cfg_t), 64,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_POLICY_CFG] != NULL), false);

    slabs_[HAL_SLAB_NWSEC_POLICY_SVC] =
        slab::factory("nwsec_policy_svc", HAL_SLAB_NWSEC_POLICY_SVC,
                      sizeof(hal::nwsec_policy_svc_t), 64,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_POLICY_SVC] != NULL), false);

    slabs_[HAL_SLAB_NWSEC_POLICY_APPID] =
        slab::factory("nwsec_policy_appid", HAL_SLAB_NWSEC_POLICY_APPID,
                      sizeof(hal::nwsec_policy_appid_t),
                      64, true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_POLICY_APPID] != NULL), false);

    // TODO - cleanup !! why same slab id for the 2 slabs below ?
    slabs_[HAL_SLAB_NWSEC_RULE] =
        slab::factory("nwsec_rule", HAL_SLAB_NWSEC_RULE,
                      sizeof(hal::nwsec_rule_t), 1024,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_RULE] != NULL), false);

    slabs_[HAL_SLAB_IPV4_RULE] =
        slab::factory("ipv4_rule", HAL_SLAB_IPV4_RULE,
                      sizeof(hal::ipv4_rule_t), 1024,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_IPV4_RULE] != NULL), false);


    slabs_[HAL_SLAB_RULE_DATA] =
        slab::factory("ipv4_rule", HAL_SLAB_RULE_DATA,
                      sizeof(hal::rule_data_t), 1024,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_RULE_DATA] != NULL), false);

    slabs_[HAL_SLAB_PROXY] = slab::factory("proxy", HAL_SLAB_PROXY,
                                           sizeof(hal::proxy_t), HAL_MAX_PROXY,
                                           false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_PROXY] != NULL), false);

    slabs_[HAL_SLAB_PROXY_FLOW_INFO] =
        slab::factory("proxy-flow-infi", HAL_SLAB_PROXY_FLOW_INFO,
                      sizeof(hal::proxy_flow_info_t),
                      HAL_MAX_PROXY_FLOWS,
                      false, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_PROXY_FLOW_INFO] != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// init() function to instantiate all the config db init state
//------------------------------------------------------------------------------
bool
hal_cfg_db::init(hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    mmgr_ = mmgr;
    HAL_ASSERT_RETURN((init_pss(hal_cfg, mmgr) == true), false);
    HAL_ASSERT_RETURN((init_vss(hal_cfg) == true), false);
    return true;
}

//------------------------------------------------------------------------------
// initialize the state that is not valid after a process restart
//------------------------------------------------------------------------------
void
hal_cfg_db::init_on_restart(hal_cfg_t *hal_cfg) {
    init_vss(hal_cfg);
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_cfg_db::hal_cfg_db()
{
    memset(slabs_, 0, sizeof(slabs_));
    mmgr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_cfg_db::~hal_cfg_db()
{
    uint32_t    i;

    for (i = HAL_SLAB_PI_MIN; i < HAL_SLAB_PI_MAX; i++) {
        if (slabs_[i]) {
            slab::destroy(TO_SLAB_PTR(slabs_[i]));
        }
    }
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_cfg_db *
hal_cfg_db::factory(hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    void          *mem;
    hal_cfg_db    *cfg_db;

    if (mmgr) {
        mem = mmgr->alloc(sizeof(hal_cfg_db), 4, true);
    } else {
        mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_cfg_db));
    }

    if (mem) {
        cfg_db = new(mem) hal_cfg_db();
        if (cfg_db->init(hal_cfg, mmgr) == false) {
            cfg_db->~hal_cfg_db();
            if (mmgr) {
                mmgr->free(mem);
            } else {
                HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
            }
            return NULL;
        }
        return cfg_db;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// destroy method
//------------------------------------------------------------------------------
void
hal_cfg_db::destroy(hal_cfg_db *cfg_db)
{
    shmmgr    *mmgr;

    if (!cfg_db) {
        return;
    }
    mmgr = cfg_db->mmgr_;
    cfg_db->~hal_cfg_db();
    if (mmgr) {
        mmgr->free(cfg_db);
    } else {
        HAL_FREE(HAL_MEM_ALLOC_INFRA, cfg_db);
    }
}

//----------------------------------------------------------------------------
// gives the slab of a slab id
//----------------------------------------------------------------------------
slab *
hal_cfg_db::get_slab(hal_slab_t slab_id)
{
    if (slab_id >= HAL_SLAB_PI_MAX) {
        return NULL;
    }
    return TO_SLAB_PTR(slabs_[slab_id]);
}

//------------------------------------------------------------------------------
// API to call before processing any packet by FTE, any operation by config
// thread or periodic thread etc.
// NOTE: once opened, cfg db has to be closed properly and reserved version
//       should be released/committed or else next open will fail
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db::db_open(cfg_op_t cfg_op)
{
    // if the cfg db was already opened by this thread, error out
    if (t_cfg_db_ctxt.cfg_db_open_) {
        HAL_TRACE_ERR("Failed to open cfg db, opened already, thread {}",
                      hal_get_current_thread()->name());
        return HAL_RET_ERR;
    }

    // take a read lock irrespective of whether the db is open in read/write
    // mode, for write mode, we will eventually take a write lock when needed
    g_hal_state->cfg_db()->rlock();

    t_cfg_db_ctxt.cfg_op_ = cfg_op;
    t_cfg_db_ctxt.cfg_db_open_ = true;
    HAL_TRACE_DEBUG("{} acquired rlock, opened cfg db, cfg op : {}",
                    hal_get_current_thread()->name(), cfg_op);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// API to call after processing any packet by FTE, any operation by config
// thread or periodic thread etc. If successful, this will make the currently
// reserved (and cached) version of the DB valid. In case of failure, the
// currently reserved version will not be marked as valid and object updates
// made with this reserved version are left as they are ... they are either
// cleaned up when we touch those objects next time, or by periodic thread that
// will release instances of objects with invalid versions (or versions that
// slide out of the valid-versions window)
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db::db_close(void)
{
    if (t_cfg_db_ctxt.cfg_db_open_) {
        t_cfg_db_ctxt.cfg_db_open_ = FALSE;
        t_cfg_db_ctxt.cfg_op_ = CFG_OP_NONE;
        g_hal_state->cfg_db()->runlock();
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// register a config object's meta information
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db::register_cfg_object(hal_obj_id_t obj_id, uint32_t obj_sz)
{
    if ((obj_id <= HAL_OBJ_ID_NONE) || (obj_id >= HAL_OBJ_ID_MAX)) {
        return HAL_RET_INVALID_ARG;
    }
    //obj_meta_[obj_id].obj_sz = obj_sz;
    return HAL_RET_OK;
}

#if 0
//------------------------------------------------------------------------------
// return a config object's size given its id
//------------------------------------------------------------------------------
uint32_t
hal_cfg_db::object_size(hal_obj_id_t obj_id) const
{
    if ((obj_id <= HAL_OBJ_ID_NONE) || (obj_id >= HAL_OBJ_ID_MAX)) {
        return 0;
    }
    return obj_meta_[obj_id].obj_sz;
}
#endif

//------------------------------------------------------------------------------
// initialize DBs and caches that needs to be persisted across restarts/upgrades
//------------------------------------------------------------------------------
bool
hal_oper_db::init_pss(hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    HAL_HT_CREATE("hal-handle", hal_handle_id_ht_,
                  HAL_MAX_HANDLES >> 1,
                  hal::hal_handle_id_get_key_func,
                  hal::hal_handle_id_compute_hash_func,
                  hal::hal_handle_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((hal_handle_id_ht_ != NULL), false);

    // initialize vrf related data structures
    HAL_HT_CREATE("vrf", vrf_id_ht_,
                  HAL_MAX_VRFS >> 1,
                  hal::vrf_id_get_key_func,
                  hal::vrf_id_compute_hash_func,
                  hal::vrf_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((vrf_id_ht_ != NULL), false);

    // initialize network related data structures
    HAL_HT_CREATE("nw", network_key_ht_,
                  HAL_MAX_NETWORKS >> 1,
                  hal::network_get_key_func,
                  hal::network_compute_hash_func,
                  hal::network_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((network_key_ht_ != NULL), false);

    // initialize security profile related data structures
    HAL_HT_CREATE("nwsec-profile", nwsec_profile_id_ht_,
                  HAL_MAX_NWSEC_PROFILES >> 1,
                  hal::nwsec_profile_id_get_key_func,
                  hal::nwsec_profile_id_compute_hash_func,
                  hal::nwsec_profile_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((nwsec_profile_id_ht_ != NULL), false);

    // initialize L2 segment related data structures
    HAL_HT_CREATE("l2seg", l2seg_id_ht_,
                  HAL_MAX_L2SEGMENTS >> 1,
                  hal::l2seg_id_get_key_func,
                  hal::l2seg_id_compute_hash_func,
                  hal::l2seg_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((l2seg_id_ht_ != NULL), false);

    HAL_HT_CREATE("EP L2", ep_l2_ht_,
                  HAL_MAX_ENDPOINTS >> 1,
                  hal::ep_get_l2_key_func,
                  hal::ep_compute_l2_hash_func,
                  hal::ep_compare_l2_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((ep_l2_ht_ != NULL), false);

    HAL_HT_CREATE("EP L3", ep_l3_entry_ht_,
                  HAL_MAX_ENDPOINTS,     // twice that of EPs, 1 IPv4,  IPv6 per EP
                  hal::ep_get_l3_key_func,
                  hal::ep_compute_l3_hash_func,
                  hal::ep_compare_l3_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((ep_l3_entry_ht_ != NULL), false);

    // initialize mc entry related data structures
    HAL_HT_CREATE("mcast", mc_key_ht_,
                  HAL_MAX_MC_ENTRIES >> 1,
                  hal::mc_entry_get_key_func,
                  hal::mc_entry_compute_hash_func,
                  hal::mc_entry_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((mc_key_ht_ != NULL), false);

    // initialize LIF related data structures
    HAL_HT_CREATE("lif", lif_id_ht_,
                  HAL_MAX_LIFS >> 1,
                  hal::lif_id_get_key_func,
                  hal::lif_id_compute_hash_func,
                  hal::lif_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((lif_id_ht_ != NULL), false);

    // initialize interface related data structures
    HAL_HT_CREATE("interface", if_id_ht_,
                  HAL_MAX_INTERFACES >> 1,
                  hal::if_id_get_key_func,
                  hal::if_id_compute_hash_func,
                  hal::if_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((if_id_ht_ != NULL), false);

    // initialize flow/session related data structures
    HAL_HT_CREATE("session-handle", session_hal_handle_ht_,
                  HAL_MAX_SESSIONS >> 1,
                  hal::session_get_handle_key_func,
                  hal::session_compute_handle_hash_func,
                  hal::session_compare_handle_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((session_hal_handle_ht_ != NULL), false);

    HAL_HT_CREATE("iflow", session_hal_iflow_ht_,
                  HAL_MAX_SESSIONS >> 1,
                  hal::session_get_iflow_key_func,
                  hal::session_compute_iflow_hash_func,
                  hal::session_compare_iflow_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((session_hal_iflow_ht_ != NULL), false);

    HAL_HT_CREATE("rflow", session_hal_rflow_ht_,
                  HAL_MAX_SESSIONS >> 1,
                  hal::session_get_rflow_key_func,
                  hal::session_compute_rflow_hash_func,
                  hal::session_compare_rflow_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((session_hal_rflow_ht_ != NULL), false);

    // initialize l4lb related data structures
    HAL_HT_CREATE("L4 LB", l4lb_ht_,
                  HAL_MAX_L4LB_SERVICES >> 1,
                  hal::l4lb_get_key_func,
                  hal::l4lb_compute_key_hash_func,
                  hal::l4lb_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((l4lb_ht_ != NULL), false);

    // initialize Qos-class related data structures
    HAL_HT_CREATE("QoS-Class", qos_class_ht_,
                  HAL_MAX_QOS_CLASSES >> 1,
                  hal::qos_class_get_key_func,
                  hal::qos_class_compute_hash_func,
                  hal::qos_class_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((qos_class_ht_ != NULL), false);

    // initialize Copp related data structures
    HAL_HT_CREATE("COPP", copp_ht_,
                  HAL_MAX_COPPS >> 1,
                  hal::copp_get_key_func,
                  hal::copp_compute_hash_func,
                  hal::copp_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((copp_ht_ != NULL), false);

    // initialize acl related data structures
    HAL_HT_CREATE("ACL", acl_ht_,
                  HAL_MAX_ACLS >> 1,
                  hal::acl_get_key_func,
                  hal::acl_compute_hash_func,
                  hal::acl_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((acl_ht_ != NULL), false);

    HAL_HT_CREATE("security-group", nwsec_group_ht_,
                  HAL_MAX_NW_SEC_GROUP_CFG >> 1,
                  hal::nwsec_group_get_key_func,
                  hal::nwsec_group_compute_hash_func,
                  hal::nwsec_group_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((nwsec_group_ht_ != NULL), false);

    // initialize NAT related data structures
    HAL_HT_CREATE("natpool", nat_pool_ht_,
                  HAL_MAX_NAT_POOLS >> 1,
                  hal::nat_pool_get_key_func,
                  hal::nat_pool_compute_hash_func,
                  hal::nat_pool_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((nat_pool_ht_ != NULL), false);

    HAL_HT_CREATE("nat-addr-map", nat_mapping_ht_,
                  HAL_MAX_NAT_ADDR_MAP >> 1,
                  hal::nat_mapping_get_key_func,
                  hal::nat_mapping_compute_hash_func,
                  hal::nat_mapping_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((nat_mapping_ht_ != NULL), false);

    // initialize nexthop related data structures
    HAL_HT_CREATE("nexthop", nexthop_id_ht_,
                  HAL_MAX_NEXTHOPS >> 1,
                  hal::nexthop_id_get_key_func,
                  hal::nexthop_id_compute_hash_func,
                  hal::nexthop_id_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((nexthop_id_ht_ != NULL), false);

    // initialize route related data structures
    HAL_HT_CREATE("route", route_ht_,
                  HAL_MAX_ROUTES >> 1,
                  hal::route_get_key_func,
                  hal::route_compute_hash_func,
                  hal::route_compare_key_func,
                  true, mmgr);
    HAL_ASSERT_RETURN((route_ht_ != NULL), false);

    HAL_ASSERT_RETURN((route_acl_create() == HAL_RET_OK), false);

    if (hal_cfg->features == HAL_FEATURE_SET_GFT) {
        HAL_HT_CREATE("gft-profiles",
                      gft_exact_match_profile_id_ht_,
                      HAL_MAX_GFT_EXACT_MATCH_PROFILES >> 1,
                      hal::gft_exact_match_profile_id_get_key_func,
                      hal::gft_exact_match_profile_id_compute_hash_func,
                      hal::gft_exact_match_profile_id_compare_key_func,
                      true, mmgr);
        HAL_ASSERT_RETURN((gft_exact_match_profile_id_ht_ != NULL), false);

        HAL_HT_CREATE("gft-transpositions",
                      gft_hdr_transposition_profile_id_ht_,
                      HAL_MAX_GFT_HDR_TRANSPOSITION_PROFILES >> 1,
                      hal::gft_hdr_transposition_profile_id_get_key_func,
                      hal::gft_hdr_transposition_profile_id_compute_hash_func,
                      hal::gft_hdr_transposition_profile_id_compare_key_func,
                      true, mmgr);
        HAL_ASSERT_RETURN((gft_hdr_transposition_profile_id_ht_ != NULL), false);

        HAL_HT_CREATE("gft-flow",
                      gft_exact_match_flow_entry_id_ht_,
                      HAL_MAX_GFT_EXACT_MATCH_FLOW_ENTRIES >> 1,
                      hal::gft_exact_match_flow_entry_id_get_key_func,
                      hal::gft_exact_match_flow_entry_id_compute_hash_func,
                      hal::gft_exact_match_flow_entry_id_compare_key_func,
                      true, mmgr);
        HAL_ASSERT_RETURN((gft_exact_match_flow_entry_id_ht_ != NULL), false);
    }

    return true;
}

//------------------------------------------------------------------------------
// initialize DBs and caches that don't to be persisted (i.e., volatile) across
// restarts/upgrades and these will have to be rebuilt after restart/upgrade
//------------------------------------------------------------------------------
bool
hal_oper_db::init_vss(hal_cfg_t *hal_cfg)
{
    event_mgr_ = eventmgr::factory(HAL_MAX_EVENTS);
    HAL_ASSERT_RETURN((event_mgr_ != NULL), false);

    // initialize Crypto Cert store related data structures
    crypto_cert_store_id_ht_ = ht::factory(HAL_MAX_CRYPTO_CERT_STORE_ELEMS,
                                           hal::crypto_cert_store_get_key_func,
                                           hal::crypto_cert_store_compute_hash_func,
                                           hal::crypto_cert_store_compare_key_func);
    HAL_ASSERT_RETURN((crypto_cert_store_id_ht_ != NULL), false);

    // initialize TLS CB related data structures
    HAL_HT_CREATE("tlscb", tlscb_id_ht_,
                  HAL_MAX_TLSCB >> 1,
                  hal::tlscb_get_key_func,
                  hal::tlscb_compute_hash_func,
                  hal::tlscb_compare_key_func);
    HAL_ASSERT_RETURN((tlscb_id_ht_ != NULL), false);

    // initialize TCB CB related data structures
    HAL_HT_CREATE("tcpcb", tcpcb_id_ht_,
                  HAL_MAX_TCPCB >> 1,
                  hal::tcpcb_get_key_func,
                  hal::tcpcb_compute_hash_func,
                  hal::tcpcb_compare_key_func);
    HAL_ASSERT_RETURN((tcpcb_id_ht_ != NULL), false);

    qos_cmap_pcp_bmp_ = bitmap::factory(HAL_MAX_DOT1Q_PCP_VALS, true);
    HAL_ASSERT_RETURN((qos_cmap_pcp_bmp_ != NULL), false);

    qos_cmap_dscp_bmp_ = bitmap::factory(HAL_MAX_IP_DSCP_VALS, true);
    HAL_ASSERT_RETURN((qos_cmap_dscp_bmp_ != NULL), false);

    // initialize WRing related data structures
    HAL_HT_CREATE("wringid", wring_id_ht_,
                  HAL_MAX_WRING >> 1,
                  hal::wring_get_key_func,
                  hal::wring_compute_hash_func,
                  hal::wring_compare_key_func);
    HAL_ASSERT_RETURN((wring_id_ht_ != NULL), false);

    // initialize proxy service related data structures
    HAL_HT_CREATE("proxy-type", proxy_type_ht_,
                  HAL_MAX_PROXY >> 1,
                  hal::proxy_get_key_func,
                  hal::proxy_compute_hash_func,
                  hal::proxy_compare_key_func);
    HAL_ASSERT_RETURN((proxy_type_ht_ != NULL), false);

    // initialize IPSEC CB related data structures
    HAL_HT_CREATE("ipseccb", ipseccb_id_ht_,
                  HAL_MAX_IPSECCB/2,
                  hal::ipseccb_get_key_func,
                  hal::ipseccb_compute_hash_func,
                  hal::ipseccb_compare_key_func);
    HAL_ASSERT_RETURN((ipseccb_id_ht_ != NULL), false);

    // initialize CPU CB related data structures
    HAL_HT_CREATE("cpucb", cpucb_id_ht_,
                  HAL_MAX_CPUCB >> 1,
                  hal::cpucb_get_key_func,
                  hal::cpucb_compute_hash_func,
                  hal::cpucb_compare_key_func);
    HAL_ASSERT_RETURN((cpucb_id_ht_ != NULL), false);

    // initialize Raw Redirect CB related data structures
    HAL_HT_CREATE("rawrcb", rawrcb_id_ht_,
                  HAL_MAX_RAWRCB_HT_SIZE >> 1,
                  hal::rawrcb_get_key_func,
                  hal::rawrcb_compute_hash_func,
                  hal::rawrcb_compare_key_func);
    HAL_ASSERT_RETURN((rawrcb_id_ht_ != NULL), false);

    // initialize Raw Chain CB related data structures
    HAL_HT_CREATE("rawccb", rawccb_id_ht_,
                  HAL_MAX_RAWCCB_HT_SIZE >> 1,
                  hal::rawccb_get_key_func,
                  hal::rawccb_compute_hash_func,
                  hal::rawccb_compare_key_func);
    HAL_ASSERT_RETURN((rawccb_id_ht_ != NULL), false);

    // initialize Raw Redirect CB related data structures
    HAL_HT_CREATE("proxyrcb", proxyrcb_id_ht_,
                  HAL_MAX_PROXYRCB_HT_SIZE >> 1,
                  hal::proxyrcb_get_key_func,
                  hal::proxyrcb_compute_hash_func,
                  hal::proxyrcb_compare_key_func);
    HAL_ASSERT_RETURN((proxyrcb_id_ht_ != NULL), false);

    // initialize Raw Chain CB related data structures
    HAL_HT_CREATE("proxyccb", proxyccb_id_ht_,
                  HAL_MAX_PROXYCCB_HT_SIZE >> 1,
                  hal::proxyccb_get_key_func,
                  hal::proxyccb_compute_hash_func,
                  hal::proxyccb_compare_key_func);
    HAL_ASSERT_RETURN((proxyccb_id_ht_ != NULL), false);

    HAL_HT_CREATE("nwsec-policy-cfg", nwsec_policy_cfg_ht_,
                  HAL_MAX_VRFS >> 1,
                  hal::nwsec_policy_cfg_get_key_func,
                  hal::nwsec_policy_cfg_compute_hash_func,
                  hal::nwsec_policy_cfg_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_policy_cfg_ht_ != NULL), false);

    HAL_HT_CREATE("sfw policy", nwsec_policy_ht_,
                  HAL_MAX_VRFS >> 1,
                  hal::nwsec_policy_get_key_func,
                  hal::nwsec_policy_compute_hash_func,
                  hal::nwsec_policy_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_policy_ht_ != NULL), false);

    HAL_HT_CREATE("nat policy", nat_policy_ht_,
                  HAL_MAX_VRFS >> 1,
                  hal::nat_cfg_pol_key_func_get,
                  hal::nat_cfg_pol_hash_func_compute,
                  hal::nat_cfg_pol_key_func_compare);
    HAL_ASSERT_RETURN((nat_policy_ht_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// init() function to instantiate all the oper db init state
//------------------------------------------------------------------------------
bool
hal_oper_db::init(hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    mmgr_ = mmgr;
    HAL_ASSERT_RETURN((init_pss(hal_cfg, mmgr) == true), false);
    HAL_ASSERT_RETURN((init_vss(hal_cfg) == true), false);
    return true;
}

//------------------------------------------------------------------------------
// initialize the state that is not valid after a process restart
//------------------------------------------------------------------------------
void
hal_oper_db::init_on_restart(hal_cfg_t *hal_cfg) {
    init_vss(hal_cfg);
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_oper_db::hal_oper_db()
{
    /// keep the below stuff ///
    mmgr_ = NULL;
    hal_handle_id_ht_  = NULL;
    vrf_id_ht_ = NULL;
    network_key_ht_ = NULL;
    nwsec_profile_id_ht_ = NULL;
    l2seg_id_ht_ = NULL;
    ep_l2_ht_ = NULL;
    ep_l3_entry_ht_ = NULL;
    mc_key_ht_ = NULL;
    lif_id_ht_ = NULL;
    if_id_ht_ = NULL;
    session_hal_handle_ht_= NULL;
    session_hal_iflow_ht_ = NULL;
    session_hal_rflow_ht_ = NULL;
    l4lb_ht_ = NULL;
    nwsec_policy_cfg_ht_ = NULL;
    nwsec_policy_ht_ = NULL;
    nat_policy_ht_ = NULL;
    nwsec_group_ht_ = NULL;
    qos_class_ht_ = NULL;
    qos_cmap_pcp_bmp_ = NULL;
    qos_cmap_dscp_bmp_ = NULL;
    copp_ht_ = NULL;
    acl_ht_ = NULL;
    crypto_cert_store_id_ht_ = NULL;
    tlscb_id_ht_ = NULL;
    tcpcb_id_ht_ = NULL;
    wring_id_ht_ = NULL;
    proxy_type_ht_ = NULL;
    ipseccb_id_ht_ = NULL;
    ipsec_sa_id_ht_ = NULL;
    cpucb_id_ht_ = NULL;
    rawrcb_id_ht_ = NULL;
    app_redir_if_id_ = HAL_IFINDEX_INVALID;
    rawccb_id_ht_ = NULL;
    proxyrcb_id_ht_ = NULL;
    proxyccb_id_ht_ = NULL;
    crypto_cert_store_id_ht_ = NULL;
    gft_exact_match_profile_id_ht_ = NULL;
    gft_hdr_transposition_profile_id_ht_ = NULL;
    gft_exact_match_flow_entry_id_ht_ = NULL;
    nat_pool_ht_ = NULL;
    nat_mapping_ht_ = NULL;
    nexthop_id_ht_ = NULL;
    route_ht_ = NULL;

    forwarding_mode_ = HAL_FORWARDING_MODE_NONE;
    infra_vrf_handle_ = HAL_HANDLE_INVALID;
    event_mgr_ = NULL;
    memset(&mytep_ip_, 0, sizeof(mytep_ip_));
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_oper_db::~hal_oper_db()
{
    hal_handle_id_ht_ ? ht::destroy(hal_handle_id_ht_, mmgr_) : HAL_NOP;
    vrf_id_ht_ ? ht::destroy(vrf_id_ht_, mmgr_) : HAL_NOP;
    network_key_ht_ ? ht::destroy(network_key_ht_, mmgr_) : HAL_NOP;
    nwsec_profile_id_ht_ ? ht::destroy(nwsec_profile_id_ht_, mmgr_) : HAL_NOP;
    l2seg_id_ht_ ? ht::destroy(l2seg_id_ht_, mmgr_) : HAL_NOP;
    ep_l2_ht_ ? ht::destroy(ep_l2_ht_, mmgr_) : HAL_NOP;
    ep_l3_entry_ht_ ? ht::destroy(ep_l3_entry_ht_, mmgr_) : HAL_NOP;
    mc_key_ht_ ? ht::destroy(mc_key_ht_, mmgr_) : HAL_NOP;
    lif_id_ht_ ? ht::destroy(lif_id_ht_, mmgr_) : HAL_NOP;
    if_id_ht_ ? ht::destroy(if_id_ht_, mmgr_) : HAL_NOP;
    session_hal_handle_ht_ ? ht::destroy(session_hal_handle_ht_) : HAL_NOP;
    session_hal_iflow_ht_ ? ht::destroy(session_hal_iflow_ht_) : HAL_NOP;
    session_hal_rflow_ht_ ? ht::destroy(session_hal_rflow_ht_) : HAL_NOP;
    l4lb_ht_ ? ht::destroy(l4lb_ht_, mmgr_) : HAL_NOP;
    crypto_cert_store_id_ht_ ? ht::destroy(crypto_cert_store_id_ht_) : HAL_NOP;
    tlscb_id_ht_ ? ht::destroy(tlscb_id_ht_) : HAL_NOP;
    tcpcb_id_ht_ ? ht::destroy(tcpcb_id_ht_) : HAL_NOP;
    qos_class_ht_ ? ht::destroy(qos_class_ht_, mmgr_) : HAL_NOP;
    qos_cmap_pcp_bmp_ ? bitmap::destroy(qos_cmap_pcp_bmp_) : HAL_NOP;
    qos_cmap_dscp_bmp_ ? bitmap::destroy(qos_cmap_dscp_bmp_) : HAL_NOP;
    copp_ht_ ? ht::destroy(copp_ht_, mmgr_) : HAL_NOP;
    acl_ht_ ? ht::destroy(acl_ht_, mmgr_) : HAL_NOP;
    wring_id_ht_ ? ht::destroy(wring_id_ht_) : HAL_NOP;
    proxy_type_ht_ ? ht::destroy(proxy_type_ht_) : HAL_NOP;
    ipseccb_id_ht_ ? ht::destroy(ipseccb_id_ht_) : HAL_NOP;
    ipsec_sa_id_ht_ ? ht::destroy(ipsec_sa_id_ht_) : HAL_NOP;
    cpucb_id_ht_ ? ht::destroy(cpucb_id_ht_) : HAL_NOP;
    rawrcb_id_ht_ ? ht::destroy(rawrcb_id_ht_) : HAL_NOP;
    rawccb_id_ht_ ? ht::destroy(rawccb_id_ht_) : HAL_NOP;
    proxyrcb_id_ht_ ? ht::destroy(proxyrcb_id_ht_) : HAL_NOP;
    proxyccb_id_ht_ ? ht::destroy(proxyccb_id_ht_) : HAL_NOP;
    nwsec_policy_cfg_ht_ ? ht::destroy(nwsec_policy_cfg_ht_) : HAL_NOP;
    nwsec_policy_ht_ ? ht::destroy(nwsec_policy_ht_) : HAL_NOP;
    nat_policy_ht_ ? ht::destroy(nat_policy_ht_) : HAL_NOP;
    nwsec_group_ht_ ? ht::destroy(nwsec_group_ht_) : HAL_NOP;
    nat_pool_ht_ ? ht::destroy(nat_pool_ht_) : HAL_NOP;
    nat_mapping_ht_ ? ht::destroy(nat_mapping_ht_) : HAL_NOP;
    nexthop_id_ht_ ? ht::destroy(nexthop_id_ht_) : HAL_NOP;
    route_ht_ ? ht::destroy(route_ht_) : HAL_NOP;
    gft_exact_match_profile_id_ht_ ? ht::destroy(gft_exact_match_profile_id_ht_, mmgr_) : HAL_NOP;
    gft_hdr_transposition_profile_id_ht_ ? ht::destroy(gft_hdr_transposition_profile_id_ht_, mmgr_) : HAL_NOP;
    gft_exact_match_flow_entry_id_ht_ ? ht::destroy(gft_exact_match_flow_entry_id_ht_, mmgr_) : HAL_NOP;
    event_mgr_ ? eventmgr::destroy(event_mgr_) : HAL_NOP;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_oper_db *
hal_oper_db::factory(hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    void           *mem;
    hal_oper_db    *oper_db;

    if (mmgr) {
        mem = mmgr->alloc(sizeof(hal_oper_db), 4, true);
    } else {
        mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_oper_db));
    }

    if (mem) {
        oper_db = new(mem) hal_oper_db();
        if (oper_db->init(hal_cfg, mmgr) == false) {
            oper_db->~hal_oper_db();
            if (mmgr) {
                mmgr->free(mem);
            } else {
                HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
            }
            return NULL;
        }
        return oper_db;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// destroy method
//------------------------------------------------------------------------------
void
hal_oper_db::destroy(hal_oper_db *oper_db)
{
    shmmgr    *mmgr;

    if (!oper_db) {
        return;
    }
    mmgr = oper_db->mmgr_;
    oper_db->~hal_oper_db();
    if (mmgr) {
        mmgr->free(oper_db);
    } else {
        HAL_FREE(HAL_MEM_ALLOC_INFRA, oper_db);
    }
}

//------------------------------------------------------------------------------
// common cleanup method
//------------------------------------------------------------------------------
void
hal_state::cleanup(void) {
    if (cfg_db_) {
        hal_cfg_db::destroy(cfg_db_);
    }
    if (oper_db_) {
        hal_oper_db::destroy(oper_db_);
    }
    if (api_stats_) {
        if (mmgr_) {
            mmgr_->free(api_stats_);
        } else {
            HAL_FREE(HAL_MEM_ALLOC_API_STATS, api_stats_);
        }
    }
    mmgr_ = NULL;
    obj_meta_ = NULL;
}

//------------------------------------------------------------------------------
// constructor method
//------------------------------------------------------------------------------
hal_state::hal_state(hal_obj_meta **obj_meta, hal_cfg_t *hal_cfg, shmmgr *mmgr)
{
    mmgr_ = mmgr;
    cfg_db_ = NULL;
    oper_db_ = NULL;
    api_stats_ = NULL;
    catalog_ = NULL;
    obj_meta_ = obj_meta;

    cfg_db_ = hal_cfg_db::factory(hal_cfg, mmgr);
    HAL_ASSERT_GOTO(cfg_db_, error);

    oper_db_ = hal_oper_db::factory(hal_cfg, mmgr);
    HAL_ASSERT_GOTO(oper_db_, error);

    if (mmgr) {
        api_stats_ =
            (hal_stats_t *)mmgr->alloc(sizeof(hal_stats_t) * HAL_API_MAX,
                                       4, true);
    } else {
        api_stats_ = (hal_stats_t *)HAL_CALLOC(HAL_MEM_ALLOC_API_STATS,
                                        sizeof(hal_stats_t) * HAL_API_MAX);
    }
    HAL_ASSERT_GOTO(api_stats_, error);
    return;

error:

    cleanup();
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state::~hal_state() {
    cleanup();
}

//------------------------------------------------------------------------------
// initialize the state that is not valid after a process restart
//------------------------------------------------------------------------------
void
hal_state::init_on_restart(hal_cfg_t *hal_cfg) {
    catalog_ = NULL;
    cfg_db_->init_on_restart(hal_cfg);
    oper_db_->init_on_restart(hal_cfg);
}

//------------------------------------------------------------------------------
// common object marshalling hook
//------------------------------------------------------------------------------
static bool
hal_obj_marshall (void *obj, void *ctxt)
{
    hal_ret_t            ret;
    hal_state_mctxt_t    *mctxt;
    tlv_t                *tlv;

    mctxt = (hal_state_mctxt_t *)ctxt;
    tlv = (tlv_t *)mctxt->mem;
    tlv->type = mctxt->obj_id;
    ret = mctxt->marshall_cb(obj, tlv->val,
                             mctxt->len - sizeof(tlv_t), &tlv->len);
    // tlv->len == 0: Marshall CB may not have been implemented. So skip
    //                adding to the persistent memory
    if (ret != HAL_RET_OK || tlv->len == 0) {
        HAL_TRACE_ERR("Marshalling failed for obj id {}, buf len available {}",
                      tlv->type, mctxt->len);
        // TODO: abort ??
        goto end;
    }
    mctxt->mem += sizeof(tlv_t) + tlv->len;
    mctxt->len -= sizeof(tlv_t) + tlv->len;

end:
    return false;    // don't stop the walk
}

//------------------------------------------------------------------------------
// common object unmarshalling hook
//------------------------------------------------------------------------------
static hal_ret_t
hal_obj_unmarshall (hal_state_umctxt_t *umctxt)
{
    HAL_ASSERT((umctxt != NULL) && (umctxt->obj != NULL) && (umctxt->len > 0));
    HAL_TRACE_DEBUG("Unmarshalling obj id {}, len {}",
                    umctxt->obj_id, umctxt->len);
    umctxt->unmarshall_cb(umctxt->obj, umctxt->len);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// preserve the state in the class in the given persistent memory and return
// the number of bytes used up
// this is called when the upgrade starts
//------------------------------------------------------------------------------
uint64_t
hal_state::preserve_state(void)
{
    uint32_t                       obj_id, avail_sz;
    hal_state_mctxt_t              mctxt = { 0 };
    hal_state_hints               *h3s_hints;
    slab                           *slab_obj;
    fixed_managed_shared_memory    *fm_shm_mgr;

    g_h3s_shmmgr =
        shmmgr::factory(HAL_SERIALIZED_STATE_STORE,
                        HAL_SERIALIZED_STATE_STORE_SIZE,
                        sdk::lib::SHM_CREATE_ONLY,
                        (void *)HAL_SERIALIZED_STATE_STORE_VADDR);
    if (g_h3s_shmmgr == NULL) {
        HAL_TRACE_ERR("Failed to create HAL serialized state store");
        return 0;
    }

    // construct our root (aka. hints) object
    fm_shm_mgr = (fixed_managed_shared_memory *)g_h3s_shmmgr->mmgr();
    h3s_hints = fm_shm_mgr->construct<hal_state_hints>(HAL_STATE_HINTS)();
    HAL_ASSERT(h3s_hints != NULL);

    // initialize the marshalling context
    mctxt.len = avail_sz = g_h3s_shmmgr->free_size() - 32;    // 32 bytes for internal meta of boost mem mgr.
    mctxt.mem = (uint8_t *)g_h3s_shmmgr->alloc(avail_sz);
    HAL_ASSERT(mctxt.mem != NULL);
    h3s_hints->set_pi_state(mctxt.mem);

    // walk all objects and preserve the state, if needed
    for (obj_id = (uint32_t)HAL_OBJ_ID_MIN;
         obj_id < (uint32_t)HAL_OBJ_ID_MAX; obj_id++) {
        if (obj_meta_[obj_id] == NULL ||
            obj_meta_[obj_id]->marshall_cb() == NULL) {
            continue;
        }
        mctxt.obj_id = (uint32_t)obj_id;
        mctxt.marshall_cb = obj_meta_[obj_id]->marshall_cb();
        slab_obj = cfg_db_->get_slab(obj_meta_[obj_id]->slab_id());
        HAL_ASSERT(slab_obj != NULL);
        HAL_TRACE_DEBUG("Preserving state in slab {} for obj id {}",
                        slab_obj->name(), obj_id);
        slab_obj->walk(hal_obj_marshall, &mctxt);
        HAL_TRACE_DEBUG("Preserved state in slab {}, state store size {}, "
                        "used {}, free {}", slab_obj->name(), avail_sz,
                        avail_sz - mctxt.len, mctxt.len);
    }
    h3s_hints->set_pi_state_len(avail_sz - mctxt.len);

    HAL_TRACE_ERR("Total marshalled state len is {}", avail_sz - mctxt.len);
    return (avail_sz - mctxt.len);
}

//------------------------------------------------------------------------------
// restore the state from given memory segment
//------------------------------------------------------------------------------
hal_ret_t
hal_state::restore_state(void)
{
    hal_state_hints                *h3s_hints;
    fixed_managed_shared_memory    *fm_shm_mgr;
    uint8_t                        *state;
    uint32_t                       obj_id, state_len;
    tlv_t                          *tlv;
    hal_state_umctxt_t             umctxt = { 0 };

    fm_shm_mgr = (fixed_managed_shared_memory *)g_h3s_shmmgr->mmgr();
    std::pair<hal_state_hints *, std::size_t> h3shints_info =
        fm_shm_mgr->find<hal_state_hints>(HAL_STATE_HINTS);
    if ((h3s_hints = h3shints_info.first) == NULL) {
        HAL_TRACE_ERR("Failed to find HAL state hints in state store");
        return HAL_RET_ERR;
    }

    // start unmarshalling objs
    state = (uint8_t *)h3s_hints->pi_state();
    state_len = h3s_hints->pi_state_len();
    HAL_TRACE_DEBUG("Total marshalled state len found is {}", state_len);
    while (state_len) {
        tlv = (tlv_t *)state;
        if (!obj_meta_[obj_id = tlv->type]) {
            // no need to carry over this object's state into new version
            HAL_TRACE_ERR("Skipping state restoration for obj {}", obj_id);
            goto skip;
        }
#if 0
        slab_obj = get_slab(obj_meta_[obj_id].slab_id);
        if (slab_obj) {
            HAL_TRACE_ERR("Skipping state restoration for obj {}, slab {} "
                          "not found", obj_id, obj_meta_[obj_id].slab_id);
            state += sizeof(tlv_t) + tlv->len;
            state_len -= sizeof(tlv_t) + tlv->len
            continue;
        }
#endif
        // now unmarshall this object
        umctxt.obj_id = obj_id;
        umctxt.obj = tlv->val;
        umctxt.len = tlv->len;
        umctxt.unmarshall_cb = obj_meta_[obj_id]->unmarshall_cb();
        hal_obj_unmarshall(&umctxt);

skip:

        state += sizeof(tlv_t) + tlv->len;
        state_len -= sizeof(tlv_t) + tlv->len;
        HAL_TRACE_DEBUG("state remaining is {}", state_len);
    }

    // restore PI state to new slabs
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize HAL state
//------------------------------------------------------------------------------
hal_ret_t
hal_state_init (hal_cfg_t *hal_cfg)
{
    bool    h2s_exists = false, h3s_exists = false;
    bool    shm_mode = hal_cfg->shm_mode;

    // do object meta initialization
    hal_obj_meta_init();

    // check if memory segments of interest exist
    h2s_exists = shmmgr::exists(HAL_STATE_STORE,
                                (void *)HAL_STATE_STORE_VADDR);
    h3s_exists = shmmgr::exists(HAL_SERIALIZED_STATE_STORE,
                                (void *)HAL_SERIALIZED_STATE_STORE_VADDR);

    if (!shm_mode) {
        // stateless restart or upgrade
        if (h2s_exists) {
            // stateless restart, nuke detected state store
            HAL_TRACE_DEBUG("Detected stateless process restart, "
                            "freeing state store");
            shmmgr::remove(HAL_STATE_STORE);
        }
        if (h3s_exists) {
            // stateless upgrade case, nuke detected state store
            HAL_TRACE_DEBUG("Detected stateless upgrade, "
                            "freeing state store");
            shmmgr::remove(HAL_SERIALIZED_STATE_STORE);
        }
        // instantiate HAL state in regular linux memory
        g_hal_state = new hal_state(g_obj_meta, hal_cfg, nullptr);
    } else if (h2s_exists) {
        // stateful restart case
        HAL_TRACE_DEBUG("Stateful restart detected, restoring state");
        g_h2s_shmmgr =
            shmmgr::factory(HAL_STATE_STORE, HAL_STATE_STORE_SIZE,
                            sdk::lib::SHM_OPEN_ONLY,
                            (void *)HAL_STATE_STORE_VADDR);
        HAL_ABORT(g_h2s_shmmgr != NULL);

        // reconstruct hal state
        fixed_managed_shared_memory    *fm_shm_mgr;
        fm_shm_mgr = (fixed_managed_shared_memory *)g_h2s_shmmgr->mmgr();
        std::pair<hal_state *, std::size_t> h2sinfo =
            fm_shm_mgr->find<hal_state>(HAL_STATE_OBJ);
        if ((g_hal_state = h2sinfo.first) == NULL) {
            HAL_TRACE_ERR("Failed to find HAL state obj in state store");
            return HAL_RET_ERR;
        }

        // there may be some state in g_hal_state that needs to be
        // reset/reinitialized (e.g., pointers to objects in non-shared memory
        // like catalog pointer etc. aren't valid after restart, so need to
        // reset to NULL at this point .. the pointers to objects that exist in
        // shared memory are still valid, so we don't reset them
        g_hal_state->init_on_restart(hal_cfg);
        HAL_TRACE_DEBUG("HAL state obj found, state restored");
    } else if (h3s_exists) {
        // stateful upgrade case
        HAL_TRACE_DEBUG("Stateful upgrade detected, restoring state store");
        // open serialized state store to restore the state from
        g_h3s_shmmgr =
            shmmgr::factory(HAL_SERIALIZED_STATE_STORE,
                            HAL_SERIALIZED_STATE_STORE_SIZE,
                            sdk::lib::SHM_OPEN_ONLY,
                            (void *)HAL_SERIALIZED_STATE_STORE_VADDR);
        HAL_ABORT(g_h3s_shmmgr != NULL);

        // open state store to restore the state to
        g_h2s_shmmgr =
            shmmgr::factory(HAL_STATE_STORE, HAL_STATE_STORE_SIZE,
                            sdk::lib::SHM_CREATE_ONLY,
                            (void *)HAL_STATE_STORE_VADDR);
        HAL_ABORT(g_h2s_shmmgr != NULL);

        // instantiate HAL state in persistent memory
        fixed_managed_shared_memory    *fm_shm_mgr;
        fm_shm_mgr = (fixed_managed_shared_memory *)g_h2s_shmmgr->mmgr();
        g_hal_state =
            fm_shm_mgr->construct<hal_state>(HAL_STATE_OBJ)(g_obj_meta,
                                                            hal_cfg,
                                                            g_h2s_shmmgr);

        // restore preserved state
        g_hal_state->restore_state();

        // nuke serialized state store now that we finished restoring all state
        shmmgr::destroy(g_h3s_shmmgr);
    } else {
        // coming up in shm mode, but no existing state store
        HAL_TRACE_DEBUG("Creating new HAL state store in shared memory");
        g_h2s_shmmgr =
            shmmgr::factory(HAL_STATE_STORE, HAL_STATE_STORE_SIZE,
                            sdk::lib::SHM_CREATE_ONLY,
                            (void *)HAL_STATE_STORE_VADDR);
        HAL_ABORT(g_h2s_shmmgr != NULL);

        // instantiate HAL state in persistent memory
        fixed_managed_shared_memory    *fm_shm_mgr;
        fm_shm_mgr = (fixed_managed_shared_memory *)g_h2s_shmmgr->mmgr();
        g_hal_state =
            fm_shm_mgr->construct<hal_state>(HAL_STATE_OBJ)(g_obj_meta, hal_cfg,
                                                            g_h2s_shmmgr);
    }

    // in all cases g_hal_state must be setup by now
    HAL_ASSERT_RETURN((g_hal_state != NULL), HAL_RET_ERR);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// free an element back to given slab specified by its id
//------------------------------------------------------------------------------
hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_HANDLE:
        g_hal_state->hal_handle_slab()->free(elem);
        break;

    case HAL_SLAB_HANDLE_HT_ENTRY:
        g_hal_state->hal_handle_ht_entry_slab()->free(elem);
        break;

    case HAL_SLAB_HANDLE_LIST_ENTRY:
        g_hal_state->hal_handle_list_entry_slab()->free(elem);
        break;

    case HAL_SLAB_HANDLE_ID_HT_ENTRY:
        g_hal_state->hal_handle_id_ht_entry_slab()->free(elem);
        break;

    case HAL_SLAB_HANDLE_ID_LIST_ENTRY:
        g_hal_state->hal_handle_id_list_entry_slab()->free(elem);
        break;

    case HAL_SLAB_VRF:
        g_hal_state->vrf_slab()->free(elem);
        break;

    case HAL_SLAB_NETWORK:
        g_hal_state->network_slab()->free(elem);
        break;

    case HAL_SLAB_L2SEG:
        g_hal_state->l2seg_slab()->free(elem);
        break;

    case HAL_SLAB_MC_ENTRY:
        g_hal_state->mc_entry_slab()->free(elem);
        break;

    case HAL_SLAB_LIF:
        g_hal_state->lif_slab()->free(elem);
        break;

    case HAL_SLAB_IF:
        g_hal_state->if_slab()->free(elem);
        break;

    case HAL_SLAB_ENIC_L2SEG_ENTRY:
        g_hal_state->enic_l2seg_entry_slab()->free(elem);
        break;

    case HAL_SLAB_EP:
        g_hal_state->ep_slab()->free(elem);
        break;

    case HAL_SLAB_EP_IP_ENTRY:
        g_hal_state->ep_ip_entry_slab()->free(elem);
        break;

    case HAL_SLAB_EP_L3_ENTRY:
        g_hal_state->ep_l3_entry_slab()->free(elem);
        break;

    case HAL_SLAB_FLOW:
        g_hal_state->flow_slab()->free(elem);
        break;

    case HAL_SLAB_SESSION:
        g_hal_state->session_slab()->free(elem);
        break;

    case HAL_SLAB_SECURITY_PROFILE:
        g_hal_state->nwsec_profile_slab()->free(elem);
        break;

    case HAL_SLAB_NWSEC_GROUP:
        g_hal_state->nwsec_group_slab()->free(elem);
        break;

    case HAL_SLAB_NWSEC_RULE:
        g_hal_state->nwsec_rule_slab()->free(elem);
        break;

    case HAL_SLAB_IPV4_RULE:
        g_hal_state->ipv4_rule_slab()->free(elem);
        break;

    case HAL_SLAB_RULE_DATA:
        g_hal_state->rule_data_slab()->free(elem);
        break;

    case HAL_SLAB_NWSEC_POLICY_CFG:
        g_hal_state->nwsec_policy_cfg_slab()->free(elem);
        break;

    case HAL_SLAB_NWSEC_POLICY_RULES:
        g_hal_state->nwsec_policy_rules_slab()->free(elem);
        break;

    case HAL_SLAB_NWSEC_POLICY_SVC:
        g_hal_state->nwsec_policy_svc_slab()->free(elem);
        break;

    case HAL_SLAB_CRYPTO_CERT_STORE:
        g_hal_state->crypto_cert_store_slab()->free(elem);
        break;

    case HAL_SLAB_TLSCB:
        g_hal_state->tlscb_slab()->free(elem);
        break;

    case HAL_SLAB_TCPCB:
        g_hal_state->tcpcb_slab()->free(elem);
        break;

    case HAL_SLAB_QOS_CLASS:
        g_hal_state->qos_class_slab()->free(elem);
        break;

    case HAL_SLAB_ACL:
        g_hal_state->acl_slab()->free(elem);
        break;

    case HAL_SLAB_WRING:
        g_hal_state->wring_slab()->free(elem);
        break;

    case HAL_SLAB_PROXY:
        g_hal_state->proxy_slab()->free(elem);
        break;

    case HAL_SLAB_PROXY_FLOW_INFO:
        g_hal_state->proxy_flow_info_slab()->free(elem);
        break;

    case HAL_SLAB_IPSECCB:
        g_hal_state->ipseccb_slab()->free(elem);

    case HAL_SLAB_CPUCB:
        g_hal_state->cpucb_slab()->free(elem);
        break;

    case HAL_SLAB_RAWRCB:
        g_hal_state->rawrcb_slab()->free(elem);
        break;

    case HAL_SLAB_RAWCCB:
        g_hal_state->rawccb_slab()->free(elem);
        break;

    case HAL_SLAB_PROXYRCB:
        g_hal_state->proxyrcb_slab()->free(elem);
        break;

    case HAL_SLAB_PROXYCCB:
        g_hal_state->proxyccb_slab()->free(elem);
        break;

    case HAL_SLAB_DHCP_LEARN:
        //dhcp_trans_t::dhcplearn_slab()->free(elem);
        break;

    case HAL_SLAB_ARP_LEARN:
        //arp_trans_t::arplearn_slab()->free(elem);
        break;

    case HAL_SLAB_EVENT_MAP:
        g_hal_state->event_mgr()->event_map_slab()->free(elem);
        break;

    case HAL_SLAB_EVENT_MAP_LISTENER:
        g_hal_state->event_mgr()->event_listener_slab()->free(elem);
        break;

    case HAL_SLAB_EVENT_LISTENER:
        g_hal_state->event_mgr()->listener_slab()->free(elem);
        break;

    case HAL_SLAB_COPP:
        g_hal_state->copp_slab()->free(elem);
        break;

    case HAL_SLAB_GFT_EXACT_MATCH_PROFILE:
        g_hal_state->gft_exact_match_profile_slab()->free(elem);
        break;

    case HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE:
        g_hal_state->gft_hdr_transposition_profile_slab()->free(elem);
        break;

    case HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY:
        g_hal_state->gft_exact_match_flow_entry_slab()->free(elem);
        break;

    case HAL_SLAB_V4ADDR_LIST_ELEM:
        g_hal_state->v4addr_list_elem_slab()->free(elem);
        break;

    case HAL_SLAB_V6ADDR_LIST_ELEM:
        g_hal_state->v6addr_list_elem_slab()->free(elem);
        break;

    case HAL_SLAB_PORT_LIST_ELEM:
        g_hal_state->port_list_elem_slab()->free(elem);
        break;

    case HAL_SLAB_NAT_CFG_RULE:
        g_hal_state->nat_cfg_rule_slab()->free(elem);
        break;

    case HAL_SLAB_NAT_CFG_POL:
        g_hal_state->nat_cfg_pol_slab()->free(elem);
        break;

    case HAL_SLAB_NEXTHOP:
        g_hal_state->nexthop_slab()->free(elem);
        break;

    case HAL_SLAB_NAT_POOL:
        g_hal_state->nat_pool_slab()->free(elem);
        break;

    case HAL_SLAB_ROUTE:
        g_hal_state->route_slab()->free(elem);
        break;

    case HAL_SLAB_ROUTE_ACL_RULE:
        g_hal_state->route_acl_rule_slab()->free(elem);
        break;

    case HAL_SLAB_ROUTE_ACL_USERDATA:
        g_hal_state->route_acl_userdata_slab()->free(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        HAL_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
        break;
    }

    return HAL_RET_OK;
}

shmmgr *
hal_mmgr (void)
{
    return g_hal_state->mmgr();
}

}    // namespace hal
