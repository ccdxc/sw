// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <boost/interprocess/managed_shared_memory.hpp>
#include "nic/hal/hal.hpp"
#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/mcast/multicast.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/security/nwsec.hpp"
#include "nic/hal/src/internal/event.hpp"
#include "nic/hal/src/internal/tls_proxy_cb.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/src/security/nwsec.hpp"
#include "nic/include/nwsec_group_api.hpp"
#include "nic/hal/src/security/nwsec_group.hpp"
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
#include "nic/hal/src/gft/gft.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "sdk/twheel.hpp"
#include "sdk/shmmgr.hpp"

// name of the HAL state store segment
#define HAL_STATE_STORE                         "h2s"
#define HAL_STATE_STORE_SIZE                    0x80000000     // 2G
#define HAL_SERIALIZED_STATE_STORE              "h3s"
#define HAL_SERIALIZED_STATE_STORE_SIZE         0x80000000     // 2G
#define HAL_STATE_STORE_VADDR                   0x400000000    // starting from 16G
#define HAL_SERIALIZED_STATE_STORE_VADDR        0x480000000    // starting from 18G
#define HAL_STATE_OBJ                           "halstate"

using namespace boost::interprocess;

namespace hal {

// global instance of all HAL state including config, operational states
class hal_state    *g_hal_state;
class shmmgr       *g_h2s_shmmgr, *g_h3s_shmmgr;

//------------------------------------------------------------------------------
// initialize DBs and caches that needs to be persisted across restarts/upgrades
//------------------------------------------------------------------------------
bool
hal_cfg_db::init_pss(shmmgr *mmgr)
{

    // initialize vrf related data structures
    vrf_id_ht_ = ht::factory(HAL_MAX_VRFS,
                             hal::vrf_id_get_key_func,
                             hal::vrf_id_compute_hash_func,
                             hal::vrf_id_compare_key_func,
                             true, mmgr);
    HAL_ASSERT_RETURN((vrf_id_ht_ != NULL), false);

    // initialize network related data structures
    network_key_ht_ = ht::factory(HAL_MAX_VRFS,
                                  hal::network_get_key_func,
                                  hal::network_compute_hash_func,
                                  hal::network_compare_key_func,
                                  true, mmgr);
    HAL_ASSERT_RETURN((network_key_ht_ != NULL), false);

    // initialize security profile related data structures
    nwsec_profile_id_ht_ = ht::factory(HAL_MAX_NWSEC_PROFILES,
                                       hal::nwsec_profile_id_get_key_func,
                                       hal::nwsec_profile_id_compute_hash_func,
                                       hal::nwsec_profile_id_compare_key_func,
                                       true, mmgr);
    HAL_ASSERT_RETURN((nwsec_profile_id_ht_ != NULL), false);

    // initialize L2 segment related data structures
    l2seg_id_ht_ = ht::factory(HAL_MAX_L2SEGMENTS,
                               hal::l2seg_id_get_key_func,
                               hal::l2seg_id_compute_hash_func,
                               hal::l2seg_id_compare_key_func,
                               true, mmgr);
    HAL_ASSERT_RETURN((l2seg_id_ht_ != NULL), false);

    // initialize mc entry related data structures
    mc_key_ht_ = ht::factory(HAL_MAX_MC_ENTRIES,
                             hal::mc_entry_get_key_func,
                             hal::mc_entry_compute_hash_func,
                             hal::mc_entry_compare_key_func,
                             true, mmgr);
    HAL_ASSERT_RETURN((mc_key_ht_ != NULL), false);

    // initialize LIF related data structures
    lif_id_ht_ = ht::factory(HAL_MAX_LIFS,
                             hal::lif_id_get_key_func,
                             hal::lif_id_compute_hash_func,
                             hal::lif_id_compare_key_func,
                             true, mmgr);
    HAL_ASSERT_RETURN((lif_id_ht_ != NULL), false);

    // initialize interface related data structures
    if_id_ht_ = ht::factory(HAL_MAX_INTERFACES,
                            hal::if_id_get_key_func,
                            hal::if_id_compute_hash_func,
                            hal::if_id_compare_key_func,
                            true, mmgr);
    HAL_ASSERT_RETURN((if_id_ht_ != NULL), false);

    // initialize flow/session related data structures
    session_id_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                 hal::session_get_key_func,
                                 hal::session_compute_hash_func,
                                 hal::session_compare_key_func,
                                 true, mmgr);
    HAL_ASSERT_RETURN((session_id_ht_ != NULL), false);

    session_hal_handle_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                         hal::session_get_handle_key_func,
                                         hal::session_compute_handle_hash_func,
                                         hal::session_compare_handle_key_func,
                                         true, mmgr);
    HAL_ASSERT_RETURN((session_hal_handle_ht_ != NULL), false);

    session_hal_iflow_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                        hal::session_get_iflow_key_func,
                                        hal::session_compute_iflow_hash_func,
                                        hal::session_compare_iflow_key_func,
                                        true, mmgr);
    HAL_ASSERT_RETURN((session_hal_iflow_ht_ != NULL), false);

    session_hal_rflow_ht_ = ht::factory(HAL_MAX_SESSIONS,
                                        hal::session_get_rflow_key_func,
                                        hal::session_compute_rflow_hash_func,
                                        hal::session_compare_rflow_key_func,
                                        true, mmgr);
    HAL_ASSERT_RETURN((session_hal_rflow_ht_ != NULL), false);

    // initialize l4lb related data structures
    l4lb_ht_ = ht::factory(HAL_MAX_L4LB_SERVICES,
                           hal::l4lb_get_key_func,
                           hal::l4lb_compute_key_hash_func,
                           hal::l4lb_compare_key_func,
                           true, mmgr);
    HAL_ASSERT_RETURN((l4lb_ht_ != NULL), false);

    // initialize Qos-class related data structures
    qos_class_ht_ = ht::factory(HAL_MAX_QOS_CLASSES,
                                hal::qos_class_get_key_func,
                                hal::qos_class_compute_hash_func,
                                hal::qos_class_compare_key_func,
                                true, mmgr);
    HAL_ASSERT_RETURN((qos_class_ht_ != NULL), false);

    // initialize Copp related data structures
    copp_ht_ = ht::factory(HAL_MAX_COPPS,
                           hal::copp_get_key_func,
                           hal::copp_compute_hash_func,
                           hal::copp_compare_key_func,
                           true, mmgr);
    HAL_ASSERT_RETURN((copp_ht_ != NULL), false);

    // initialize acl related data structures
    acl_ht_ = ht::factory(HAL_MAX_ACLS,
                          hal::acl_get_key_func,
                          hal::acl_compute_hash_func,
                          hal::acl_compare_key_func,
                          true, mmgr);
    HAL_ASSERT_RETURN((acl_ht_ != NULL), false);

    nwsec_group_ht_ = ht::factory(HAL_MAX_NW_SEC_GROUP_CFG,
                                  hal::nwsec_group_get_key_func,
                                  hal::nwsec_group_compute_hash_func,
                                  hal::nwsec_group_compare_key_func,
                                  true, mmgr);
    HAL_ASSERT_RETURN((nwsec_group_ht_ != NULL), false);

    gft_exact_match_profile_id_ht_ = ht::factory(HAL_MAX_GFT_EXACT_MATCH_PROFILES,
                                                 hal::gft_exact_match_profile_id_get_key_func,
                                                 hal::gft_exact_match_profile_id_compute_hash_func,
                                                 hal::gft_exact_match_profile_id_compare_key_func,
                                                 true, mmgr);
    HAL_ASSERT_RETURN((gft_exact_match_profile_id_ht_ != NULL), false);

    gft_hdr_transposition_profile_id_ht_ =
        ht::factory(HAL_MAX_GFT_HDR_TRANSPOSITION_PROFILES,
                    hal::gft_hdr_transposition_profile_id_get_key_func,
                    hal::gft_hdr_transposition_profile_id_compute_hash_func,
                    hal::gft_hdr_transposition_profile_id_compare_key_func,
                    true, mmgr);
    HAL_ASSERT_RETURN((gft_hdr_transposition_profile_id_ht_ != NULL), false);

    gft_exact_match_flow_entry_id_ht_ = ht::factory(HAL_MAX_GFT_EXACT_MATCH_FLOW_ENTRIES,
                                                    hal::gft_exact_match_flow_entry_id_get_key_func,
                                                    hal::gft_exact_match_flow_entry_id_compute_hash_func,
                                                    hal::gft_exact_match_flow_entry_id_compare_key_func,
                                                    true, mmgr);
    HAL_ASSERT_RETURN((gft_exact_match_flow_entry_id_ht_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// initialize DBs and caches that don't to be persisted (i.e., volatile) across
// restarts/upgrades and these will have to be rebuilt after restart/upgrade
//------------------------------------------------------------------------------
bool
hal_cfg_db::init_vss(void)
{
    // initialize TLS CB related data structures
    tlscb_id_ht_ = ht::factory(HAL_MAX_TLSCB,
                               hal::tlscb_get_key_func,
                               hal::tlscb_compute_hash_func,
                               hal::tlscb_compare_key_func);
    HAL_ASSERT_RETURN((tlscb_id_ht_ != NULL), false);

    // initialize TCB CB related data structures
    tcpcb_id_ht_ = ht::factory(HAL_MAX_TCPCB,
                               hal::tcpcb_get_key_func,
                               hal::tcpcb_compute_hash_func,
                               hal::tcpcb_compare_key_func);
    HAL_ASSERT_RETURN((tcpcb_id_ht_ != NULL), false);

    qos_cmap_pcp_bmp_ = bitmap::factory(HAL_MAX_DOT1Q_PCP_VALS, true);
    HAL_ASSERT_RETURN((qos_cmap_pcp_bmp_ != NULL), false);

    qos_cmap_dscp_bmp_ = bitmap::factory(HAL_MAX_IP_DSCP_VALS, true);
    HAL_ASSERT_RETURN((qos_cmap_dscp_bmp_ != NULL), false);

    // initialize WRing related data structures
    wring_id_ht_ = ht::factory(HAL_MAX_WRING,
                               hal::wring_get_key_func,
                               hal::wring_compute_hash_func,
                               hal::wring_compare_key_func);
    HAL_ASSERT_RETURN((wring_id_ht_ != NULL), false);

    // initialize proxy service related data structures
    proxy_type_ht_ = ht::factory(HAL_MAX_PROXY,
                                 hal::proxy_get_key_func,
                                 hal::proxy_compute_hash_func,
                                 hal::proxy_compare_key_func);
    HAL_ASSERT_RETURN((proxy_type_ht_ != NULL), false);

    // initialize IPSEC CB related data structures
    ipseccb_id_ht_ = ht::factory(HAL_MAX_IPSECCB,
                                 hal::ipseccb_get_key_func,
                                 hal::ipseccb_compute_hash_func,
                                 hal::ipseccb_compare_key_func);
    HAL_ASSERT_RETURN((ipseccb_id_ht_ != NULL), false);

    // initialize CPU CB related data structures
    cpucb_id_ht_ = ht::factory(HAL_MAX_CPUCB,
                               hal::cpucb_get_key_func,
                               hal::cpucb_compute_hash_func,
                               hal::cpucb_compare_key_func);
    HAL_ASSERT_RETURN((cpucb_id_ht_ != NULL), false);

    // initialize Raw Redirect CB related data structures
    rawrcb_id_ht_ = ht::factory(HAL_MAX_RAWRCB_HT_SIZE,
                                hal::rawrcb_get_key_func,
                                hal::rawrcb_compute_hash_func,
                                hal::rawrcb_compare_key_func);
    HAL_ASSERT_RETURN((rawrcb_id_ht_ != NULL), false);

    // initialize Raw Chain CB related data structures
    rawccb_id_ht_ = ht::factory(HAL_MAX_RAWCCB_HT_SIZE,
                                hal::rawccb_get_key_func,
                                hal::rawccb_compute_hash_func,
                                hal::rawccb_compare_key_func);
    HAL_ASSERT_RETURN((rawccb_id_ht_ != NULL), false);

    // initialize Raw Redirect CB related data structures
    proxyrcb_id_ht_ = ht::factory(HAL_MAX_PROXYRCB_HT_SIZE,
                                  hal::proxyrcb_get_key_func,
                                  hal::proxyrcb_compute_hash_func,
                                  hal::proxyrcb_compare_key_func);
    HAL_ASSERT_RETURN((proxyrcb_id_ht_ != NULL), false);

    // initialize Raw Chain CB related data structures
    proxyccb_id_ht_ = ht::factory(HAL_MAX_PROXYCCB_HT_SIZE,
                                  hal::proxyccb_get_key_func,
                                  hal::proxyccb_compute_hash_func,
                                  hal::proxyccb_compare_key_func);
    HAL_ASSERT_RETURN((proxyccb_id_ht_ != NULL), false);

    nwsec_policy_cfg_ht_ = ht::factory(HAL_MAX_NW_SEC_POLICY_CFG,
                                       hal::nwsec_policy_cfg_get_key_func,
                                       hal::nwsec_policy_cfg_compute_hash_func,
                                       hal::nwsec_policy_cfg_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_policy_cfg_ht_ != NULL), false);

    nwsec_policy_ht_ = ht::factory(HAL_MAX_NW_SEC_POLICY_CFG,
                                   hal::nwsec_policy_get_key_func,
                                   hal::nwsec_policy_compute_hash_func,
                                   hal::nwsec_policy_compare_key_func);
    HAL_ASSERT_RETURN((nwsec_policy_ht_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// init() function to instantiate all the config db init state
//------------------------------------------------------------------------------
bool
hal_cfg_db::init(shmmgr *mmgr)
{
    mmgr_ = mmgr;
    HAL_ASSERT_RETURN((init_pss(mmgr) == true), false);
    HAL_ASSERT_RETURN((init_vss() == true), false);
    return true;
}

//------------------------------------------------------------------------------
// initialize the state that is not valid after a process restart
//------------------------------------------------------------------------------
void
hal_cfg_db::init_on_restart(void) {
    init_vss();
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_cfg_db::hal_cfg_db()
{
    mmgr_ = NULL;
    vrf_id_ht_ = NULL;
    network_key_ht_ = NULL;
    nwsec_profile_id_ht_ = NULL;
    l2seg_id_ht_ = NULL;
    mc_key_ht_ = NULL;
    lif_id_ht_ = NULL;
    if_id_ht_ = NULL;
    session_id_ht_ = NULL;
    session_hal_handle_ht_ = NULL;
    session_hal_iflow_ht_  = NULL;
    session_hal_rflow_ht_  = NULL;
    l4lb_ht_ = NULL;
    nwsec_policy_cfg_ht_  = NULL;
    nwsec_policy_ht_      = NULL;
    nwsec_group_ht_       = NULL;
    qos_class_ht_ = NULL;
    qos_cmap_pcp_bmp_ = NULL;
    qos_cmap_dscp_bmp_ = NULL;
    copp_ht_ = NULL;
    acl_ht_ = NULL;
    tlscb_id_ht_ = NULL;
    tcpcb_id_ht_ = NULL;
    wring_id_ht_ = NULL;
    proxy_type_ht_ = NULL;
    ipseccb_id_ht_ = NULL;
    cpucb_id_ht_ = NULL;
    rawrcb_id_ht_ = NULL;
    app_redir_if_id_ = HAL_IFINDEX_INVALID;
    rawccb_id_ht_ = NULL;
    proxyrcb_id_ht_ = NULL;
    proxyccb_id_ht_ = NULL;
    gft_exact_match_profile_id_ht_ = NULL;
    gft_hdr_transposition_profile_id_ht_ = NULL;
    gft_exact_match_flow_entry_id_ht_ = NULL;
    forwarding_mode_ = HAL_FORWARDING_MODE_NONE;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_cfg_db::~hal_cfg_db()
{
    vrf_id_ht_ ? ht::destroy(vrf_id_ht_, mmgr_) : HAL_NOP;
    network_key_ht_ ? ht::destroy(network_key_ht_, mmgr_) : HAL_NOP;
    nwsec_profile_id_ht_ ? ht::destroy(nwsec_profile_id_ht_, mmgr_) : HAL_NOP;
    l2seg_id_ht_ ? ht::destroy(l2seg_id_ht_, mmgr_) : HAL_NOP;
    mc_key_ht_ ? ht::destroy(mc_key_ht_, mmgr_) : HAL_NOP;
    lif_id_ht_ ? ht::destroy(lif_id_ht_, mmgr_) : HAL_NOP;
    if_id_ht_ ? ht::destroy(if_id_ht_, mmgr_) : HAL_NOP;
    session_id_ht_ ? ht::destroy(session_id_ht_, mmgr_) : HAL_NOP;
    session_hal_handle_ht_ ? ht::destroy(session_hal_handle_ht_) : HAL_NOP;
    session_hal_iflow_ht_ ? ht::destroy(session_hal_iflow_ht_) : HAL_NOP;
    session_hal_rflow_ht_ ? ht::destroy(session_hal_rflow_ht_) : HAL_NOP;
    l4lb_ht_ ? ht::destroy(l4lb_ht_, mmgr_) : HAL_NOP;
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
    cpucb_id_ht_ ? ht::destroy(cpucb_id_ht_) : HAL_NOP;
    rawrcb_id_ht_ ? ht::destroy(rawrcb_id_ht_) : HAL_NOP;
    rawccb_id_ht_ ? ht::destroy(rawccb_id_ht_) : HAL_NOP;
    proxyrcb_id_ht_ ? ht::destroy(proxyrcb_id_ht_) : HAL_NOP;
    proxyccb_id_ht_ ? ht::destroy(proxyccb_id_ht_) : HAL_NOP;
    nwsec_policy_cfg_ht_ ? ht::destroy(nwsec_policy_cfg_ht_) : HAL_NOP;
    nwsec_policy_ht_ ? ht::destroy(nwsec_policy_ht_) : HAL_NOP;
    nwsec_group_ht_ ? ht::destroy(nwsec_group_ht_) : HAL_NOP;
    gft_exact_match_profile_id_ht_ ? ht::destroy(gft_exact_match_profile_id_ht_, mmgr_) : HAL_NOP;
    gft_hdr_transposition_profile_id_ht_ ? ht::destroy(gft_hdr_transposition_profile_id_ht_, mmgr_) : HAL_NOP;
    gft_exact_match_flow_entry_id_ht_ ? ht::destroy(gft_exact_match_flow_entry_id_ht_, mmgr_) : HAL_NOP;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_cfg_db *
hal_cfg_db::factory(shmmgr *mmgr)
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
        if (cfg_db->init(mmgr) == false) {
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
// initialize DBs and caches that needs to be persisted across restarts/upgrades
//------------------------------------------------------------------------------
bool
hal_oper_db::init_pss(shmmgr *mmgr)
{
    hal_handle_id_ht_ = ht::factory(HAL_MAX_HANDLES,
                                    hal::hal_handle_id_get_key_func,
                                    hal::hal_handle_id_compute_hash_func,
                                    hal::hal_handle_id_compare_key_func,
                                    true, mmgr);
    HAL_ASSERT_RETURN((hal_handle_id_ht_ != NULL), false);

    ep_l2_ht_ = ht::factory(HAL_MAX_ENDPOINTS,
                            hal::ep_get_l2_key_func,
                            hal::ep_compute_l2_hash_func,
                            hal::ep_compare_l2_key_func,
                            true, mmgr);
    HAL_ASSERT_RETURN((ep_l2_ht_ != NULL), false);

    ep_l3_entry_ht_ = ht::factory(HAL_MAX_ENDPOINTS << 1,
                                  hal::ep_get_l3_key_func,
                                  hal::ep_compute_l3_hash_func,
                                  hal::ep_compare_l3_key_func,
                                  true, mmgr);
    HAL_ASSERT_RETURN((ep_l3_entry_ht_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// initialize DBs and caches that don't to be persisted (i.e., volatile) across
// restarts/upgrades and these will have to be rebuilt after restart/upgrade
//------------------------------------------------------------------------------
bool
hal_oper_db::init_vss(void)
{
    event_mgr_ = eventmgr::factory(HAL_MAX_EVENTS);
    HAL_ASSERT_RETURN((event_mgr_ != NULL), false);

    return true;
}

//------------------------------------------------------------------------------
// init() function to instantiate all the oper db init state
//------------------------------------------------------------------------------
bool
hal_oper_db::init(shmmgr *mmgr)
{
    mmgr_ = mmgr;
    HAL_ASSERT_RETURN((init_pss(mmgr) == true), false);
    HAL_ASSERT_RETURN((init_vss() == true), false);
    return true;
}

//------------------------------------------------------------------------------
// initialize the state that is not valid after a process restart
//------------------------------------------------------------------------------
void
hal_oper_db::init_on_restart(void) {
    init_vss();
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_oper_db::hal_oper_db()
{
    infra_vrf_handle_ = HAL_HANDLE_INVALID;
    event_mgr_ = NULL;
    hal_handle_id_ht_  = NULL;
    ep_l2_ht_ = NULL;
    ep_l3_entry_ht_ = NULL;
    memset(&mytep_ip, 0, sizeof(mytep_ip));
    mmgr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_oper_db::~hal_oper_db()
{
    hal_handle_id_ht_ ? ht::destroy(hal_handle_id_ht_, mmgr_) : HAL_NOP;
    ep_l2_ht_ ? ht::destroy(ep_l2_ht_, mmgr_) : HAL_NOP;
    ep_l3_entry_ht_ ? ht::destroy(ep_l3_entry_ht_, mmgr_) : HAL_NOP;
    event_mgr_ ? eventmgr::destroy(event_mgr_) : HAL_NOP;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_oper_db *
hal_oper_db::factory(shmmgr *mmgr)
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
        if (oper_db->init(mmgr) == false) {
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
// initialize DBs and caches that needs to be persisted across restarts/upgrades
//------------------------------------------------------------------------------
bool
hal_mem_db::init_pss(shmmgr *mmgr)
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

    return true;
}

//------------------------------------------------------------------------------
// initialize DBs and caches that don't to be persisted (i.e., volatile) across
// restarts/upgrades and these will have to be rebuilt after restart/upgrade
//------------------------------------------------------------------------------
bool
hal_mem_db::init_vss(void)
{
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
        slab::factory("ipv4_rule", HAL_SLAB_NWSEC_RULE,
                      sizeof(hal::nwsec_rule_t), 1024,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_RULE] != NULL), false);

    slabs_[HAL_SLAB_NWSEC_RULE] =
        slab::factory("ipv4_rule", HAL_SLAB_NWSEC_RULE,
                      sizeof(hal::ipv4_rule_t), 1024,
                      true, true, true);
    HAL_ASSERT_RETURN((slabs_[HAL_SLAB_NWSEC_RULE] != NULL), false);

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
// init() function to instantiate all the mem db init state
//------------------------------------------------------------------------------
bool
hal_mem_db::init(shmmgr *mmgr)
{
    mmgr_ = mmgr;
    HAL_ASSERT_RETURN((init_pss(mmgr) == true), false);
    HAL_ASSERT_RETURN((init_vss() == true), false);
    return true;
}

//------------------------------------------------------------------------------
// initialize the state that is not valid after a process restart
//------------------------------------------------------------------------------
void
hal_mem_db::init_on_restart(void) {
    init_vss();
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_mem_db::hal_mem_db()
{
    memset(slabs_, 0, sizeof(slabs_));
    mmgr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_mem_db::~hal_mem_db()
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
hal_mem_db *
hal_mem_db::factory(shmmgr *mmgr)
{
    void          *mem;
    hal_mem_db    *mem_db;

    if (mmgr) {
        mem = mmgr->alloc(sizeof(hal_mem_db), 4, true);
    } else {
        mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_mem_db));
    }

    if (mem) {
        mem_db = new(mem) hal_mem_db();
        if (mem_db->init(mmgr) == false) {
            mem_db->~hal_mem_db();
            if (mmgr) {
                mmgr->free(mem);
            } else {
                HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
            }
            return NULL;
        }
        return mem_db;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// destroy method
//------------------------------------------------------------------------------
void
hal_mem_db::destroy(hal_mem_db *mem_db)
{
    shmmgr    *mmgr;

    if (!mem_db) {
        return;
    }
    mmgr = mem_db->mmgr_;
    mem_db->~hal_mem_db();
    if (mmgr) {
        mmgr->free(mem_db);
    } else {
        HAL_FREE(HAL_MEM_ALLOC_INFRA, mem_db);
    }
}

//----------------------------------------------------------------------------
// gives the slab of a slab id
//----------------------------------------------------------------------------
slab *
hal_mem_db::get_slab(hal_slab_t slab_id)
{
    if (slab_id >= HAL_SLAB_PI_MAX) {
        return NULL;
    }
    return TO_SLAB_PTR(slabs_[slab_id]);
}

#if 0
//------------------------------------------------------------------------------
// init() function to instantiate all HAL init state
//------------------------------------------------------------------------------
bool
hal_state::init(shmmgr *mmgr)
{
    mmgr_ = mmgr;
    cfg_db_ = hal_cfg_db::factory(mmgr);
    oper_db_ = hal_oper_db::factory(mmgr);
    mem_db_ = hal_mem_db::factory(mmgr);
    if (mmgr) {
        api_stats_ =
            (hal_stats_t *)mmgr->alloc(sizeof(hal_stats_t) * HAL_API_MAX,
                                       4, true);
    } else {
        api_stats_ = (hal_stats_t *)HAL_CALLOC(HAL_MEM_ALLOC_API_STATS,
                                        sizeof(hal_stats_t) * HAL_API_MAX);
    }

    HAL_ASSERT_GOTO((cfg_db_ && oper_db_ && mem_db_ && api_stats_), cleanup);
    return true;

cleanup:

    if (cfg_db_) {
        hal_cfg_db::destroy(cfg_db_);
    }
    if (oper_db_) {
        hal_oper_db::destroy(oper_db_);
    }
    if (mem_db_) {
        hal_mem_db::destroy(mem_db_);
    }
    if (api_stats_) {
        if (mmgr) {
            mmgr->free(api_stats_);
        } else {
            HAL_FREE(HAL_MEM_ALLOC_API_STATS, api_stats_);
        }
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
    catalog_ = NULL;
    api_stats_ = NULL;
    mmgr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state::~hal_state() {
    if (cfg_db_) {
        hal_cfg_db::destroy(cfg_db_);
    }
    if (oper_db_) {
        hal_oper_db::destroy(oper_db_);
    }
    if (mem_db_) {
        hal_mem_db::destroy(mem_db_);
    }
    if (api_stats_) {
        if (mmgr_) {
            mmgr_->free(api_stats_);
        } else {
            HAL_FREE(HAL_MEM_ALLOC_API_STATS, api_stats_);
        }
    }
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_state *
hal_state::factory(shmmgr *mmgr)
{
    void         *mem;
    hal_state    *state;

    if (mmgr) {
        mem = mmgr->alloc(sizeof(hal_state), 4, true);
    } else {
        mem = HAL_CALLOC(HAL_MEM_ALLOC_INFRA, sizeof(hal_state));
    }

    if (mem) {
        state = new (mem) hal_state();
        if (state->init(mmgr) == false) {
            state->~hal_state();
            if (mmgr) {
                mmgr->free(mem);
            } else {
                HAL_FREE(HAL_MEM_ALLOC_INFRA, mem);
            }
            return NULL;
        }
        return state;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// destroy method
//------------------------------------------------------------------------------
void
hal_state::destroy(hal_state *state)
{
    shmmgr    *mmgr;

    if (!state) {
        return;
    }
    mmgr = state->mmgr_;
    state->~hal_state();
    if (mmgr) {
        mmgr->free(state);
    } else {
        HAL_FREE(HAL_MEM_ALLOC_INFRA, state);
    }
}
#endif

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
    if (mem_db_) {
        hal_mem_db::destroy(mem_db_);
    }
    if (api_stats_) {
        if (mmgr_) {
            mmgr_->free(api_stats_);
        } else {
            HAL_FREE(HAL_MEM_ALLOC_API_STATS, api_stats_);
        }
    }
}

//------------------------------------------------------------------------------
// constructor method
//------------------------------------------------------------------------------
hal_state::hal_state(shmmgr *mmgr)
{
    mmgr_ = mmgr;
    cfg_db_ = NULL;
    oper_db_ = NULL;
    mem_db_ = NULL;
    api_stats_ = NULL;
    catalog_ = NULL;

    cfg_db_ = hal_cfg_db::factory(mmgr);
    HAL_ASSERT_GOTO(cfg_db_, error);

    oper_db_ = hal_oper_db::factory(mmgr);
    HAL_ASSERT_GOTO(oper_db_, error);

    mem_db_ = hal_mem_db::factory(mmgr);
    HAL_ASSERT_GOTO(mem_db_, error);
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
hal_state::init_on_restart(void) {
    catalog_ = NULL;
    cfg_db_->init_on_restart();
    oper_db_->init_on_restart();
    mem_db_->init_on_restart();
}

//------------------------------------------------------------------------------
// preserve the state in the class in the given persistent memory and return
// the number of bytes used up
//------------------------------------------------------------------------------
uint64_t
hal_state::preserve_state(void *mem, uint32_t len)
{
    return 0;
}

//------------------------------------------------------------------------------
// restore the state from given memory segment
//------------------------------------------------------------------------------
uint64_t
hal_state::restore_state(void *mem)
{
    return 0;
}

//------------------------------------------------------------------------------
// one time memory related initialization for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_mem_init (bool shm_mode)
{
    bool    h2s_exists = false, h3s_exists = false;

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
        g_hal_state = new hal_state();
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
        g_hal_state->init_on_restart();
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

        // TODO:
        // 1. restore state
        // 2. nuke HAL_SERIALIZED_STATE_STORE (i.e., g_h3s_shmmgr->destroy())
    } else {
        // coming up in shm mode, but no existing state store
        HAL_TRACE_DEBUG("Creating new HAL state store in shared memory");
        g_h2s_shmmgr =
            shmmgr::factory(HAL_STATE_STORE, HAL_STATE_STORE_SIZE,
                            sdk::lib::SHM_CREATE_ONLY,
                            (void *)HAL_STATE_STORE_VADDR);
        HAL_ABORT(g_h2s_shmmgr != NULL);

        // instantiate HAL state in regular linux memory
        //g_hal_state = hal_state::factory(g_h2s_shmmgr);
        fixed_managed_shared_memory    *fm_shm_mgr;
        fm_shm_mgr = (fixed_managed_shared_memory *)g_h2s_shmmgr->mmgr();
        g_hal_state = fm_shm_mgr->construct<hal_state>(HAL_STATE_OBJ)(g_h2s_shmmgr);
        //g_hal_state = new hal_state(g_h2s_shmmgr);
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

    case HAL_SLAB_RULEDB:
        g_hal_state->ruledb_slab()->free(elem);
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
