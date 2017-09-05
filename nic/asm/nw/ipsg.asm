#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ipsg_k k;
struct ipsg_d d;
struct phv_   p;

%%

ipsg_miss:
  seq         c1, k.tcp_valid, TRUE
  seq         c2, k.l4_metadata_tcp_normalization_en, TRUE
  // if we have bal instructin which takes cExpr then below two
  // instructions can be merged into one.
  // David is looking into it.
  setcf       c1, [c1 & c2]
  bal.c1      r7, f_tcp_stateless_normalization
  seq         c1, k.control_metadata_ipsg_enable, FALSE
  nop.c1.e
  seq         c2, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  seq         c3, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  bcf         [c2|c3], ipsg_drop
  nop
  nop.e
  nop

.align
ipsg_hit:
  seq         c1, k.tcp_valid, TRUE
  seq         c2, k.l4_metadata_tcp_normalization_en, TRUE
  // if we have bal instructin which takes cExpr then below two
  // instructions can be merged into one.
  // David is looking into it.
  setcf       c1, [c1 & c2]
  bal.c1      r7, f_tcp_stateless_normalization
  seq         c1, k.control_metadata_ipsg_enable, FALSE
  nop.c1.e
  sne         c1, k.control_metadata_src_lport, d.u.ipsg_hit_d.src_lport
  sne         c2, k.ethernet_srcAddr, d.u.ipsg_hit_d.mac
  // sne         c3, k.vlan_tag_valid, d.u.ipsg_hit_d.vlan_valid
  sne         c4, k.{vlan_tag_vid_sbit0_ebit3, vlan_tag_vid_sbit4_ebit11}, d.u.ipsg_hit_d.vlan_id
  // bcf         [c1|c2|c3|c4], ipsg_drop
  bcf         [c1|c2|c4], ipsg_drop
  nop
  nop.e
  nop


.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_drop:
  phvwr.e     p.control_metadata_drop_reason[DROP_IPSG], 1
  phvwr       p.capri_intrinsic_drop, 1


// r7 - Register for return.
// c1 - Don't use in this function as the caller is updating it
//      in delay slot and we don't want to redo that again. Other option
//      is when we are doing the jr we have to execute the same instruction
//      in jr delay slot.
// c2 - C2 is only used to check the next configuration knob in the delay
//      slot. So don't use anywhere in the other logic.
// General order of execution is
// 1. Check for knob Allow
// 2. If knob is not allow then check for the error condition.
// 3. If there is no error in packet. Jump to next knob.
// 4. If packet has the error then check if knob is drop and drop it.
// 5. If its not drop the only option is Edit, edit the packet and
//    jump to next knob.

f_tcp_stateless_normalization:
lb_tcp_rsvd_flags:
  seq         c2, k.l4_metadata_tcp_rsvd_flags_action, NORMALIZATION_ACTION_ALLOW
  b.c2        lb_tcp_unexpected_mss
  seq         c2, k.l4_metadata_tcp_unexpected_mss_action, NORMALIZATION_ACTION_ALLOW
  sne         c3, k.tcp_res, r0
  b.!c3       lb_tcp_unexpected_mss
  seq         c4, k.l4_metadata_tcp_rsvd_flags_action, NORMALIZATION_ACTION_DROP
  phvwr.c4.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c4    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_res, 0


// C2 has lb_tcp_unexpected_mss_action == ALLOW
lb_tcp_unexpected_mss:
  b.c2        lb_tcp_unexpected_win_scale
  seq         c2, k.l4_metadata_tcp_unexpected_win_scale_action, NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_mss_valid, TRUE
  bcf         ![c3 & c4], lb_tcp_unexpected_win_scale
  seq         c3, k.l4_metadata_tcp_unexpected_mss_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  // TBD


lb_tcp_unexpected_win_scale:
  b.c2        lb_tcp_urg_flag_not_set
  seq         c2, k.l4_metadata_tcp_urg_flag_not_set_action, NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c4, k.tcp_option_ws_valid, TRUE
  bcf         ![c3 & c4], lb_tcp_urg_flag_not_set
  seq         c3, k.l4_metadata_tcp_unexpected_win_scale_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  // TBD


lb_tcp_urg_flag_not_set:
  b.c2        lb_tcp_urg_payload_missing
  seq         c2, k.l4_metadata_tcp_urg_payload_missing_action, NORMALIZATION_ACTION_ALLOW
  smneb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  sne         c4, k.tcp_urgentPtr, r0
  bcf         ![c3 & c4], lb_tcp_urg_payload_missing
  seq         c3, k.l4_metadata_tcp_urg_flag_not_set_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_urgentPtr, 0

lb_tcp_urg_payload_missing:
  b.c2        lb_tcp_urg_ptr_not_set
  seq         c2, k.l4_metadata_tcp_urg_ptr_not_set_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  sne         c4, k.tcp_urgentPtr, r0
  seq         c5, k.l4_metadata_tcp_data_len, r0
  bcf         ![c3 & c4 & c5], lb_tcp_urg_ptr_not_set
  seq         c3, k.l4_metadata_tcp_urg_payload_missing_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_urgentPtr, 0
  phvwrmi     p.tcp_flags, 0x0, TCP_FLAG_URG


lb_tcp_urg_ptr_not_set:
  b.c2        lb_tcp_rst_with_data
  seq         c2, k.l4_metadata_tcp_rst_with_data_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_URG, TCP_FLAG_URG
  seq         c4, k.tcp_urgentPtr, r0
  bcf         ![c3 & c4], lb_tcp_rst_with_data
  seq         c3, k.l4_metadata_tcp_urg_ptr_not_set_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwrmi     p.tcp_flags, 0x0, TCP_FLAG_URG


lb_tcp_rst_with_data:
  b.c2        lb_tcp_invalid_flags
  seq         c2, k.l4_metadata_tcp_invalid_flags_drop, ACT_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
  sne         c4, k.l4_metadata_tcp_data_len, r0
  bcf         ![c3 & c4], lb_tcp_invalid_flags
  seq         c3, k.l4_metadata_tcp_rst_with_data_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit option: TBD
  // 1. Change the l4_metadata.tcp_data_len to zero
  // 2. Update IP header total len
  // 3. Update the intrinsic header frame size.

lb_tcp_invalid_flags:
  b.c2        lb_tcp_flags_nonsyn_noack
  seq         c2, k.l4_metadata_tcp_flags_nonsyn_noack_drop, ACT_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_RST, TCP_FLAG_SYN|TCP_FLAG_RST
  smeqb       c4, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, TCP_FLAG_SYN|TCP_FLAG_FIN
  bcf         ![c3 | c4], lb_tcp_flags_nonsyn_noack
  seq         c3, k.l4_metadata_tcp_invalid_flags_drop, ACT_DROP
  // Trying to be symmetric with rest of the blocks of code. Ideally
  // The above line can be a nop and we can unconditionally drop the packet
  // only allow and drop are the only options. No edit.
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1



lb_tcp_flags_nonsyn_noack:
  b.c2        lb_tcp_unexpected_echo_ts
  seq         c2, k.l4_metadata_tcp_unexpected_echo_ts_action, NORMALIZATION_ACTION_ALLOW
  smeqb       c3, k.tcp_flags, TCP_FLAG_SYN, 0x0
  smeqb       c4, k.tcp_flags, TCP_FLAG_ACK, 0x0
  bcf         ![c3 & c4], lb_tcp_unexpected_echo_ts
  seq         c3, k.l4_metadata_tcp_flags_nonsyn_noack_drop, ACT_DROP
  // Trying to be symmetric with rest of the blocks of code. Ideally
  // The above line can be a nop and we can unconditionally drop the packet
  // only allow and drop are the only options. No edit.
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1


lb_tcp_unexpected_echo_ts:
  jr.c2       r7
  smeqb       c3, k.tcp_flags, TCP_FLAG_ACK, 0x0
  seq         c4, k.tcp_option_timestamp_valid, TRUE
  sne         c5, k.tcp_option_timestamp_prev_echo_ts, r0
  setcf       c3, [c3 & c4 & c5]
  jr.!c3      r7
  seq         c3, k.l4_metadata_tcp_unexpected_echo_ts_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // If not Allow/Drop then its EDIT
  phvwr       p.tcp_option_timestamp_prev_echo_ts, 0
  jr          r7
  nop

