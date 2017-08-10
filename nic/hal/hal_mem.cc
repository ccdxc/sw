#include <hal.hpp>
#include <hal_state.hpp>
#include <tenant.hpp>
#include <l2segment.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <session.hpp>
#include <nwsec.hpp>
#include <tlscb.hpp>
#include <tcpcb.hpp>
#include <qos.hpp>
 
namespace hal {

class hal_state    *g_hal_state;

//------------------------------------------------------------------------------
// init() function to instantiate all the slabs
//------------------------------------------------------------------------------
bool
hal_state::init(void)
{
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

    lif_queue_slab_ = slab::factory("LIF_QUEUE", HAL_SLAB_LIF_QUEUE,
                              sizeof(hal::lif_queue_t), 8,
                             false, true, true, true);
    HAL_ASSERT_RETURN((lif_queue_slab_ != NULL), false);

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

    if_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_INTERFACES);
    HAL_ASSERT_RETURN((if_hwid_idxr_ != NULL), false);

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

    nwsec_profile_slab_ = slab::factory("SecProfile", HAL_SLAB_SECURITY_PROFILE,
                                        sizeof(hal::nwsec_profile_t), 4, false,
                                        false, true, true);
    HAL_ASSERT_RETURN((nwsec_profile_slab_ != NULL), false);

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
    lif_queue_slab_ = NULL;
    lif_id_ht_ = NULL;
    lif_hal_handle_ht_ = NULL;

    if_slab_ = NULL;
    if_hwid_idxr_ = NULL;
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
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state::~hal_state()
{
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
    lif_queue_slab_ ? delete lif_queue_slab_ : HAL_NOP;
    lif_id_ht_ ? delete lif_id_ht_ : HAL_NOP;
    lif_hal_handle_ht_ ? delete lif_hal_handle_ht_ : HAL_NOP;

    if_slab_ ? delete if_slab_ : HAL_NOP;
    if_hwid_idxr_ ? delete if_hwid_idxr_ : HAL_NOP;
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

}

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

    case HAL_SLAB_LIF_QUEUE:
        g_hal_state->lif_queue_slab()->free_(elem);
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

    case HAL_SLAB_BUF_POOL:
        g_hal_state->buf_pool_slab()->free_(elem);
        break;

    case HAL_SLAB_QUEUE:
        g_hal_state->queue_slab()->free_(elem);
        break;

    case HAL_SLAB_POLICER:
        g_hal_state->policer_slab()->free_(elem);
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
