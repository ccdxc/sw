// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/port.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/src/event.hpp"
#include "nic/hal/src/nwsec_group.hpp"
#include "nic/hal/src/tlscb.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/hal/src/qos.hpp"
#include "nic/hal/src/acl.hpp"
#include "nic/hal/src/wring.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/ipseccb.hpp"
#include "nic/hal/src/l4lb.hpp"
#include "nic/hal/src/cpucb.hpp"
#include "nic/hal/src/rawrcb.hpp"
#include "nic/hal/src/rawccb.hpp"
#include "nic/hal/src/dos.hpp"
#include "nic/hal/plugins/network/ep_learn/arp/arp_trans.hpp"
#include "nic/hal/plugins/network/ep_learn/dhcp/dhcp_trans.hpp"
 
namespace hal {

// global instance of all HAL state including config, operational states
class hal_state    *g_hal_state;

typedef struct cfg_db_dirty_objs_s cfg_db_dirty_objs_t;
struct cfg_db_dirty_objs_s {
    void                   *obj;     // object itself (TODO: how do I know what
                                     // type of object this is to free to right
                                     // slab) ?? will a base class help ??
    cfg_version_t          ver;      // version of this object
    cfg_db_dirty_objs_t    *next;    // next object in the cache
} __PACK__;

//------------------------------------------------------------------------------
// init() function to instantiate all the config db init state
//------------------------------------------------------------------------------
bool
hal_cfg_db::init(void)
{
    // initialize vrf related data structures
    vrf_id_ht_ = ht::factory(HAL_MAX_VRFS,
                                hal::vrf_id_get_key_func,
                                hal::vrf_id_compute_hash_func,
                                hal::vrf_id_compare_key_func);
    HAL_ASSERT_RETURN((vrf_id_ht_ != NULL), false);

    // initialize network related data structures
    network_key_ht_ = ht::factory(HAL_MAX_VRFS,
                                  hal::network_get_key_func,
                                  hal::network_compute_hash_func,
                                  hal::network_compare_key_func);
    HAL_ASSERT_RETURN((network_key_ht_ != NULL), false);

    // initialize security profile related data structures
    nwsec_profile_id_ht_ = ht::factory(HAL_MAX_NWSEC_PROFILES,
                                       hal::nwsec_profile_id_get_key_func,
                                       hal::nwsec_profile_id_compute_hash_func,
                                       hal::nwsec_profile_id_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_profile_id_ht_ != NULL), false);

    // initialize L2 segment related data structures
    l2seg_id_ht_ = ht::factory(HAL_MAX_L2SEGMENTS,
                               hal::l2seg_id_get_key_func,
                               hal::l2seg_id_compute_hash_func,
                               hal::l2seg_id_compare_key_func);
    HAL_ASSERT_RETURN((l2seg_id_ht_ != NULL), false);

    // initialize LIF related data structures
    lif_id_ht_ = ht::factory(HAL_MAX_LIFS,
                             hal::lif_id_get_key_func,
                             hal::lif_id_compute_hash_func,
                             hal::lif_id_compare_key_func);
    HAL_ASSERT_RETURN((lif_id_ht_ != NULL), false);

    // initialize interface related data structures
    if_id_ht_ = ht::factory(HAL_MAX_INTERFACES,
                            hal::if_id_get_key_func,
                            hal::if_id_compute_hash_func,
                            hal::if_id_compare_key_func);
    HAL_ASSERT_RETURN((if_id_ht_ != NULL), false);

    // initialize port related data structures
    port_id_ht_ = ht::factory(HAL_MAX_PORTS,
                              hal::port_id_get_key_func,
                              hal::port_id_compute_hash_func,
                              hal::port_id_compare_key_func);
    HAL_ASSERT_RETURN((port_id_ht_ != NULL), false);

    // initialize flow/session related data structures
    session_id_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                 hal::session_get_key_func,
                                 hal::session_compute_hash_func,
                                 hal::session_compare_key_func);
    HAL_ASSERT_RETURN((session_id_ht_ != NULL), false);

    session_hal_handle_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                         hal::session_get_handle_key_func,
                                         hal::session_compute_handle_hash_func,
                                         hal::session_compare_handle_key_func);
    HAL_ASSERT_RETURN((session_hal_handle_ht_ != NULL), false);

    session_hal_iflow_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                    hal::session_get_iflow_key_func,
                                    hal::session_compute_iflow_hash_func,
                                    hal::session_compare_iflow_key_func);
    HAL_ASSERT_RETURN((session_hal_iflow_ht_ != NULL), false);

    session_hal_rflow_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                    hal::session_get_rflow_key_func,
                                    hal::session_compute_rflow_hash_func,
                                    hal::session_compare_rflow_key_func);
    HAL_ASSERT_RETURN((session_hal_rflow_ht_ != NULL), false);

    // initialize l4lb related data structures
    l4lb_ht_ = ht::factory(HAL_MAX_L4LB_SERVICES,
                           hal::l4lb_get_key_func,
                           hal::l4lb_compute_key_hash_func,
                           hal::l4lb_compare_key_func);
    HAL_ASSERT_RETURN((l4lb_ht_ != NULL), false);

    l4lb_hal_handle_ht_ = ht::factory(HAL_MAX_IPSECCB,
                                      hal::l4lb_get_handle_key_func,
                                      hal::l4lb_compute_handle_hash_func,
                                      hal::l4lb_compare_handle_key_func);
    HAL_ASSERT_RETURN((l4lb_hal_handle_ht_ != NULL), false);
    // initialize TLS CB related data structures
    tlscb_id_ht_ = ht::factory(HAL_MAX_TLSCB,
                               hal::tlscb_get_key_func,
                               hal::tlscb_compute_hash_func,
                               hal::tlscb_compare_key_func);
    HAL_ASSERT_RETURN((tlscb_id_ht_ != NULL), false);

    tlscb_hal_handle_ht_ = ht::factory(HAL_MAX_TLSCB,
                                       hal::tlscb_get_handle_key_func,
                                       hal::tlscb_compute_handle_hash_func,
                                       hal::tlscb_compare_handle_key_func);
    HAL_ASSERT_RETURN((tlscb_hal_handle_ht_ != NULL), false);

    // initialize TCB CB related data structures
    tcpcb_id_ht_ = ht::factory(HAL_MAX_TCPCB,
                               hal::tcpcb_get_key_func,
                               hal::tcpcb_compute_hash_func,
                               hal::tcpcb_compare_key_func);
    HAL_ASSERT_RETURN((tcpcb_id_ht_ != NULL), false);

    tcpcb_hal_handle_ht_ = ht::factory(HAL_MAX_TCPCB,
                                       hal::tcpcb_get_handle_key_func,
                                       hal::tcpcb_compute_handle_hash_func,
                                       hal::tcpcb_compare_handle_key_func);
    HAL_ASSERT_RETURN((tcpcb_hal_handle_ht_ != NULL), false);

    // initialize Buf-Pool related data structures
    buf_pool_id_ht_ = ht::factory(HAL_MAX_BUF_POOLS,
                                  hal::buf_pool_get_key_func,
                                  hal::buf_pool_compute_hash_func,
                                  hal::buf_pool_compare_key_func);
    HAL_ASSERT_RETURN((buf_pool_id_ht_ != NULL), false);

    buf_pool_hal_handle_ht_ = ht::factory(HAL_MAX_BUF_POOLS,
                                          hal::buf_pool_get_handle_key_func,
                                          hal::buf_pool_compute_handle_hash_func,
                                          hal::buf_pool_compare_handle_key_func);
    HAL_ASSERT_RETURN((buf_pool_hal_handle_ht_ != NULL), false);

    // initialize Queue related data structures
    queue_id_ht_ = ht::factory(HAL_MAX_QUEUE_NODES,
                               hal::queue_get_key_func,
                               hal::queue_compute_hash_func,
                               hal::queue_compare_key_func);
    HAL_ASSERT_RETURN((queue_id_ht_ != NULL), false);

    queue_hal_handle_ht_ = ht::factory(HAL_MAX_QUEUE_NODES,
                                       hal::queue_get_handle_key_func,
                                       hal::queue_compute_handle_hash_func,
                                       hal::queue_compare_handle_key_func);
    HAL_ASSERT_RETURN((queue_hal_handle_ht_ != NULL), false);

    // initialize Policer related data structures
    ingress_policer_id_ht_ = ht::factory(HAL_MAX_POLICERS,
                                         hal::policer_get_key_func,
                                         hal::policer_compute_hash_func,
                                         hal::policer_compare_key_func);
    HAL_ASSERT_RETURN((ingress_policer_id_ht_ != NULL), false);

    ingress_policer_hal_handle_ht_ = ht::factory(HAL_MAX_POLICERS,
                                                 hal::policer_get_handle_key_func,
                                                 hal::policer_compute_handle_hash_func,
                                                 hal::policer_compare_handle_key_func);
    HAL_ASSERT_RETURN((ingress_policer_hal_handle_ht_ != NULL), false);

    egress_policer_id_ht_ = ht::factory(HAL_MAX_POLICERS,
                                        hal::policer_get_key_func,
                                        hal::policer_compute_hash_func,
                                        hal::policer_compare_key_func);
    HAL_ASSERT_RETURN((egress_policer_id_ht_ != NULL), false);

    egress_policer_hal_handle_ht_ = ht::factory(HAL_MAX_POLICERS,
                                                hal::policer_get_handle_key_func,
                                                hal::policer_compute_handle_hash_func,
                                                hal::policer_compare_handle_key_func);
    HAL_ASSERT_RETURN((egress_policer_hal_handle_ht_ != NULL), false);

    // initialize Acl related data structures
    acl_id_ht_ = ht::factory(HAL_MAX_ACLS,
                             hal::acl_get_key_func,
                             hal::acl_compute_hash_func,
                             hal::acl_compare_key_func);
    HAL_ASSERT_RETURN((acl_id_ht_ != NULL), false);

    acl_hal_handle_ht_ = ht::factory(HAL_MAX_ACLS,
                                     hal::acl_get_handle_key_func,
                                     hal::acl_compute_handle_hash_func,
                                     hal::acl_compare_handle_key_func);
    HAL_ASSERT_RETURN((acl_hal_handle_ht_ != NULL), false);

    // initialize WRing related data structures
    wring_id_ht_ = ht::factory(HAL_MAX_WRING,
                               hal::wring_get_key_func,
                               hal::wring_compute_hash_func,
                               hal::wring_compare_key_func);
    HAL_ASSERT_RETURN((wring_id_ht_ != NULL), false);

    wring_hal_handle_ht_ = ht::factory(HAL_MAX_WRING,
                                       hal::wring_get_handle_key_func,
                                       hal::wring_compute_handle_hash_func,
                                       hal::wring_compare_handle_key_func);
    HAL_ASSERT_RETURN((wring_hal_handle_ht_ != NULL), false);

    // initialize proxy service related data structures
    proxy_slab_ = slab::factory("proxy", HAL_SLAB_PROXY,
                                sizeof(hal::proxy_t), HAL_MAX_PROXY,
                                false, true, true, true);
    HAL_ASSERT_RETURN((proxy_slab_ != NULL), false);

    proxy_flow_info_slab_ = slab::factory("proxy-flow-infi", HAL_SLAB_PROXY_FLOW_INFO,
                                sizeof(hal::proxy_flow_info_t), HAL_MAX_PROXY_FLOWS,
                                false, true, true, true);
    HAL_ASSERT_RETURN((proxy_flow_info_slab_ != NULL), false);

    proxy_type_ht_ = ht::factory(HAL_MAX_PROXY,
                                 hal::proxy_get_key_func,
                                 hal::proxy_compute_hash_func,
                                 hal::proxy_compare_key_func);
    HAL_ASSERT_RETURN((proxy_type_ht_ != NULL), false);

    proxy_hal_handle_ht_ = ht::factory(HAL_MAX_PROXY,
                                       hal::proxy_get_handle_key_func,
                                       hal::proxy_compute_handle_hash_func,
                                       hal::proxy_compare_handle_key_func);
    HAL_ASSERT_RETURN((proxy_hal_handle_ht_ != NULL), false);

    // initialize IPSEC CB related data structures
    ipseccb_id_ht_ = ht::factory(HAL_MAX_IPSECCB,
                                 hal::ipseccb_get_key_func,
                                 hal::ipseccb_compute_hash_func,
                                 hal::ipseccb_compare_key_func);
    HAL_ASSERT_RETURN((ipseccb_id_ht_ != NULL), false);

    ipseccb_hal_handle_ht_ = ht::factory(HAL_MAX_IPSECCB,
                                         hal::ipseccb_get_handle_key_func,
                                         hal::ipseccb_compute_handle_hash_func,
                                         hal::ipseccb_compare_handle_key_func);
    HAL_ASSERT_RETURN((ipseccb_hal_handle_ht_ != NULL), false);
    
    // initialize CPU CB related data structures
    cpucb_id_ht_ = ht::factory(HAL_MAX_CPUCB,
                               hal::cpucb_get_key_func,
                               hal::cpucb_compute_hash_func,
                               hal::cpucb_compare_key_func);
    HAL_ASSERT_RETURN((cpucb_id_ht_ != NULL), false);

    cpucb_hal_handle_ht_ = ht::factory(HAL_MAX_CPUCB,
                                       hal::cpucb_get_handle_key_func,
                                       hal::cpucb_compute_handle_hash_func,
                                       hal::cpucb_compare_handle_key_func);
    HAL_ASSERT_RETURN((cpucb_hal_handle_ht_ != NULL), false);

    // initialize Raw Redirect CB related data structures
    rawrcb_id_ht_ = ht::factory(HAL_MAX_RAWRCB,
                                hal::rawrcb_get_key_func,
                                hal::rawrcb_compute_hash_func,
                                hal::rawrcb_compare_key_func);
    HAL_ASSERT_RETURN((rawrcb_id_ht_ != NULL), false);

    rawrcb_hal_handle_ht_ = ht::factory(HAL_MAX_RAWRCB,
                                        hal::rawrcb_get_handle_key_func,
                                        hal::rawrcb_compute_handle_hash_func,
                                        hal::rawrcb_compare_handle_key_func);
    HAL_ASSERT_RETURN((rawrcb_hal_handle_ht_ != NULL), false);

 
    // initialize Raw Chain CB related data structures
    rawccb_id_ht_ = ht::factory(HAL_MAX_RAWCCB,
                                hal::rawccb_get_key_func,
                                hal::rawccb_compute_hash_func,
                                hal::rawccb_compare_key_func);
    HAL_ASSERT_RETURN((rawccb_id_ht_ != NULL), false);

    rawccb_hal_handle_ht_ = ht::factory(HAL_MAX_RAWCCB,
                                        hal::rawccb_get_handle_key_func,
                                        hal::rawccb_compute_handle_hash_func,
                                        hal::rawccb_compare_handle_key_func);
    HAL_ASSERT_RETURN((rawccb_hal_handle_ht_ != NULL), false);

    nwsec_policy_cfg_ht_ = ht::factory(HAL_MAX_NW_SEC_POLICY_CFG,
                                       hal::nwsec_policy_cfg_get_key_func,
                                       hal::nwsec_policy_cfg_compute_hash_func,
                                       hal::nwsec_policy_cfg_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_policy_cfg_ht_ != NULL), false);

    nwsec_group_ht_ = ht::factory(HAL_MAX_NW_SEC_GROUP_CFG,
                                  hal::nwsec_group_get_key_func,
                                  hal::nwsec_group_compute_hash_func,
                                  hal::nwsec_group_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_policy_cfg_ht_ != NULL), false);

    // initialize arp learning related data structures
    arplearn_key_ht_ =
        ht::factory(HAL_MAX_ARP_TRANS, hal::network::arptrans_get_key_func,
                    hal::network::arptrans_compute_hash_func,
                    hal::network::arptrans_compare_key_func);
    HAL_ASSERT_RETURN((arplearn_key_ht_ != NULL), false);

    arplearn_ip_entry_ht_ =
            ht::factory(HAL_MAX_ARP_TRANS,
                        hal::network::arptrans_get_ip_entry_key_func,
                        hal::network::arptrans_compute_ip_entry_hash_func,
                        hal::network::arptrans_compare_ip_entry_key_func);
    HAL_ASSERT_RETURN((arplearn_ip_entry_ht_ != NULL), false);

    // initialize dhcp learning related data structures
    dhcplearn_key_ht_ =
        ht::factory(HAL_MAX_DHCP_TRANS, hal::network::dhcptrans_get_key_func,
                    hal::network::dhcptrans_compute_hash_func,
                    hal::network::dhcptrans_compare_key_func);
    HAL_ASSERT_RETURN((dhcplearn_key_ht_ != NULL), false);

    dhcplearn_ip_entry_ht_ =
            ht::factory(HAL_MAX_DHCP_TRANS,
                        hal::network::dhcptrans_get_ip_entry_key_func,
                        hal::network::dhcptrans_compute_ip_entry_hash_func,
                        hal::network::dhcptrans_compare_ip_entry_key_func);
    HAL_ASSERT_RETURN((dhcplearn_ip_entry_ht_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_cfg_db::hal_cfg_db()
{
    vrf_id_ht_ = NULL;

    network_key_ht_ = NULL;

    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;

    l2seg_id_ht_ = NULL;

    lif_id_ht_ = NULL;
    if_id_ht_ = NULL;
    port_id_ht_ = NULL;

    ep_hal_handle_ht_ = NULL;

    session_id_ht_ = NULL;
    session_hal_handle_ht_ = NULL;
    session_hal_iflow_ht_  = NULL;
    session_hal_rflow_ht_  = NULL;

    l4lb_ht_ = NULL;
    l4lb_hal_handle_ht_ = NULL;

    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;
    nwsec_policy_cfg_ht_  = NULL;
    nwsec_group_ht_       = NULL;

    buf_pool_id_ht_ = NULL;
    buf_pool_hal_handle_ht_ = NULL;

    queue_id_ht_ = NULL;
    queue_hal_handle_ht_ = NULL;

    ingress_policer_id_ht_ = NULL;
    ingress_policer_hal_handle_ht_ = NULL;
    egress_policer_id_ht_ = NULL;
    egress_policer_hal_handle_ht_ = NULL;

    acl_id_ht_ = NULL;
    acl_hal_handle_ht_ = NULL;
 
    tlscb_id_ht_ = NULL;
    tlscb_hal_handle_ht_ = NULL;
 
    tcpcb_id_ht_ = NULL;
    tcpcb_hal_handle_ht_ = NULL;
    
    wring_id_ht_ = NULL;
    wring_hal_handle_ht_ = NULL;
    
    proxy_slab_ = NULL;
    proxy_flow_info_slab_ = NULL;
    proxy_type_ht_ = NULL;
    proxy_hal_handle_ht_ = NULL;

    ipseccb_id_ht_ = NULL;
    ipseccb_hal_handle_ht_ = NULL;
    
    cpucb_id_ht_ = NULL;
    cpucb_hal_handle_ht_ = NULL;

    rawrcb_id_ht_ = NULL;
    rawrcb_hal_handle_ht_ = NULL;

    rawccb_id_ht_ = NULL;
    rawccb_hal_handle_ht_ = NULL;

    forwarding_mode_ = HAL_FORWARDING_MODE_NONE;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_cfg_db *
hal_cfg_db::factory(void)
{
    void          *mem;
    hal_cfg_db    *cfg_db;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_cfg_db));
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    cfg_db = new(mem) hal_cfg_db();
    if (cfg_db->init() == false) {
        cfg_db->~hal_cfg_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
        return NULL;
    }

    return cfg_db;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_cfg_db::~hal_cfg_db()
{
    vrf_id_ht_ ? delete vrf_id_ht_ : HAL_NOP;

    network_key_ht_ ? delete network_key_ht_ : HAL_NOP;

    nwsec_profile_id_ht_ ? delete nwsec_profile_id_ht_ : HAL_NOP;
    nwsec_profile_hal_handle_ht_ ? delete nwsec_profile_hal_handle_ht_ : HAL_NOP;

    nwsec_policy_cfg_ht_ ? delete nwsec_policy_cfg_ht_ : HAL_NOP;
    nwsec_group_ht_ ? delete nwsec_group_ht_ : HAL_NOP;

    l2seg_id_ht_ ? delete l2seg_id_ht_ : HAL_NOP;

    lif_id_ht_ ? delete lif_id_ht_ : HAL_NOP;
    if_id_ht_ ? delete if_id_ht_ : HAL_NOP;
    port_id_ht_ ? delete port_id_ht_ : HAL_NOP;

    ep_hal_handle_ht_ ? delete ep_hal_handle_ht_ : HAL_NOP;

    session_id_ht_ ? delete session_id_ht_ : HAL_NOP;
    session_hal_handle_ht_ ? delete session_hal_handle_ht_ : HAL_NOP;
    session_hal_iflow_ht_ ? delete session_hal_iflow_ht_ : HAL_NOP;
    session_hal_rflow_ht_ ? delete session_hal_rflow_ht_ : HAL_NOP;

    l4lb_ht_ ? delete l4lb_ht_ : HAL_NOP;
    l4lb_hal_handle_ht_ ? delete l4lb_hal_handle_ht_ : HAL_NOP;

    buf_pool_id_ht_ ? delete buf_pool_id_ht_ : HAL_NOP;
    buf_pool_hal_handle_ht_ ? delete buf_pool_hal_handle_ht_ : HAL_NOP;

    tlscb_id_ht_ ? delete tlscb_id_ht_ : HAL_NOP;
    tlscb_hal_handle_ht_ ? delete tlscb_hal_handle_ht_ : HAL_NOP;
 
    tcpcb_id_ht_ ? delete tcpcb_id_ht_ : HAL_NOP;
    tcpcb_hal_handle_ht_ ? delete tcpcb_hal_handle_ht_ : HAL_NOP;

    queue_id_ht_ ? delete queue_id_ht_ : HAL_NOP;
    queue_hal_handle_ht_ ? delete queue_hal_handle_ht_ : HAL_NOP;

    ingress_policer_id_ht_ ? delete ingress_policer_id_ht_ : HAL_NOP;
    ingress_policer_hal_handle_ht_ ? delete ingress_policer_hal_handle_ht_ : HAL_NOP;
    egress_policer_id_ht_ ? delete egress_policer_id_ht_ : HAL_NOP;
    egress_policer_hal_handle_ht_ ? delete egress_policer_hal_handle_ht_ : HAL_NOP;

    wring_id_ht_ ? delete wring_id_ht_ : HAL_NOP;
    wring_hal_handle_ht_ ? delete wring_hal_handle_ht_ : HAL_NOP;
    
    proxy_slab_ ? delete proxy_slab_ : HAL_NOP;
    proxy_flow_info_slab_ ? delete proxy_flow_info_slab_ : HAL_NOP;
    proxy_type_ht_ ? delete proxy_type_ht_ : HAL_NOP;
    proxy_hal_handle_ht_ ? delete proxy_hal_handle_ht_ : HAL_NOP;
    
    acl_id_ht_ ? delete acl_id_ht_ : HAL_NOP;
    acl_hal_handle_ht_ ? delete acl_hal_handle_ht_ : HAL_NOP;

    ipseccb_id_ht_ ? delete ipseccb_id_ht_ : HAL_NOP;
    ipseccb_hal_handle_ht_ ? delete ipseccb_hal_handle_ht_ : HAL_NOP;

    cpucb_id_ht_ ? delete cpucb_id_ht_ : HAL_NOP;
    cpucb_hal_handle_ht_ ? delete cpucb_hal_handle_ht_ : HAL_NOP;

    rawrcb_id_ht_ ? delete rawrcb_id_ht_ : HAL_NOP;
    rawrcb_hal_handle_ht_ ? delete rawrcb_hal_handle_ht_ : HAL_NOP;

    rawccb_id_ht_ ? delete rawccb_id_ht_ : HAL_NOP;
    rawccb_hal_handle_ht_ ? delete rawccb_hal_handle_ht_ : HAL_NOP;
}

void
hal_cfg_db::set_forwarding_mode(std::string modestr)
{
    if (modestr == "default") {
        forwarding_mode_ = HAL_FORWARDING_MODE_DEFAULT;
    } else if (modestr == "host-pinned") {
        forwarding_mode_ = HAL_FORWARDING_MODE_HOST_PINNED;
    } else if (modestr == "classic") {
        forwarding_mode_ = HAL_FORWARDING_MODE_CLASSIC;
    } else {
        HAL_ASSERT(FALSE);
    }
    return;
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
    obj_meta_[obj_id].obj_sz = obj_sz;
    return HAL_RET_OK;
}

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

//------------------------------------------------------------------------------
// init() function to instantiate all the oper db init state
//------------------------------------------------------------------------------
bool
hal_oper_db::init(void)
{
    hal_handle_id_ht_ = ht::factory(HAL_MAX_HANDLES,
                                    hal::hal_handle_id_get_key_func,
                                    hal::hal_handle_id_compute_hash_func,
                                    hal::hal_handle_id_compare_key_func);
    HAL_ASSERT_RETURN((hal_handle_id_ht_ != NULL), false);

    ep_l2_ht_ = ht::factory(HAL_MAX_ENDPOINTS,
                            hal::ep_get_l2_key_func,
                            hal::ep_compute_l2_hash_func,
                            hal::ep_compare_l2_key_func);
    HAL_ASSERT_RETURN((ep_l2_ht_ != NULL), false);

    ep_l3_entry_ht_ = ht::factory(HAL_MAX_ENDPOINTS << 1,
                                  hal::ep_get_l3_key_func,
                                  hal::ep_compute_l3_hash_func,
                                  hal::ep_compare_l3_key_func);
    HAL_ASSERT_RETURN((ep_l3_entry_ht_ != NULL), false);

    flow_ht_ = ht::factory(HAL_MAX_FLOWS,
                           hal::flow_get_key_func,
                           hal::flow_compute_hash_func,
                           hal::flow_compare_key_func);
    HAL_ASSERT_RETURN((flow_ht_ != NULL), false);

    for (uint32_t port = 0; port < HAL_ARRAY_SIZE(cos_in_use_bmp_); port++) {
        cos_in_use_bmp_[port] = bitmap::factory(HAL_MAX_COSES,
                                                true);
        HAL_ASSERT_RETURN((cos_in_use_bmp_[port] != NULL), false);
    }

    event_mgr_ = eventmgr::factory(HAL_MAX_EVENTS);
    HAL_ASSERT_RETURN((event_mgr_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_oper_db::hal_oper_db()
{
    uint32_t    i;

    hal_handle_id_ht_  = NULL;
    infra_l2seg_ = NULL;
    ep_l2_ht_ = NULL;
    ep_l3_entry_ht_ = NULL;
    flow_ht_ = NULL;
    for (i = 0; i < HAL_ARRAY_SIZE(cos_in_use_bmp_); i++) {
        cos_in_use_bmp_[i] = NULL;
    }
    event_mgr_ = NULL;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_oper_db *
hal_oper_db::factory(void)
{
    void           *mem;
    hal_oper_db    *oper_db;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_oper_db));
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    oper_db = new(mem) hal_oper_db();
    if (oper_db->init() == false) {
        oper_db->~hal_oper_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
        return NULL;
    }

    return oper_db;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_oper_db::~hal_oper_db()
{
    uint32_t    i;

    hal_handle_id_ht_ ? delete hal_handle_id_ht_ : HAL_NOP;

    infra_l2seg_ = NULL;

    // if_hwid_ht_ ? delete if_hwid_ht_ : HAL_NOP;

    ep_l2_ht_ ? delete ep_l2_ht_ : HAL_NOP;
    ep_l3_entry_ht_ ? delete ep_l3_entry_ht_ : HAL_NOP;

    flow_ht_ ? delete flow_ht_ : HAL_NOP;

    for (i = 0; i < HAL_ARRAY_SIZE(cos_in_use_bmp_); i++) {
        if (cos_in_use_bmp_[i]) {
            delete cos_in_use_bmp_[i];
        }
    }

    event_mgr_ ? delete event_mgr_ : HAL_NOP;
}

//------------------------------------------------------------------------------
// init() function to instantiate all the mem db init state
//------------------------------------------------------------------------------
bool
hal_mem_db::init(void)
{
    // initialize slab for HAL handles
    hal_handle_slab_ = slab::factory("hal-handle",
                                     HAL_SLAB_HANDLE, sizeof(hal_handle),
                                     64, true, true, true, true);
    HAL_ASSERT_RETURN((hal_handle_slab_ != NULL), false);

    hal_handle_ht_entry_slab_ = slab::factory("hal-handle-ht-entry",
                                              HAL_SLAB_HANDLE_HT_ENTRY,
                                              sizeof(hal_handle_ht_entry_t),
                                              64, true, true, true, true);
    HAL_ASSERT_RETURN((hal_handle_ht_entry_slab_ != NULL), false);

    hal_handle_list_entry_slab_ = slab::factory("hal-handle-list-entry",
                                                HAL_SLAB_HANDLE_LIST_ENTRY,
                                                sizeof(hal_handle_list_entry_t),
                                                64, true, true, true, true);
    HAL_ASSERT_RETURN((hal_handle_list_entry_slab_ != NULL), false);

	hal_handle_id_ht_entry_slab_ = slab::factory("hal-handle-id-ht-entry",
                                                 HAL_SLAB_HANDLE_ID_HT_ENTRY,
                                                 sizeof(hal_handle_id_ht_entry_t),
                                                 64, true, true, true, true);
    HAL_ASSERT_RETURN((hal_handle_id_ht_entry_slab_ != NULL), false);

    hal_handle_id_list_entry_slab_ = slab::factory("hal-handle-id-list-entry",
                                                   HAL_SLAB_HANDLE_ID_LIST_ENTRY,
                                                   sizeof(hal_handle_id_list_entry_t),
                                                   64, true, true, true, true);
    HAL_ASSERT_RETURN((hal_handle_id_list_entry_slab_ != NULL), false);

	// initialize vrf related data structures
	vrf_slab_ = slab::factory("vrf", HAL_SLAB_VRF,
                                 sizeof(hal::vrf_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((vrf_slab_ != NULL), false);

    // initialize network related data structures
    network_slab_ = slab::factory("network", HAL_SLAB_NETWORK,
                                 sizeof(hal::network_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((network_slab_ != NULL), false);

    // initialize security profile related data structures
    nwsec_profile_slab_ = slab::factory("nwsec-profile",
                                        HAL_SLAB_SECURITY_PROFILE,
                                        sizeof(hal::nwsec_profile_t), 8,
                                        false, true, true, true);
    HAL_ASSERT_RETURN((nwsec_profile_slab_ != NULL), false);
    
    // initialize dos policy related data structures
    dos_policy_slab_ = slab::factory("dos-policy",
                                      HAL_SLAB_DOS_POLICY,
                                      sizeof(hal::dos_policy_t), 8,
                                      false, true, true, true);
    HAL_ASSERT_RETURN((dos_policy_slab_ != NULL), false);

    // initialize dos policy sg list related data structures
    dos_policy_sg_list_entry_slab_ = slab::factory("dos-policy-sg-list",
                                      HAL_SLAB_DOS_POLICY_SG_LIST,
                                      sizeof(hal::dos_policy_sg_list_entry_t), 8,
                                      false, true, true, true);
    HAL_ASSERT_RETURN((dos_policy_sg_list_entry_slab_ != NULL), false);

    // initialize L2 segment related data structures
    l2seg_slab_ = slab::factory("l2seg", HAL_SLAB_L2SEG,
                                sizeof(hal::l2seg_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((l2seg_slab_ != NULL), false);

    // initialize LIF related data structures
    lif_slab_ = slab::factory("LIF", HAL_SLAB_LIF,
                              sizeof(hal::lif_t), 8,
                             false, true, true, true);
    HAL_ASSERT_RETURN((lif_slab_ != NULL), false);

    // initialize interface related data structures
    if_slab_ = slab::factory("interface", HAL_SLAB_IF,
                             sizeof(hal::if_t), 16,
                             false, true, true, true);
    HAL_ASSERT_RETURN((if_slab_ != NULL), false);

    // initialize enic l2seg entry related data structures
    enic_l2seg_entry_slab_ = slab::factory("interface", HAL_SLAB_IF,
                             sizeof(hal::if_l2seg_entry_t), 16,
                             false, true, true, true);
    HAL_ASSERT_RETURN((enic_l2seg_entry_slab_ != NULL), false);

    // initialize port related data structures
    port_slab_ = slab::factory("port", HAL_SLAB_PORT,
                               sizeof(hal::port_t), 8,
                               false, true, true, true);
    HAL_ASSERT_RETURN((port_slab_ != NULL), false);

    // initialize endpoint related data structures
    ep_slab_ = slab::factory("EP", HAL_SLAB_EP, sizeof(hal::ep_t), 128,
                             true, true, true, true);
    HAL_ASSERT_RETURN((ep_slab_ != NULL), false);

    ep_ip_entry_slab_ = slab::factory("EP IP entry", HAL_SLAB_EP_IP_ENTRY,
                                      sizeof(hal::ep_ip_entry_t),
                                      64, true, true, true, true);
    HAL_ASSERT_RETURN((ep_ip_entry_slab_ != NULL), false);

    ep_l3_entry_slab_ = slab::factory("EP L3 entry", HAL_SLAB_EP_L3_ENTRY,
                                      sizeof(hal::ep_l3_entry_t),
                                      64, true, true, true, true);
    HAL_ASSERT_RETURN((ep_l3_entry_slab_ != NULL), false);

    // initialize flow/session related data structures
    flow_slab_ = slab::factory("Flow", HAL_SLAB_FLOW, sizeof(hal::flow_t), 128,
                               true, true, true, true);
    HAL_ASSERT_RETURN((flow_slab_ != NULL), false);

    session_slab_ = slab::factory("Session", HAL_SLAB_SESSION,
                                  sizeof(hal::session_t), 128,
                                  true, true, true, true);
    HAL_ASSERT_RETURN((session_slab_ != NULL), false);

    // initialize l4lb related data structures
    l4lb_slab_ = slab::factory("L4LB", HAL_SLAB_L4LB,
                               sizeof(hal::l4lb_service_entry_t), 16,
                               true, true, true, true);
    HAL_ASSERT_RETURN((l4lb_slab_ != NULL), false);

    // initialize TLS CB related data structures
    tlscb_slab_ = slab::factory("tlscb", HAL_SLAB_TLSCB,
                                sizeof(hal::tlscb_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((tlscb_slab_ != NULL), false);

    // initialize TCB CB related data structures
    tcpcb_slab_ = slab::factory("tcpcb", HAL_SLAB_TCPCB,
                                sizeof(hal::tcpcb_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((tcpcb_slab_ != NULL), false);

    // initialize Buf-Pool related data structures
    buf_pool_slab_ = slab::factory("BufPool", HAL_SLAB_BUF_POOL,
                                   sizeof(hal::buf_pool_t), 8,
                                   false, true, true, true);
    HAL_ASSERT_RETURN((buf_pool_slab_ != NULL), false);

    // initialize Queue related data structures
    queue_slab_ = slab::factory("Queue", HAL_SLAB_QUEUE,
                                sizeof(hal::queue_t), 8,
                                false, true, true, true);
    HAL_ASSERT_RETURN((queue_slab_ != NULL), false);

    // initialize Policer related data structures
    policer_slab_ = slab::factory("Policer", HAL_SLAB_POLICER,
                                  sizeof(hal::policer_t), 8,
                                  false, true, true, true);
    HAL_ASSERT_RETURN((policer_slab_ != NULL), false);

    // initialize WRing related data structures
    wring_slab_ = slab::factory("wring", HAL_SLAB_WRING,
                                sizeof(hal::wring_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((wring_slab_ != NULL), false);

    // initialize Acl related data structures
    acl_slab_ = slab::factory("Acl", HAL_SLAB_ACL,
                              sizeof(hal::acl_t), 8,
                              false, true, true, true);
    HAL_ASSERT_RETURN((acl_slab_ != NULL), false);

    // initialize IPSEC CB related data structures
    ipseccb_slab_ = slab::factory("ipseccb", HAL_SLAB_IPSECCB,
                                  sizeof(hal::ipseccb_t), 16,
                                  false, true, true, true);
    HAL_ASSERT_RETURN((ipseccb_slab_ != NULL), false);

    // initialize CPU CB related data structures
    cpucb_slab_ = slab::factory("cpucb", HAL_SLAB_CPUCB,
                                sizeof(hal::cpucb_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((cpucb_slab_ != NULL), false);

    // initialize Raw Redirect CB related data structures
    rawrcb_slab_ = slab::factory("rawrcb", HAL_SLAB_RAWRCB,
                                 sizeof(hal::rawrcb_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((rawrcb_slab_ != NULL), false);

    // initialize Raw Chain CB related data structures
    rawccb_slab_ = slab::factory("rawccb", HAL_SLAB_RAWCCB,
                                 sizeof(hal::rawccb_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((rawccb_slab_ != NULL), false);

    nwsec_policy_rules_slab_ = slab::factory("nwsec_policy_rules", HAL_SLAB_NWSEC_POLICY_RULES,
                                             sizeof(hal::nwsec_policy_rules_t), 64,
                                             true, true, true, true);
    HAL_ASSERT_RETURN((nwsec_policy_rules_slab_ != NULL), false);

    nwsec_policy_cfg_slab_ = slab::factory("nwsec_policy_cfg", HAL_SLAB_NWSEC_POLICY_CFG,
                                           sizeof(hal::nwsec_policy_cfg_t), 64,
                                           true, true, true, true);
    HAL_ASSERT_RETURN((nwsec_policy_cfg_slab_ != NULL), false);

    nwsec_policy_svc_slab_ = slab::factory("nwsec_policy_svc", HAL_SLAB_NWSEC_POLICY_SVC,
                                           sizeof(hal::nwsec_policy_svc_t), 64,
                                           true, true, true, true);
    HAL_ASSERT_RETURN((nwsec_policy_svc_slab_ != NULL), false);

    nwsec_group_slab_ = slab::factory("nwsec_group", HAL_SLAB_NWSEC_GROUP,
                                       sizeof(hal::nwsec_group_t), 64,
                                       true, true, true, true);
    HAL_ASSERT_RETURN((nwsec_group_slab_ != NULL), false);

    
    arplearn_slab_ = slab::factory("arpLearn", HAL_SLAB_ARP_LEARN,
                                   sizeof(hal::network::arp_trans_t), 16, false,
                                   true, true, true);
    HAL_ASSERT_RETURN((arplearn_slab_ != NULL), false);

    dhcplearn_slab_ = slab::factory("dhcpLearn", HAL_SLAB_DHCP_LEARN,
                                    sizeof(hal::network::dhcp_trans_t), 16,
                                    false, true, true, true);
    HAL_ASSERT_RETURN((dhcplearn_slab_ != NULL), false);
    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_mem_db::hal_mem_db()
{
    hal_handle_slab_ = NULL;
    hal_handle_ht_entry_slab_ = NULL;
    hal_handle_list_entry_slab_ = NULL;
    hal_handle_id_ht_entry_slab_ = NULL;
    hal_handle_id_list_entry_slab_ = NULL;
    //hal_del_cache_entry_slab_ = NULL;
    vrf_slab_ = NULL;
    network_slab_ = NULL;
    nwsec_profile_slab_ = NULL;
    dos_policy_slab_ = NULL;
    dos_policy_sg_list_entry_slab_ = NULL;
    nwsec_group_slab_  = NULL;
    nwsec_policy_rules_slab_ = NULL;
    nwsec_policy_cfg_slab_ = NULL;
    nwsec_policy_svc_slab_ = NULL;
    l2seg_slab_ = NULL;
    lif_slab_ = NULL;
    if_slab_ = NULL;
    enic_l2seg_entry_slab_ = NULL;
    port_slab_ = NULL;
    ep_slab_ = NULL;
    ep_ip_entry_slab_ = NULL;
    ep_l3_entry_slab_ = NULL;
    flow_slab_ = NULL;
    session_slab_ = NULL;
    l4lb_slab_ = NULL;
    tlscb_slab_ = NULL;
    tcpcb_slab_ = NULL;
    buf_pool_slab_ = NULL;
    queue_slab_ = NULL;
    policer_slab_ = NULL;
    wring_slab_ = NULL;
    acl_slab_ = NULL;
    ipseccb_slab_ = NULL;
    cpucb_slab_ = NULL;
    rawrcb_slab_ = NULL;
    rawccb_slab_ = NULL;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_mem_db *
hal_mem_db::factory(void)
{
    void          *mem;
    hal_mem_db    *mem_db;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_mem_db));
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    mem_db = new(mem) hal_mem_db();
    if (mem_db->init() == false) {
        mem_db->~hal_mem_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
        return NULL;
    }

    return mem_db;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_mem_db::~hal_mem_db()
{
    hal_handle_slab_ ? delete hal_handle_slab_ : HAL_NOP;
    hal_handle_ht_entry_slab_ ? delete hal_handle_ht_entry_slab_ : HAL_NOP;
    hal_handle_list_entry_slab_ ? delete hal_handle_list_entry_slab_ : HAL_NOP;
    hal_handle_id_ht_entry_slab_ ? delete hal_handle_id_ht_entry_slab_ : HAL_NOP;
    hal_handle_id_list_entry_slab_ ? delete hal_handle_id_list_entry_slab_ : HAL_NOP;
    vrf_slab_ ? delete vrf_slab_ : HAL_NOP;
    network_slab_ ? delete network_slab_ : HAL_NOP;
    nwsec_profile_slab_ ? delete nwsec_profile_slab_ : HAL_NOP;
    dos_policy_slab_ ? delete dos_policy_slab_ : HAL_NOP;
    dos_policy_sg_list_entry_slab_ ? delete dos_policy_sg_list_entry_slab_ : HAL_NOP;
    nwsec_group_slab_ ? delete nwsec_group_slab_ : HAL_NOP;
    nwsec_policy_rules_slab_ ? delete nwsec_policy_rules_slab_ : HAL_NOP;
    nwsec_policy_cfg_slab_ ? delete nwsec_policy_cfg_slab_ : HAL_NOP;
    nwsec_policy_svc_slab_ ? delete nwsec_policy_svc_slab_ : HAL_NOP;
    l2seg_slab_ ? delete l2seg_slab_ : HAL_NOP;
    lif_slab_ ? delete lif_slab_ : HAL_NOP;
    if_slab_ ? delete if_slab_ : HAL_NOP;
    enic_l2seg_entry_slab_ ? delete enic_l2seg_entry_slab_ : HAL_NOP;
    port_slab_ ? delete port_slab_ : HAL_NOP;
    ep_slab_ ? delete ep_slab_ : HAL_NOP;
    ep_ip_entry_slab_ ? delete ep_ip_entry_slab_ : HAL_NOP;
    ep_l3_entry_slab_ ? delete ep_l3_entry_slab_ : HAL_NOP;
    flow_slab_ ? delete flow_slab_ : HAL_NOP;
    session_slab_ ? delete session_slab_ : HAL_NOP;
    l4lb_slab_ ? delete l4lb_slab_ : HAL_NOP;
    tlscb_slab_ ? delete tlscb_slab_ : HAL_NOP;
    tcpcb_slab_ ? delete tcpcb_slab_ : HAL_NOP;
    buf_pool_slab_ ? delete buf_pool_slab_ : HAL_NOP;
    queue_slab_ ? delete queue_slab_ : HAL_NOP;
    policer_slab_ ? delete policer_slab_ : HAL_NOP;
    wring_slab_ ? delete wring_slab_ : HAL_NOP;
    acl_slab_ ? delete acl_slab_ : HAL_NOP;
    ipseccb_slab_ ? delete ipseccb_slab_ : HAL_NOP;
    cpucb_slab_ ? delete cpucb_slab_ : HAL_NOP;
    rawrcb_slab_ ? delete rawrcb_slab_ : HAL_NOP;
    rawrcb_slab_ = NULL;
    rawccb_slab_ ? delete rawccb_slab_ : HAL_NOP;
    rawccb_slab_ = NULL;
}

//----------------------------------------------------------------------------
// gives the slab of a slab id
//----------------------------------------------------------------------------
#define GET_SLAB(slab_name)                                 \
    if (slab_name && slab_name->get_slab_id() == slab_id) { \
        return slab_name;                                   \
    }

slab *
hal_mem_db::get_slab(hal_slab_t slab_id)
{
    GET_SLAB(hal_handle_slab_);
    GET_SLAB(hal_handle_ht_entry_slab_);
    GET_SLAB(hal_handle_list_entry_slab_);
    GET_SLAB(hal_handle_id_ht_entry_slab_);
    GET_SLAB(hal_handle_id_list_entry_slab_);
    GET_SLAB(vrf_slab_);
    GET_SLAB(network_slab_);
    GET_SLAB(dos_policy_slab_);
    GET_SLAB(dos_policy_sg_list_entry_slab_);
    GET_SLAB(nwsec_profile_slab_);
    GET_SLAB(nwsec_group_slab_);
    GET_SLAB(nwsec_policy_rules_slab_);
    GET_SLAB(nwsec_policy_cfg_slab_);
    GET_SLAB(nwsec_policy_svc_slab_);
    GET_SLAB(l2seg_slab_);
    GET_SLAB(lif_slab_);
    GET_SLAB(if_slab_);
    GET_SLAB(port_slab_);
    GET_SLAB(ep_slab_);
    GET_SLAB(ep_ip_entry_slab_);
    GET_SLAB(ep_l3_entry_slab_);
    GET_SLAB(flow_slab_);
    GET_SLAB(session_slab_);
    GET_SLAB(l4lb_slab_);
    GET_SLAB(tlscb_slab_);
    GET_SLAB(tcpcb_slab_);
    GET_SLAB(buf_pool_slab_);
    GET_SLAB(queue_slab_);
    GET_SLAB(policer_slab_);
    GET_SLAB(wring_slab_);
    GET_SLAB(acl_slab_);
    GET_SLAB(ipseccb_slab_);
    GET_SLAB(cpucb_slab_);

    return NULL;
}

//------------------------------------------------------------------------------
// init() function to instantiate all HAL init state
//------------------------------------------------------------------------------
bool
hal_state::init(void)
{
    cfg_db_ = hal_cfg_db::factory();
    oper_db_ = hal_oper_db::factory();
    mem_db_ = hal_mem_db::factory();

    HAL_ASSERT_GOTO((cfg_db_ && oper_db_ && mem_db_), cleanup);
    return true;

cleanup:

    if (cfg_db_) {
        cfg_db_->~hal_cfg_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, cfg_db_);
    }

    if (oper_db_) {
        oper_db_->~hal_oper_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, oper_db_);
    }

    if (mem_db_) {
        mem_db_->~hal_mem_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem_db_);
    }
    return false;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_state::hal_state()
{
    cfg_db_ = NULL;
    oper_db_ = NULL;
    mem_db_ = NULL;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_state *
hal_state::factory(void)
{
    void         *mem;
    hal_state    *state;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_state));
    HAL_ABORT(mem != NULL);
    state = new (mem) hal_state();
    if (state->init() == false) {
        state->~hal_state();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
	    return NULL;
    }

    return state;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state::~hal_state()
{
    if (cfg_db_) {
        cfg_db_->~hal_cfg_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, cfg_db_);
    }

    if (oper_db_) {
        oper_db_->~hal_oper_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, oper_db_);
    }

    if (mem_db_) {
        mem_db_->~hal_mem_db();
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem_db_);
    }
}

//------------------------------------------------------------------------------
// one time memory related initialization for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_mem_init (void)
{
    g_hal_state = hal_state::factory();
    HAL_ASSERT_RETURN((g_hal_state != NULL), HAL_RET_ERR);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Save hal_cfg parsed from JSON to hal_state.
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_init (hal_cfg_t *hal_cfg)
{
    HAL_TRACE_INFO("{}: Setting forwarding_mode to {}", __FUNCTION__,
                   hal_cfg->forwarding_mode);
    g_hal_state->set_forwarding_mode(hal_cfg->forwarding_mode);
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
        g_hal_state->hal_handle_slab()->free_(elem);
        break;

    case HAL_SLAB_HANDLE_HT_ENTRY:
        g_hal_state->hal_handle_ht_entry_slab()->free_(elem);
        break;

    case HAL_SLAB_HANDLE_LIST_ENTRY:
        g_hal_state->hal_handle_list_entry_slab()->free_(elem);
        break;

    case HAL_SLAB_HANDLE_ID_HT_ENTRY:
        g_hal_state->hal_handle_id_ht_entry_slab()->free_(elem);
        break;

    case HAL_SLAB_HANDLE_ID_LIST_ENTRY:
        g_hal_state->hal_handle_id_list_entry_slab()->free_(elem);
        break;

    case HAL_SLAB_VRF:
        g_hal_state->vrf_slab()->free_(elem);
        break;

    case HAL_SLAB_NETWORK:
        g_hal_state->network_slab()->free_(elem);
        break;

    case HAL_SLAB_L2SEG:
        g_hal_state->l2seg_slab()->free_(elem);
        break;

    case HAL_SLAB_LIF:
        g_hal_state->lif_slab()->free_(elem);
        break;

    case HAL_SLAB_IF:
        g_hal_state->if_slab()->free_(elem);
        break;

    case HAL_SLAB_ENIC_L2SEG_ENTRY:
        g_hal_state->enic_l2seg_entry_slab()->free(elem);
        break;

    case HAL_SLAB_PORT:
        g_hal_state->port_slab()->free_(elem);
        break;

    case HAL_SLAB_EP:
        g_hal_state->ep_slab()->free_(elem);
        break;

    case HAL_SLAB_EP_IP_ENTRY:
        g_hal_state->ep_ip_entry_slab()->free_(elem);
        break;

    case HAL_SLAB_EP_L3_ENTRY:
        g_hal_state->ep_l3_entry_slab()->free_(elem);
        break;

    case HAL_SLAB_FLOW:
        g_hal_state->flow_slab()->free_(elem);
        break;

    case HAL_SLAB_SESSION:
        g_hal_state->session_slab()->free_(elem);
        break;

    case HAL_SLAB_SECURITY_PROFILE:
        g_hal_state->nwsec_profile_slab()->free_(elem);
        break;

    case HAL_SLAB_NWSEC_GROUP:
        g_hal_state->nwsec_group_slab()->free_(elem);
        break;
    
    case HAL_SLAB_NWSEC_POLICY_CFG:
        g_hal_state->nwsec_policy_cfg_slab()->free_(elem);
        break;

    case HAL_SLAB_NWSEC_POLICY_RULES:
        g_hal_state->nwsec_policy_rules_slab()->free_(elem);
        break;

    case HAL_SLAB_NWSEC_POLICY_SVC:
        g_hal_state->nwsec_policy_svc_slab()->free_(elem);
        break;

    case HAL_SLAB_TLSCB:
        g_hal_state->tlscb_slab()->free_(elem);
        break;

    case HAL_SLAB_TCPCB:
        g_hal_state->tcpcb_slab()->free_(elem);
        break;

    case HAL_SLAB_BUF_POOL:
        g_hal_state->buf_pool_slab()->free_(elem);
        break;

    case HAL_SLAB_QUEUE:
        g_hal_state->queue_slab()->free_(elem);
        break;

    case HAL_SLAB_POLICER:
        g_hal_state->policer_slab()->free_(elem);
        break;

    case HAL_SLAB_ACL:
        g_hal_state->acl_slab()->free_(elem);
        break;

    case HAL_SLAB_WRING:
        g_hal_state->wring_slab()->free_(elem);
        break;

    case HAL_SLAB_PROXY:
        g_hal_state->proxy_slab()->free_(elem);
        break;

    case HAL_SLAB_PROXY_FLOW_INFO:
        g_hal_state->proxy_flow_info_slab()->free_(elem);
        break;

    case HAL_SLAB_IPSECCB:
        g_hal_state->ipseccb_slab()->free_(elem);
    
    case HAL_SLAB_CPUCB:
        g_hal_state->cpucb_slab()->free_(elem);
        break;

    case HAL_SLAB_RAWRCB:
        g_hal_state->rawrcb_slab()->free_(elem);
        break;

    case HAL_SLAB_RAWCCB:
        g_hal_state->rawccb_slab()->free_(elem);
        break;

    case HAL_SLAB_DHCP_LEARN:
        g_hal_state->dhcplearn_slab()->free_(elem);
        break;

    case HAL_SLAB_ARP_LEARN:
        g_hal_state->arplearn_slab()->free_(elem);
        break;

    case HAL_SLAB_EVENT_MAP:
        g_hal_state->event_mgr()->event_map_slab()->free_(elem);
        break;

    case HAL_SLAB_EVENT_MAP_LISTENER:
        g_hal_state->event_mgr()->event_listener_slab()->free_(elem);
        break;

    case HAL_SLAB_EVENT_LISTENER:
        g_hal_state->event_mgr()->listener_slab()->free_(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        HAL_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
        break;
    }

    return HAL_RET_OK;
}

}    // namespace hal
