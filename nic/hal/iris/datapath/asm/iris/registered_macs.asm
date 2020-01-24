#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct registered_macs_k k;
struct registered_macs_d d;
struct phv_              p;

%%

registered_macs:
  seq         c2, k.p4plus_to_p4_dst_lport_valid, TRUE
  bcf         [c2], registered_macs_done
  nop
  phvwr       p.qos_metadata_qos_class_id, k.capri_intrinsic_tm_oq
  bcf         [c1], registered_macs_hit
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  phvwr       p.control_metadata_registered_mac_miss, TRUE
  seq         c2, k.flow_lkp_metadata_lkp_reg_mac_vrf, r0
  bcf         [c2], registered_macs_input_properites_miss_drop
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  bcf         [c1], registered_macs_all_multicast
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_BROADCAST
  bcf         [c1], registered_macs_broadcast
  seq.e       c1, k.control_metadata_mseg_bm_mc_repls, TRUE
  nop

registered_macs_hit:
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  bcf         [c1], registered_macs_unicast
registered_macs_hit_common:
  seq         c1, d.registered_macs_d.multicast_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_ptr, d.registered_macs_d.dst_lport
  phvwr.!c1   p.control_metadata_clear_promiscuous_repl, FALSE
  phvwr.!c1   p.control_metadata_dst_lport, d.registered_macs_d.dst_lport
  seq         c1, d.registered_macs_d.flow_learn, 1
  phvwr.c1    p.control_metadata_flow_learn, TRUE
  seq         c1, d.registered_macs_d.tunnel_rewrite_en, 1
  phvwr.c1    p.rewrite_metadata_tunnel_rewrite_index, \
               d.registered_macs_d.tunnel_rewrite_index
  seq.e       c1, d.registered_macs_d.l4_profile_en, TRUE
  phvwr.c1    p.l4_metadata_profile_idx, d.registered_macs_d.l4_profile_idx

registered_macs_unicast:
  seq         c1, k.control_metadata_if_label_check_en, TRUE
  sne         c2, k.control_metadata_src_if_label, d.registered_macs_d.dst_if_label
  seq         c3, k.control_metadata_if_label_check_fail_drop, TRUE
  setcf       c4, [c1 & c2 & c3]
  phvwr.c4.e  p.control_metadata_drop_reason[DROP_IF_LABEL_MISMATCH], 1
  phvwr.c4    p.capri_intrinsic_drop, 1
  setcf       c4, [c1 & c2 & !c3]
  phvwr.c4.e  p.control_metadata_registered_mac_miss, TRUE
registered_macs_hit_common1:
  seq         c1, d.registered_macs_d.multicast_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_ptr, d.registered_macs_d.dst_lport
  phvwr.!c1   p.control_metadata_dst_lport, d.registered_macs_d.dst_lport
  seq         c1, d.registered_macs_d.flow_learn, 1
  phvwr.c1    p.control_metadata_flow_learn, TRUE
  seq.e       c1, d.registered_macs_d.tunnel_rewrite_en, 1
  phvwr.c1    p.rewrite_metadata_tunnel_rewrite_index, d.registered_macs_d.tunnel_rewrite_index

registered_macs_all_multicast:
  seq.e       c1, k.control_metadata_mseg_bm_bc_repls, TRUE
  phvwr.c1    p.control_metadata_flow_learn, TRUE

registered_macs_broadcast:
  phvwr.c1    p.control_metadata_flow_learn, TRUE

registered_macs_done:
  nop.e
  nop

// registered_macs:
//   phvwr       p.qos_metadata_qos_class_id, k.capri_intrinsic_tm_oq
//   phvwr       p.control_metadata_registered_mac_nic_mode, d.registered_macs_d.nic_mode
//   bcf         [c1], registered_macs_hit
//   phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
//   phvwr       p.control_metadata_registered_mac_miss, TRUE
//   seq         c2, k.flow_lkp_metadata_lkp_classic_vrf, r0
//   bcf         [c2], registered_macs_input_properites_miss_drop
//   seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
//   bcf         [c1], registered_macs_all_multicast
//   seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_BROADCAST
//   bcf         [c1], registered_macs_broadcast
// registered_macs_promiscuous:
//   add.!c1     r1, k.control_metadata_flow_miss_idx, 2
//   phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
//   phvwr       p.capri_intrinsic_tm_replicate_ptr, r1
// 
// registered_macs_hit:
//   seq         c1, d.registered_macs_d.multicast_en, 1
//   phvwr.c1.e  p.capri_intrinsic_tm_replicate_en, 1
//   phvwr.c1    p.capri_intrinsic_tm_replicate_ptr, d.registered_macs_d.dst_lport
//   phvwr.!c1.e p.control_metadata_dst_lport, d.registered_macs_d.dst_lport
//   nop
// 
// registered_macs_broadcast:
//   phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
//   phvwr       p.capri_intrinsic_tm_replicate_ptr, k.control_metadata_flow_miss_idx
// 
// registered_macs_all_multicast:
//   add         r1, k.control_metadata_flow_miss_idx, 1
//   phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
//   phvwr       p.capri_intrinsic_tm_replicate_ptr, r1

registered_macs_input_properites_miss_drop:
  phvwr.e       p.control_metadata_drop_reason[DROP_INPUT_PROPERTIES_MISS], 1
  phvwr         p.capri_intrinsic_drop, 1
/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
registered_macs_error:
  nop.e
  nop
