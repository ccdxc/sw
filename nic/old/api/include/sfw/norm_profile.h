//-----------------------------------------------------------------------------
// Normalization Profile HAL API related public data structures and APIs
//-----------------------------------------------------------------------------
#ifndef __NORM_PROFILE_H__
#define __NORM_PROFILE_H__

#include <base.h>
#include <eth.h>


//------------------------------------------------------------------------------
// Normalization Profile key
//------------------------------------------------------------------------------
typedef struct norm_profile_key_s {
    uint32_t        id;    // Normalization Profile ID
} __PACK__ norm_profile_key_t;


//------------------------------------------------------------------------------
// TBD: valid bits for key/handle
//------------------------------------------------------------------------------
typedef struct norm_profile_key_handle_u {
    norm_profile_key_t      key;
    hal_handle_t            hndl;
    uint8_t               __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ norm_profile_key_handle_t;

typedef struct norm_profile_key_handles_s {
    uint32_t                 num_khs;
    norm_profile_key_handle_t    khs[0];
} __PACK__ norm_profile_key_handles_t;

//------------------------------------------------------------------------------
// Normalization Profile data
// TBD: valid bits for each of the data fields
//------------------------------------------------------------------------------
typedef struct norm_profile_data_s {

    uint8_t         stateful_check_en:1;
    uint8_t         ip_normalization_en:1;
    uint8_t         tcp_normalization_en:1;
    uint8_t         icmp_normalization_en:1;
    uint8_t         ip_normalization_action:2;
    uint8_t         tcp_normalization_action:2;
    uint8_t         icmp_normalization_action:2;
    uint8_t         tcp_syncookie_en:1
    uint8_t         tcp_split_handshake_detect_en:1;
    uint8_t         tcp_split_handshake_action:1;
    uint8_t         ip_ttl_change_detect_en:1;
    uint8_t         tcp_connection_setup_timeout:4;
    uint8_t         session_inactivity_timeout:12;
    uint8_t         tcp_non_syn_first_pkt_action:1;
    uint8_t         tcp_xmas_tree_packet_drop_action:1;
    uint8_t         ipv4_ping_of_death_drop_action:1;
    uint8_t         ipv4_fragment_too_small_drop_action:1;

    // IPv4 Normalization Actions
    uint8_t         ip_res_action:2;
    uint8_t         ip_df_action:2;
    uint8_t         ip_options_action:2;
    uint8_t         ip_new_tos_action:2;
    uint8_t         ip_payload_action:2;

    // ICMP Normalization Actions
    uint8_t         icmp_src_quench_msg_action:2;
    uint8_t         icmp_redirect_msg_action:2:
    uint8_t         icmp_info_req_msg_action:2;
    uint8_t         icmp_addr_req_msg_action:2;
    uint8_t         icmp_traceroute_msg_action:2;
    uint8_t         icmp_rsvd_type_msg_action:2;
    uint8_t         icmp_bad_request_action:2;
    uint8_t         icmp_request_response_action:2;
    uint8_t         icmp_redirect_action:2;
    uint8_t         icmp_code_removal_action:2;

    // TCP Normalization Actions
    uint8_t         tcp_res_action:2;
    uint8_t         tcp_nosyn_mss_action:2;
    uint8_t         tcp_nosyn_ws_action:2;
    uint8_t         tcp_urg_flag_noptr_action:2;
    uint8_t         tcp_nourg_flag_ptr_action:2;
    uint8_t         tcp_urg_flag_ptr_nopayload_action:2;
    uint8_t         tcp_echots_nonzero_ack_zero_action:2;
    uint8_t         tcp_rst_flag_datapresent_action:2;
    uint8_t         tcp_datalen_gt_mss_action:2;
    uint8_t         tcp_datalen_gt_ws_action:2;
    uint8_t         tcp_noneg_ts_present_action:2;
    uint8_t         tcp_neg_ts_not_present_action:2;
    uint8_t         tcp_flags_combination:2;
    uint8_t         tcp_flags_nonsyn_noack_action:2;
    uint8_t         tcp_xmas_tree_packet_action:2;


    // Valid bits
    uint8_t         __stateful_check_en_valid:1;
    uint8_t         __ip_normalization_en_valid:1;
    uint8_t         __tcp_normalization_en_valid:1;
    uint8_t         __icmp_normalization_en_valid:1;
    uint8_t         __ip_normalization_action_valid:1;
    uint8_t         __tcp_normalization_action_valid:1;
    uint8_t         __icmp_normalization_action_valid:1;
    uint8_t         __tcp_syncookie_en_valid
    uint8_t         __tcp_split_handshake_detect_en_valid:1;
    uint8_t         __tcp_split_handshake_action_valid:1;
    uint8_t         __ip_ttl_change_detect_en_valid:1;
    uint8_t         __tcp_connection_setup_timeout_valid:1;
    uint8_t         __session_inactivity_timeout_valid2:1;
    uint8_t         __tcp_non_syn_first_pkt_action_valid:1;
    uint8_t         __tcp_xmas_tree_packet_drop_action_valid:1;
    uint8_t         __ipv4_ping_of_death_drop_action_valid:1;
    uint8_t         __ipv4_fragment_too_small_drop_action_valid:1;

    // IPv4 Normalization Actions
    uint8_t         __ip_res_action_valid:1;
    uint8_t         __ip_df_action_valid:1;
    uint8_t         __ip_options_action_valid:1;
    uint8_t         __ip_new_tos_action_valid:1;
    uint8_t         __ip_payload_action_valid:1;

    // ICMP Normalization Actions
    uint8_t         __icmp_src_quench_msg_action_valid:1;
    uint8_t         __icmp_redirect_msg_action_valid:1;
    uint8_t         __icmp_info_req_msg_action_valid:1;
    uint8_t         __icmp_addr_req_msg_action_valid:1;
    uint8_t         __icmp_traceroute_msg_action_valid:1;
    uint8_t         __icmp_rsvd_type_msg_action_valid:1;
    uint8_t         __icmp_bad_request_action_valid:1;
    uint8_t         __icmp_request_response_action_valid:1;
    uint8_t         __icmp_redirect_action_valid:1;
    uint8_t         __icmp_code_removal_action_valid:1;

    // TCP Normalization Actions
    uint8_t         __tcp_res_action_valid:1;
    uint8_t         __tcp_nosyn_mss_action_valid:1;
    uint8_t         __tcp_nosyn_ws_action_valid:1;
    uint8_t         __tcp_urg_flag_noptr_action_valid:1;
    uint8_t         __tcp_nourg_flag_ptr_action_valid:1;
    uint8_t         __tcp_urg_flag_ptr_nopayload_action_valid:1;
    uint8_t         __tcp_echots_nonzero_ack_zero_action_valid:1;
    uint8_t         __tcp_rst_flag_datapresent_action_valid:1;
    uint8_t         __tcp_datalen_gt_mss_action_valid:1;
    uint8_t         __tcp_datalen_gt_ws_action_valid:1;
    uint8_t         __tcp_noneg_ts_present_action_valid:1;
    uint8_t         __tcp_neg_ts_not_present_action_valid:1;
    uint8_t         __tcp_flags_combination_valid:1;
    uint8_t         __tcp_flags_nonsyn_noack_action_valid:1;
    uint8_t         __tcp_xmas_tree_packet_action_valid:1;
} __PACK__ norm_profile_data_t;

//------------------------------------------------------------------------------
// Normalization Profile operational state to be exposed to user or Ux
//     - should include counters, if any,
//     - any other events, timestamps etc.
//     - whatever helps in troubleshooting
//------------------------------------------------------------------------------
typedef struct norm_profile_oper_s {
    hal_ret_t      reason_code;
} __PACK__ norm_profile_oper_t;

//------------------------------------------------------------------------------
// Full Normalization Profile with key, data, operational state
//------------------------------------------------------------------------------
typedef struct norm_profile_s {
    norm_profile_key_handle_t    kh;
    norm_profile_data_t          data;
    norm_profile_oper_t          oper;
} __PACK__ norm_profile_t;

typedef struct norm_profiles_s {
    uint32_t            num_entries;
    norm_profile_t      entries[0];
} __PACK__ norm_profiles_t;

//------------------------------------------------------------------------------
// walk/query(s) filter for L2 entry Db
//------------------------------------------------------------------------------
typedef struct norm_profile_filter_s {
    uint32_t          segid;
    mac_addr_t        mac;
    ifindex_t         ifindex;
    vrf_id_t          vrf;       // <=== parent's parent !!
                                 // Can agent help here to breakdown per segid
                                 // here ??
    uint8_t           __segid_valid:1;
    uint8_t           __mac_valid:1;
    uint8_t           __ifindex_valid:1;
    uint8_t           __vrf_valid:1;
} __PACK__ norm_profile_filter_t;

//------------------------------------------------------------------------------
// CRUD APIs
//------------------------------------------------------------------------------
hal_ret_t hal_create_norm_profile(norm_profiles_t *norm_profiles __INOUT__);    // handles filled by API
hal_ret_t hal_read_norm_profile(norm_profile_key_handles_t *khs __IN__,
                                norm_profile_filter_t *filter __IN__,
                                norm_profiles_t **norm_profiles __OUT__);       // memory freed by caller
                                                                                // using free_entries() API
hal_ret_t hal_update_norm_profile(l2_entries_t *l2_entries __IN__);             // 'oper' is ignored
hal_ret_t hal_update_norm_profile_by_key(norm_profile_key_handles_t *khs,
                                         norm_profile_data_t *data __IN__);     // 'oper' is ignored
hal_ret_t hal_delete_norm_profile(norm_profile_key_handles_t *khs __IN__);


#endif    // __NORM_PROFILE_H__
