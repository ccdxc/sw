// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <hal.hpp>
#include <hal_state.hpp>
#include <tenant.hpp>
#include <network.hpp>
#include <l2segment.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <l4lb.hpp>
#include <session.hpp>
#include <nwsec.hpp>
#include <tlscb.hpp>
#include <tcpcb.hpp>
#include <qos.hpp>
#include <acl.hpp>
#include <wring.hpp>
#include <proxy.hpp>
#include <ipseccb.hpp>
#include <cpucb.hpp>
 
namespace hal {

extern thread_local thread *t_curr_thread;

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
// HAL config db APIs store some context in the cfg_db_ctxt_t
// NOTE: this context is per thread, not for the whole process
//------------------------------------------------------------------------------
typedef struct cfg_db_ctxt_s {
    bool                   cfg_db_open_;    // true if cfg db is opened
    cfg_op_t               cfg_op_;         // cfg operation for which db is opened
    cfg_version_t          ver_in_use_;     // version we are starting the operation with
    cfg_version_t          rsvd_ver_;       // version to commit db modifications with
    cfg_db_dirty_objs_t    *dirty_objs;     // dirty object list to be committed
} cfg_db_ctxt_t;
thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

//------------------------------------------------------------------------------
// init() function to instantiate all the config db init state
//------------------------------------------------------------------------------
bool
hal_cfg_db::init(void)
{
    HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);

    // initialize tenant related data structures
    tenant_id_ht_ = ht::factory(HAL_MAX_VRFS,
                                hal::tenant_get_key_func,
                                hal::tenant_compute_hash_func,
                                hal::tenant_compare_key_func);
    HAL_ASSERT_RETURN((tenant_id_ht_ != NULL), false);

    tenant_hal_handle_ht_ = ht::factory(HAL_MAX_VRFS,
                                        hal::tenant_get_handle_key_func,
                                        hal::tenant_compute_handle_hash_func,
                                        hal::tenant_compare_handle_key_func);
    HAL_ASSERT_RETURN((tenant_hal_handle_ht_ != NULL), false);

    // initialize network related data structures
    network_key_ht_ = ht::factory(HAL_MAX_VRFS,
                                  hal::network_get_key_func,
                                  hal::network_compute_hash_func,
                                  hal::network_compare_key_func);
    HAL_ASSERT_RETURN((network_key_ht_ != NULL), false);

    network_hal_handle_ht_ = ht::factory(HAL_MAX_VRFS,
                                         hal::network_get_handle_key_func,
                                         hal::network_compute_handle_hash_func,
                                         hal::network_compare_handle_key_func);
    HAL_ASSERT_RETURN((network_hal_handle_ht_ != NULL), false);

    // initialize security profile related data structures
    nwsec_profile_id_ht_ = ht::factory(HAL_MAX_NWSEC_PROFILES,
                                       hal::nwsec_profile_get_key_func,
                                       hal::nwsec_profile_compute_hash_func,
                                       hal::nwsec_profile_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_profile_id_ht_ != NULL), false);

    nwsec_profile_hal_handle_ht_ =
        ht::factory(HAL_MAX_NWSEC_PROFILES,
                    hal::nwsec_profile_get_handle_key_func,
                    hal::nwsec_profile_compute_handle_hash_func,
                    hal::nwsec_profile_compare_handle_key_func);
    HAL_ASSERT_RETURN((nwsec_profile_hal_handle_ht_ != NULL), false);

    // initialize L2 segment related data structures
    l2seg_id_ht_ = ht::factory(HAL_MAX_L2SEGMENTS,
                               hal::l2seg_get_key_func,
                               hal::l2seg_compute_hash_func,
                               hal::l2seg_compare_key_func);
    HAL_ASSERT_RETURN((l2seg_id_ht_ != NULL), false);

    l2seg_hal_handle_ht_ = ht::factory(HAL_MAX_L2SEGMENTS,
                                       hal::l2seg_get_handle_key_func,
                                       hal::l2seg_compute_handle_hash_func,
                                       hal::l2seg_compare_handle_key_func);
    HAL_ASSERT_RETURN((l2seg_hal_handle_ht_ != NULL), false);

    // initialize LIF related data structures
    lif_id_ht_ = ht::factory(HAL_MAX_LIFS,
                             hal::lif_get_key_func,
                             hal::lif_compute_hash_func,
                             hal::lif_compare_key_func);
    HAL_ASSERT_RETURN((lif_id_ht_ != NULL), false);

    lif_hal_handle_ht_ = ht::factory(HAL_MAX_INTERFACES,
                                     hal::lif_get_handle_key_func,
                                     hal::lif_compute_handle_hash_func,
                                     hal::lif_compare_handle_key_func);
    HAL_ASSERT_RETURN((lif_hal_handle_ht_ != NULL), false);

    // initialize interface related data structures
    if_id_ht_ = ht::factory(HAL_MAX_INTERFACES,
                            hal::if_get_key_func,
                            hal::if_compute_hash_func,
                            hal::if_compare_key_func);
    HAL_ASSERT_RETURN((if_id_ht_ != NULL), false);

    if_hal_handle_ht_ = ht::factory(HAL_MAX_INTERFACES,
                                    hal::if_get_handle_key_func,
                                    hal::if_compute_handle_hash_func,
                                    hal::if_compare_handle_key_func);
    HAL_ASSERT_RETURN((if_hal_handle_ht_ != NULL), false);

    // initialize endpoint related data structures
    ep_hal_handle_ht_ = ht::factory(HAL_MAX_ENDPOINTS,
                                    hal::ep_get_handle_key_func,
                                    hal::ep_compute_handle_hash_func,
                                    hal::ep_compare_handle_key_func);
    HAL_ASSERT_RETURN((ep_hal_handle_ht_ != NULL), false);

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
    HAL_ASSERT_RETURN((if_hal_handle_ht_ != NULL), false);

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

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_cfg_db::hal_cfg_db()
{
    tenant_id_ht_ = NULL;
    tenant_hal_handle_ht_ = NULL;

    network_key_ht_ = NULL;
    network_hal_handle_ht_ = NULL;

    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;

    l2seg_id_ht_ = NULL;
    l2seg_hal_handle_ht_ = NULL;

    lif_id_ht_ = NULL;
    lif_hal_handle_ht_ = NULL;

    if_id_ht_ = NULL;
    if_hal_handle_ht_ = NULL;

    ep_hal_handle_ht_ = NULL;

    session_id_ht_ = NULL;
    session_hal_handle_ht_ = NULL;

    l4lb_ht_ = NULL;
    l4lb_hal_handle_ht_ = NULL;

    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;

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
    
    proxy_type_ht_ = NULL;
    proxy_hal_handle_ht_ = NULL;

    ipseccb_id_ht_ = NULL;
    ipseccb_hal_handle_ht_ = NULL;
    
    cpucb_id_ht_ = NULL;
    cpucb_hal_handle_ht_ = NULL;
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
    HAL_SPINLOCK_DESTROY(&slock_);

    tenant_id_ht_ ? delete tenant_id_ht_ : HAL_NOP;
    tenant_hal_handle_ht_ ? delete tenant_hal_handle_ht_ : HAL_NOP;

    network_key_ht_ ? delete network_key_ht_ : HAL_NOP;
    network_hal_handle_ht_ ? delete network_hal_handle_ht_ : HAL_NOP;

    nwsec_profile_id_ht_ ? delete nwsec_profile_id_ht_ : HAL_NOP;
    nwsec_profile_hal_handle_ht_ ? delete nwsec_profile_hal_handle_ht_ : HAL_NOP;

    l2seg_id_ht_ ? delete l2seg_id_ht_ : HAL_NOP;
    l2seg_hal_handle_ht_ ? delete l2seg_hal_handle_ht_ : HAL_NOP;

    lif_id_ht_ ? delete lif_id_ht_ : HAL_NOP;
    lif_hal_handle_ht_ ? delete lif_hal_handle_ht_ : HAL_NOP;

    if_id_ht_ ? delete if_id_ht_ : HAL_NOP;
    if_hal_handle_ht_ ? delete if_hal_handle_ht_ : HAL_NOP;

    ep_hal_handle_ht_ ? delete ep_hal_handle_ht_ : HAL_NOP;

    session_id_ht_ ? delete session_id_ht_ : HAL_NOP;
    session_hal_handle_ht_ ? delete session_hal_handle_ht_ : HAL_NOP;

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

    proxy_type_ht_ ? delete proxy_type_ht_ : HAL_NOP;
    proxy_hal_handle_ht_ ? delete proxy_hal_handle_ht_ : HAL_NOP;
    
    acl_id_ht_ ? delete acl_id_ht_ : HAL_NOP;
    acl_hal_handle_ht_ ? delete acl_hal_handle_ht_ : HAL_NOP;

    ipseccb_id_ht_ ? delete ipseccb_id_ht_ : HAL_NOP;
    ipseccb_hal_handle_ht_ ? delete ipseccb_hal_handle_ht_ : HAL_NOP;

    cpucb_id_ht_ ? delete cpucb_id_ht_ : HAL_NOP;
    cpucb_hal_handle_ht_ ? delete cpucb_hal_handle_ht_ : HAL_NOP;
}

//------------------------------------------------------------------------------
// init() function to instantiate all the oper db init state
//------------------------------------------------------------------------------
bool
hal_oper_db::init(void)
{
    if_hwid_ht_ = ht::factory(HAL_MAX_INTERFACES,
                              hal::if_get_hw_key_func,
                              hal::if_compute_hw_hash_func,
                              hal::if_compare_hw_key_func);
    HAL_ASSERT_RETURN((if_hwid_ht_ != NULL), false);

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

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_oper_db::hal_oper_db()
{
    uint32_t    i;

    infra_l2seg_ = NULL;
    if_hwid_ht_ = NULL;
    ep_l2_ht_ = NULL;
    ep_l3_entry_ht_ = NULL;
    flow_ht_ = NULL;
    for (i = 0; i < HAL_ARRAY_SIZE(cos_in_use_bmp_); i++) {
        cos_in_use_bmp_[i] = NULL;
    }
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

    infra_l2seg_ = NULL;

    if_hwid_ht_ ? delete if_hwid_ht_ : HAL_NOP;

    ep_l2_ht_ ? delete ep_l2_ht_ : HAL_NOP;
    ep_l3_entry_ht_ ? delete ep_l3_entry_ht_ : HAL_NOP;

    flow_ht_ ? delete flow_ht_ : HAL_NOP;

    for (i = 0; i < HAL_ARRAY_SIZE(cos_in_use_bmp_); i++) {
        if (cos_in_use_bmp_[i]) {
            delete cos_in_use_bmp_[i];
        }
    }
}

//------------------------------------------------------------------------------
// init() function to instantiate all the mem db init state
//------------------------------------------------------------------------------
bool
hal_mem_db::init(void)
{
    // initialize tenant related data structures
    tenant_slab_ = slab::factory("tenant", HAL_SLAB_TENANT,
                                 sizeof(hal::tenant_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((tenant_slab_ != NULL), false);

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

    // initialize proxy service related data structures
    proxy_slab_ = slab::factory("proxy", HAL_SLAB_PROXY,
                                sizeof(hal::proxy_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((proxy_slab_ != NULL), false);

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

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_mem_db::hal_mem_db()
{
    tenant_slab_ = NULL;
    network_slab_ = NULL;
    nwsec_profile_slab_ = NULL;
    l2seg_slab_ = NULL;
    lif_slab_ = NULL;
    if_slab_ = NULL;
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
    proxy_slab_ = NULL;
    acl_slab_ = NULL;
    ipseccb_slab_ = NULL;
    cpucb_slab_ = NULL;
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
    tenant_slab_ ? delete tenant_slab_ : HAL_NOP;
    network_slab_ ? delete network_slab_ : HAL_NOP;
    nwsec_profile_slab_ ? delete nwsec_profile_slab_ : HAL_NOP;
    l2seg_slab_ ? delete l2seg_slab_ : HAL_NOP;
    lif_slab_ ? delete lif_slab_ : HAL_NOP;
    if_slab_ ? delete if_slab_ : HAL_NOP;
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
    proxy_slab_ ? delete proxy_slab_ : HAL_NOP;
    acl_slab_ ? delete acl_slab_ : HAL_NOP;
    ipseccb_slab_ ? delete ipseccb_slab_ : HAL_NOP;
    cpucb_slab_ ? delete cpucb_slab_ : HAL_NOP;
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

#if 0
//------------------------------------------------------------------------------
// init() function to instantiate all the slabs
//------------------------------------------------------------------------------
bool
hal_state::init(void)
{
    // version 0 indicates, no config received by HAL
    cfg_db_ver_ = 0;
    HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);

    // initialize tenant related data structures
    tenant_slab_ = slab::factory("tenant", HAL_SLAB_TENANT,
                                 sizeof(hal::tenant_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((tenant_slab_ != NULL), false);

    tenant_id_ht_ = ht::factory(HAL_MAX_VRFS,
                                hal::tenant_get_key_func,
                                hal::tenant_compute_hash_func,
                                hal::tenant_compare_key_func);
    HAL_ASSERT_RETURN((tenant_id_ht_ != NULL), false);

    tenant_hal_handle_ht_ = ht::factory(HAL_MAX_VRFS,
                                        hal::tenant_get_handle_key_func,
                                        hal::tenant_compute_handle_hash_func,
                                        hal::tenant_compare_handle_key_func);
    HAL_ASSERT_RETURN((tenant_hal_handle_ht_ != NULL), false);

    // initialize network related data structures
    network_slab_ = slab::factory("network", HAL_SLAB_NETWORK,
                                 sizeof(hal::network_t), 16,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((network_slab_ != NULL), false);

    network_key_ht_ = ht::factory(HAL_MAX_VRFS,
                                hal::network_get_key_func,
                                hal::network_compute_hash_func,
                                hal::network_compare_key_func);
    HAL_ASSERT_RETURN((network_key_ht_ != NULL), false);

    network_hal_handle_ht_ = ht::factory(HAL_MAX_VRFS,
                                        hal::network_get_handle_key_func,
                                        hal::network_compute_handle_hash_func,
                                        hal::network_compare_handle_key_func);
    HAL_ASSERT_RETURN((network_hal_handle_ht_ != NULL), false);

    // initialize security profile related data structures
    nwsec_profile_slab_ = slab::factory("nwsec-profile",
                                        HAL_SLAB_SECURITY_PROFILE,
                                        sizeof(hal::nwsec_profile_t), 8,
                                        false, true, true, true);
    HAL_ASSERT_RETURN((nwsec_profile_slab_ != NULL), false);

    nwsec_profile_id_ht_ = ht::factory(HAL_MAX_NWSEC_PROFILES,
                                       hal::nwsec_profile_get_key_func,
                                       hal::nwsec_profile_compute_hash_func,
                                       hal::nwsec_profile_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_profile_id_ht_ != NULL), false);

    nwsec_profile_hal_handle_ht_ =
        ht::factory(HAL_MAX_NWSEC_PROFILES,
                    hal::nwsec_profile_get_handle_key_func,
                    hal::nwsec_profile_compute_handle_hash_func,
                    hal::nwsec_profile_compare_handle_key_func);
    HAL_ASSERT_RETURN((nwsec_profile_hal_handle_ht_ != NULL), false);

    // initialize L2 segment related data structures
    l2seg_slab_ = slab::factory("l2seg", HAL_SLAB_L2SEG,
                                sizeof(hal::l2seg_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((l2seg_slab_ != NULL), false);

    l2seg_id_ht_ = ht::factory(HAL_MAX_L2SEGMENTS,
                               hal::l2seg_get_key_func,
                               hal::l2seg_compute_hash_func,
                               hal::l2seg_compare_key_func);
    HAL_ASSERT_RETURN((l2seg_id_ht_ != NULL), false);

    l2seg_hal_handle_ht_ = ht::factory(HAL_MAX_L2SEGMENTS,
                                       hal::l2seg_get_handle_key_func,
                                       hal::l2seg_compute_handle_hash_func,
                                       hal::l2seg_compare_handle_key_func);
    HAL_ASSERT_RETURN((l2seg_hal_handle_ht_ != NULL), false);

    // initialize LIF related data structures
    lif_slab_ = slab::factory("LIF", HAL_SLAB_LIF,
                              sizeof(hal::lif_t), 8,
                             false, true, true, true);
    HAL_ASSERT_RETURN((lif_slab_ != NULL), false);

    lif_id_ht_ = ht::factory(HAL_MAX_LIFS,
                             hal::lif_get_key_func,
                             hal::lif_compute_hash_func,
                             hal::lif_compare_key_func);
    HAL_ASSERT_RETURN((lif_id_ht_ != NULL), false);

    lif_hal_handle_ht_ = ht::factory(HAL_MAX_INTERFACES,
                                     hal::lif_get_handle_key_func,
                                     hal::lif_compute_handle_hash_func,
                                     hal::lif_compare_handle_key_func);
    HAL_ASSERT_RETURN((lif_hal_handle_ht_ != NULL), false);

    // initialize interface related data structures
    if_slab_ = slab::factory("interface", HAL_SLAB_IF,
                             sizeof(hal::if_t), 16,
                             false, true, true, true);
    HAL_ASSERT_RETURN((if_slab_ != NULL), false);

    if_id_ht_ = ht::factory(HAL_MAX_INTERFACES,
                            hal::if_get_key_func,
                            hal::if_compute_hash_func,
                            hal::if_compare_key_func);
    HAL_ASSERT_RETURN((if_id_ht_ != NULL), false);

    if_hwid_ht_ = ht::factory(HAL_MAX_INTERFACES,
                              hal::if_get_hw_key_func,
                              hal::if_compute_hw_hash_func,
                              hal::if_compare_hw_key_func);
    HAL_ASSERT_RETURN((if_hwid_ht_ != NULL), false);

    if_hal_handle_ht_ = ht::factory(HAL_MAX_INTERFACES,
                                    hal::if_get_handle_key_func,
                                    hal::if_compute_handle_hash_func,
                                    hal::if_compare_handle_key_func);
    HAL_ASSERT_RETURN((if_hal_handle_ht_ != NULL), false);

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

    ep_hal_handle_ht_ = ht::factory(HAL_MAX_ENDPOINTS,
                                    hal::ep_get_handle_key_func,
                                    hal::ep_compute_handle_hash_func,
                                    hal::ep_compare_handle_key_func);
    HAL_ASSERT_RETURN((ep_hal_handle_ht_ != NULL), false);

    // initialize flow/session related data structures
    flow_slab_ = slab::factory("Flow", HAL_SLAB_FLOW, sizeof(hal::flow_t), 128,
                               true, true, true, true);
    HAL_ASSERT_RETURN((flow_slab_ != NULL), false);

    session_slab_ = slab::factory("Session", HAL_SLAB_SESSION,
                                  sizeof(hal::session_t), 128,
                                  true, true, true, true);
    HAL_ASSERT_RETURN((session_slab_ != NULL), false);

    flow_ht_ = ht::factory(HAL_MAX_FLOWS,
                           hal::flow_get_key_func,
                           hal::flow_compute_hash_func,
                           hal::flow_compare_key_func);
    HAL_ASSERT_RETURN((flow_ht_ != NULL), false);

    session_id_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                 hal::session_get_key_func,
                                 hal::session_compute_hash_func,
                                 hal::session_compare_key_func);
    HAL_ASSERT_RETURN((session_id_ht_ != NULL), false);

    session_hal_handle_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                         hal::session_get_handle_key_func,
                                         hal::session_compute_handle_hash_func,
                                         hal::session_compare_handle_key_func);
    HAL_ASSERT_RETURN((if_hal_handle_ht_ != NULL), false);

    // initialize TLS CB related data structures
    tlscb_slab_ = slab::factory("tlscb", HAL_SLAB_TLSCB,
                                sizeof(hal::tlscb_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((tlscb_slab_ != NULL), false);

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
    tcpcb_slab_ = slab::factory("tcpcb", HAL_SLAB_TCPCB,
                                sizeof(hal::tcpcb_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((tcpcb_slab_ != NULL), false);

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
    buf_pool_slab_ = slab::factory("BufPool", HAL_SLAB_BUF_POOL,
                              sizeof(hal::buf_pool_t), 8,
                             false, true, true, true);
    HAL_ASSERT_RETURN((buf_pool_slab_ != NULL), false);

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

    for (uint32_t port = 0; port < HAL_MAX_TM_PORTS; port++) {
        cos_in_use_bmp_[port] = bitmap::factory(HAL_MAX_COSES,
                                                true);
        HAL_ASSERT_RETURN((cos_in_use_bmp_[port] != NULL), false);
    }

    // initialize Queue related data structures
    queue_slab_ = slab::factory("Queue", HAL_SLAB_QUEUE,
                              sizeof(hal::queue_t), 8,
                             false, true, true, true);
    HAL_ASSERT_RETURN((queue_slab_ != NULL), false);

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
    policer_slab_ = slab::factory("Policer", HAL_SLAB_POLICER,
                              sizeof(hal::policer_t), 8,
                             false, true, true, true);
    HAL_ASSERT_RETURN((policer_slab_ != NULL), false);

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
    acl_slab_ = slab::factory("Acl", HAL_SLAB_ACL,
                              sizeof(hal::acl_t), 8,
                              false, true, true, true);
    HAL_ASSERT_RETURN((acl_slab_ != NULL), false);

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
    wring_slab_ = slab::factory("wring", HAL_SLAB_WRING,
                                sizeof(hal::wring_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((wring_slab_ != NULL), false);

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
                                sizeof(hal::proxy_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((proxy_slab_ != NULL), false);

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
    ipseccb_slab_ = slab::factory("ipseccb", HAL_SLAB_IPSECCB,
                                  sizeof(hal::ipseccb_t), 16,
                                  false, true, true, true);
    HAL_ASSERT_RETURN((ipseccb_slab_ != NULL), false);

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

    // initialize l4lb related data structures
    l4lb_slab_ = slab::factory("L4LB", HAL_SLAB_L4LB, sizeof(hal::l4lb_service_entry_t), 16,
                             true, true, true, true);
    HAL_ASSERT_RETURN((l4lb_slab_ != NULL), false);

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
    
    // initialize CPU CB related data structures
    cpucb_slab_ = slab::factory("cpucb", HAL_SLAB_CPUCB,
                                sizeof(hal::cpucb_t), 16,
                                false, true, true, true);
    HAL_ASSERT_RETURN((cpucb_slab_ != NULL), false);

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

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_state::hal_state()
{
    tenant_slab_ = NULL;
    tenant_id_ht_ = NULL;
    tenant_hal_handle_ht_ = NULL;

    nwsec_profile_slab_ = NULL;
    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;

    l2seg_slab_ = NULL;
    l2seg_id_ht_ = NULL;
    l2seg_hal_handle_ht_ = NULL;

    lif_slab_ = NULL;
    lif_id_ht_ = NULL;
    lif_hal_handle_ht_ = NULL;

    if_slab_ = NULL;
    if_id_ht_ = NULL;
    if_hwid_ht_ = NULL;
    if_hal_handle_ht_ = NULL;

    ep_slab_ = NULL;
    ep_ip_entry_slab_ = NULL;
    ep_l3_entry_slab_ = NULL;
    ep_l2_ht_ = NULL;
    ep_l3_entry_ht_ = NULL;

    flow_slab_ = NULL;
    session_slab_ = NULL;
    flow_ht_ = NULL;
    session_id_ht_ = NULL;
    session_hal_handle_ht_ = NULL;

    nwsec_profile_slab_ = NULL;
    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;

    buf_pool_slab_ = NULL;
    buf_pool_id_ht_ = NULL;
    buf_pool_hal_handle_ht_ = NULL;

    queue_slab_ = NULL;
    queue_id_ht_ = NULL;
    queue_hal_handle_ht_ = NULL;

    policer_slab_ = NULL;
    ingress_policer_id_ht_ = NULL;
    ingress_policer_hal_handle_ht_ = NULL;
    egress_policer_id_ht_ = NULL;
    egress_policer_hal_handle_ht_ = NULL;

    acl_slab_ = NULL;
    acl_id_ht_ = NULL;
    acl_hal_handle_ht_ = NULL;
    
    tlscb_slab_ = NULL;
    tlscb_id_ht_ = NULL;
    tlscb_hal_handle_ht_ = NULL;
 
    tcpcb_slab_ = NULL;
    tcpcb_id_ht_ = NULL;
    tcpcb_hal_handle_ht_ = NULL;
    
    wring_slab_ = NULL;
    wring_id_ht_ = NULL;
    wring_hal_handle_ht_ = NULL;
    
    proxy_slab_ = NULL;
    proxy_type_ht_ = NULL;
    proxy_hal_handle_ht_ = NULL;

    infra_l2seg_ = NULL;
    
    ipseccb_slab_ = NULL;
    ipseccb_id_ht_ = NULL;
    ipseccb_hal_handle_ht_ = NULL;
    
    cpucb_slab_ = NULL;
    cpucb_id_ht_ = NULL;
    cpucb_hal_handle_ht_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state::~hal_state()
{
    HAL_SPINLOCK_DESTROY(&slock_);

    tenant_slab_ ? delete tenant_slab_ : HAL_NOP;
    tenant_id_ht_ ? delete tenant_id_ht_ : HAL_NOP;
    tenant_hal_handle_ht_ ? delete tenant_hal_handle_ht_ : HAL_NOP;

    nwsec_profile_slab_ ? delete nwsec_profile_slab_ : HAL_NOP;
    nwsec_profile_id_ht_ ? delete nwsec_profile_id_ht_ : HAL_NOP;
    nwsec_profile_hal_handle_ht_ ? delete nwsec_profile_hal_handle_ht_ : HAL_NOP;

    l2seg_slab_ ? delete l2seg_slab_ : HAL_NOP;
    l2seg_id_ht_ ? delete l2seg_id_ht_ : HAL_NOP;
    l2seg_hal_handle_ht_ ? delete l2seg_hal_handle_ht_ : HAL_NOP;

    lif_slab_ ? delete lif_slab_ : HAL_NOP;
    lif_id_ht_ ? delete lif_id_ht_ : HAL_NOP;
    lif_hal_handle_ht_ ? delete lif_hal_handle_ht_ : HAL_NOP;

    if_slab_ ? delete if_slab_ : HAL_NOP;
    if_id_ht_ ? delete if_id_ht_ : HAL_NOP;
    if_hwid_ht_ ? delete if_hwid_ht_ : HAL_NOP;
    if_hal_handle_ht_ ? delete if_hal_handle_ht_ : HAL_NOP;

    ep_slab_ ? delete ep_slab_ : HAL_NOP;
    ep_ip_entry_slab_ ? delete ep_ip_entry_slab_ : HAL_NOP;
    ep_l3_entry_slab_ ? delete ep_l3_entry_slab_ : HAL_NOP;
    ep_l2_ht_ ? delete ep_l2_ht_ : HAL_NOP;
    ep_l3_entry_ht_ ? delete ep_l3_entry_ht_ : HAL_NOP;

    flow_slab_ ? delete flow_slab_ : HAL_NOP;
    session_slab_ ? delete session_slab_ : HAL_NOP;
    flow_ht_ ? delete flow_ht_ : HAL_NOP;
    session_id_ht_ ? delete session_id_ht_ : HAL_NOP;
    session_hal_handle_ht_ ? delete session_hal_handle_ht_ : HAL_NOP;

    nwsec_profile_slab_ ? delete nwsec_profile_slab_ : HAL_NOP;
    nwsec_profile_id_ht_ ? delete nwsec_profile_id_ht_ : HAL_NOP;
    nwsec_profile_hal_handle_ht_ ? delete nwsec_profile_hal_handle_ht_ : HAL_NOP;

    buf_pool_slab_ ? delete buf_pool_slab_ : HAL_NOP;
    buf_pool_id_ht_ ? delete buf_pool_id_ht_ : HAL_NOP;
    buf_pool_hal_handle_ht_ ? delete buf_pool_hal_handle_ht_ : HAL_NOP;

    queue_slab_ ? delete queue_slab_ : HAL_NOP;
    queue_id_ht_ ? delete queue_id_ht_ : HAL_NOP;
    queue_hal_handle_ht_ ? delete queue_hal_handle_ht_ : HAL_NOP;

    policer_slab_ ? delete policer_slab_ : HAL_NOP;
    ingress_policer_id_ht_ ? delete ingress_policer_id_ht_ : HAL_NOP;
    ingress_policer_hal_handle_ht_ ? delete ingress_policer_hal_handle_ht_ : HAL_NOP;
    egress_policer_id_ht_ ? delete egress_policer_id_ht_ : HAL_NOP;
    egress_policer_hal_handle_ht_ ? delete egress_policer_hal_handle_ht_ : HAL_NOP;

    acl_slab_ ? delete acl_slab_ : HAL_NOP;
    acl_id_ht_ ? delete acl_id_ht_ : HAL_NOP;
    acl_hal_handle_ht_ ? delete acl_hal_handle_ht_ : HAL_NOP;

    tlscb_slab_ ? delete tlscb_slab_ : HAL_NOP;
    tlscb_id_ht_ ? delete tlscb_id_ht_ : HAL_NOP;
    tlscb_hal_handle_ht_ ? delete tlscb_hal_handle_ht_ : HAL_NOP;
 
    tcpcb_slab_ ? delete tcpcb_slab_ : HAL_NOP;
    tcpcb_id_ht_ ? delete tcpcb_id_ht_ : HAL_NOP;
    tcpcb_hal_handle_ht_ ? delete tcpcb_hal_handle_ht_ : HAL_NOP;

    wring_slab_ ? delete wring_slab_ : HAL_NOP;
    wring_id_ht_ ? delete wring_id_ht_ : HAL_NOP;
    wring_hal_handle_ht_ ? delete wring_hal_handle_ht_ : HAL_NOP;
    
    proxy_slab_ ? delete proxy_slab_ : HAL_NOP;
    proxy_type_ht_ ? delete proxy_type_ht_ : HAL_NOP;
    proxy_hal_handle_ht_ ? delete proxy_hal_handle_ht_ : HAL_NOP;
    
    ipseccb_slab_ ? delete ipseccb_slab_ : HAL_NOP;
    ipseccb_id_ht_ ? delete ipseccb_id_ht_ : HAL_NOP;
    ipseccb_hal_handle_ht_ ? delete ipseccb_hal_handle_ht_ : HAL_NOP;

    l4lb_slab_ ? delete l4lb_slab_ : HAL_NOP;
    l4lb_ht_ ? delete l4lb_ht_ : HAL_NOP;
    l4lb_hal_handle_ht_ ? delete l4lb_hal_handle_ht_ : HAL_NOP;
    
    cpucb_slab_ ? delete cpucb_slab_ : HAL_NOP;
    cpucb_id_ht_ ? delete cpucb_id_ht_ : HAL_NOP;
    cpucb_hal_handle_ht_ ? delete cpucb_hal_handle_ht_ : HAL_NOP;
}

//------------------------------------------------------------------------------
// helper function to
// 1. read the current config db version and
// 2. mark that version as in-use for current thread
// NOTE: this has to be done atomically, otherwise in between these two steps
//       config thread can think that current version is not in use while FTE
//       is done with step 1 and not 2. All threads (config, FTEs etc.) all must
//       use this API before accessing the config Db
//------------------------------------------------------------------------------
cfg_version_t
hal_state::cfg_db_get_current_version(void)
{
    int              free_slot = -1;
    uint32_t         i;
    cfg_version_t    ver;

    HAL_SPINLOCK_LOCK(&slock_);
    ver = cfg_db_ver_;
    for (i = 0; i < HAL_ARRAY_SIZE(cfg_ver_in_use_); i++) {
        if (!cfg_ver_in_use_[i].valid) {
            free_slot = i;
        } else if (cfg_ver_in_use_[i].ver == ver) {
            // version already in use, just bump up the refcount
            cfg_ver_in_use_[i].usecnt++;
            goto end;
        }
    }

    // mark this version as "in-use" for the 1st time
    if (free_slot < 0) {
        // some thread didn't bother to release versions it was using
        HAL_ASSERT(FALSE);
    }
    cfg_ver_in_use_[free_slot].ver = ver;
    cfg_ver_in_use_[free_slot].usecnt = 1;
    cfg_ver_in_use_[free_slot].valid = TRUE;

end:

    HAL_SPINLOCK_UNLOCK(&slock_);
    return ver;
}

//------------------------------------------------------------------------------
// helper function to release current config db version that was marked as
// in-use by this thread earlier
//------------------------------------------------------------------------------
hal_ret_t
hal_state::cfg_db_release_version_in_use(cfg_version_t ver)
{
    uint32_t         i;

    HAL_SPINLOCK_LOCK(&slock_);
    for (i = 0; i < HAL_ARRAY_SIZE(cfg_ver_in_use_); i++) {
        if (cfg_ver_in_use_[i].valid && (cfg_ver_in_use_[i].ver == ver)) {
            cfg_ver_in_use_[i].usecnt--;
            if (cfg_ver_in_use_[i].usecnt == 0) {
                cfg_ver_in_use_[i].valid = FALSE;
                cfg_ver_in_use_[i].ver = FALSE;
            }
        }
    }
    HAL_SPINLOCK_UNLOCK(&slock_);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// helper function to reserve a version for future config write/commit
// atomically
//------------------------------------------------------------------------------
cfg_version_t
hal_state::cfg_db_reserve_version(void)
{
    uint32_t         i;
    cfg_version_t    ver = 0;     // 0 ==> invalid, make a macro (TODO) ?

    HAL_SPINLOCK_LOCK(&slock_);
    ver = HAL_ATOMIC_INC_UINT32(&max_rsvd_ver_, 1);
    for (i = 0; i < HAL_ARRAY_SIZE(cfg_ver_rsvd_); i++) {
        if (!cfg_ver_rsvd_[i].valid) {
            cfg_ver_rsvd_[i].ver = ver;
            cfg_ver_rsvd_[i].valid = TRUE;
            goto end;
        }
    }
    HAL_TRACE_ERR("Failed to reserve cfg version");

end:

    HAL_SPINLOCK_UNLOCK(&slock_);
    return ver;
}

//------------------------------------------------------------------------------
// API to call before processing any packet by FTE, any operation by config
// thread or periodic thread etc.
// NOTE: once opened, cfg db has to be closed properly and reserved version
//       should be released/committed or else next open will fail
//------------------------------------------------------------------------------
hal_ret_t
hal_state::cfg_db_open(cfg_op_t cfg_op)
{
    // if the cfg db was already opened by this thread, error out
    if (t_cfg_db_ctxt.cfg_db_open_) {
        HAL_TRACE_ERR("Failed to open cfg db, opened already, thread {}",
                      t_curr_thread->name());
        return HAL_RET_ERR;
    }

    // get the current version of the db and mark it as in-use
    t_cfg_db_ctxt.ver_in_use_ = cfg_db_get_current_version();
    if (cfg_op == CFG_OP_READ) {
        // get the current max valid version
        t_cfg_db_ctxt.rsvd_ver_ = t_cfg_db_ctxt.ver_in_use_;
    } else {
        // reserve a db version for later commit
        t_cfg_db_ctxt.rsvd_ver_ = cfg_db_reserve_version();
    }
    t_cfg_db_ctxt.cfg_op_ = cfg_op;
    t_cfg_db_ctxt.cfg_db_open_ = true;
    HAL_TRACE_DEBUG("{} opened cfg db, cfg op : {}, rsvd version : {}",
                    t_curr_thread->name(), cfg_op, t_cfg_db_ctxt.rsvd_ver_);

    return HAL_RET_OK;
}
#endif

#if 0
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
hal_state::cfg_db_close(void)
{
    hal_ret_t    ret;

    if (t_cfg_db_ctxt.cfg_db_open_) {
        if (t_cfg_db_ctxt.cfg_op_ == CFG_OP_WRITE) {
            // TODO: commit this version
            ret =
                g_hal_state->cfg_db_version_commit(t_cfg_db_ctxt.rsvd_ver_);
            // TODO: commit reserved version and update the DB version to that
            ret = cfg_db_versions_.validate_version(t_cfg_db_ctxt.rsvd_ver_);

            cfg_db_ver_ = t_cfg_db_ctxt.rsvd_ver_;

            if (ret != HAL_RET_OK) {
                // TODO: mark t_cfg_db_ctxt.rsvd_ver_ as invalid
                g_hal_state->cfg_db_version_invalidate(t_cfg_db_ctxt.rsvd_ver_);
            }
        } else {
            // release the version, indicating that we are done using rsvd_ver_
            g_hal_state->cfg_db_version_release(t_cfg_db_ctxt.rsvd_ver_);
        }

        // successful or not, close the DB so the app can reopen and retry
        t_cfg_db_ctxt.cfg_db_open_ = FALSE;
        t_cfg_db_ctxt.cfg_op_ = CFG_OP_NONE;
        t_cfg_db_ctxt.rsvd_ver_ = 0;
    }
    return HAL_RET_OK;
}
#endif

#if 0
//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_state *
hal_state::factory(void)
{
    hal_state *state;

    state = new hal_state();
    HAL_ASSERT_RETURN((state != NULL), NULL);
    if (state->init() == false) {
        delete state;
        return NULL;
    }
    return state;
}
#endif

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
// free an element back to given slab specified by its id
//------------------------------------------------------------------------------
hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_TENANT:
        g_hal_state->tenant_slab()->free_(elem);
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

    case HAL_SLAB_IPSECCB:
        g_hal_state->ipseccb_slab()->free_(elem);
    
    case HAL_SLAB_CPUCB:
        g_hal_state->cpucb_slab()->free_(elem);
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
