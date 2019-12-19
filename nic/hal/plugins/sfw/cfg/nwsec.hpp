//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NWSEC_HPP__
#define __NWSEC_HPP__

#include "nic/include/base.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/include/pd.hpp"

#include "gen/proto/nwsec.pb.h"
#include "gen/proto/kh.pb.h"
#include "nic/sdk/lib/indexer/indexer.hpp"

#define HAL_NWSEC_INVALID_SG_ID        uint32_t (~0)
using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

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
using nwsec::SecurityFlowGateGetRequest;
using nwsec::SecurityFlowGateGetResponseMsg;
using nwsec::SecurityFlowGateDeleteRequest;
using nwsec::SecurityFlowGateDeleteResponseMsg;

namespace hal {

typedef struct nwsec_profile_s {
    sdk_spinlock_t        slock;                  // lock to protect this structure
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
    uint32_t              tcp_normalize_mss;

    uint32_t              multicast_src_drop:1;
    uint32_t              policy_enforce_en:1;
    uint32_t              flow_learn_en:1;

    uint32_t              tcp_half_open_session_limit;
    uint32_t              udp_active_session_limit;
    uint32_t              icmp_active_session_limit;
    uint32_t              other_active_session_limit;

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
    SDK_SPINLOCK_LOCK(&nwsec_profile->slock);
}

static inline void
nwsec_profile_unlock(nwsec_profile_t *nwsec_profile, const char *fname,
                     int lineno, const char *fxname)
{
    SDK_SPINLOCK_UNLOCK(&nwsec_profile->slock);
}

extern nwsec_profile_t *find_nwsec_profile_by_id(nwsec_profile_id_t profile_id);
extern nwsec_profile_t *find_nwsec_profile_by_handle(hal_handle_t handle);
nwsec_profile_t *nwsec_prof_lookup_key_or_handle (const SecurityProfileKeyHandle& kh);
extern void *nwsec_profile_id_get_key_func(void *entry);
extern uint32_t nwsec_profile_id_key_size(void);
hal_ret_t find_nwsec_by_key_or_handle(const SecurityProfileKeyHandle& kh,
                                      nwsec_profile_t **nwsec_profile);

// forward declarations
hal_ret_t securityprofile_create(nwsec::SecurityProfileSpec& spec,
                                 nwsec::SecurityProfileResponse *rsp);
hal_ret_t securityprofile_update(nwsec::SecurityProfileSpec& spec,
                                 nwsec::SecurityProfileResponse *rsp);
hal_ret_t securityprofile_delete(nwsec::SecurityProfileDeleteRequest& req,
                                 nwsec::SecurityProfileDeleteResponse *rsp);
hal_ret_t securityprofile_get(nwsec::SecurityProfileGetRequest& req,
                              nwsec::SecurityProfileGetResponseMsg *rsp);

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
hal_ret_t
security_flow_gate_get(nwsec::SecurityFlowGateGetRequest&      req,
                       nwsec::SecurityFlowGateGetResponseMsg   *res);
hal_ret_t
security_flow_gate_delete(nwsec::SecurityFlowGateDeleteRequest&      req,
                          nwsec::SecurityFlowGateDeleteResponseMsg   *res);

// upgrade APIs
hal_ret_t nwsec_prof_store_cb (void *obj, uint8_t *mem, uint32_t len,
                               uint32_t *mlen);
uint32_t nwsec_prof_restore_cb (void *obj, uint32_t len);

}    // namespace hal

#endif    // __NWSEC_HPP__

