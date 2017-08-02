#include <hal.hpp>
#include <hal_state.hpp>
#include <tenant.hpp>
#include <l2segment.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <session.hpp>
#include <nwsec.hpp>

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

    tcp_state_slab_ = slab::factory("TCP state", HAL_SLAB_TCP_STATE,
                                    sizeof(hal::tcp_state_t), 128,
                                    true, true, true, true);
    HAL_ASSERT_RETURN((tcp_state_slab_ != NULL), false);

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
    tcp_state_slab_ = NULL;
    flow_ht_ = NULL;
    session_id_ht_ = NULL;
    session_hal_handle_ht_ = NULL;

    nwsec_profile_slab_ = NULL;
    nwsec_profile_id_ht_ = NULL;
    nwsec_profile_hal_handle_ht_ = NULL;
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
    tcp_state_slab_ ? delete tcp_state_slab_ : HAL_NOP;
    session_slab_ ? delete session_slab_ : HAL_NOP;
    flow_ht_ ? delete flow_ht_ : HAL_NOP;
    session_id_ht_ ? delete session_id_ht_ : HAL_NOP;
    session_hal_handle_ht_ ? delete session_hal_handle_ht_ : HAL_NOP;

    nwsec_profile_slab_ ? delete nwsec_profile_slab_ : HAL_NOP;
    nwsec_profile_id_ht_ ? delete nwsec_profile_id_ht_ : HAL_NOP;
    nwsec_profile_hal_handle_ht_ ? delete nwsec_profile_hal_handle_ht_ : HAL_NOP;
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

    case HAL_SLAB_TCP_STATE:
        g_hal_state->tcp_state_slab()->free_(elem);
        break;

    case HAL_SLAB_SECURITY_PROFILE:
        g_hal_state->nwsec_profile_slab()->free_(elem);
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
