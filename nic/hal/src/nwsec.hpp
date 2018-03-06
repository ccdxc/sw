// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __NWSEC_HPP__
#define __NWSEC_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/pd.hpp"

#define HAL_NWSEC_INVALID_SG_ID        uint32_t (~0)
using sdk::lib::ht_ctxt_t;

using kh::SecurityProfileKeyHandle;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileRequestMsg;
using nwsec::SecurityProfileStatus;
using nwsec::SecurityProfileResponse;
using nwsec::SecurityProfileResponseMsg;
using nwsec::SecurityProfileDeleteRequest;
using nwsec::SecurityProfileDeleteResponse;
using nwsec::SecurityProfileDeleteRequestMsg;
using nwsec::SecurityProfileDeleteResponseMsg;
using nwsec::SecurityProfileGetRequest;
using nwsec::SecurityProfileGetRequestMsg;
using nwsec::SecurityProfileStats;
using nwsec::SecurityProfileGetResponse;
using nwsec::SecurityProfileGetResponseMsg;

namespace hal {

typedef struct nwsec_profile_s {
    hal_spinlock_t        slock;                  // lock to protect this structure
    nwsec_profile_id_t    profile_id;             // profile id

    uint32_t              session_idle_timeout;
    uint32_t              tcp_cnxn_setup_timeout;
    uint32_t              tcp_close_timeout;
    uint32_t              tcp_half_closed_timeout;
    uint32_t              tcp_drop_timeout;
    uint32_t              udp_drop_timeout;
    uint32_t              icmp_drop_timeout;
    uint32_t              drop_timeout;
    uint32_t              tcp_timeout;
    uint32_t              udp_timeout;
    uint32_t              icmp_timeout;

    uint32_t              cnxn_tracking_en:1;
    uint32_t              ipsg_en:1;
    uint32_t              tcp_rtt_estimate_en:1;
    uint32_t              ip_normalization_en:1;
    uint32_t              tcp_normalization_en:1;
    uint32_t              icmp_normalization_en:1;
    uint32_t              ip_reassembly_en:1;

    uint32_t              ip_ttl_change_detect_en:1;
    uint32_t              ip_src_guard_en:1;
    uint32_t              ip_rsvd_flags_action:2;
    uint32_t              ip_df_action:2;
    uint32_t              ip_options_action:2;
    uint32_t              ip_invalid_len_action:2;
    uint32_t              ip_spoof_pkt_drop:1;
    uint32_t              ip_loose_src_routing_pkt_drop:1;
    uint32_t              ip_malformed_option_pkt_drop:1;
    uint32_t              ip_record_route_option_pkt_drop:1;
    uint32_t              ip_strict_src_routing_pkt_drop:1;
    uint32_t              ip_ts_option_pkt_drop:1;
    uint32_t              ip_unknown_option_pkt_drop:1;
    uint32_t              ip_stream_id_option_pkt_drop:1;
    uint32_t              ip_rsvd_fld_set_pkt_drop:1;
    uint32_t              ip_clear_df_bit:1;
    uint32_t              ip_normalize_ttl:8;

    uint32_t              ipv6_anycast_src_drop:1;
    uint32_t              ipv6_v4_compatible_addr_drop:1;
    uint32_t              ipv6_needless_ip_frag_hdr_drop:1;
    uint32_t              ipv6_invalid_options_pkt_drop:1;
    uint32_t              ipv6_rsvd_fld_set_pkt_drop:1;
    uint32_t              ipv6_rtg_hdr_pkt_drop:1;
    uint32_t              ipv6_dst_options_hdr_pkt_drop:1;
    uint32_t              ipv6_hop_by_hop_options_pkt_drop:1;


    uint32_t              icmp_redirect_msg_drop:1;
    uint32_t              icmp_deprecated_msgs_drop:1;
    uint32_t              icmp_invalid_code_action:2;
    uint32_t              icmp_dst_unreach_ignore_payload:1;
    uint32_t              icmp_param_prblm_ignore_payload:1;
    uint32_t              icmp_pkt_too_big_ignore_payload:1;
    uint32_t              icmp_redirect_ignore_payload:1;
    uint32_t              icmp_time_exceed_ignore_payload:1;
    uint32_t              icmp_error_drop:1;
    uint32_t              icmp_fragments_drop:1;
    uint32_t              icmp_large_pkt_drop:1;
    uint32_t              icmp_ping_zero_id_drop:1;
    uint32_t              icmp_need_frag_suppress:1;
    uint32_t              icmp_time_exceed_suppress:1;
    uint32_t              icmpv6_large_msg_mtu_small_drop:1;


    uint32_t              tcp_non_syn_first_pkt_drop:1;
    uint32_t              tcp_split_handshake_drop:1;
    uint32_t              tcp_rsvd_flags_action:2;
    uint32_t              tcp_unexpected_mss_action:2;
    uint32_t              tcp_unexpected_win_scale_action:2;
    uint32_t              tcp_unexpected_sack_perm_action:2;
    uint32_t              tcp_urg_ptr_not_set_action:2;
    uint32_t              tcp_urg_flag_not_set_action:2;
    uint32_t              tcp_urg_payload_missing_action:2;
    uint32_t              tcp_rst_with_data_action:2;
    uint32_t              tcp_data_len_gt_mss_action:2;
    uint32_t              tcp_data_len_gt_win_size_action:2;
    uint32_t              tcp_unexpected_ts_option_action:2;
    uint32_t              tcp_unexpected_sack_option_action:2;
    uint32_t              tcp_unexpected_echo_ts_action:2;
    uint32_t              tcp_ts_not_present_drop:1;
    uint32_t              tcp_invalid_flags_drop:1;
    uint32_t              tcp_nonsyn_noack_drop:1;
    uint32_t              tcp_syn_with_data_drop:1;
    uint32_t              tcp_syn_ack_with_data_drop:1;
    uint32_t              tcp_overlapping_segments_drop:1;
    uint32_t              tcp_strip_timestamp_option:1;
    uint32_t              tcp_conn_track_bypass_window_err:1;
    uint32_t              tcp_urg_flag_ptr_clear:1;
    uint32_t              tcp_mss;

    uint32_t              multicast_src_drop:1;

    hal_handle_t          hal_handle;             // HAL allocated handle

    // operational state of nwsec profile
    dllist_ctxt_t         vrf_list_head;

    // PD state
    void                  *pd;                    // all PD specific state
} __PACK__ nwsec_profile_t;

typedef struct nwsec_create_app_ctxt_s {
} __PACK__ nwsec_create_app_ctxt_t;

typedef struct nwsec_update_app_ctxt_s {
    bool        ipsg_changed;                       // ipsg changed
    bool        nwsec_changed;                      // Any field changed

    // valid for ipsg_changed
    // uint32_t    ipsg_en:1;                          // new ipsg_en value

    // valid for any change
    SecurityProfileSpec *spec;
} __PACK__ nwsec_update_app_ctxt_t;

// max. number of security profiles supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NWSEC_PROFILES                       256

static inline void 
nwsec_profile_lock(nwsec_profile_t *nwsec_profile, const char *fname, 
                   int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking nwsec_profile:{} from {}:{}:{}", 
                    __FUNCTION__, nwsec_profile->profile_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&nwsec_profile->slock);
}

static inline void 
nwsec_profile_unlock(nwsec_profile_t *nwsec_profile, const char *fname, 
                     int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking nwsec_profile:{} from {}:{}:{}", 
                    __FUNCTION__, nwsec_profile->profile_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&nwsec_profile->slock);
}

// allocate a security profile instance
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

// initialize a security profile instance
static inline nwsec_profile_t *
nwsec_profile_init (nwsec_profile_t *sec_prof)
{
    if (!sec_prof) {
        return NULL;
    }
    memset(sec_prof, 0, sizeof(nwsec_profile_t));


    HAL_SPINLOCK_INIT(&sec_prof->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    sdk::lib::dllist_reset(&sec_prof->vrf_list_head);

    return sec_prof;
}

// allocate and initialize a security profile instance
static inline nwsec_profile_t *
nwsec_profile_alloc_init (void)
{
    return nwsec_profile_init(nwsec_profile_alloc());
}

// free security profile instance
static inline hal_ret_t
nwsec_profile_free (nwsec_profile_t *sec_prof)
{
    HAL_SPINLOCK_DESTROY(&sec_prof->slock);
    hal::delay_delete_to_slab(HAL_SLAB_SECURITY_PROFILE, sec_prof);
    return HAL_RET_OK;
}

// find a security profile instance by its id
static inline nwsec_profile_t *
find_nwsec_profile_by_id (nwsec_profile_id_t profile_id)
{
    // return (nwsec_profile_t *)g_hal_state->nwsec_profile_id_ht()->lookup(&profile_id);
    hal_handle_id_ht_entry_t    *entry;
    nwsec_profile_t             *sec_prof;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        nwsec_profile_id_ht()->lookup(&profile_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_SECURITY_PROFILE);

        sec_prof = (nwsec_profile_t *)hal_handle_get_obj(entry->handle_id);
        return sec_prof;
    }
    return NULL;
}

// find a security profile instance by its handle
static inline nwsec_profile_t *
find_nwsec_profile_by_handle (hal_handle_t handle)
{
    // return (nwsec_profile_t *)g_hal_state->nwsec_profile_hal_handle_ht()->lookup(&handle);
    // check for object type
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_SECURITY_PROFILE) {
        HAL_TRACE_DEBUG("{}:failed to find nwsec profile with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    // HAL_ASSERT(hal_handle->obj_id() == 
    //           HAL_OBJ_ID_SECURITY_PROFILE);
    return (nwsec_profile_t *)hal_handle_get_obj(handle); 
}

extern void *nwsec_profile_id_get_key_func(void *entry);
extern uint32_t nwsec_profile_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool nwsec_profile_id_compare_key_func(void *key1, void *key2);
nwsec_profile_t *nwsec_lookup_key_or_handle (const SecurityProfileKeyHandle& kh);

// Forward declarations
hal_ret_t securityprofile_create(nwsec::SecurityProfileSpec& spec,
                                 nwsec::SecurityProfileResponse *rsp);

hal_ret_t securityprofile_update(nwsec::SecurityProfileSpec& spec,
                                 nwsec::SecurityProfileResponse *rsp);

hal_ret_t securityprofile_delete(nwsec::SecurityProfileDeleteRequest& req,
                                 nwsec::SecurityProfileDeleteResponse *rsp);

hal_ret_t securityprofile_get(nwsec::SecurityProfileGetRequest& req,
                              nwsec::SecurityProfileGetResponseMsg *rsp);

using nwsec::DoSPolicySpec;
using nwsec::DoSPolicyStatus;
using nwsec::DoSPolicyRequestMsg;
using nwsec::DoSPolicyResponse;
using nwsec::DoSPolicyResponseMsg;
using nwsec::DoSPolicyDeleteRequest;
using nwsec::DoSPolicyDeleteRequestMsg;
using nwsec::DoSPolicyDeleteResponse;
using nwsec::DoSPolicyDeleteResponseMsg;
using nwsec::DoSPolicyGetRequest;
using nwsec::DoSPolicyStats;
using nwsec::DoSPolicyGetResponse;
using nwsec::DoSPolicyGetRequestMsg;
using nwsec::DoSPolicyGetResponseMsg;
using nwsec::IngressDoSPolicy;
using nwsec::EgressDoSPolicy;
using nwsec::DoSProtectionSpec;
using nwsec::DoSService;

typedef struct dos_service_s {
    uint8_t         ip_proto;       // IP protocol
    bool            is_icmp;        // Is ICMP msg type ?
    union {
        uint16_t    dport;          // TCP or UDP port, 0 for other protos
        struct {
            uint8_t    icmp_msg_type;   // ICMP msg type
            uint8_t    icmp_msg_code;   // ICMP code
        } __PACK__;
    };
} __PACK__ dos_service_t;

typedef struct dos_session_limits_s {
    uint32_t    max_sessions;       // max. no of sessions
    uint32_t    blocking_timeout;   // cool off period once session count
                                    // comes below the above limit
} __PACK__ dos_session_limits_t;

// DoS aggregate policer
typedef struct dos_policer_s {
    uint32_t    bytes_per_sec;      // max. bytes per sec
    uint32_t    peak_rate;          // bytes-per-sec
    uint32_t    burst_size;         // bytes
} __PACK__ dos_policer_t;

typedef struct dos_flood_limits_s {
    uint32_t restrict_pps;          // restrict pps limit
    uint32_t restrict_burst_pps;    // restrict pps burst
    uint32_t restrict_duration;     // restrict action duration (secs)

    uint32_t protect_pps;           // protect pps limit
    uint32_t protect_burst_pps;     // protect pps burst
    uint32_t protect_duration;      // protect action duration (secs)
} __PACK__ dos_flood_limits_t;

typedef struct dos_policy_prop_s {
    dos_service_t         service;                // service attachment point
    dos_policer_t         policer;                // DoS agg. policer config
    dos_flood_limits_t    tcp_syn_flood_limits;   // TCP syn flood limits
    dos_flood_limits_t    udp_flood_limits;       // UDP flood limits
    dos_flood_limits_t    icmp_flood_limits;      // ICMP flood limits
    dos_flood_limits_t    other_flood_limits;     // Other flood limits
    dos_session_limits_t  session_limits;         // Session limits
    uint32_t              session_setup_rate;     // 0 means no limits
    uint32_t              peer_sg_id;             // Peer security group
} __PACK__ dos_policy_prop_t;

typedef struct dos_policy_s {
    hal_spinlock_t        slock;                  // lock to protect this structure
    hal_handle_t          hal_handle;             // HAL allocated handle
    hal_handle_t          vrf_handle;          // vrf handle 
    
    bool                  ingr_pol_valid;
    bool                  egr_pol_valid;
    
    dos_policy_prop_t     ingress;
    dos_policy_prop_t     egress;

    // PD state
    void                  *pd;                    // all PD specific state
    // Security group list
    dllist_ctxt_t         sg_list_head;           // List of security groups
} __PACK__ dos_policy_t;

typedef struct dos_policy_sg_list_entry_s {
    int              sg_id;
    dllist_ctxt_t    dllist_ctxt;
} __PACK__ dos_policy_sg_list_entry_t;

typedef struct dos_policy_create_app_ctx_s {
} __PACK__ dos_policy_create_app_ctx_t;

typedef struct dos_policy_update_app_ctx_s {
} __PACK__ dos_policy_update_app_ctx_t;

// max. number of dos policies supported
#define HAL_MAX_DOS_POLICIES                       256

// allocate a dos policy instance
static inline dos_policy_t *
dos_policy_alloc (void)
{
    dos_policy_t    *dos_policy;

    dos_policy = (dos_policy_t *)g_hal_state->dos_policy_slab()->alloc();
    if (dos_policy == NULL) {
        return NULL;
    }
    return dos_policy;
}

// initialize a dos policy instance
static inline dos_policy_t *
dos_policy_init (dos_policy_t *dos_policy)
{
    if (!dos_policy) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&dos_policy->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    // dos_policy->ht_ctxt.reset();
    // dos_policy->hal_handle_ht_ctxt.reset();

    return dos_policy;
}

// allocate and initialize a dos policy instance
static inline dos_policy_t *
dos_policy_alloc_init (void)
{
    return dos_policy_init(dos_policy_alloc());
}

// free dos policy instance
static inline hal_ret_t
dos_policy_free (dos_policy_t *dos_policy)
{
    HAL_SPINLOCK_DESTROY(&dos_policy->slock);
    hal::delay_delete_to_slab(HAL_SLAB_DOS_POLICY, dos_policy);
    return HAL_RET_OK;
}

// find a dos policy instance by its handle
static inline dos_policy_t *
find_dos_policy_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    // check for object type
    HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
               HAL_OBJ_ID_DOS_POLICY);
    return (dos_policy_t *)hal_handle_get_obj(handle); 
}

extern uint32_t dos_policy_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool dos_policy_id_compare_key_func(void *key1, void *key2);
dos_policy_t *dos_policy_lookup_handle (const nwsec::DoSPolicySpec& dp);

hal_ret_t dospolicy_create(nwsec::DoSPolicySpec& spec,
                           nwsec::DoSPolicyResponse *rsp);

hal_ret_t dospolicy_update(nwsec::DoSPolicySpec& spec,
                           nwsec::DoSPolicyResponse *rsp);

hal_ret_t dospolicy_delete(nwsec::DoSPolicyDeleteRequest& req,
                           nwsec::DoSPolicyDeleteResponse *rsp);

hal_ret_t dospolicy_get(nwsec::DoSPolicyGetRequest& req,
                        nwsec::DoSPolicyGetResponseMsg *rsp);

using nwsec::SecurityGroupSpec;
using nwsec::SecurityGroupRequestMsg;
using nwsec::SecurityGroupStatus;
using nwsec::SecurityGroupResponse;
using nwsec::SecurityGroupGetResponse;
using kh::SecurityGroupKeyHandle;
using nwsec::SecurityGroupPolicySpec;
using nwsec::SecurityGroupPolicyRequestMsg;
using nwsec::SecurityGroupPolicyStatus;
using nwsec::SecurityGroupPolicyResponse;
using nwsec::SecurityGroupPolicyGetResponse;

hal_ret_t securitygroup_create(nwsec::SecurityGroupSpec& req,
                               nwsec::SecurityGroupResponse *rsp);

hal_ret_t securitygroup_update(nwsec::SecurityGroupSpec& req,
                               nwsec::SecurityGroupResponse *rsp);

hal_ret_t securitygroup_delete(nwsec::SecurityGroupDeleteRequest& req,
                               nwsec::SecurityGroupDeleteResponseMsg *rsp);

hal_ret_t securitygroup_get(nwsec::SecurityGroupGetRequest& req,
                             nwsec::SecurityGroupGetResponseMsg *rsp);

hal_ret_t securitygrouppolicy_create(nwsec::SecurityGroupPolicySpec& req,
                                       nwsec::SecurityGroupPolicyResponse *rsp);

hal_ret_t securitygrouppolicy_update(nwsec::SecurityGroupPolicySpec& req,
                                       nwsec::SecurityGroupPolicyResponse *rsp);

hal_ret_t securitygrouppolicy_delete(nwsec::SecurityGroupPolicyDeleteRequest& req,
                                       nwsec::SecurityGroupPolicyDeleteResponse *rsp);

hal_ret_t securitygrouppolicy_get(nwsec::SecurityGroupPolicyGetRequest& req,
                                    nwsec::SecurityGroupPolicyGetResponseMsg *rsp);

hal_ret_t
securitypolicy_create(nwsec::SecurityPolicySpec&      spec,
                       nwsec::SecurityPolicyResponse   *res);

hal_ret_t
securitypolicy_update(nwsec::SecurityPolicySpec&      spec,
                       nwsec::SecurityPolicyResponse   *res);

hal_ret_t
securitypolicy_delete(nwsec::SecurityPolicyDeleteRequest&   req,
                       nwsec::SecurityPolicyDeleteResponse   *res);

hal_ret_t
securitypolicy_get(nwsec::SecurityPolicyGetRequest&      req,
                    nwsec::SecurityPolicyGetResponseMsg   *res);

}    // namespace hal

#endif    // __NWSEC_HPP__

