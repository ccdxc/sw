#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct l4_profile_k k;
struct l4_profile_d d;
struct phv_         p;

%%

l4_profile:
  ASSERT_PHVWR(p, l4_metadata_ip_normalization_en, l4_metadata_ip_invalid_len_action,
               d, u.l4_profile_d.ip_normalization_en, u.l4_profile_d.ip_invalid_len_action)
  phvwr       p.{l4_metadata_ip_normalization_en ...\
                 l4_metadata_ip_invalid_len_action}, \
              d.{u.l4_profile_d.ip_normalization_en ...\
                 u.l4_profile_d.ip_invalid_len_action}

  phvwr       p.l4_metadata_tcp_normalization_en, d.u.l4_profile_d.tcp_normalization_en
  phvwr       p.l4_metadata_ip_normalize_ttl, d.u.l4_profile_d.ip_normalize_ttl
  phvwr       p.l4_metadata_ip_fragment_drop, d.u.l4_profile_d.ip_fragment_drop

  ASSERT_PHVWR(p, l4_metadata_icmp_deprecated_msgs_drop, l4_metadata_icmp_invalid_code_action,
               d, u.l4_profile_d.icmp_deprecated_msgs_drop, u.l4_profile_d.icmp_invalid_code_action)
  phvwr       p.{l4_metadata_icmp_deprecated_msgs_drop, \
                 l4_metadata_icmp_redirect_msg_drop, \
                 l4_metadata_icmp_invalid_code_action}, \
              d.{u.l4_profile_d.icmp_deprecated_msgs_drop, \
                 u.l4_profile_d.icmp_redirect_msg_drop, \
                 u.l4_profile_d.icmp_invalid_code_action}

  ASSERT_PHVWR(p, l4_metadata_tcp_rsvd_flags_action, l4_metadata_tcp_flags_nonsyn_noack_drop,
               d, u.l4_profile_d.tcp_rsvd_flags_action, u.l4_profile_d.tcp_flags_nonsyn_noack_drop)
  phvwr       p.{l4_metadata_tcp_rsvd_flags_action, \
                 l4_metadata_tcp_unexpected_mss_action, \
                 l4_metadata_tcp_unexpected_win_scale_action, \
                 l4_metadata_tcp_urg_ptr_not_set_action, \
                 l4_metadata_tcp_urg_flag_not_set_action, \
                 l4_metadata_tcp_urg_payload_missing_action, \
                 l4_metadata_tcp_unexpected_echo_ts_action, \
                 l4_metadata_tcp_rst_with_data_action, \
                 l4_metadata_tcp_data_len_gt_mss_action, \
                 l4_metadata_tcp_data_len_gt_win_size_action, \
                 l4_metadata_tcp_unexpected_ts_option_action, \
                 l4_metadata_tcp_ts_not_present_drop, \
                 l4_metadata_tcp_flags_nonsyn_noack_drop}, \
              d.{u.l4_profile_d.tcp_rsvd_flags_action, \
                 u.l4_profile_d.tcp_unexpected_mss_action, \
                 u.l4_profile_d.tcp_unexpected_win_scale_action, \
                 u.l4_profile_d.tcp_urg_ptr_not_set_action, \
                 u.l4_profile_d.tcp_urg_flag_not_set_action, \
                 u.l4_profile_d.tcp_urg_payload_missing_action, \
                 u.l4_profile_d.tcp_unexpected_echo_ts_action, \
                 u.l4_profile_d.tcp_rst_with_data_action, \
                 u.l4_profile_d.tcp_data_len_gt_mss_action, \
                 u.l4_profile_d.tcp_data_len_gt_win_size_action, \
                 u.l4_profile_d.tcp_unexpected_ts_option_action, \
                 u.l4_profile_d.tcp_ts_not_present_drop, \
                 u.l4_profile_d.tcp_flags_nonsyn_noack_drop}

  phvwr       p.l4_metadata_tcp_invalid_flags_drop, d.u.l4_profile_d.tcp_invalid_flags_drop
  bal         r7, f_ip_normalization
  phvwr       p.l4_metadata_tcp_split_handshake_detect_en, d.u.l4_profile_d.tcp_split_handshake_detect_en
  phvwr       p.l4_metadata_tcp_non_syn_first_pkt_drop, d.u.l4_profile_d.tcp_non_syn_first_pkt_drop
  phvwr.e     p.l4_metadata_tcp_split_handshake_drop, d.u.l4_profile_d.tcp_split_handshake_drop
  phvwr       p.l4_metadata_ip_ttl_change_detect_en, d.u.l4_profile_d.ip_ttl_change_detect_en


// c7 will retain the tunnel_terminate state throughout this function
// c6 - IPv6
// c5 - IPv4
f_ip_normalization:
  seq         c1, d.u.l4_profile_d.ip_normalization_en, 1
  jr.!c1      r7
  seq         c5, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  seq         c6, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  b.c6        lb_ip_norm_header_length
  setcf       c1, [c5 | c6]
  jr.!c1      r7
  seq         c7, k.tunnel_metadata_tunnel_terminate, 1
  seq         c4, d.u.l4_profile_d.ip_rsvd_flags_action, NORMALIZATION_ACTION_ALLOW
  b.c4        lb_ip_norm_df_bit
  seq         c4, d.u.l4_profile_d.ip_df_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c1, k.flow_lkp_metadata_ipv4_flags, IP_FLAGS_RSVD_MASK, IP_FLAGS_RSVD_MASK
  b.!c1       lb_ip_norm_df_bit
  seq         c1, d.u.l4_profile_d.ip_rsvd_flags_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  phvwrmi.c7  p.inner_ipv4_flags, 0, IP_FLAGS_RSVD_MASK
  phvwrmi.!c7 p.ipv4_flags, 0, IP_FLAGS_RSVD_MASK

lb_ip_norm_df_bit:
  b.c4        lb_ip_norm_options
  seq         c4, d.u.l4_profile_d.ip_options_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c1, k.flow_lkp_metadata_ipv4_flags, IP_FLAGS_DF_MASK, IP_FLAGS_DF_MASK
  b.!c1       lb_ip_norm_options
  seq         c1, d.u.l4_profile_d.ip_df_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  phvwrmi.!c7 p.ipv4_flags, 0, IP_FLAGS_DF_MASK
  phvwrmi.c7  p.inner_ipv4_flags, 0, IP_FLAGS_DF_MASK

lb_ip_norm_options:
  b.c4        lb_ip_norm_header_length
  nop         // since the branch to above label comes from multiple places we will
              // have to compute the check for option is allow or not after we branch.
  slt         c1, 5, k.flow_lkp_metadata_ipv4_hlen
  b.!c1       lb_ip_norm_header_length
  seq         c1, d.u.l4_profile_d.ip_options_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  // We also need to update
  // 1. IPv4 hlen in packet
  // 2. IPv4 Total Len in packet
  // 3. IP header checksum update, meaning change the header valid bit
  //    to the one which will trigger checksum update
  // 4. control_metadata.packet_len needs to be reduced.
  b.c7        lb_ip_norm_df_bit_tunnel_terminate
  phvwr.!c7   p.ipv4_options_blob_valid, 0
  phvwr       p.ipv4_ihl, 5
  add         r1, r0, k.flow_lkp_metadata_ipv4_hlen, 2
  sub         r1, r1, 20 // Option length
  sub         r2, k.ipv4_totalLen, r1
  phvwr       p.ipv4_totalLen, r2
  sub         r2, k.control_metadata_packet_len, r1
  phvwr       p.control_metadata_packet_len, r2


lb_ip_norm_df_bit_tunnel_terminate:
  // We can enable it once Parag makes changes for inner_ipv4_options
  // phvwr.c7  p.inner_ipv4_option_blob_valid, 0


lb_ip_norm_header_length:
  jr r7
  nop

lb_ip_norm_ttl:
  jr r7
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nop:
  nop.e
  nop
