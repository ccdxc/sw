#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_info_k k;
struct flow_info_d d;
struct phv_        p;

%%

flow_info:
  /* egress port/vf */
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  seq         c1, d.u.flow_info_d.multicast_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_ptr, d.u.flow_info_d.dst_lport
  phvwr.!c1   p.control_metadata_dst_lport, d.u.flow_info_d.dst_lport

  /* output queue selection */
  phvwr       p.capri_intrinsic_tm_oq, d.u.flow_info_d.ingress_tm_oqueue
  phvwr       p.control_metadata_egress_tm_oqueue, d.u.flow_info_d.egress_tm_oqueue

  /* qid */
  seq         c1, d.u.flow_info_d.qid_en, 1
  phvwr.c1    p.control_metadata_qid, d.u.flow_info_d.tunnel_vnid
  phvwr.c1    p.control_metadata_qtype, d.u.flow_info_d.qtype

  /* mirror session id */
  phvwr       p.capri_intrinsic_tm_span_session, d.u.flow_info_d.ingress_mirror_session_id
  phvwr       p.control_metadata_egress_mirror_session_id, d.u.flow_info_d.egress_mirror_session_id

  /* logging */
  phvwr       p.capri_intrinsic_tm_cpu, d.u.flow_info_d.log_en

  /* policer indicies */
  phvwr       p.policer_metadata_ingress_policer_index, d.u.flow_info_d.ingress_policer_index
  phvwr       p.policer_metadata_egress_policer_index, d.u.flow_info_d.egress_policer_index

  /* flow info */
  phvwr       p.flow_info_metadata_session_state_index, d.u.flow_info_d.session_state_index
  phvwr       p.flow_info_metadata_flow_ttl, d.u.flow_info_d.flow_ttl
  phvwr       p.flow_info_metadata_flow_role, d.u.flow_info_d.flow_role

  /* ttl change detected */
#if 0
  seq         c1, d.u.flow_info_d.flow_ttl, k.{flow_lkp_metadata_ip_ttl_sbit0_ebit2, \
                                                flow_lkp_metadata_ip_ttl_sbit3_ebit7}
#endif /* OLD CODE */
  seq         c1, d.u.flow_info_d.flow_ttl, k.{flow_lkp_metadata_ip_ttl_sbit0_ebit0, \
                                                flow_lkp_metadata_ip_ttl_sbit1_ebit7}
  phvwr.c1    p.flow_info_metadata_flow_ttl_change_detected, k.l4_metadata_ip_ttl_change_detect_en

  /* Flow Connection Tracking enable */
  phvwr       p.l4_metadata_flow_conn_track, d.u.flow_info_d.flow_conn_track

  /* rewrite info */
  phvwr       p.rewrite_metadata_rewrite_index, d.u.flow_info_d.rewrite_index
  phvwr       p.rewrite_metadata_flags, d.u.flow_info_d.rewrite_flags

  ASSERT_PHVWR(p, nat_metadata_nat_ip, nat_metadata_twice_nat_idx,
               d, u.flow_info_d.nat_ip, u.flow_info_d.twice_nat_idx)
  phvwr       p.{nat_metadata_nat_ip, \
                 nat_metadata_nat_l4_port, \
                 nat_metadata_twice_nat_idx}, \
              d.{u.flow_info_d.nat_ip, \
                 u.flow_info_d.nat_l4_port, \
                 u.flow_info_d.twice_nat_idx}

  /* tunnel info */
  phvwr       p.tunnel_metadata_tunnel_originate, d.u.flow_info_d.tunnel_originate
  ASSERT_PHVWR(p, rewrite_metadata_tunnel_rewrite_index, rewrite_metadata_tunnel_vnid,
               d, u.flow_info_d.tunnel_rewrite_index, u.flow_info_d.tunnel_vnid)
  phvwr.e     p.{rewrite_metadata_tunnel_rewrite_index, \
                 rewrite_metadata_tunnel_vnid}, \
              d.{u.flow_info_d.tunnel_rewrite_index, \
                 u.flow_info_d.tunnel_vnid}

  /* qos info */
  ASSERT_PHVWR(p, qos_metadata_cos_en, qos_metadata_dscp,
               d, u.flow_info_d.cos_en, u.flow_info_d.dscp)
  phvwr       p.{qos_metadata_cos_en, \
                 qos_metadata_cos, \
                 qos_metadata_dscp_en, \
                 qos_metadata_dscp}, \
              d.{u.flow_info_d.cos_en, \
                 u.flow_info_d.cos, \
                 u.flow_info_d.dscp_en, \
                 u.flow_info_d.dscp}

.align
flow_miss:
  seq         c1, k.flow_lkp_metadata_lkp_vrf, r0
  bcf         [c1], flow_miss_input_properites_miss_drop
  seq         c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_TCP
  smneb       c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq         c3, k.l4_metadata_tcp_non_syn_first_pkt_drop, ACT_DROP
  bcf         [c1&c2&c3], flow_miss_tcp_non_syn_first_pkt_drop
  add         r1, r0, k.control_metadata_flow_miss_action
  phvwr       p.control_metadata_flow_miss, 1
  phvwr       p.capri_intrinsic_tm_oq, k.control_metadata_flow_miss_tm_oqueue
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
  phvwr.c1.e  p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_ptr, k.control_metadata_flow_miss_idx
  phvwr.!c1.e p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  phvwr.!c1   p.control_metadata_dst_lport, CPU_LPORT
  .brcase FLOW_MISS_ACTION_REDIRECT
  phvwr.e     p.rewrite_metadata_tunnel_rewrite_index, k.control_metadata_flow_miss_idx
  nop
  .brend

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
