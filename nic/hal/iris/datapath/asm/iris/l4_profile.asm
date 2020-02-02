#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_l4_profile_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct l4_profile_k_ k;
struct l4_profile_d  d;
struct phv_          p;

%%

l4_profile:
  phvwr       p.l4_metadata_icmp_deprecated_msgs_drop, d.u.l4_profile_d.icmp_deprecated_msgs_drop
  // or          r1, d.u.l4_profile_d.ip_rsvd_flags_action, \
  //               d.u.l4_profile_d.ip_normalization_en, 2
  // phvwr       p.{l4_metadata_ip_normalization_en, \
  //                l4_metadata_ip_rsvd_flags_action}, r1
  phvwr       p.l4_metadata_ip_normalization_en, d.u.l4_profile_d.ip_normalization_en
  phvwr       p.l4_metadata_ip_rsvd_flags_action, d.u.l4_profile_d.ip_rsvd_flags_action

  phvwr       p.l4_metadata_ip_fragment_drop, d.u.l4_profile_d.ip_fragment_drop
  phvwr       p.l4_metadata_ip_df_action, d.u.l4_profile_d.ip_df_action
  phvwr       p.l4_metadata_ip_options_action, d.u.l4_profile_d.ip_options_action
  phvwr       p.l4_metadata_ip_invalid_len_action, d.u.l4_profile_d.ip_invalid_len_action

  // or          r1, d.u.l4_profile_d.ip_invalid_len_action, d.u.l4_profile_d.ip_options_action, 2
  // or          r1, r1, d.u.l4_profile_d.ip_df_action, 4
  // or          r1, r1, d.u.l4_profile_d.ip_fragment_drop, 6

  // phvwr       p.{l4_metadata_ip_fragment_drop, \
  //                l4_metadata_ip_df_action, \
  //                l4_metadata_ip_options_action, \
  //                l4_metadata_ip_invalid_len_action}, r1

  seq         c1, k.tcp_valid, TRUE
  phvwr.c1    p.l4_metadata_tcp_normalization_en, d.u.l4_profile_d.tcp_normalization_en

  // Code begin to update tcp_data_len for IPv6 options case. Added here as we couldn't
  // add it in input_mapping_native and tunnel because of key makers being full.
  // Overhead for good packet is 2 instructions.
  seq         c2, k.l3_metadata_ip_option_seen, TRUE
  balcf       r7, [c1 & c2], lb_l4_profile_ip_option_update_tcp_data_len
  seq         c1, k.icmp_valid, TRUE // Delay slot, critical path code.
  // Code end for updating tcp_data_len

  phvwr.c1    p.l4_metadata_icmp_normalization_en, d.u.l4_profile_d.icmp_normalization_en

  ASSERT_PHVWR(p, l4_metadata_icmp_redirect_msg_drop, l4_metadata_tcp_flags_nonsyn_noack_drop,
               d, u.l4_profile_d.icmp_redirect_msg_drop, u.l4_profile_d.tcp_flags_nonsyn_noack_drop)
  phvwr       p.{l4_metadata_icmp_redirect_msg_drop, \
                 l4_metadata_icmp_invalid_code_action, \
                 l4_metadata_tcp_rsvd_flags_action, \
                 l4_metadata_tcp_unexpected_mss_action, \
                 l4_metadata_tcp_unexpected_win_scale_action, \
                 l4_metadata_tcp_unexpected_sack_perm_action, \
                 l4_metadata_tcp_urg_ptr_not_set_action, \
                 l4_metadata_tcp_urg_flag_not_set_action, \
                 l4_metadata_tcp_urg_payload_missing_action, \
                 l4_metadata_tcp_unexpected_echo_ts_action, \
                 l4_metadata_tcp_rst_with_data_action, \
                 l4_metadata_tcp_data_len_gt_mss_action, \
                 l4_metadata_tcp_data_len_gt_win_size_action, \
                 l4_metadata_tcp_unexpected_ts_option_action, \
                 l4_metadata_tcp_unexpected_sack_option_action, \
                 l4_metadata_tcp_ts_not_present_drop, \
                 l4_metadata_tcp_flags_nonsyn_noack_drop}, \
              d.{u.l4_profile_d.icmp_redirect_msg_drop, \
                 u.l4_profile_d.icmp_invalid_code_action, \
                 u.l4_profile_d.tcp_rsvd_flags_action, \
                 u.l4_profile_d.tcp_unexpected_mss_action, \
                 u.l4_profile_d.tcp_unexpected_win_scale_action, \
                 u.l4_profile_d.tcp_unexpected_sack_perm_action, \
                 u.l4_profile_d.tcp_urg_ptr_not_set_action, \
                 u.l4_profile_d.tcp_urg_flag_not_set_action, \
                 u.l4_profile_d.tcp_urg_payload_missing_action, \
                 u.l4_profile_d.tcp_unexpected_echo_ts_action, \
                 u.l4_profile_d.tcp_rst_with_data_action, \
                 u.l4_profile_d.tcp_data_len_gt_mss_action, \
                 u.l4_profile_d.tcp_data_len_gt_win_size_action, \
                 u.l4_profile_d.tcp_unexpected_ts_option_action, \
                 u.l4_profile_d.tcp_unexpected_sack_option_action, \
                 u.l4_profile_d.tcp_ts_not_present_drop, \
                 u.l4_profile_d.tcp_flags_nonsyn_noack_drop}

  phvwrpair   p.l4_metadata_ip_normalize_ttl, \
                d.u.l4_profile_d.ip_normalize_ttl, \
                p.{l4_metadata_tcp_invalid_flags_drop, \
                   l4_metadata_tcp_non_syn_first_pkt_drop}, \
                d.{u.l4_profile_d.tcp_invalid_flags_drop, \
                   u.l4_profile_d.tcp_non_syn_first_pkt_drop}
  phvwr       p.{l4_metadata_tcp_split_handshake_detect_en, \
                l4_metadata_tcp_split_handshake_drop}, \
                d.{u.l4_profile_d.tcp_split_handshake_detect_en, \
                u.l4_profile_d.tcp_split_handshake_drop}
  phvwr       p.l4_metadata_flow_learn_cfg_en, d.u.l4_profile_d.flow_learn_cfg_en
  seq         c1, k.ipv4_valid, TRUE
  phvwr.c1    p.l4_metadata_policy_enf_cfg_en, d.u.l4_profile_d.policy_enf_cfg_en
  phvwr       p.l4_metadata_ip_bm_mc_policy_enf_cfg_en, \
                d.u.l4_profile_d.ip_bm_mc_policy_enf_cfg_en
  phvwr       p.l4_metadata_uuc_fl_pe_sup_en, \
                d.u.l4_profile_d.uuc_fl_pe_sup_en

  bbeq        k.p4plus_to_p4_valid, TRUE, f_p4plus_to_p4_1
  phvwrpair   p.l4_metadata_ip_ttl_change_detect_en, \
                d.u.l4_profile_d.ip_ttl_change_detect_en, \
                p.l4_metadata_tcp_normalize_mss, \
                d.u.l4_profile_d.tcp_normalize_mss
  nop.e
  nop


// Don't use c1 in this code. It is used in delay slot of caller for optimal path
// r7 is the return address
// Packet is already confirmed to be a TCP Packet with IP options.
// Need to take care if its V4/V6 and Tunnel/Native
// We only need to adjust the tcp_data_len for V6 only and not for V4 options.
lb_l4_profile_ip_option_update_tcp_data_len:
  seq         c2, k.ipv6_valid, TRUE
  seq         c3, k.inner_ipv6_valid, TRUE
  seq         c4, k.tunnel_metadata_tunnel_terminate, TRUE
  setcf       c5, [c2 & !c4] // Native IPv6
  setcf       c6, [c3 & c4]  // Tunnel with Inner IPv6
  sub.c5      r1, k.l4_metadata_tcp_data_len, k.l3_metadata_ipv6_options_len
  sub.c6      r1, k.l4_metadata_tcp_data_len, k.l3_metadata_inner_ipv6_options_len
  setcf       c2, [c5 | c6]
  jr          r7
  phvwr.c2    p.l4_metadata_tcp_data_len, r1

  // Code end for updating tcp_data_len
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nop:
  nop.e
  nop

#include "p4plus_to_p4_1.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
l4_profile_error:
  nop.e
  nop
