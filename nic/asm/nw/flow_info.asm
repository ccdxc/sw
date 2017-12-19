#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_info_k k;
struct flow_info_d d;
struct phv_        p;

%%

flow_info:
  /* expected src lif check */
  seq         c1, d.u.flow_info_d.expected_src_lif_check_en, TRUE
  sne.c1      c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
  sne.c1      c1, k.control_metadata_src_lif, d.u.flow_info_d.expected_src_lif
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_SRC_LIF_MISMATCH], 1
  phvwr.c1    p.capri_intrinsic_drop, 1

  /* egress port/vf */
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  phvwr       p.capri_intrinsic_tm_replicate_en, d.u.flow_info_d.multicast_en
  phvwr       p.capri_intrinsic_tm_replicate_ptr, d.u.flow_info_d.multicast_ptr
  phvwr       p.control_metadata_dst_lport, d.u.flow_info_d.dst_lport

  /* qos class selection */
  seq         c1, d.u.flow_info_d.qos_class_en, 1
  phvwr.c1    p.qos_metadata_qos_class_id, d.u.flow_info_d.qos_class_id

  /* qid */
  seq         c1, d.u.flow_info_d.qid_en, 1
  phvwr.c1    p.control_metadata_qid, d.u.flow_info_d.tunnel_vnid
  phvwr.c1    p.control_metadata_qtype, d.u.flow_info_d.qtype

  /* mirror session id */
  phvwr       p.capri_intrinsic_tm_span_session, d.u.flow_info_d.ingress_mirror_session_id
  phvwr       p.control_metadata_egress_mirror_session_id, d.u.flow_info_d.egress_mirror_session_id

  /* logging */
  phvwr       p.capri_intrinsic_tm_cpu, d.u.flow_info_d.log_en

  /* flow info */
  phvwr       p.flow_info_metadata_session_state_index, d.u.flow_info_d.session_state_index
  phvwr       p.flow_info_metadata_flow_ttl, d.u.flow_info_d.flow_ttl
  phvwr       p.flow_info_metadata_flow_role, d.u.flow_info_d.flow_role

  /* ttl change detected */
  seq         c1, d.u.flow_info_d.flow_ttl, k.flow_lkp_metadata_ip_ttl
  phvwr.c1    p.flow_info_metadata_flow_ttl_change_detected, k.l4_metadata_ip_ttl_change_detect_en

  /* Flow Connection Tracking enable */
  phvwr       p.l4_metadata_flow_conn_track, d.u.flow_info_d.flow_conn_track

  /* rewrite info */
  phvwr       p.rewrite_metadata_rewrite_index, d.u.flow_info_d.rewrite_index
  phvwr       p.rewrite_metadata_flags, d.u.flow_info_d.rewrite_flags

  phvwr       p.nat_metadata_nat_ip, d.u.flow_info_d.nat_ip
  ASSERT_PHVWR(p, nat_metadata_nat_l4_port, nat_metadata_twice_nat_idx,
               d, u.flow_info_d.nat_l4_port, u.flow_info_d.twice_nat_idx)
  phvwr       p.{nat_metadata_nat_l4_port, \
                 nat_metadata_twice_nat_idx}, \
              d.{u.flow_info_d.nat_l4_port, \
                 u.flow_info_d.twice_nat_idx}

  /* tunnel info */
  phvwr.e     p.tunnel_metadata_tunnel_originate, d.u.flow_info_d.tunnel_originate
  phvwrpair   p.rewrite_metadata_tunnel_rewrite_index[9:0], \
                d.u.flow_info_d.tunnel_rewrite_index, \
              p.rewrite_metadata_tunnel_vnid, d.u.flow_info_d.tunnel_vnid


.align
flow_miss:
  seq         c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  bcf         [c1], validate_ipv4_flow_key
  seq         c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  bcf         [c1], validate_ipv6_flow_key
  phvwr       p.control_metadata_flow_miss, 1

flow_miss_common:
  seq         c1, k.flow_lkp_metadata_lkp_vrf, r0
  bcf         [c1], flow_miss_input_properites_miss_drop
  seq         c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_TCP
  smneb       c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c3, k.l4_metadata_tcp_non_syn_first_pkt_drop, ACT_DROP
  bcf         [c1&c2&c3], flow_miss_tcp_non_syn_first_pkt_drop
  add         r1, r0, k.control_metadata_flow_miss_action
  phvwr       p.qos_metadata_qos_class_id, k.control_metadata_flow_miss_qos_class_id
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  .brbegin
  br          r1[1:0]
  phvwr       p.control_metadata_flow_miss_ingress, 1
  .brcase FLOW_MISS_ACTION_CPU
  phvwr.e     p.control_metadata_dst_lport, CPU_LPORT
  nop
  .brcase FLOW_MISS_ACTION_DROP
  phvwr.e     p.control_metadata_drop_reason[DROP_FLOW_MISS], 1
  phvwr       p.capri_intrinsic_drop, 1
  .brcase FLOW_MISS_ACTION_FLOOD
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  seq.!c1     c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_BROADCAST
  bcf         [c1], flow_miss_multicast
  phvwr.!c1.e p.control_metadata_dst_lport, CPU_LPORT
  nop
  .brcase FLOW_MISS_ACTION_REDIRECT
  phvwr.e     p.rewrite_metadata_tunnel_rewrite_index, k.control_metadata_flow_miss_idx
  nop
  .brend

flow_miss_multicast:
  seq         c1, k.control_metadata_allow_flood, TRUE
  bcf         [!c1], flow_miss_drop
  phvwr.c1    p.capri_intrinsic_tm_replicate_en, 1
  phvwr       p.capri_intrinsic_tm_replicate_ptr, k.control_metadata_flow_miss_idx
  phvwr       p.rewrite_metadata_rewrite_index, k.flow_miss_metadata_rewrite_index
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, k.flow_miss_metadata_tunnel_rewrite_index
  phvwr.e     p.rewrite_metadata_tunnel_vnid, k.flow_miss_metadata_tunnel_vnid
  phvwr       p.tunnel_metadata_tunnel_originate, k.flow_miss_metadata_tunnel_originate

flow_miss_drop:
  phvwr.e     p.control_metadata_drop_reason[DROP_FLOW_MISS], 1
  phvwr       p.capri_intrinsic_drop, 1

flow_miss_input_properites_miss_drop:
  phvwr.e     p.control_metadata_drop_reason[DROP_INPUT_PROPERTIES_MISS], 1
  phvwr       p.capri_intrinsic_drop, 1

flow_miss_tcp_non_syn_first_pkt_drop:
  phvwr.e     p.control_metadata_drop_reason[DROP_TCP_NON_SYN_FIRST_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_hit_drop:
  /* set drop bit */
  phvwr.e     p.control_metadata_drop_reason[DROP_FLOW_HIT], 1
  phvwr       p.capri_intrinsic_drop, 1

.align
flow_hit_from_vm_bounce:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_hit_to_vm_bounce:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nop:
  nop.e
  nop

validate_ipv4_flow_key:
  sub         r7, r0, 1
  seq         c1, k.flow_lkp_metadata_lkp_src[31:24], 0x7f
  seq         c2, k.flow_lkp_metadata_lkp_src[31:28], 0xe
  seq         c3, k.flow_lkp_metadata_lkp_src[31:0], r7[31:0]
  seq         c4, k.flow_lkp_metadata_lkp_dst[31:0], r0
  seq         c5, k.flow_lkp_metadata_lkp_dst[31:24], 0x7f
  seq         c6, k.flow_lkp_metadata_lkp_src[31:0], \
                  k.flow_lkp_metadata_lkp_dst[31:0]
  bcf         [c1|c2|c3|c4|c5|c6], malformed_flow_key
  nop
  b           flow_miss_common
  nop

validate_ipv6_flow_key:
  // srcAddr => r2(hi) r3(lo)
  add         r2, r0, k.flow_lkp_metadata_lkp_src[127:64]
  add         r3, r0, k.flow_lkp_metadata_lkp_src[63:0]
  // dstAddr ==> r4(hi), r5(lo)
  add         r4, r0, k.flow_lkp_metadata_lkp_dst[127:64]
  add         r5, r0, k.flow_lkp_metadata_lkp_dst[63:0]

  add         r6, r0, 1
  seq         c1, r4, r0
  seq         c2, r5, r0
  seq         c3, r5, r6
  andcf       c1, [c2|c3]
  seq         c2, r2, r0
  seq         c3, r3, r6
  andcf       c2, [c3]
  seq         c3, r2[63:56], 0xff
  seq         c4, r2, r4
  seq         c5, r3, r5
  andcf       c4, [c5]
  bcf         [c1|c2|c3|c4], malformed_flow_key
  nop
  b           flow_miss_common
  nop

malformed_flow_key:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
