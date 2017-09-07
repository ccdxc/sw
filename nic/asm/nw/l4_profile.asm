#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct l4_profile_k k;
struct l4_profile_d d;
struct phv_         p;

%%

l4_profile:
  phvwr       p.l4_metadata_ip_normalization_en, d.u.l4_profile_d.ip_normalization_en
  phvwr       p.l4_metadata_icmp_normalization_en, d.u.l4_profile_d.icmp_normalization_en
  phvwr       p.l4_metadata_tcp_normalization_en, d.u.l4_profile_d.tcp_normalization_en
  phvwr       p.l4_metadata_ip_rsvd_flags_action, d.u.l4_profile_d.ip_rsvd_flags_action
  phvwr       p.l4_metadata_ip_df_action, d.u.l4_profile_d.ip_df_action
  phvwr       p.l4_metadata_ip_options_action, d.u.l4_profile_d.ip_options_action
  phvwr       p.l4_metadata_ip_invalid_len_action, d.u.l4_profile_d.ip_invalid_len_action
  phvwr       p.l4_metadata_ip_normalize_ttl, d.u.l4_profile_d.ip_normalize_ttl
  phvwr       p.l4_metadata_ip_fragment_drop, d.u.l4_profile_d.ip_fragment_drop
  phvwr       p.l4_metadata_icmp_deprecated_msgs_drop, d.u.l4_profile_d.icmp_deprecated_msgs_drop
  phvwr       p.l4_metadata_icmp_redirect_msg_drop, d.u.l4_profile_d.icmp_redirect_msg_drop
  phvwr       p.l4_metadata_icmp_invalid_code_action, d.u.l4_profile_d.icmp_invalid_code_action
  phvwr       p.l4_metadata_tcp_rsvd_flags_action, d.u.l4_profile_d.tcp_rsvd_flags_action
  phvwr       p.l4_metadata_tcp_unexpected_mss_action, d.u.l4_profile_d.tcp_unexpected_mss_action
  phvwr       p.l4_metadata_tcp_unexpected_win_scale_action, d.u.l4_profile_d.tcp_unexpected_win_scale_action
  phvwr       p.l4_metadata_tcp_urg_ptr_not_set_action, d.u.l4_profile_d.tcp_urg_ptr_not_set_action
  phvwr       p.l4_metadata_tcp_urg_flag_not_set_action, d.u.l4_profile_d.tcp_urg_flag_not_set_action
  phvwr       p.l4_metadata_tcp_urg_payload_missing_action, d.u.l4_profile_d.tcp_urg_payload_missing_action
  phvwr       p.l4_metadata_tcp_unexpected_echo_ts_action, d.u.l4_profile_d.tcp_unexpected_echo_ts_action
  phvwr       p.l4_metadata_tcp_rst_with_data_action, d.u.l4_profile_d.tcp_rst_with_data_action
  phvwr       p.l4_metadata_tcp_data_len_gt_mss_action, d.u.l4_profile_d.tcp_data_len_gt_mss_action
  phvwr       p.l4_metadata_tcp_data_len_gt_win_size_action, d.u.l4_profile_d.tcp_data_len_gt_win_size_action
  phvwr       p.l4_metadata_tcp_unexpected_ts_option_action, d.u.l4_profile_d.tcp_unexpected_ts_option_action
  phvwr       p.l4_metadata_tcp_ts_not_present_drop, d.u.l4_profile_d.tcp_ts_not_present_drop
  phvwr       p.l4_metadata_tcp_flags_nonsyn_noack_drop, d.u.l4_profile_d.tcp_flags_nonsyn_noack_drop
  phvwr       p.l4_metadata_tcp_invalid_flags_drop, d.u.l4_profile_d.tcp_invalid_flags_drop
  phvwr       p.l4_metadata_tcp_non_syn_first_pkt_drop, d.u.l4_profile_d.tcp_non_syn_first_pkt_drop
  phvwr       p.l4_metadata_tcp_split_handshake_detect_en, d.u.l4_profile_d.tcp_split_handshake_detect_en
  phvwr       p.l4_metadata_tcp_split_handshake_drop, d.u.l4_profile_d.tcp_split_handshake_drop
  phvwr       p.l4_metadata_ip_ttl_change_detect_en, d.u.l4_profile_d.ip_ttl_change_detect_en
  bal         r7, validate_tunneled_packet2
  xor         r6, -1, r0
  bal         r7, f_ip_normalization
  nop
  seq         c2, k.p4plus_to_p4_valid, TRUE
  bal.c2      r7, f_p4plus_to_p4_apps
  nop
  nop.e
  nop

validate_tunneled_packet2:
  seq         c1, k.tunnel_metadata_tunnel_terminate, 1
  jr.!c1      r7
  seq         c1, k.inner_ipv4_valid, 1
  bcf         [!c1], validate_tunneled_packet2_ipv6
  seq         c1, k.inner_ipv4_srcAddr[31:24], 0x7f
  seq         c2, k.inner_ipv4_srcAddr[31:28], 0xe
  seq         c3, k.inner_ipv4_srcAddr, r6
  seq         c4, k.inner_ipv4_dstAddr, 0
  seq         c5, k.inner_ipv4_dstAddr[31:24], 0x7f
  seq         c6, k.inner_ipv4_srcAddr, k.inner_ipv4_dstAddr
  bcf         [c1|c2|c3|c4|c5|c6], malformed_tunneled_packet
  nop
  jr          r7
  nop

validate_tunneled_packet2_ipv6:
  seq         c1, k.inner_ipv6_valid, 1
  jr.!c1      r7
  add         r1, r0, 1
  seq         c1, k.inner_ipv6_dstAddr[127:64], r0
  seq         c2, k.inner_ipv6_dstAddr[63:0], r0
  seq         c3, k.inner_ipv6_dstAddr[63:0], r1
  andcf       c1, [c2|c3]
  // inner_srcAddr => r2(hi) r3(lo)
  add         r2, r0, k.{inner_ipv6_srcAddr_sbit0_ebit31,inner_ipv6_srcAddr_sbit32_ebit63}
  or          r3, k.inner_ipv6_srcAddr_sbit64_ebit127, r0
  seq         c2, r2, r0
  seq         c3, r3, r1
  andcf       c2, [c3]
  seq         c3, r2[63:56], 0xff
  seq         c4, r2, k.inner_ipv6_dstAddr[127:64]
  seq         c5, r3, k.inner_ipv6_dstAddr[63:0]
  bcf         [c1|c2|c3|c4|c5], malformed_tunneled_packet
  nop
  jr          r7
  nop

malformed_tunneled_packet:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1

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
  seq         c1, d.u.l4_profile_d.ip_rsvd_flags_action, NORMALIZATION_ACTION_ALLOW
  b.c1        lb_ip_norm_df_bit
  smeqb       c1, k.flow_lkp_metadata_ipv4_flags, IP_FLAGS_RSVD_MASK, IP_FLAGS_RSVD_MASK
  b.!c1       lb_ip_norm_df_bit
  seq         c1, d.u.l4_profile_d.ip_rsvd_flags_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  seq         c7, k.tunnel_metadata_tunnel_terminate, 1
  phvwrmi.c7  p.inner_ipv4_flags, 0, IP_FLAGS_RSVD_MASK
  phvwrmi.!c7 p.ipv4_flags, 0, IP_FLAGS_RSVD_MASK

lb_ip_norm_df_bit:
  seq         c1, d.u.l4_profile_d.ip_df_action, NORMALIZATION_ACTION_ALLOW
  b.c1        lb_ip_norm_options
  smeqb       c1, k.flow_lkp_metadata_ipv4_flags, IP_FLAGS_DF_MASK, IP_FLAGS_DF_MASK
  b.!c1       lb_ip_norm_options
  seq         c1, d.u.l4_profile_d.ip_df_action, NORMALIZATION_ACTION_DROP
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_IP_NORMALIZATION], 1
  phvwr.c1    p.capri_intrinsic_drop, 1
  // Edit Case. Need to check whether to update inner or outer
  phvwrmi.c7  p.inner_ipv4_flags, 0, IP_FLAGS_DF_MASK
  phvwrmi.!c7 p.ipv4_flags, 0, IP_FLAGS_DF_MASK

lb_ip_norm_options:
  jr r7
  nop

lb_ip_norm_header_length:
  jr r7
  nop

lb_ip_norm_ttl:
  jr r7
  nop

f_p4plus_to_p4_apps:
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN
  seq         c3, k.ipv4_valid, TRUE
  seq         c4, k.ipv6_valid, TRUE
  andcf       c3, [c2]
  andcf       c4, [c2]
  phvwr.c3    p.ipv4_totalLen, k.p4plus_to_p4_ip_len
  phvwr.c4    p.ipv6_payloadLen, k.p4plus_to_p4_ip_len
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN
  phvwr.c2    p.udp_len, k.p4plus_to_p4_udp_len
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO
  add         r1, k.tcp_seqNo, k.p4plus_to_p4_tcp_seq_delta
  phvwr.c2    p.tcp_seqNo, r1
  jr          r7
  phvwr       p.p4plus_to_p4_valid, FALSE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nop:
  nop.e
  nop
