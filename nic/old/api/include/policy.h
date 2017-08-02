//------------------------------------------------------------------------------
// generic policy header file to capture global configuration
// TODO: move stuff specific to each service policy (like sfw/nat etc.) to
// specific header files, as applicable
//------------------------------------------------------------------------------
#ifndef __POLICY_H__
#define __POLICY_H__

#include <base.hpp>
#include <ip.hpp>
#include <l4.hpp>
#include <log.hpp>

//------------------------------------------------------------------------------
// Packet rormalization actions
//------------------------------------------------------------------------------
#define FW_NORM_ACTION_NONE                          0
#define FW_NORM_ACTION_DROP                          1
#define FW_NORM_ACTION_CPU                           2
#define FW_NORM_ACTION_EDIT                          3

//------------------------------------------------------------------------------
// L4 profile
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// profile id HAL_DEFAULT_L4_PROFILE_ID is reserved for the system, any
// vif/vrf/BD etc. not tied to any profile is subject to this system default L4
// profile. system default profile with id HAL_DEFAULT_L4_PROFILE_ID is created
// during init time
//------------------------------------------------------------------------------
#define HAL_DEFAULT_L4_PROFILE_ID                    0

typedef l4_profile_key_t    uint16_t;
typedef union l4_profile_key_handle_u {
    l4_profile_key_t        key;
    hal_handle_t            hndl;
} __PACK__ l4_profile_key_handle_t;

typedef struct l4_profile_key_handles_s {
    uint32_t                       num_khs;
    l4_profile_key_handle_t        khs[0];
} __PACK__ l4_profile_key_handles_t;

typedef struct l4_profile_data_s {
    uint32_t        conn_track_en:1;
    uint32_t        session_idle_timeout;
    uint8_t         tcp_connection_setup_timeout;   // 3-way handshake timeout
    uint8_t         tcp_close_timeout;              // timeout once FIN is seen on one side
    uint8_t         tcp_close_wait_timeout;         // timeout once FIN is seen on both sides
    uint32_t        max_sessions;                   // max. sessions on the NIC

    //uint32_t        ip_ttl_change_detect_en:1;    // TBD - always enabled !!
    uint32_t        ip_tos_rw_en:1;
    uint32_t        ip_small_frag_drop:1;
    uint32_t        ip_rsvd_flags_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        ip_df_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        ip_options_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        ip_invalid_len_action:FW_NORM_ACTION_BIT_WIDTH;

    uint32_t        icmp_src_quench_msg_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_redirect_msg_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_info_req_msg_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_addr_req_msg_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_traceroute_msg_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_rsvd_type_msg_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_bad_request_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_request_response_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_redirect_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        icmp_code_non_zero_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        ipv4_ping_of_death_drop_action:FW_NORM_ACTION_BIT_WIDTH;

    uint32_t        tcp_non_syn_first_pkt_drop:1;
    uint32_t        tcp_syncookie_en:1;
    uint32_t        tcp_split_handshake_detect_en:1;
    uint32_t        tcp_split_handshake_drop:1;
    uint32_t        tcp_rsvd_flags_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_unexpected_mss_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_unexpected_win_scale_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_urg_ptr_not_set_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_urg_flag_not_set_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_urg_payload_missing_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_echots_nonzero_ack_zero_action:FW_NORM_ACTION_BIT_WIDTH; // <=== ??
    uint32_t        tcp_rst_with_data_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_data_len_gt_mss_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_data_len_gt_win_size_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_unexpected_ts_option_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_ts_not_present_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_invalid_flags_action:FW_NORM_ACTION_BIT_WIDTH;
    uint32_t        tcp_ack_flag_not_present_action:FW_NORM_ACTION_BIT_WIDTH;
} __PACK__ l4_profile_data_t;

typedef struct l4_profile_oper_s {
} __PACK__ l4_profile_oper_t;

typedef struct l4_profile_s {
    l4_profile_key_handle_t        key;
    l4_profile_data_t              data;
    l4_profile_oper_t              oper;
} __PACK__ l4_profile_t;

typedef struct l4_profiles_s {
    uint32_t        num_entries;
    l4_profile_t    entries[0];
} __PACK__ l4_profiles_t;

//------------------------------------------------------------------------------
// walk/query(s) filter for L4 profile db
//------------------------------------------------------------------------------
typedef struct l4_profile_filter_s {
    vrf_id_t    vrf;
} l4_profile_filter_t;

//------------------------------------------------------------------------------
// global policy - all global NIC level knobs must be captured here
// TODO: we can point to a default profile from here
//------------------------------------------------------------------------------
typedef struct svc_policy_global_s {
    log_policy_t        log_policy;                // multiple in future (per level?)
    uint32_t            max_session_rate;          // max. session rate supported (drop after that)
    uint32_t            l4_profile_id;             // ==> confirm !!!
} __PACK__ svc_policy_global_t;

//------------------------------------------------------------------------------
// public APIs
//------------------------------------------------------------------------------
// helper functions
hal_ret_t hal_init_l4_profile_key_handles(l4_profile_key_handles_t *khs __INOUT__);
hal_ret_t hal_init_l4_profile_data(l4_profile_data_t *data __INOUT__);
hal_ret_t hal_init_l4_profiles(l4_profiles_t *entries __INOUT__);
hal_ret_t hal_free_l4_profiles(l4_profiles_t *entries __IN__);       // free to respective slab

// CRUD APIs
hal_ret_t hal_create_l4_profile(l4_profiles_t *l4_profiles__INOUT__);      // handles filled by API
hal_ret_t hal_retrieve_l4_profile(l4_profile_key_handles_t *khs __IN__,
                                  l4_profile_data_t *filter __IN__,
                                  l4_profiles_t **l4_profiles __OUT__);    // memory freed by caller
                                                                           // using free_entries() API
hal_ret_t hal_update_l4_profile(l4_profiles_t *l4_profiles __IN__);        // 'oper' is ignored
hal_ret_t hal_delete_l4_profile(l4_profile_key_handles_t *khs __IN__);

#endif    // __POLICY_H__

