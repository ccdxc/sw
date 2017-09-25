#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ddos_src_dst_k   k;
struct ddos_src_dst_d   d;
struct phv_             p;

%%

nop:
  seq         c1, k.icmp_valid, TRUE
  seq         c2, k.l4_metadata_icmp_normalization_en, TRUE
  balcf       r7, [c1& c2], f_icmp_stateless_normalization
  nop
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ddos_src_dst_hit:
  seq       c1, k.icmp_valid, TRUE
  seq       c2, k.l4_metadata_icmp_normalization_en, TRUE
  balcf     r7, [c1 & c2], f_icmp_stateless_normalization
  seq       c1, k.control_metadata_flow_miss_ingress, FALSE
  nop.c1.e

  smeqb     c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
  bcf       [c1], ddos_type_syn

  seq       c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_ICMP
  bcf       [c1], ddos_type_icmp

  seq       c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_UDP
  bcf       [c1], ddos_type_udp

  add       r1, DDOS_TYPE_ANY, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

ddos_type_syn:
  add       r1, DDOS_TYPE_SYN, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

ddos_type_icmp:
  add       r1, DDOS_TYPE_ICMP, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

ddos_type_udp:
  add       r1, DDOS_TYPE_UDP, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop



// r7 - Register for return.
// c1 - Don't use in this function as the caller is updating it
//      in delay slot and we don't want to redo that again. Other option
//      is when we are doing the jr we have to execute the same instruction
//      in jr delay slot.

f_icmp_stateless_normalization:
lb_icmp_deprecated_msgs:
  add         r1, k.icmp_typeCode[15:8], r0
  seq         c2, k.l4_metadata_icmp_deprecated_msgs_drop, ACT_ALLOW
  b.c2        lb_icmp_redirect_msg
  seq         c2, k.l4_metadata_icmp_redirect_msg_drop, ACT_ALLOW
  // Tried using indexb here but it didn't reduce the instruction count
  // for the current values we are checking (4, 6, 15-39)
  seq         c3, r1, 4
  seq         c4, r1, 6
  sle         c5, 15, r1
  sle         c6, r1, 39
  setcf       c5, [c5 & c6]
  bcf         ![c3 | c4 | c5], lb_icmp_redirect_msg
  nop
  phvwr.e     p.control_metadata_drop_reason[DROP_ICMP_NORMALIZATION], 1
  phvwr       p.capri_intrinsic_drop, 1


lb_icmp_redirect_msg:
  b.c2        lb_icmp_code_removal
  seq         c2, k.l4_metadata_icmp_invalid_code_action, \
                     NORMALIZATION_ACTION_ALLOW
  seq         c3, r1, 5
  b.!c3       lb_icmp_code_removal
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_ICMP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1


lb_icmp_code_removal:
  jr.c2       r7
  seq         c3, r1, 0 // Echo Reply
  seq         c4, r1, 8 // Echo Req
  sne         c5, k.icmp_typeCode[7:0], 0
  andcf       c5, [c3 | c4]
  jr.!c5      r7
  seq         c3, k.l4_metadata_icmp_invalid_code_action, \
                     NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_ICMP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit case is the default option, Make code as zero.
  phvwrmi     p.icmp_typeCode, 0x0, 0xff
  jr         r7
  nop



