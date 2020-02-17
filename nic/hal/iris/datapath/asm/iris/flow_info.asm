#include "nw.h"
#include "ingress.h"
#include "INGRESS_flow_info_k.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct flow_info_k_ k;
struct flow_info_d  d;
struct phv_         p;

%%

flow_info:
  seq           c1, r5[0], 0
  b.!c1         f_flow_info_thread_1
  //bbne          r5[0], 0, f_flow_info_thread_1
  /* egress port/vf */
  seq           c1, d.u.flow_info_d.dst_lport_en, TRUE
  phvwr.c1      p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  phvwr.c1      p.control_metadata_dst_lport, d.u.flow_info_d.dst_lport

  // nat info
  phvwr         p.nat_metadata_nat_l4_port, d.u.flow_info_d.nat_l4_port
  phvwr         p.nat_metadata_twice_nat_idx, d.u.flow_info_d.twice_nat_idx

  seq           c1, d.u.flow_info_d.multicast_en, TRUE
  bcf           [c1], flow_info_multicast
  phvwr         p.nat_metadata_nat_ip, d.u.flow_info_d.nat_ip
  seq           c1, d.u.flow_info_d.flow_only_policy, TRUE
  bcf           [c1], flow_info_policy_only
  nop.!c1.e
  nop

flow_info_multicast:
  phvwr.e       p.capri_intrinsic_tm_replicate_en, 1
  phvwr         p.capri_intrinsic_tm_replicate_ptr, d.u.flow_info_d.multicast_ptr

flow_info_policy_only:
  seq           c1, k.control_metadata_skip_flow_update, TRUE
  bcf           [c1], flow_skip
  seq           c1, k.control_metadata_from_cpu, TRUE
  bcf           [!c1], flow_info_policy_only_not_from_cpu
flow_info_policy_only_from_cpu:
  seq           c2, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  bcf           [c2], flow_info_policy_only_from_cpu_ucast
  seq           c2, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  bcf           [c2], flow_info_policy_only_from_cpu_mcast
flow_info_policy_only_from_cpu_bcast:
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 3
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

flow_info_policy_only_from_cpu_ucast:
  // Prom MSeg/BM Repls
  seq           c1, k.control_metadata_registered_mac_miss, TRUE
  phvwr.c1      p.capri_intrinsic_tm_replicate_en, 1
  add.c1        r1, k.capri_intrinsic_tm_replicate_ptr, 5
  phvwr.c1      p.capri_intrinsic_tm_replicate_ptr, r1
  nop.e
  nop

flow_info_policy_only_from_cpu_mcast:
  phvwr        p.capri_intrinsic_tm_replicate_en, 1
  seq          c1, k.control_metadata_registered_mac_miss, TRUE
  add.c1       r1, k.capri_intrinsic_tm_replicate_ptr, 4
  add.!c1      r1, k.capri_intrinsic_tm_replicate_ptr, 1
  phvwr.e      p.capri_intrinsic_tm_replicate_ptr, r1
  nop

flow_info_policy_only_not_from_cpu:
  seq           c2, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  bcf           [c2], flow_info_policy_only_not_from_cpu_ucast
  seq           c2, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  bcf           [c2], flow_info_policy_only_not_from_cpu_mcast
flow_info_policy_only_not_from_cpu_bcast:
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 6
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

flow_info_policy_only_not_from_cpu_ucast:
  // Prom Mgmt + Prom MSeg/BM Repls
  seq           c1, k.control_metadata_registered_mac_miss, TRUE
  phvwr.c1      p.capri_intrinsic_tm_replicate_en, 1
  add.c1        r1, k.capri_intrinsic_tm_replicate_ptr, 8
  phvwr.c1      p.capri_intrinsic_tm_replicate_ptr, r1
  nop.e
  nop

flow_info_policy_only_not_from_cpu_mcast:
  phvwr        p.capri_intrinsic_tm_replicate_en, 1
  seq          c1, k.control_metadata_registered_mac_miss, TRUE
  add.c1       r1, k.capri_intrinsic_tm_replicate_ptr, 7
  add.!c1      r1, k.capri_intrinsic_tm_replicate_ptr, 2
  phvwr.e      p.capri_intrinsic_tm_replicate_ptr, r1
  nop

flow_skip:
  phvwr.e         p.l4_metadata_flow_conn_track, FALSE
  nop

  
f_flow_info_thread_1:
  /* qos class selection */
  seq           c1, d.u.flow_info_d.qos_class_en, 1
  phvwr.c1      p.qos_metadata_qos_class_id, d.u.flow_info_d.qos_class_id
  
  /* Skip if its a deja-vu packet */
  seq           c1, k.control_metadata_skip_flow_update, TRUE
  bcf           [c1], flow_skip

  /* mirror session id and logging */
  phvwrpair.!c1 p.capri_intrinsic_tm_span_session, \
                    d.u.flow_info_d.ingress_mirror_session_id, \
                    p.capri_intrinsic_tm_cpu, d.u.flow_info_d.log_en
  phvwr         p.control_metadata_egress_mirror_session_id, \
                    d.u.flow_info_d.egress_mirror_session_id
  seq           c1, d.u.flow_info_d.mirror_on_drop_overwrite, 1
  phvwr.c1      p.{control_metadata_mirror_on_drop_en, \
                   control_metadata_mirror_on_drop_session_id}, \
                    d.{u.flow_info_d.mirror_on_drop_en, \
                       u.flow_info_d.mirror_on_drop_session_id}

  /* Commenting this to bring down the # of phvwr done in this table */
  /* ttl change detected */
  sne           c1, d.u.flow_info_d.flow_ttl, k.flow_lkp_metadata_ip_ttl
  phvwr.c1      p.flow_info_metadata_flow_ttl_change_detected, \
                    k.l4_metadata_ip_ttl_change_detect_en
  /* flow info */
  phvwr         p.flow_info_metadata_flow_role, d.u.flow_info_d.flow_role
  phvwr.e       p.flow_info_metadata_session_state_index, \
                    d.u.flow_info_d.session_state_index

  /* Flow Connection Tracking enable */
  phvwr.f       p.l4_metadata_flow_conn_track, d.u.flow_info_d.flow_conn_track




// flow_info:
//   seq           c1, r5[0], 0
//   b.!c1         f_flow_info_thread_1
//   //bbne          r5[0], 0, f_flow_info_thread_1
//   /* expected src lif check */
//   seq           c1, d.u.flow_info_d.expected_src_lif_check_en, TRUE
//   sne.c1        c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
//   sne.c1        c1, k.control_metadata_src_lif, d.u.flow_info_d.expected_src_lif
//   phvwr.c1.e    p.control_metadata_drop_reason[DROP_SRC_LIF_MISMATCH], 1
//   phvwr.c1      p.capri_intrinsic_drop, 1
// 
//   /* egress port/vf */
//   phvwrpair     p.capri_intrinsic_tm_oport, TM_PORT_EGRESS, \
//                     p.{capri_intrinsic_tm_replicate_ptr,\
//                        capri_intrinsic_tm_replicate_en}, \
//                     d.{u.flow_info_d.multicast_ptr, \
//                        u.flow_info_d.multicast_en}
//   phvwr         p.control_metadata_dst_lport, d.u.flow_info_d.dst_lport
// 
//   /* rewrite info */
//   phvwr         p.tunnel_metadata_tunnel_originate[0], \
//                     d.u.flow_info_d.tunnel_originate
//   or            r1, d.u.flow_info_d.tunnel_vnid, \
//                     d.u.flow_info_d.tunnel_rewrite_index, 24
//   or            r1, r1, d.u.flow_info_d.rewrite_index, 40
//   phvwr         p.{rewrite_metadata_rewrite_index, \
//                    rewrite_metadata_tunnel_rewrite_index, \
//                    rewrite_metadata_tunnel_vnid}, r1
//   phvwr         p.rewrite_metadata_flags, d.u.flow_info_d.rewrite_flags
// 
//   // nat info
//   phvwr         p.nat_metadata_nat_l4_port, d.u.flow_info_d.nat_l4_port
//   phvwr.e       p.nat_metadata_twice_nat_idx, d.u.flow_info_d.twice_nat_idx
//   phvwr.f       p.nat_metadata_nat_ip, d.u.flow_info_d.nat_ip
// 
// f_flow_info_thread_1:
//   /* qos class selection */
//   seq           c1, d.u.flow_info_d.qos_class_en, 1
//   phvwr.c1      p.qos_metadata_qos_class_id, d.u.flow_info_d.qos_class_id
// 
//   /* qid */
//   seq           c1, d.u.flow_info_d.qid_en, 1
//   phvwr.c1      p.control_metadata_qid, d.u.flow_info_d.tunnel_vnid
//   phvwr.c1      p.control_metadata_qtype, d.u.flow_info_d.qtype
// 
//   /* mirror session id and logging */
//   phvwrpair     p.capri_intrinsic_tm_span_session, \
//                     d.u.flow_info_d.ingress_mirror_session_id, \
//                     p.capri_intrinsic_tm_cpu, d.u.flow_info_d.log_en
//   phvwr         p.control_metadata_egress_mirror_session_id, \
//                     d.u.flow_info_d.egress_mirror_session_id
//   seq           c1, d.u.flow_info_d.mirror_on_drop_overwrite, 1
//   phvwr.c1      p.{control_metadata_mirror_on_drop_en, \
//                    control_metadata_mirror_on_drop_session_id}, \
//                     d.{u.flow_info_d.mirror_on_drop_en, \
//                        u.flow_info_d.mirror_on_drop_session_id}
// 
//   /* Commenting this to bring down the # of phvwr done in this table */
//   /* ttl change detected */
//   sne           c1, d.u.flow_info_d.flow_ttl, k.flow_lkp_metadata_ip_ttl
//   phvwr.c1      p.flow_info_metadata_flow_ttl_change_detected, \
//                     k.l4_metadata_ip_ttl_change_detect_en
//   /* flow info */
//   phvwr         p.flow_info_metadata_flow_role, d.u.flow_info_d.flow_role
//   phvwr.e       p.flow_info_metadata_session_state_index, \
//                     d.u.flow_info_d.session_state_index
// 
//   /* Flow Connection Tracking enable */
//   phvwr.f       p.l4_metadata_flow_conn_track, d.u.flow_info_d.flow_conn_track


// .align
// flow_miss:
//   seq           c1, r5[0], 0
//   nop.!c1.e
//   seq           c1, k.control_metadata_mode_switch_en, FALSE
//   crestore      [c3-c2], k.{control_metadata_registered_mac_nic_mode, \
//                             control_metadata_registered_mac_miss}, 0x3
//   // mode_swich_en == FALSE || [reg_mac_miss == FALSE && nic_mode == SMART]
//   orcf          c1, [!c2 & !c3]
//   bcf           [!c1], flow_miss_classic
//   seq           c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
//   bcf           [c1], validate_ipv4_flow_key
//   seq           c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
//   bcf           [c1], validate_ipv6_flow_key
//   phvwr         p.control_metadata_i2e_flags[P4_I2E_FLAGS_FLOW_MISS], 1

.align
flow_miss:
  seq           c1, r5[0], 0
  nop.!c1.e
  phvwr         p.control_metadata_flow_miss_ingress, 1
  phvwr         p.control_metadata_i2e_flags[P4_I2E_FLAGS_FLOW_MISS], 1
  seq           c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  bcf           [c1], validate_ipv4_flow_key
  seq           c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  bcf           [c1], validate_ipv6_flow_key
  
flow_miss_common:
  seq           c1, k.flow_lkp_metadata_lkp_vrf, r0
  bcf           [c1], flow_miss_input_properites_miss_drop
  seq           c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_TCP
  smneb         c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  seq           c3, k.l4_metadata_tcp_non_syn_first_pkt_drop, ACT_DROP
  bcf           [c1&c2&c3], flow_miss_tcp_non_syn_first_pkt_drop
  seq           c2, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  bcf           [c2], flow_miss_unicast
  seq           c2, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  bcf           [c2], flow_miss_multicast

flow_miss_broadcast:
  seq           c1, k.control_metadata_flow_learn, FALSE
  seq           c2, k.l4_metadata_ip_bm_mc_policy_enf_cfg_en, FALSE
  seq           c3, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  bcf           [c1|c2|c3], flow_miss_broadcast_mgmt_mseg_bm
  seq           c4, k.l4_metadata_policy_enf_cfg_en, TRUE
  bcf           [c4], flow_miss_broadcast_sup_copy_mgmt
  seq           c4, k.l4_metadata_flow_learn_cfg_en, TRUE
  bcf           [c4], flow_miss_broadcast_mgmt_mseg_bm
  phvwr.c4      p.capri_intrinsic_tm_cpu, 1
  nop

flow_miss_broadcast_mgmt_mseg_bm:
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 6
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

flow_miss_broadcast_sup_copy_mgmt:
  phvwr.e       p.capri_intrinsic_tm_cpu, 1
  phvwr         p.capri_intrinsic_tm_replicate_en, 1 // repl_ptr comes from inp_props

  
flow_miss_unicast:
  seq           c1, k.control_metadata_registered_mac_miss, TRUE
  bcf           [!c1], flow_miss_unicast_reg_mac_hit
flow_miss_unicast_reg_mac_miss:
  seq           c1, k.control_metadata_flow_learn, FALSE
  bcf           [c1], flow_miss_unicast_prom_mgmt_mseg_bm
  seq           c1, k.l4_metadata_policy_enf_cfg_en, TRUE
  bcf           [c1], flow_miss_unicast_sup_copy_prom_mgmt
  seq           c1, k.l4_metadata_flow_learn_cfg_en, TRUE
  bcf           [c1], flow_miss_unicast_sup_copy_prom_mgmt_prom_mseg_bm
  // Prom Mgmt & Prom MSeg/BM
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 8
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1


flow_miss_unicast_reg_mac_hit:
  seq           c1, k.control_metadata_flow_learn, TRUE
  bcf           [!c1], flow_done
  seq           c1, k.l4_metadata_policy_enf_cfg_en, TRUE
  phvwr.c1      p.control_metadata_dst_lport, CPU_LPORT
  phvwr.c1      p.rewrite_metadata_tunnel_rewrite_index, 0
  phvwr.c1      p.rewrite_metadata_rewrite_index, 0
  phvwr.c1      p.qos_metadata_qos_class_id, k.control_metadata_flow_miss_qos_class_id
  seq           c2, k.l4_metadata_flow_learn_cfg_en, TRUE
  setcf         c3, [!c1 & c2]
  phvwr.c3      p.capri_intrinsic_tm_cpu, 1
  nop.e
  nop

flow_miss_unicast_sup_redirect:
  phvwr        p.control_metadata_dst_lport, CPU_LPORT
  phvwr        p.rewrite_metadata_tunnel_rewrite_index, 0
  phvwr.e      p.rewrite_metadata_rewrite_index, 0
  phvwr.f      p.qos_metadata_qos_class_id, k.control_metadata_flow_miss_qos_class_id

// Prom Mgmt & Prom MSeg/BM
flow_miss_unicast_prom_mgmt_mseg_bm:
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 8
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

// Prom Mgmt
flow_miss_unicast_sup_copy_prom_mgmt:
  seq           c1, k.control_metadata_uuc_fl_pe_sup_en, TRUE
  phvwr.c1      p.capri_intrinsic_tm_cpu, 1
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 2
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

// Prom Mgmt & Prom MSeg/BM + SUP Copy
flow_miss_unicast_sup_copy_prom_mgmt_prom_mseg_bm:
  seq           c1, k.control_metadata_uuc_fl_pe_sup_en, TRUE
  phvwr.c1      p.capri_intrinsic_tm_cpu, 1
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 8
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1
   

flow_miss_multicast:
  seq           c1, k.control_metadata_registered_mac_miss, TRUE
  bcf           [!c1], flow_miss_multicast_reg_mac_hit
  seq           c1, k.control_metadata_flow_learn, FALSE
  seq           c2, k.l4_metadata_ip_bm_mc_policy_enf_cfg_en, FALSE
  seq           c3, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  bcf           [c1|c2|c3], flow_miss_multicast_miss_mgmt_mseg_bm
  seq           c4, k.l4_metadata_policy_enf_cfg_en, TRUE
  bcf           [c4], flow_miss_multicast_miss_sup_copy_mgmt
  seq           c4, k.l4_metadata_flow_learn_cfg_en, TRUE
  bcf           [c4], flow_miss_multicast_miss_mgmt_mseg_bm
  phvwr.c4      p.capri_intrinsic_tm_cpu, 1

flow_miss_multicast_miss_mgmt_mseg_bm:
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 7
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

flow_miss_multicast_miss_sup_copy_mgmt:
  phvwr.e       p.capri_intrinsic_tm_cpu, 1
  phvwr         p.capri_intrinsic_tm_replicate_en, 1 // repl_ptr comes from inp_props


flow_miss_multicast_reg_mac_hit:
  seq           c1, k.control_metadata_flow_learn, FALSE
  seq           c2, k.l4_metadata_ip_bm_mc_policy_enf_cfg_en, FALSE
  seq           c3, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  bcf           [c1|c2|c3], flow_miss_multicast_mgmt_mseg_bm
  seq           c4, k.l4_metadata_policy_enf_cfg_en, TRUE
  bcf           [c4], flow_miss_multicast_sup_copy_mgmt
  seq           c4, k.l4_metadata_flow_learn_cfg_en, TRUE
  bcf           [c4], flow_miss_multicast_mgmt_mseg_bm
  phvwr.c4      p.capri_intrinsic_tm_cpu, 1

flow_miss_multicast_mgmt_mseg_bm:
  phvwr         p.capri_intrinsic_tm_replicate_en, 1
  add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 2
  phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

flow_miss_multicast_sup_copy_mgmt:
  phvwr.e       p.capri_intrinsic_tm_cpu, 1
  phvwr         p.capri_intrinsic_tm_replicate_en, 1 // repl_ptr comes from inp_props
  
flow_miss_tcp_non_syn_first_pkt_drop:
  phvwr.e       p.control_metadata_drop_reason[DROP_TCP_NON_SYN_FIRST_PKT], 1
  phvwr         p.capri_intrinsic_drop, 1

flow_miss_input_properites_miss_drop:
  phvwr.e       p.control_metadata_drop_reason[DROP_INPUT_PROPERTIES_MISS], 1
  phvwr         p.capri_intrinsic_drop, 1

flow_done:
  nop.e
  nop


// flow_miss_common:
//   seq           c1, k.flow_lkp_metadata_lkp_vrf, r0
//   bcf           [c1], flow_miss_input_properites_miss_drop
//   seq           c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_TCP
//   smneb         c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
//   seq           c3, k.l4_metadata_tcp_non_syn_first_pkt_drop, ACT_DROP
//   bcf           [c1&c2&c3], flow_miss_tcp_non_syn_first_pkt_drop
//   phvwr         p.qos_metadata_qos_class_id, k.control_metadata_flow_miss_qos_class_id
//   phvwr         p.control_metadata_flow_miss_ingress, 1
//   seq           c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
//   b.c1          flow_miss_unicast
//   phvwr         p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
//   add           r1, r0, k.control_metadata_mdest_flow_miss_action
//   .brbegin
//   br            r1[1:0]
//   nop
//   .brcase FLOW_MISS_ACTION_CPU
//   phvwr.e       p.control_metadata_dst_lport, CPU_LPORT
//   nop
//   .brcase FLOW_MISS_ACTION_DROP
//   phvwr.e       p.control_metadata_drop_reason[DROP_FLOW_MISS], 1
//   phvwr         p.capri_intrinsic_drop, 1
//   .brcase FLOW_MISS_ACTION_FLOOD
//   b             flow_miss_multicast
//   nop
//   .brcase FLOW_MISS_ACTION_REDIRECT
//   phvwr.e       p.rewrite_metadata_tunnel_rewrite_index, \
//                     k.control_metadata_flow_miss_idx
//   nop
//   .brend

// flow_miss_unicast:
//   phvwr.e       p.control_metadata_dst_lport, CPU_LPORT
//   nop
// 
// flow_miss_multicast:
//   seq           c1, k.control_metadata_allow_flood, TRUE
//   bcf           [!c1], flow_miss_not_allow_flood
//   phvwrpair.c1  p.capri_intrinsic_tm_replicate_ptr, k.control_metadata_flow_miss_idx, \
//                      p.capri_intrinsic_tm_replicate_en, 1
//   phvwr         p.tunnel_metadata_tunnel_originate[0], \
//                     k.flow_miss_metadata_tunnel_originate
//   or            r2, k.flow_miss_metadata_rewrite_index_s8_e11, \
//                    k.flow_miss_metadata_rewrite_index_s0_e7, 4
//   phvwr         p.rewrite_metadata_rewrite_index[11:0], r2
//   phvwr         p.rewrite_metadata_tunnel_vnid, k.flow_miss_metadata_tunnel_vnid
//   or.e          r1, k.flow_miss_metadata_tunnel_rewrite_index_s8_e9, \
//                     k.flow_miss_metadata_tunnel_rewrite_index_s0_e7, 2
//   phvwr.f       p.rewrite_metadata_tunnel_rewrite_index[9:0], r1
// 
// flow_miss_not_allow_flood:
//   bcf           [!c2], flow_miss_mdest_not_pinned_drop
//   add           r1, r0, k.control_metadata_flow_miss_idx
//   phvwrpair.e   p.capri_intrinsic_tm_replicate_ptr, r1, \
//                     p.capri_intrinsic_tm_replicate_en, 1
//   nop
// 
// flow_miss_mdest_not_pinned_drop:
//   phvwr.e       p.control_metadata_drop_reason[DROP_MULTI_DEST_NOT_PINNED_UPLINK], 1
//   phvwr         p.capri_intrinsic_drop, 1
// 
// flow_miss_input_properites_miss_drop:
//   phvwr.e       p.control_metadata_drop_reason[DROP_INPUT_PROPERTIES_MISS], 1
//   phvwr         p.capri_intrinsic_drop, 1
// 
// flow_miss_tcp_non_syn_first_pkt_drop:
//   phvwr.e       p.control_metadata_drop_reason[DROP_TCP_NON_SYN_FIRST_PKT], 1
//   phvwr         p.capri_intrinsic_drop, 1
// 
// flow_miss_classic:
//   phvwr         p.control_metadata_nic_mode, NIC_MODE_CLASSIC
//   seq           c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_BROADCAST
//   bcf           [c1], flow_miss_classic_broadcast
// flow_miss_classic_multicast:
//   seq           c1, k.control_metadata_registered_mac_miss, FALSE
//   seq           c2, k.control_metadata_allow_flood, TRUE
//   setcf         c3, [c1&c2]
//   nop.!c3.e
//   add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 1
//   phvwr         p.capri_intrinsic_tm_replicate_ptr, r1
// 
// flow_miss_classic_broadcast:
//   seq           c1, k.control_metadata_allow_flood, TRUE
//   nop.!c1.e
//   add.e         r1, k.capri_intrinsic_tm_replicate_ptr, 3
//   phvwr         p.capri_intrinsic_tm_replicate_ptr, r1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_hit_drop:
  seq           c1, d.u.flow_hit_drop_d.mirror_on_drop_overwrite, 1
  phvwr.c1      p.{control_metadata_mirror_on_drop_en, \
                   control_metadata_mirror_on_drop_session_id}, \
                    d.{u.flow_hit_drop_d.mirror_on_drop_en, \
                       u.flow_hit_drop_d.mirror_on_drop_session_id}
  /* set drop bit */
  phvwr.e       p.control_metadata_drop_reason[DROP_FLOW_HIT], 1
  phvwr         p.capri_intrinsic_drop, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_info_from_cpu:
  phvwr         p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  seq.e         c1, k.p4plus_to_p4_dst_lport_valid, TRUE
  phvwr.c1      p.control_metadata_dst_lport, k.p4plus_to_p4_dst_lport
  // nop.!c1.e
  // or.e          r1, k.p4plus_to_p4_dst_lport_s8_e10, \
  //                   k.p4plus_to_p4_dst_lport_s0_e7, 3
  // phvwr         p.control_metadata_dst_lport, r1

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
  sub           r7, r0, 1
  or            r6, k.flow_lkp_metadata_lkp_src_s104_e127, \
                    k.flow_lkp_metadata_lkp_src_s96_e103, 24
  seq           c1, r6[31:24], 0x7f
  seq           c2, r6[31:28], 0xe
  seq           c3, r6[31:0], r7[31:0]
  seq           c4, k.flow_lkp_metadata_lkp_dst[31:0], r0
  seq           c5, k.flow_lkp_metadata_lkp_dst[31:24], 0x7f
  seq           c6, r6[31:0], k.flow_lkp_metadata_lkp_dst[31:0]
  seq.c6        c6, k.l4_metadata_ip_normalization_en, TRUE
  bcf           [c1|c2|c3|c4|c5|c6], malformed_flow_key
  nop
  b             flow_miss_common
  nop

validate_ipv6_flow_key:
  // srcAddr => r2(hi) r3(lo)
  add           r2, r0, k.flow_lkp_metadata_lkp_src_s0_e95[95:32]
  or            r1, k.flow_lkp_metadata_lkp_src_s104_e127, \
                    k.flow_lkp_metadata_lkp_src_s96_e103, 24
  or            r3, r1, k.flow_lkp_metadata_lkp_src_s0_e95[31:0], 32

  // dstAddr ==> r4(hi), r5(lo)
  add           r4, r0, k.flow_lkp_metadata_lkp_dst[127:64]
  add           r5, r0, k.flow_lkp_metadata_lkp_dst[63:0]

  add           r6, r0, 1
  seq           c1, r4, r0
  seq           c2, r5, r0
  seq           c3, r5, r6
  andcf         c1, [c2|c3]
  seq           c2, r2, r0
  seq           c3, r3, r6
  andcf         c2, [c3]
  seq           c3, r2[63:56], 0xff
  seq           c4, r2, r4
  seq.c4        c4, r3, r5
  seq.c4        c4, k.l4_metadata_ip_normalization_en, TRUE
  bcf           [c1|c2|c3|c4], malformed_flow_key
  nop
  b             flow_miss_common
  nop

malformed_flow_key:
  phvwr.e       p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_info_error:
  nop.e
  nop
