//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/sfw/alg_utils/alg_db.hpp"

namespace hal {

//----------------------------------------------------------------------------
// get security profile's key
//----------------------------------------------------------------------------
void *
nwsec_profile_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    nwsec_profile_t             *nwsec = NULL;

    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nwsec = (nwsec_profile_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(nwsec->profile_id);
}

//----------------------------------------------------------------------------
// hash table profile_id key size
//----------------------------------------------------------------------------
uint32_t
nwsec_profile_id_key_size ()
{
    return sizeof(nwsec_profile_id_t);
}

//----------------------------------------------------------------------------
// allocate a security profile instance
//----------------------------------------------------------------------------
static inline nwsec_profile_t *
nwsec_profile_alloc (void)
{
    nwsec_profile_t    *sec_prof;

    sec_prof = (nwsec_profile_t *)g_hal_state->nwsec_profile_slab()->alloc();
    if (sec_prof == NULL) {
        return NULL;
    }
    return sec_prof;
}

//----------------------------------------------------------------------------
// initialize a security profile instance
//----------------------------------------------------------------------------
static inline nwsec_profile_t *
nwsec_profile_init (nwsec_profile_t *sec_prof)
{
    if (!sec_prof) {
        return NULL;
    }
    memset(sec_prof, 0, sizeof(nwsec_profile_t));


    SDK_SPINLOCK_INIT(&sec_prof->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    sdk::lib::dllist_reset(&sec_prof->vrf_list_head);

    return sec_prof;
}

//----------------------------------------------------------------------------
// allocate and initialize a security profile instance
//----------------------------------------------------------------------------
static inline nwsec_profile_t *
nwsec_profile_alloc_init (void)
{
    return nwsec_profile_init(nwsec_profile_alloc());
}

//----------------------------------------------------------------------------
// free security profile instance
//----------------------------------------------------------------------------
static inline hal_ret_t
nwsec_profile_free (nwsec_profile_t *sec_prof)
{
    SDK_SPINLOCK_DESTROY(&sec_prof->slock);
    hal::delay_delete_to_slab(HAL_SLAB_SECURITY_PROFILE, sec_prof);
    return HAL_RET_OK;
}

static inline hal_ret_t
nwsec_profile_cleanup (nwsec_profile_t *sec_prof)
{
    nwsec_profile_free(sec_prof);

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// find a security profile instance by its id
//----------------------------------------------------------------------------
nwsec_profile_t *
find_nwsec_profile_by_id (nwsec_profile_id_t profile_id)
{
    // return (nwsec_profile_t *)g_hal_state->nwsec_profile_id_ht()->lookup(&profile_id);
    hal_handle_id_ht_entry_t    *entry;
    nwsec_profile_t             *sec_prof;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        nwsec_profile_id_ht()->lookup(&profile_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_SECURITY_PROFILE);

        sec_prof = (nwsec_profile_t *)hal_handle_get_obj(entry->handle_id);
        return sec_prof;
    }
    return NULL;
}

//----------------------------------------------------------------------------
// find a security profile instance by its handle
//----------------------------------------------------------------------------
nwsec_profile_t *
find_nwsec_profile_by_handle (hal_handle_t handle)
{
    // return (nwsec_profile_t *)g_hal_state->nwsec_profile_hal_handle_ht()->lookup(&handle);
    // check for object type
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_SECURITY_PROFILE) {
        HAL_TRACE_DEBUG("Failed to find nwsec profile with handle {}", handle);
        return NULL;
    }
    return (nwsec_profile_t *)hal_handle_get_obj(handle);
}

//------------------------------------------------------------------------------
// Deprecated: lookup nwsec from key or handle. Use nwsec_lookup_key_or_handle
//------------------------------------------------------------------------------
hal_ret_t
find_nwsec_by_key_or_handle (const SecurityProfileKeyHandle& kh,
                             nwsec_profile_t **nwsec)
{
    *nwsec = NULL;
    if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileId) {
        *nwsec = find_nwsec_profile_by_id(kh.profile_id());
        if (*nwsec == NULL) {
            return HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        }
    } else if (kh.key_or_handle_case() ==
               SecurityProfileKeyHandle::kProfileHandle) {
        *nwsec = find_nwsec_profile_by_handle(kh.profile_handle());
        if (*nwsec == NULL) {
            return HAL_RET_HANDLE_INVALID;
        }
    } else {
        return HAL_RET_KEY_HANDLE_NOT_SPECIFIED;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// lookup nwsec from key or handle
//------------------------------------------------------------------------------
nwsec_profile_t *
nwsec_prof_lookup_key_or_handle (const SecurityProfileKeyHandle& kh)
{
    nwsec_profile_t *nwsec = NULL;

    if (kh.key_or_handle_case() == SecurityProfileKeyHandle::kProfileId) {
        nwsec = find_nwsec_profile_by_id(kh.profile_id());
    } else if (kh.key_or_handle_case() ==
        SecurityProfileKeyHandle::kProfileHandle) {
        nwsec = find_nwsec_profile_by_handle(kh.profile_handle());
    }

    return nwsec;
}

//------------------------------------------------------------------------------
// insert a nwsec to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nwsec_add_to_db (nwsec_profile_t *nwsec, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding to nwsec profile db");
    // allocate an entry to establish mapping from security profile id to handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from security profile id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->nwsec_profile_id_ht()->
        insert_with_key(&nwsec->profile_id, entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add nwsec id to handle mapping, "
                      "err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // TODO: Check if this is the right place
    nwsec->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a nwsec from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
nwsec_del_from_db (nwsec_profile_t *nwsec)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("removing from nwsec profile id hash table");
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->nwsec_profile_id_ht()->
        remove(&nwsec->profile_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// function that reads from update spec and updates what changed
//------------------------------------------------------------------------------
#define NWSEC_SPEC_CHECK(fname) nwsec->fname != (uint32_t)spec.fname()
static inline hal_ret_t
nwsec_handle_update (SecurityProfileSpec& spec, nwsec_profile_t *nwsec,
                     nwsec_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;

    if (NWSEC_SPEC_CHECK(session_idle_timeout) ||
        NWSEC_SPEC_CHECK(tcp_cnxn_setup_timeout) ||
        NWSEC_SPEC_CHECK(tcp_close_timeout) ||
        NWSEC_SPEC_CHECK(tcp_half_closed_timeout) ||
        NWSEC_SPEC_CHECK(tcp_drop_timeout) ||
        NWSEC_SPEC_CHECK(udp_drop_timeout) ||
        NWSEC_SPEC_CHECK(icmp_drop_timeout) ||
        NWSEC_SPEC_CHECK(drop_timeout) ||
        NWSEC_SPEC_CHECK(tcp_timeout) ||
        NWSEC_SPEC_CHECK(udp_timeout) ||
        NWSEC_SPEC_CHECK(icmp_timeout) ||
        NWSEC_SPEC_CHECK(cnxn_tracking_en) ||
        NWSEC_SPEC_CHECK(ipsg_en) ||
        NWSEC_SPEC_CHECK(tcp_rtt_estimate_en) ||
        NWSEC_SPEC_CHECK(ip_normalization_en) ||
        NWSEC_SPEC_CHECK(tcp_normalization_en) ||
        NWSEC_SPEC_CHECK(icmp_normalization_en) ||
        NWSEC_SPEC_CHECK(ip_reassembly_en) ||
        NWSEC_SPEC_CHECK(ip_ttl_change_detect_en) ||
        NWSEC_SPEC_CHECK(ip_src_guard_en) ||
        NWSEC_SPEC_CHECK(ip_rsvd_flags_action) ||
        NWSEC_SPEC_CHECK(ip_df_action) ||
        NWSEC_SPEC_CHECK(ip_options_action) ||
        NWSEC_SPEC_CHECK(ip_invalid_len_action) ||
        NWSEC_SPEC_CHECK(ip_spoof_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_loose_src_routing_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_malformed_option_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_record_route_option_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_strict_src_routing_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_ts_option_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_unknown_option_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_stream_id_option_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_rsvd_fld_set_pkt_drop) ||
        NWSEC_SPEC_CHECK(ip_clear_df_bit) ||
        NWSEC_SPEC_CHECK(ip_normalize_ttl) ||
        NWSEC_SPEC_CHECK(ipv6_anycast_src_drop) ||
        NWSEC_SPEC_CHECK(ipv6_v4_compatible_addr_drop) ||
        NWSEC_SPEC_CHECK(ipv6_needless_ip_frag_hdr_drop) ||
        NWSEC_SPEC_CHECK(ipv6_invalid_options_pkt_drop) ||
        NWSEC_SPEC_CHECK(ipv6_rsvd_fld_set_pkt_drop) ||
        NWSEC_SPEC_CHECK(ipv6_rtg_hdr_pkt_drop) ||
        NWSEC_SPEC_CHECK(ipv6_dst_options_hdr_pkt_drop) ||
        NWSEC_SPEC_CHECK(ipv6_hop_by_hop_options_pkt_drop) ||
        NWSEC_SPEC_CHECK(icmp_invalid_code_action) ||
        NWSEC_SPEC_CHECK(icmp_deprecated_msgs_drop) ||
        NWSEC_SPEC_CHECK(icmp_redirect_msg_drop) ||
        NWSEC_SPEC_CHECK(icmp_dst_unreach_ignore_payload) ||
        NWSEC_SPEC_CHECK(icmp_param_prblm_ignore_payload) ||
        NWSEC_SPEC_CHECK(icmp_pkt_too_big_ignore_payload) ||
        NWSEC_SPEC_CHECK(icmp_redirect_ignore_payload) ||
        NWSEC_SPEC_CHECK(icmp_time_exceed_ignore_payload) ||
        NWSEC_SPEC_CHECK(icmp_error_drop) ||
        NWSEC_SPEC_CHECK(icmp_fragments_drop) ||
        NWSEC_SPEC_CHECK(icmp_large_pkt_drop) ||
        NWSEC_SPEC_CHECK(icmp_ping_zero_id_drop) ||
        NWSEC_SPEC_CHECK(icmp_need_frag_suppress) ||
        NWSEC_SPEC_CHECK(icmp_time_exceed_suppress) ||
        NWSEC_SPEC_CHECK(icmpv6_large_msg_mtu_small_drop) ||
        NWSEC_SPEC_CHECK(tcp_non_syn_first_pkt_drop) ||
        NWSEC_SPEC_CHECK(tcp_split_handshake_drop) ||
        NWSEC_SPEC_CHECK(tcp_rsvd_flags_action) ||
        NWSEC_SPEC_CHECK(tcp_unexpected_mss_action) ||
        NWSEC_SPEC_CHECK(tcp_unexpected_win_scale_action) ||
        NWSEC_SPEC_CHECK(tcp_unexpected_sack_perm_action) ||
        NWSEC_SPEC_CHECK(tcp_urg_ptr_not_set_action) ||
        NWSEC_SPEC_CHECK(tcp_urg_flag_not_set_action) ||
        NWSEC_SPEC_CHECK(tcp_urg_payload_missing_action) ||
        NWSEC_SPEC_CHECK(tcp_rst_with_data_action) ||
        NWSEC_SPEC_CHECK(tcp_data_len_gt_mss_action) ||
        NWSEC_SPEC_CHECK(tcp_data_len_gt_win_size_action) ||
        NWSEC_SPEC_CHECK(tcp_unexpected_ts_option_action) ||
        NWSEC_SPEC_CHECK(tcp_unexpected_sack_option_action) ||
        NWSEC_SPEC_CHECK(tcp_unexpected_echo_ts_action) ||
        NWSEC_SPEC_CHECK(tcp_ts_not_present_drop) ||
        NWSEC_SPEC_CHECK(tcp_invalid_flags_drop) ||
        NWSEC_SPEC_CHECK(tcp_nonsyn_noack_drop) ||
        NWSEC_SPEC_CHECK(tcp_syn_with_data_drop) ||
        NWSEC_SPEC_CHECK(tcp_syn_ack_with_data_drop) ||
        NWSEC_SPEC_CHECK(tcp_overlapping_segments_drop) ||
        NWSEC_SPEC_CHECK(tcp_strip_timestamp_option) ||
        NWSEC_SPEC_CHECK(tcp_conn_track_bypass_window_err) ||
        NWSEC_SPEC_CHECK(tcp_urg_flag_ptr_clear) ||
        NWSEC_SPEC_CHECK(tcp_normalize_mss) ||
        NWSEC_SPEC_CHECK(multicast_src_drop) ||
        NWSEC_SPEC_CHECK(tcp_half_open_session_limit) ||
        NWSEC_SPEC_CHECK(udp_active_session_limit) ||
        NWSEC_SPEC_CHECK(icmp_active_session_limit) ||
        NWSEC_SPEC_CHECK(other_active_session_limit) ||
        NWSEC_SPEC_CHECK(policy_enforce_en) ||
        NWSEC_SPEC_CHECK(flow_learn_en)) {
                app_ctxt->nwsec_changed = true;
    }

    if (NWSEC_SPEC_CHECK(ipsg_en)) {
        HAL_TRACE_DEBUG("ipsg changed {} => {}",
                        nwsec->ipsg_en, spec.ipsg_en());
        app_ctxt->ipsg_changed = true;
    }

    return ret;
}

//------------------------------------------------------------------------------
// function that reads from spec and populates PI structure
//------------------------------------------------------------------------------
#define NWSEC_SPEC_ASSIGN(fname) (sec_prof->fname = spec.fname())
// initialize security profile object from the config spec
static inline void
nwsec_profile_init_from_spec (nwsec_profile_t *sec_prof,
                              const nwsec::SecurityProfileSpec& spec,
                              bool update_profile_id = false)
{

    if (update_profile_id) {
        sec_prof->profile_id = spec.key_or_handle().profile_id();
    }

    NWSEC_SPEC_ASSIGN(session_idle_timeout);
    NWSEC_SPEC_ASSIGN(tcp_cnxn_setup_timeout);
    NWSEC_SPEC_ASSIGN(tcp_close_timeout);
    NWSEC_SPEC_ASSIGN(tcp_half_closed_timeout);
    NWSEC_SPEC_ASSIGN(tcp_drop_timeout);
    NWSEC_SPEC_ASSIGN(udp_drop_timeout);
    NWSEC_SPEC_ASSIGN(icmp_drop_timeout);
    NWSEC_SPEC_ASSIGN(drop_timeout);
    NWSEC_SPEC_ASSIGN(tcp_timeout);
    NWSEC_SPEC_ASSIGN(udp_timeout);
    NWSEC_SPEC_ASSIGN(icmp_timeout);

    NWSEC_SPEC_ASSIGN(cnxn_tracking_en);
    NWSEC_SPEC_ASSIGN(ipsg_en);
    NWSEC_SPEC_ASSIGN(tcp_rtt_estimate_en);
    NWSEC_SPEC_ASSIGN(ip_normalization_en);
    NWSEC_SPEC_ASSIGN(tcp_normalization_en);
    NWSEC_SPEC_ASSIGN(icmp_normalization_en);
    NWSEC_SPEC_ASSIGN(ip_reassembly_en);
    NWSEC_SPEC_ASSIGN(ip_ttl_change_detect_en);
    NWSEC_SPEC_ASSIGN(ip_src_guard_en);

    NWSEC_SPEC_ASSIGN(ip_rsvd_flags_action);
    NWSEC_SPEC_ASSIGN(ip_df_action);
    NWSEC_SPEC_ASSIGN(ip_options_action);
    NWSEC_SPEC_ASSIGN(ip_invalid_len_action);
    NWSEC_SPEC_ASSIGN(ip_spoof_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_loose_src_routing_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_malformed_option_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_record_route_option_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_strict_src_routing_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_ts_option_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_unknown_option_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_stream_id_option_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_rsvd_fld_set_pkt_drop);
    NWSEC_SPEC_ASSIGN(ip_clear_df_bit);
    NWSEC_SPEC_ASSIGN(ip_normalize_ttl);


    NWSEC_SPEC_ASSIGN(ipv6_anycast_src_drop);
    NWSEC_SPEC_ASSIGN(ipv6_v4_compatible_addr_drop);
    NWSEC_SPEC_ASSIGN(ipv6_needless_ip_frag_hdr_drop);
    NWSEC_SPEC_ASSIGN(ipv6_invalid_options_pkt_drop);

    NWSEC_SPEC_ASSIGN(ipv6_rsvd_fld_set_pkt_drop);
    NWSEC_SPEC_ASSIGN(ipv6_rtg_hdr_pkt_drop);
    NWSEC_SPEC_ASSIGN(ipv6_dst_options_hdr_pkt_drop);
    NWSEC_SPEC_ASSIGN(ipv6_hop_by_hop_options_pkt_drop);



    NWSEC_SPEC_ASSIGN(icmp_invalid_code_action);
    NWSEC_SPEC_ASSIGN(icmp_deprecated_msgs_drop);
    NWSEC_SPEC_ASSIGN(icmp_redirect_msg_drop);
    NWSEC_SPEC_ASSIGN(icmp_dst_unreach_ignore_payload);
    NWSEC_SPEC_ASSIGN(icmp_param_prblm_ignore_payload);
    NWSEC_SPEC_ASSIGN(icmp_pkt_too_big_ignore_payload);
    NWSEC_SPEC_ASSIGN(icmp_redirect_ignore_payload);
    NWSEC_SPEC_ASSIGN(icmp_time_exceed_ignore_payload);
    NWSEC_SPEC_ASSIGN(icmp_error_drop);
    NWSEC_SPEC_ASSIGN(icmp_fragments_drop);
    NWSEC_SPEC_ASSIGN(icmp_large_pkt_drop);
    NWSEC_SPEC_ASSIGN(icmp_ping_zero_id_drop);
    NWSEC_SPEC_ASSIGN(icmp_need_frag_suppress);
    NWSEC_SPEC_ASSIGN(icmp_time_exceed_suppress);
    NWSEC_SPEC_ASSIGN(icmpv6_large_msg_mtu_small_drop);

    NWSEC_SPEC_ASSIGN(tcp_non_syn_first_pkt_drop);
    NWSEC_SPEC_ASSIGN(tcp_split_handshake_drop);
    NWSEC_SPEC_ASSIGN(tcp_rsvd_flags_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_mss_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_win_scale_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_sack_perm_action);
    NWSEC_SPEC_ASSIGN(tcp_urg_ptr_not_set_action);
    NWSEC_SPEC_ASSIGN(tcp_urg_flag_not_set_action);
    NWSEC_SPEC_ASSIGN(tcp_urg_payload_missing_action);
    NWSEC_SPEC_ASSIGN(tcp_rst_with_data_action);
    NWSEC_SPEC_ASSIGN(tcp_data_len_gt_mss_action);
    NWSEC_SPEC_ASSIGN(tcp_data_len_gt_win_size_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_ts_option_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_sack_option_action);
    NWSEC_SPEC_ASSIGN(tcp_unexpected_echo_ts_action);
    NWSEC_SPEC_ASSIGN(tcp_ts_not_present_drop);
    NWSEC_SPEC_ASSIGN(tcp_invalid_flags_drop);
    NWSEC_SPEC_ASSIGN(tcp_nonsyn_noack_drop);
    NWSEC_SPEC_ASSIGN(tcp_syn_with_data_drop);
    NWSEC_SPEC_ASSIGN(tcp_syn_ack_with_data_drop);
    NWSEC_SPEC_ASSIGN(tcp_overlapping_segments_drop);
    NWSEC_SPEC_ASSIGN(tcp_strip_timestamp_option);
    NWSEC_SPEC_ASSIGN(tcp_conn_track_bypass_window_err);
    NWSEC_SPEC_ASSIGN(tcp_urg_flag_ptr_clear);
    NWSEC_SPEC_ASSIGN(tcp_normalize_mss);

    NWSEC_SPEC_ASSIGN(multicast_src_drop);
    NWSEC_SPEC_ASSIGN(policy_enforce_en);
    NWSEC_SPEC_ASSIGN(flow_learn_en);

    NWSEC_SPEC_ASSIGN(tcp_half_open_session_limit);
    NWSEC_SPEC_ASSIGN(udp_active_session_limit);
    NWSEC_SPEC_ASSIGN(icmp_active_session_limit);
    NWSEC_SPEC_ASSIGN(other_active_session_limit);
    return;
}

//------------------------------------------------------------------------------
// validate function for nwsec profile create
//------------------------------------------------------------------------------
static hal_ret_t
validate_nwsec_create (SecurityProfileSpec& spec, SecurityProfileResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("nwsec id and handle not set in request");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            SecurityProfileKeyHandle::kProfileId) {
        HAL_TRACE_ERR("nwsec id not set in request");
        ret = HAL_RET_NWSEC_ID_INVALID;
        goto end;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// main Create cb
// - PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
nwsec_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_nwsec_profile_create_args_t  pd_nwsec_args = { 0 };
    dllist_ctxt_t                       *lnode = NULL;
    dhl_entry_t                         *dhl_entry = NULL;
    nwsec_profile_t                     *nwsec = NULL;
    pd::pd_func_args_t                  pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nwsec_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nwsec = (nwsec_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("create add cb for nwsec_prof_id:{}",
                    nwsec->profile_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_nwsec_profile_create_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    pd_func_args.pd_nwsec_profile_create = &pd_nwsec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create nwsec pd, err : {}", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as nwsec_create_add_cb() was a success
//      a. Create the flood list
//      b. Add to nwsec id hash table
//------------------------------------------------------------------------------
hal_ret_t
nwsec_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;
    hal_handle_t                hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);
    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("create commit cb for nwsec_prof_id:{}",
                    nwsec->profile_id);


    // 1. a. Add to nwsec id hash table
    ret = nwsec_add_to_db(nwsec, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add nwsec {} to db, err : {}",
                      nwsec->profile_id, ret);
        goto end;
    }

end:

    return ret;
}

static hal_ret_t
nwsec_create_abort_cleanup (nwsec_profile_t *nwsec, hal_handle_t hal_handle)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_nwsec_profile_delete_args_t  pd_nwsec_args = { 0 };
    pd::pd_func_args_t                  pd_func_args = {0};

    // 1. delete call to PD
    if (nwsec->pd) {
        pd::pd_nwsec_profile_delete_args_init(&pd_nwsec_args);
        pd_nwsec_args.nwsec_profile = nwsec;
        pd_func_args.pd_nwsec_profile_delete = &pd_nwsec_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete nwsec pd, err : {}", ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    return ret;
}

//------------------------------------------------------------------------------
// nwsec_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
nwsec_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                           ret = HAL_RET_OK;
    dllist_ctxt_t                       *lnode = NULL;
    dhl_entry_t                         *dhl_entry = NULL;
    nwsec_profile_t                     *nwsec = NULL;
    hal_handle_t                        hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("Create abort cb {}", nwsec->profile_id);
    nwsec_create_abort_cleanup(nwsec, hal_handle);
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
nwsec_prepare_rsp (SecurityProfileResponse *rsp, hal_ret_t ret,
                   hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_profile_status()->set_profile_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Security Profile dump
//-----------------------------------------------------------------------------
#define NWSEC_SPEC_FIELD_PRINT(fname) buf.write(#fname":{}, ", spec.fname())
static hal_ret_t
security_profile_spec_dump (SecurityProfileSpec& spec)
{
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug)  {
        return HAL_RET_OK;
    }

    buf.write("Security Profile Spec: ");
    if (spec.has_key_or_handle()) {
        auto kh = spec.key_or_handle();
        if (kh.key_or_handle_case() ==
            SecurityProfileKeyHandle::kProfileId) {
            buf.write("profile_id:{}, ", kh.profile_id());
        } else if (kh.key_or_handle_case() ==
                   SecurityProfileKeyHandle::kProfileHandle) {
            buf.write("profile_hdl:{}, ", kh.profile_handle());
        }
    } else {
        buf.write("profile_id_hdl:NULL, ");
    }

    // buf.write("cnxn_tracking_en: {}, ", spec.cnxn_tracking_en());
    NWSEC_SPEC_FIELD_PRINT(session_idle_timeout);
    NWSEC_SPEC_FIELD_PRINT(tcp_cnxn_setup_timeout);
    NWSEC_SPEC_FIELD_PRINT(tcp_close_timeout);
    NWSEC_SPEC_FIELD_PRINT(tcp_half_closed_timeout);
    NWSEC_SPEC_FIELD_PRINT(tcp_drop_timeout);
    NWSEC_SPEC_FIELD_PRINT(udp_drop_timeout);
    NWSEC_SPEC_FIELD_PRINT(icmp_drop_timeout);
    NWSEC_SPEC_FIELD_PRINT(drop_timeout);
    NWSEC_SPEC_FIELD_PRINT(tcp_timeout);
    NWSEC_SPEC_FIELD_PRINT(udp_timeout);
    NWSEC_SPEC_FIELD_PRINT(icmp_timeout);
    buf.write("\n");
    NWSEC_SPEC_FIELD_PRINT(cnxn_tracking_en);
    NWSEC_SPEC_FIELD_PRINT(ipsg_en);
    NWSEC_SPEC_FIELD_PRINT(tcp_rtt_estimate_en);
    NWSEC_SPEC_FIELD_PRINT(ip_normalization_en);
    NWSEC_SPEC_FIELD_PRINT(tcp_normalization_en);
    NWSEC_SPEC_FIELD_PRINT(icmp_normalization_en);
    NWSEC_SPEC_FIELD_PRINT(ip_reassembly_en);
    NWSEC_SPEC_FIELD_PRINT(ip_ttl_change_detect_en);
    NWSEC_SPEC_FIELD_PRINT(ip_src_guard_en);
    buf.write("\n");
    NWSEC_SPEC_FIELD_PRINT(ip_rsvd_flags_action);
    NWSEC_SPEC_FIELD_PRINT(ip_df_action);
    NWSEC_SPEC_FIELD_PRINT(ip_options_action);
    NWSEC_SPEC_FIELD_PRINT(ip_invalid_len_action);
    NWSEC_SPEC_FIELD_PRINT(ip_spoof_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_loose_src_routing_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_malformed_option_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_record_route_option_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_strict_src_routing_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_ts_option_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_unknown_option_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_stream_id_option_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_rsvd_fld_set_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ip_clear_df_bit);
    NWSEC_SPEC_FIELD_PRINT(ip_normalize_ttl);
    buf.write("\n");
    NWSEC_SPEC_FIELD_PRINT(ipv6_anycast_src_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_v4_compatible_addr_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_needless_ip_frag_hdr_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_invalid_options_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_rsvd_fld_set_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_rtg_hdr_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_dst_options_hdr_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(ipv6_hop_by_hop_options_pkt_drop);
    buf.write("\n");
    NWSEC_SPEC_FIELD_PRINT(icmp_invalid_code_action);
    NWSEC_SPEC_FIELD_PRINT(icmp_deprecated_msgs_drop);
    NWSEC_SPEC_FIELD_PRINT(icmp_redirect_msg_drop);
    NWSEC_SPEC_FIELD_PRINT(icmp_dst_unreach_ignore_payload);
    NWSEC_SPEC_FIELD_PRINT(icmp_param_prblm_ignore_payload);
    NWSEC_SPEC_FIELD_PRINT(icmp_pkt_too_big_ignore_payload);
    NWSEC_SPEC_FIELD_PRINT(icmp_redirect_ignore_payload);
    NWSEC_SPEC_FIELD_PRINT(icmp_time_exceed_ignore_payload);
    NWSEC_SPEC_FIELD_PRINT(icmp_error_drop);
    NWSEC_SPEC_FIELD_PRINT(icmp_fragments_drop);
    NWSEC_SPEC_FIELD_PRINT(icmp_large_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(icmp_ping_zero_id_drop);
    NWSEC_SPEC_FIELD_PRINT(icmp_need_frag_suppress);
    NWSEC_SPEC_FIELD_PRINT(icmp_time_exceed_suppress);
    NWSEC_SPEC_FIELD_PRINT(icmpv6_large_msg_mtu_small_drop);
    buf.write("\n");
    NWSEC_SPEC_FIELD_PRINT(tcp_non_syn_first_pkt_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_split_handshake_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_rsvd_flags_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_unexpected_mss_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_unexpected_win_scale_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_unexpected_sack_perm_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_urg_ptr_not_set_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_urg_flag_not_set_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_urg_payload_missing_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_rst_with_data_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_data_len_gt_mss_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_data_len_gt_win_size_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_unexpected_ts_option_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_unexpected_sack_option_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_unexpected_echo_ts_action);
    NWSEC_SPEC_FIELD_PRINT(tcp_ts_not_present_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_invalid_flags_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_nonsyn_noack_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_syn_with_data_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_syn_ack_with_data_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_overlapping_segments_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_strip_timestamp_option);
    NWSEC_SPEC_FIELD_PRINT(tcp_conn_track_bypass_window_err);
    NWSEC_SPEC_FIELD_PRINT(tcp_urg_flag_ptr_clear);
    NWSEC_SPEC_FIELD_PRINT(tcp_normalize_mss);
    NWSEC_SPEC_FIELD_PRINT(multicast_src_drop);
    NWSEC_SPEC_FIELD_PRINT(tcp_half_open_session_limit);
    NWSEC_SPEC_FIELD_PRINT(udp_active_session_limit);
    NWSEC_SPEC_FIELD_PRINT(icmp_active_session_limit);
    NWSEC_SPEC_FIELD_PRINT(other_active_session_limit);
    NWSEC_SPEC_FIELD_PRINT(policy_enforce_en);
    NWSEC_SPEC_FIELD_PRINT(flow_learn_en);
    buf.write("\n");

    HAL_TRACE_DEBUG("{}", buf.c_str());
    return ret;
}

// create an instance of security profile
hal_ret_t
securityprofile_create (SecurityProfileSpec& spec,
                        SecurityProfileResponse *rsp)
{
    hal_ret_t                       ret;
    nwsec_profile_t                 *sec_prof = NULL;
    nwsec_create_app_ctxt_t         app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("Creating security profile id {}",
                    spec.key_or_handle().profile_id());

    // dump spec
    security_profile_spec_dump(spec);

    // validate the request message
    ret = validate_nwsec_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("validation failed. ret: {}", ret);
        goto end;
    }

    // check if nwsec profile exists already, and reject if one is found
    if (find_nwsec_profile_by_id(spec.key_or_handle().profile_id())) {
        HAL_TRACE_ERR("Failed to create nwsec profile, "
                      "profile {} exists already",
                      spec.key_or_handle().profile_id());
        ret =  HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate the nwsec profile
    sec_prof = nwsec_profile_alloc_init();
    if (sec_prof == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("unable to allocate handle/memory ret: {}", ret);
        goto end;
    }

    // consume the config
    nwsec_profile_init_from_spec(sec_prof, spec, true);

    // allocate hal handle id
    sec_prof->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_PROFILE);
    if (sec_prof->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle {}",
                      sec_prof->profile_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add. nothing to populate in app ctxt
    dhl_entry.handle = sec_prof->hal_handle;
    dhl_entry.obj = sec_prof;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(sec_prof->hal_handle, &cfg_ctxt,
                             nwsec_create_add_cb,
                             nwsec_create_commit_cb,
                             nwsec_create_abort_cb,
                             nwsec_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
        if (sec_prof) {
            nwsec_profile_cleanup(sec_prof);
            sec_prof = NULL;
        }
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_CREATE_SUCCESS);
    }

    nwsec_prepare_rsp(rsp, ret,
                      sec_prof ? sec_prof->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//------------------------------------------------------------------------------
// validate nwsec update request
//------------------------------------------------------------------------------
hal_ret_t
validate_nwsec_update (SecurityProfileSpec & spec, SecurityProfileResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Handle ipsg change. Triggers vrfs to update ipsg
//------------------------------------------------------------------------------
hal_ret_t
nwsec_handle_ipsg_change (nwsec_profile_t *nwsec, nwsec_profile_t *nwsec_clone)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *curr, *next;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (!nwsec_clone) {
        return ret;
    }

    HAL_TRACE_DEBUG("sec_prof_id:{}", nwsec_clone->profile_id);
    // clone didnt have the lists moved yet. They happen in commit cb.
    // So walking vrf list in the original.
    dllist_for_each_safe(curr, next, &nwsec->vrf_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        auto ten = vrf_lookup_by_handle(entry->handle_id);
        if (!ten) {
            HAL_TRACE_ERR("unable to find vrf with handle:{}",
                          entry->handle_id);
            continue;
        }
        ret = vrf_handle_nwsec_update(ten, nwsec_clone);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to process nwsec_clone "
                          "update by vrf{}", ten->vrf_id);
        }
        // Walk each ep belonging to vrf
        g_hal_state->ep_l2_ht()->walk(ep_handle_ipsg_change_cb, ten);
    }

    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update new l2seg properties
//------------------------------------------------------------------------------
hal_ret_t
nwsec_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_nwsec_profile_update_args_t  pd_nwsec_args = { 0 };
    dllist_ctxt_t                       *lnode = NULL;
    dhl_entry_t                         *dhl_entry = NULL;
    nwsec_profile_t                     *nwsec = NULL, *nwsec_clone = NULL;
    nwsec_update_app_ctxt_t             *app_ctxt = NULL;
    pd::pd_func_args_t                  pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (nwsec_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    nwsec_clone = (nwsec_profile_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb for nwsec_prof_id:{}",
                    nwsec->profile_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_nwsec_profile_update_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    pd_nwsec_args.clone_profile = nwsec_clone;
    pd_func_args.pd_nwsec_profile_update = &pd_nwsec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update nwsec pd, err : {}", ret);
        goto end;
    }

    // IPSG changes, trigger vrfs.
    if (app_ctxt->ipsg_changed) {
        // Clone already has new value. Uncomment this
        // nwsec_clone->ipsg_en = app_ctxt->ipsg_en;
        ret = nwsec_handle_ipsg_change(nwsec, nwsec_clone);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to handle ipsg "
                          "change, err:{}", ret);
            goto end;
        }
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
nwsec_make_clone (nwsec_profile_t *nwsec, nwsec_profile_t **nwsec_clone,
                  SecurityProfileSpec& spec)
{
    pd::pd_nwsec_profile_make_clone_args_t args;
    pd::pd_func_args_t pd_func_args = {0};

    *nwsec_clone = nwsec_profile_alloc_init();
    memcpy(*nwsec_clone, nwsec, sizeof(nwsec_profile_t));

    // After clone always reset lists
    dllist_reset(&(*nwsec_clone)->vrf_list_head);

    args.nwsec_profile = nwsec;
    args.clone_profile = *nwsec_clone;
    pd_func_args.pd_nwsec_profile_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_MAKE_CLONE, &pd_func_args);

    // Keep new values in the clone
    nwsec_profile_init_from_spec(*nwsec_clone, spec);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD nwsec.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
nwsec_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                             ret = HAL_RET_OK;
    pd::pd_nwsec_profile_mem_free_args_t  pd_nwsec_args = { 0 };
    dllist_ctxt_t                       *lnode = NULL;
    dhl_entry_t                         *dhl_entry = NULL;
    nwsec_profile_t                     *nwsec = NULL, *nwsec_clone = NULL;
    pd::pd_func_args_t                  pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (nwsec_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    nwsec_clone = (nwsec_profile_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb {}",
                    nwsec->profile_id);

    // move lists
    dllist_move(&nwsec_clone->vrf_list_head, &nwsec->vrf_list_head);

    // Remove: clone already have new values in nwsec_make_clone
#if 0
    // update clone with new values
    if (app_ctxt->nwsec_changed) {
        nwsec_profile_init_from_spec(nwsec_clone, *app_ctxt->spec);
    }
#endif

    // free PD
    pd::pd_nwsec_profile_mem_free_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    pd_func_args.pd_nwsec_profile_mem_free = &pd_nwsec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_MEM_FREE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete nwsec pd, err : {}", ret);
    }

    // free PI
    nwsec_profile_free(nwsec);

    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Free the clones
//------------------------------------------------------------------------------
hal_ret_t
nwsec_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;
    pd::pd_nwsec_profile_mem_free_args_t pd_nwsec_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // assign clone as we are trying to free only the clone
    nwsec = (nwsec_profile_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update commit cb for nwsec_prof_id:{}",
                    nwsec->profile_id);

    // free PD
    pd::pd_nwsec_profile_mem_free_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    pd_func_args.pd_nwsec_profile_mem_free = &pd_nwsec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_MEM_FREE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete nwsec pd, err : {}", ret);
    }

    // free PI
    nwsec_profile_free(nwsec);

    return ret;
}

hal_ret_t
nwsec_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

// update a security profile instance
hal_ret_t
securityprofile_update (nwsec::SecurityProfileSpec& spec,
                        nwsec::SecurityProfileResponse *rsp)
{
    hal_ret_t                      ret       = HAL_RET_OK;
    nwsec_profile_t                *sec_prof = NULL;
    cfg_op_ctxt_t                  cfg_ctxt  = { 0 };
    dhl_entry_t                    dhl_entry = { 0 };
    nwsec_update_app_ctxt_t        app_ctxt  = { 0 };
    const SecurityProfileKeyHandle &kh       = spec.key_or_handle();

    HAL_TRACE_DEBUG("Rcvd nwsec {} update, handle:{}",
                    spec.key_or_handle().profile_id(),
                    spec.key_or_handle().profile_handle());

    security_profile_spec_dump(spec);

    // validate the request message
    ret = validate_nwsec_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nwsec update validation failed, err : {}", ret);
        goto end;
    }

    // lookup this security profile
    ret = find_nwsec_by_key_or_handle(spec.key_or_handle(), &sec_prof);
    if (!sec_prof) {
        HAL_TRACE_ERR("Failed to find nwsec, id {}, handle {}",
                      kh.profile_id(), kh.profile_handle());
        ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        goto end;
    }

    // check what changed
    ret = nwsec_handle_update(spec, sec_prof, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nwsec check update failed, err : {}", ret);
        goto end;
    }

    if (!app_ctxt.nwsec_changed) {
        HAL_TRACE_ERR("no change in nwsec update: noop");
        goto end;
    }

    nwsec_make_clone(sec_prof, (nwsec_profile_t **)&dhl_entry.cloned_obj, spec);

    // form ctxt and call infra update object
    app_ctxt.spec     = &spec;
    dhl_entry.handle  = sec_prof->hal_handle;
    dhl_entry.obj     = sec_prof;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(sec_prof->hal_handle, &cfg_ctxt,
                             nwsec_update_upd_cb,
                             nwsec_update_commit_cb,
                             nwsec_update_abort_cb,
                             nwsec_update_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_UPDATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_UPDATE_FAIL);
    }
    nwsec_prepare_rsp(rsp, ret,
                      sec_prof ? sec_prof->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: security profile update ");
    return ret;
}

//------------------------------------------------------------------------------
// validate nwsec delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_nwsec_delete_req (SecurityProfileDeleteRequest& req,
                           SecurityProfileDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t             *nwsec = NULL;
    pd::pd_nwsec_profile_delete_args_t pd_nwsec_args = { 0 };
    pd::pd_func_args_t          pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("Delete del cb {}", nwsec->profile_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_nwsec_profile_delete_args_init(&pd_nwsec_args);
    pd_nwsec_args.nwsec_profile = nwsec;
    pd_func_args.pd_nwsec_profile_delete = &pd_nwsec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_DELETE, &pd_func_args);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete nwsec pd, err : {}", ret);
    }
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as vrf_delete_del_cb() was a succcess
//      a. Delete from vrf id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    nwsec_profile_t                     *nwsec = NULL;
    hal_handle_t                hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec = (nwsec_profile_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("Delete commit cb {}", nwsec->profile_id);

    // a. remove from nwsec id hash table
    ret = nwsec_del_from_db(nwsec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del nwsec {} from db, err : {}",
                      nwsec->profile_id, ret);
        goto end;
    }

    // b. remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. free PI nwsec
    nwsec_profile_free(nwsec);

end:

    return ret;
}

//------------------------------------------------------------------------------
// nwsec delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// nwsec delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
nwsec_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate sec. profile delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_nwsec_delete (nwsec_profile_t *nwsec)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of l2segs
    if (dllist_count(&nwsec->vrf_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("vrfs still referring:");
        hal_print_handles_list(&nwsec->vrf_list_head);
    }

    return ret;
}

//------------------------------------------------------------------------------
// process a nwsec delete request
//------------------------------------------------------------------------------
hal_ret_t
securityprofile_delete (SecurityProfileDeleteRequest& req,
                        SecurityProfileDeleteResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    nwsec_profile_t                 *nwsec = NULL;
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };
    const SecurityProfileKeyHandle  &kh = req.key_or_handle();

    // validate the request message
    ret = validate_nwsec_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nwsec delete validation failed, err : {}", ret);
        goto end;
    }

    // lookup nwsec
    ret = find_nwsec_by_key_or_handle(kh, &nwsec);
    if (nwsec == NULL) {
        HAL_TRACE_ERR("Failed to find nwsec, id {}, handle {}",
                      kh.profile_id(), kh.profile_handle());
        ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("Deleting nwsec profile {}", nwsec->profile_id);

    // validate if there no objects referring this sec. profile
    ret = validate_nwsec_delete(nwsec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("sec prof {} delete validation failed, err : {}",
                      nwsec->profile_id, ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = nwsec->hal_handle;
    dhl_entry.obj = nwsec;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(nwsec->hal_handle, &cfg_ctxt,
                             nwsec_delete_del_cb,
                             nwsec_delete_commit_cb,
                             nwsec_delete_abort_cb,
                             nwsec_delete_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: security profile delete ");
    return ret;
}

hal_ret_t
nwsec_prof_process_get (nwsec_profile_t *sec_prof,
                        SecurityProfileGetResponse *rsp)
{
    hal_ret_t                         ret = HAL_RET_OK;
    nwsec::SecurityProfileSpec        *spec;
    pd::pd_nwsec_profile_get_args_t   args   = {0};
    pd::pd_func_args_t                pd_func_args = {0};

    // fill in the config spec of this profile
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_profile_id(sec_prof->profile_id);
    spec->set_cnxn_tracking_en(sec_prof->cnxn_tracking_en);
    spec->set_ipsg_en(sec_prof->ipsg_en);
    spec->set_tcp_rtt_estimate_en(sec_prof->tcp_rtt_estimate_en);
    spec->set_session_idle_timeout(sec_prof->session_idle_timeout);
    spec->set_tcp_cnxn_setup_timeout(sec_prof->tcp_cnxn_setup_timeout);
    spec->set_tcp_half_closed_timeout(sec_prof->tcp_half_closed_timeout);
    spec->set_tcp_close_timeout(sec_prof->tcp_close_timeout);
    spec->set_tcp_drop_timeout(sec_prof->tcp_drop_timeout);
    spec->set_udp_drop_timeout(sec_prof->udp_drop_timeout);
    spec->set_icmp_drop_timeout(sec_prof->icmp_drop_timeout);
    spec->set_drop_timeout(sec_prof->drop_timeout);
    spec->set_tcp_timeout(sec_prof->tcp_timeout);
    spec->set_udp_timeout(sec_prof->udp_timeout);
    spec->set_icmp_timeout(sec_prof->icmp_timeout);

    spec->set_ip_normalization_en(sec_prof->ip_normalization_en);
    spec->set_tcp_normalization_en(sec_prof->tcp_normalization_en);
    spec->set_icmp_normalization_en(sec_prof->icmp_normalization_en);

    spec->set_ip_ttl_change_detect_en(sec_prof->ip_ttl_change_detect_en);
    spec->set_ip_rsvd_flags_action(
             static_cast<nwsec::NormalizationAction>(sec_prof->ip_rsvd_flags_action));
    spec->set_ip_df_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_df_action));
    spec->set_ip_options_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_options_action));
    spec->set_ip_invalid_len_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_invalid_len_action));
    spec->set_ip_normalize_ttl(sec_prof->ip_normalize_ttl);

    spec->set_icmp_invalid_code_action(
          static_cast<nwsec::NormalizationAction>(sec_prof->icmp_invalid_code_action));
    spec->set_icmp_deprecated_msgs_drop(sec_prof->icmp_deprecated_msgs_drop);
    spec->set_icmp_redirect_msg_drop(sec_prof->icmp_redirect_msg_drop);

    spec->set_tcp_non_syn_first_pkt_drop(sec_prof->tcp_non_syn_first_pkt_drop);
    spec->set_tcp_split_handshake_drop(sec_prof->tcp_split_handshake_drop);
    spec->set_tcp_rsvd_flags_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rsvd_flags_action));
    spec->set_tcp_unexpected_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_mss_action));
    spec->set_tcp_unexpected_win_scale_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_win_scale_action));
    spec->set_tcp_unexpected_sack_perm_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_sack_perm_action));
    spec->set_tcp_urg_ptr_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_ptr_not_set_action));
    spec->set_tcp_urg_flag_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_flag_not_set_action));
    spec->set_tcp_urg_payload_missing_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_payload_missing_action));
    spec->set_tcp_rst_with_data_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rst_with_data_action));
    spec->set_tcp_data_len_gt_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_mss_action));
    spec->set_tcp_data_len_gt_win_size_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_win_size_action));
    spec->set_tcp_unexpected_ts_option_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_ts_option_action));
    spec->set_tcp_unexpected_sack_option_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_sack_option_action));
    spec->set_tcp_unexpected_echo_ts_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_echo_ts_action));
    spec->set_tcp_ts_not_present_drop(sec_prof->tcp_ts_not_present_drop);
    spec->set_tcp_invalid_flags_drop(sec_prof->tcp_invalid_flags_drop);
    spec->set_tcp_nonsyn_noack_drop(sec_prof->tcp_nonsyn_noack_drop);
    spec->set_tcp_normalize_mss(sec_prof->tcp_normalize_mss);

    spec->set_tcp_half_open_session_limit(sec_prof->tcp_half_open_session_limit);
    spec->set_udp_active_session_limit(sec_prof->udp_active_session_limit);
    spec->set_icmp_active_session_limit(sec_prof->icmp_active_session_limit);
    spec->set_other_active_session_limit(sec_prof->other_active_session_limit);

    // fill operational state of this profile
    rsp->mutable_status()->set_profile_handle(sec_prof->hal_handle);

    HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_GET_SUCCESS);


    // Get PD information
    args.nwsec_profile = sec_prof;
    args.rsp = rsp;
    pd_func_args.pd_nwsec_profile_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for nwsec profile id : {}. ret : {}",
                      sec_prof->profile_id, ret);
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// callback invoked from vrf hash table while processing vrf get request
//------------------------------------------------------------------------------
static bool
nwsec_prof_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t        *entry    = (hal_handle_id_ht_entry_t *)ht_entry;
    SecurityProfileGetResponseMsg   *rsp      = (SecurityProfileGetResponseMsg *)ctxt;
    SecurityProfileGetResponse      *response = rsp->add_response();
    nwsec_profile_t                 *nwsec    = NULL;

    nwsec = (nwsec_profile_t *)hal_handle_get_obj(entry->handle_id);
    nwsec_prof_process_get(nwsec, response);

    // return false here, so that we walk through all hash table entries.
    return false;
}

hal_ret_t
securityprofile_get (nwsec::SecurityProfileGetRequest& req,
                     nwsec::SecurityProfileGetResponseMsg *rsp)
{
    nwsec_profile_t *nwsec = NULL;

    if (!req.has_key_or_handle()) {
        g_hal_state->nwsec_profile_id_ht()->walk(nwsec_prof_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        nwsec = nwsec_prof_lookup_key_or_handle(kh);
        auto response = rsp->add_response();
        if (nwsec == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_GET_FAIL);
            return HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        } else {
            nwsec_prof_process_get(nwsec, response);
        }
    }

    HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_GET_SUCCESS);
    return HAL_RET_OK;
}

#if 0
hal_ret_t
securityprofile_get (nwsec::SecurityProfileGetRequest& req,
                     nwsec::SecurityProfileGetResponseMsg *resp)
{
    nwsec::SecurityProfileSpec    *spec;
    nwsec_profile_t               *sec_prof;
    nwsec::SecurityProfileGetResponse *rsp = resp->add_response();

    // key or handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_ID_INVALID);
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_GET_FAIL);
        return HAL_RET_INVALID_ARG;
    }

    // lookup this security profile
    find_nwsec_by_key_or_handle(req.key_or_handle(), &sec_prof);
    if (!sec_prof) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_GET_FAIL);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this profile
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_profile_id(sec_prof->profile_id);
    spec->set_cnxn_tracking_en(sec_prof->cnxn_tracking_en);
    spec->set_ipsg_en(sec_prof->ipsg_en);
    spec->set_tcp_rtt_estimate_en(sec_prof->tcp_rtt_estimate_en);
    spec->set_session_idle_timeout(sec_prof->session_idle_timeout);
    spec->set_tcp_cnxn_setup_timeout(sec_prof->tcp_cnxn_setup_timeout);
    spec->set_tcp_close_timeout(sec_prof->tcp_close_timeout);
    spec->set_tcp_drop_timeout(sec_prof->tcp_drop_timeout);
    spec->set_udp_drop_timeout(sec_prof->udp_drop_timeout);
    spec->set_icmp_drop_timeout(sec_prof->icmp_drop_timeout);
    spec->set_drop_timeout(sec_prof->drop_timeout);
    spec->set_tcp_timeout(sec_prof->tcp_timeout);
    spec->set_udp_timeout(sec_prof->udp_timeout);
    spec->set_icmp_timeout(sec_prof->icmp_timeout);

    spec->set_ip_normalization_en(sec_prof->ip_normalization_en);
    spec->set_tcp_normalization_en(sec_prof->tcp_normalization_en);
    spec->set_icmp_normalization_en(sec_prof->icmp_normalization_en);

    spec->set_ip_ttl_change_detect_en(sec_prof->ip_ttl_change_detect_en);
    spec->set_ip_rsvd_flags_action(
             static_cast<nwsec::NormalizationAction>(sec_prof->ip_rsvd_flags_action));
    spec->set_ip_df_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_df_action));
    spec->set_ip_options_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_options_action));
    spec->set_ip_invalid_len_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_invalid_len_action));
    spec->set_ip_normalize_ttl(sec_prof->ip_normalize_ttl);

    spec->set_icmp_invalid_code_action(
          static_cast<nwsec::NormalizationAction>(sec_prof->icmp_invalid_code_action));
    spec->set_icmp_deprecated_msgs_drop(sec_prof->icmp_deprecated_msgs_drop);
    spec->set_icmp_redirect_msg_drop(sec_prof->icmp_redirect_msg_drop);

    spec->set_tcp_non_syn_first_pkt_drop(sec_prof->tcp_non_syn_first_pkt_drop);
    spec->set_tcp_split_handshake_drop(sec_prof->tcp_split_handshake_drop);
    spec->set_tcp_rsvd_flags_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rsvd_flags_action));
    spec->set_tcp_unexpected_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_mss_action));
    spec->set_tcp_unexpected_win_scale_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_win_scale_action));
    spec->set_tcp_unexpected_sack_perm_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_sack_perm_action));
    spec->set_tcp_urg_ptr_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_ptr_not_set_action));
    spec->set_tcp_urg_flag_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_flag_not_set_action));
    spec->set_tcp_urg_payload_missing_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_payload_missing_action));
    spec->set_tcp_rst_with_data_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rst_with_data_action));
    spec->set_tcp_data_len_gt_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_mss_action));
    spec->set_tcp_data_len_gt_win_size_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_win_size_action));
    spec->set_tcp_unexpected_ts_option_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_ts_option_action));
    spec->set_tcp_unexpected_sack_option_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_sack_option_action));
    spec->set_tcp_unexpected_echo_ts_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_echo_ts_action));
    spec->set_tcp_ts_not_present_drop(sec_prof->tcp_ts_not_present_drop);
    spec->set_tcp_invalid_flags_drop(sec_prof->tcp_invalid_flags_drop);
    spec->set_tcp_nonsyn_noack_drop(sec_prof->tcp_nonsyn_noack_drop);
    spec->set_tcp_normalize_mss(sec_prof->tcp_normalize_mss);

    // fill operational state of this profile
    rsp->mutable_status()->set_profile_handle(sec_prof->hal_handle);

    HAL_API_STATS_INC(HAL_API_SECURITYPROFILE_GET_SUCCESS);

    // fill stats, if any, of this profile

    return HAL_RET_OK;
}
#endif

//-----------------------------------------------------------------------------
// Adds vrf into nwsec profile list
//-----------------------------------------------------------------------------
hal_ret_t
nwsec_prof_add_vrf (nwsec_profile_t *nwsec, vrf_t *vrf)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (nwsec == NULL || vrf == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Allocate the entry
    entry = (hal_handle_id_list_entry_t *)g_hal_state->
        hal_handle_id_list_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->handle_id = vrf->hal_handle;

    nwsec_profile_lock(nwsec, __FILENAME__, __LINE__, __func__);      // lock
    // Insert into the list
    sdk::lib::dllist_add(&nwsec->vrf_list_head, &entry->dllist_ctxt);
    nwsec_profile_unlock(nwsec, __FILENAME__, __LINE__, __func__);    // unlock

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Remove vrf from nwsec profile list
//-----------------------------------------------------------------------------
hal_ret_t
nwsec_prof_del_vrf (nwsec_profile_t *nwsec, vrf_t *vrf)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;


    nwsec_profile_lock(nwsec, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &nwsec->vrf_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == vrf->hal_handle) {
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);

            ret = HAL_RET_OK;
        }
    }
    nwsec_profile_unlock(nwsec, __FILENAME__, __LINE__, __func__);    // unlock
    return ret;
}

//-----------------------------------------------------------------------------
// stores nwsec state for updgrade
//-----------------------------------------------------------------------------
hal_ret_t
nwsec_prof_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    SecurityProfileGetResponse  nwsec_info;
    uint32_t                    serialized_state_sz;
    nwsec_profile_t             *sec_prof = (nwsec_profile_t *)obj;

    SDK_ASSERT((sec_prof != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this security profile (includes spec,
    // status & stats)
    nwsec_prof_process_get(sec_prof, &nwsec_info);
    serialized_state_sz = nwsec_info.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall Nwsec Profile {}, not enough room, "
                      "required size {}, available size {}",
                      sec_prof->profile_id, serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (nwsec_info.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize Nwsec Profile {}",
                      sec_prof->profile_id);
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled Nwsec profile {}, len {}",
                    sec_prof->profile_id, serialized_state_sz);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a nwsec prof's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
nwsec_prof_init_from_status (nwsec_profile_t *nwsec,
                             const SecurityProfileStatus& status)
{
    nwsec->hal_handle = status.profile_handle();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a vrf's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
nwsec_prof_init_from_stats (nwsec_profile_t *nwsec,
                            const SecurityProfileStats& stats)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// restore nwsec profile after upgrade
//------------------------------------------------------------------------------
static hal_ret_t
nwsec_prof_restore_add (nwsec_profile_t *nwsec,
                        const SecurityProfileGetResponse& nwsec_info)
{
    hal_ret_t                              ret;
    pd::pd_nwsec_profile_restore_args_t    pd_nwsec_prof_args = { 0 };
    pd::pd_func_args_t                     pd_func_args = {0};

    // restore pd state
    pd_nwsec_prof_args.nwsec_profile = nwsec;
    pd_nwsec_prof_args.status = &nwsec_info.status();
    pd_func_args.pd_nwsec_profile_restore = &pd_nwsec_prof_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NWSEC_PROF_RESTORE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore Nwsec profile {} pd, err : {}",
                      nwsec->profile_id, ret);
    }
    return ret;
}

//------------------------------------------------------------------------------
// commits nwsec profile after upgrade
//------------------------------------------------------------------------------
static hal_ret_t
nwsec_prof_restore_commit (nwsec_profile_t *nwsec,
                           const SecurityProfileGetResponse& nwsec_info)
{
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Committing Nwsec profile:{} restore", nwsec->profile_id);

    ret = nwsec_add_to_db(nwsec, nwsec->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore Nwsec profile {} to db, err : {}",
                      nwsec->profile_id, ret);
    }
    return ret;
}

//------------------------------------------------------------------------------
// aborts nwsec profile after upgrade
//------------------------------------------------------------------------------
static hal_ret_t
nwsec_prof_restore_abort (nwsec_profile_t *nwsec,
                          const SecurityProfileGetResponse& nwsec_info)
{
    HAL_TRACE_ERR("Aborting Nwsec Profile: {} restore", nwsec->profile_id);
    nwsec_create_abort_cleanup(nwsec, nwsec->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Nwsec prof's restore cb.
//  - restores nwsec prof to the PI and PD state before the upgrade
//------------------------------------------------------------------------------
uint32_t
nwsec_prof_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t                   ret;
    SecurityProfileGetResponse  nwsec_info;
    nwsec_profile_t             *nwsec = NULL;

    // de-serialize the object
    if (nwsec_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized vrf obj");
        SDK_ASSERT(0);
        return 0;
    }

    // allocate VRF obj from slab
    nwsec = nwsec_profile_alloc_init();
    if (nwsec == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init nwsec profile, err : {}", ret);
        return 0;
    }

    // initialize vrf attrs from its spec
    nwsec_profile_init_from_spec(nwsec, nwsec_info.spec(), true);
    nwsec_prof_init_from_status(nwsec, nwsec_info.status());
    nwsec_prof_init_from_stats(nwsec, nwsec_info.stats());

    // repopulate handle db
    hal_handle_insert(HAL_OBJ_ID_SECURITY_PROFILE, nwsec->hal_handle,
                      (void *)nwsec);

    ret = nwsec_prof_restore_add(nwsec, nwsec_info);
    if (ret != HAL_RET_OK) {
        nwsec_prof_restore_abort(nwsec, nwsec_info);
    }
    nwsec_prof_restore_commit(nwsec, nwsec_info);
    return 0;    // TODO: fix me
}

hal_ret_t
security_flow_gate_get(nwsec::SecurityFlowGateGetRequest&      req,
                       nwsec::SecurityFlowGateGetResponseMsg   *res)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = hal::plugins::alg_utils::walk_expected_flow(req, res);        
    if (ret != HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITY_FLOW_GATE_GET_FAIL);
        return HAL_RET_INVALID_ARG;
    }

    HAL_API_STATS_INC(HAL_API_SECURITY_FLOW_GATE_GET_SUCCESS);
    return HAL_RET_OK;
}

hal_ret_t
security_flow_gate_delete(nwsec::SecurityFlowGateDeleteRequest&      req,
                          nwsec::SecurityFlowGateDeleteResponseMsg   *res)
{
    return HAL_RET_OK;
}

}    // namespace hal
