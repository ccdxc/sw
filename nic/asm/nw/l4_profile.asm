#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct l4_profile_k k;
struct l4_profile_d d;
struct phv_         p;

%%

l4_profile:
  phvwr       p.l4_metadata_ip_normalization_en, d.l4_profile_d.ip_normalization_en
  phvwr       p.l4_metadata_icmp_normalization_en, d.l4_profile_d.icmp_normalization_en
  phvwr       p.l4_metadata_tcp_normalization_en, d.l4_profile_d.tcp_normalization_en
  phvwr       p.l4_metadata_ip_rsvd_flags_action, d.l4_profile_d.ip_rsvd_flags_action
  phvwr       p.l4_metadata_ip_df_action, d.l4_profile_d.ip_df_action
  phvwr       p.l4_metadata_ip_options_action, d.l4_profile_d.ip_options_action
  phvwr       p.l4_metadata_ip_invalid_len_action, d.l4_profile_d.ip_invalid_len_action
  phvwr       p.l4_metadata_ip_normalize_ttl, d.l4_profile_d.ip_normalize_ttl
  phvwr       p.l4_metadata_ip_fragment_drop, d.l4_profile_d.ip_fragment_drop
  phvwr       p.l4_metadata_icmp_deprecated_msgs_drop, d.l4_profile_d.icmp_deprecated_msgs_drop
  phvwr       p.l4_metadata_icmp_redirect_msg_drop, d.l4_profile_d.icmp_redirect_msg_drop
  phvwr       p.l4_metadata_icmp_invalid_code_action, d.l4_profile_d.icmp_invalid_code_action
  phvwr       p.l4_metadata_tcp_rsvd_flags_action, d.l4_profile_d.tcp_rsvd_flags_action
  phvwr       p.l4_metadata_tcp_unexpected_mss_action, d.l4_profile_d.tcp_unexpected_mss_action
  phvwr       p.l4_metadata_tcp_unexpected_win_scale_action, d.l4_profile_d.tcp_unexpected_win_scale_action
  phvwr       p.l4_metadata_tcp_urg_ptr_not_set_action, d.l4_profile_d.tcp_urg_ptr_not_set_action
  phvwr       p.l4_metadata_tcp_urg_flag_not_set_action, d.l4_profile_d.tcp_urg_flag_not_set_action
  phvwr       p.l4_metadata_tcp_urg_payload_missing_action, d.l4_profile_d.tcp_urg_payload_missing_action
  phvwr       p.l4_metadata_tcp_unexpected_echo_ts_action, d.l4_profile_d.tcp_unexpected_echo_ts_action
  phvwr       p.l4_metadata_tcp_rst_with_data_action, d.l4_profile_d.tcp_rst_with_data_action
  phvwr       p.l4_metadata_tcp_data_len_gt_mss_action, d.l4_profile_d.tcp_data_len_gt_mss_action
  phvwr       p.l4_metadata_tcp_data_len_gt_win_size_action, d.l4_profile_d.tcp_data_len_gt_win_size_action
  phvwr       p.l4_metadata_tcp_unexpected_ts_option_action, d.l4_profile_d.tcp_unexpected_ts_option_action
  phvwr       p.l4_metadata_tcp_ts_not_present_drop, d.l4_profile_d.tcp_ts_not_present_drop
  phvwr       p.l4_metadata_tcp_flags_nonsyn_noack_drop, d.l4_profile_d.tcp_flags_nonsyn_noack_drop
  phvwr       p.l4_metadata_tcp_invalid_flags_drop, d.l4_profile_d.tcp_invalid_flags_drop
  phvwr       p.l4_metadata_tcp_non_syn_first_pkt_drop, d.l4_profile_d.tcp_non_syn_first_pkt_drop
  phvwr       p.l4_metadata_tcp_split_handshake_detect_en, d.l4_profile_d.tcp_split_handshake_detect_en
  phvwr       p.l4_metadata_tcp_split_handshake_drop, d.l4_profile_d.tcp_split_handshake_drop
  phvwr       p.l4_metadata_ip_ttl_change_detect_en, d.l4_profile_d.ip_ttl_change_detect_en
  b           validate_tunneled_packet2
  xor         r6, -1, r0

validate_tunneled_packet2:
  seq         c1, k.tunnel_metadata_tunnel_terminate, 1
  bcf         [!c1], validate_tunneled_packet2_exit
  nop.!c1.e
  seq         c1, k.inner_ipv4_valid, 1
  bcf         [c1], validate_tunneled_packet2_ipv6
  seq         c1, k.inner_ipv4_srcAddr[31:24], 0x7f
  seq         c2, k.inner_ipv4_srcAddr[31:28], 0xe
  seq         c3, k.inner_ipv4_srcAddr, r6
  seq         c4, k.inner_ipv4_dstAddr, 0
  seq         c5, k.inner_ipv4_dstAddr[31:24], 0x7f
  seq         c6, k.inner_ipv4_srcAddr, k.inner_ipv4_dstAddr
  bcf         [c1|c2|c3|c4|c5|c6], malformed_tunneled_packet
  nop
  nop.e
  nop

validate_tunneled_packet2_ipv6:
  seq         c1, k.inner_ipv6_valid, 1
  bcf         [!c1], validate_tunneled_packet2_exit
  nop.!c1.e
  seq         c1, k.inner_ipv6_dstAddr[127:64], 0
  seq         c2, k.inner_ipv6_dstAddr[63:0], 0
  seq         c3, k.inner_ipv6_dstAddr[63:0], 1
  andcf       c1, [c2|c3]
  seq         c2, k.{inner_ipv6_srcAddr_sbit0_ebit31,inner_ipv6_srcAddr_sbit32_ebit63}, 0
  seq         c3, k.inner_ipv6_srcAddr_sbit64_ebit127, 1
  seq         c4, k.inner_ipv6_srcAddr_sbit0_ebit31[31:24], 0xff
  seq         c5, k.{inner_ipv6_srcAddr_sbit0_ebit31,inner_ipv6_srcAddr_sbit32_ebit63}, k.inner_ipv6_dstAddr[127:64]
  seq         c6, k.inner_ipv6_srcAddr_sbit64_ebit127, k.inner_ipv6_dstAddr[63:0]
  bcf         [c1|c2|c3|c4|c5|c6], malformed_tunneled_packet
  nop
  nop.e
  nop

validate_tunneled_packet2_exit:
  nop

malformed_tunneled_packet:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1

