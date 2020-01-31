#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_input_properties_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_properties_k_  k;
struct input_properties_d   d;
struct phv_                 p;

%%

input_properties:
  // if table lookup is miss, return
  nop.!c1.e
  or            r1, k.capri_intrinsic_lif_s3_e10, k.capri_intrinsic_lif_s0_e2, 8
  phvwr         p.control_metadata_dst_lport, d.input_properties_d.dst_lport
  phvwr         p.control_metadata_src_lport, d.input_properties_d.src_lport
  phvwr         p.flow_lkp_metadata_lkp_vrf, d.input_properties_d.vrf
  phvwr         p.control_metadata_src_lif, r1
  phvwr         p.capri_intrinsic_tm_replicate_ptr, \
                    d.input_properties_d.flow_miss_idx
  phvwr         p.control_metadata_ipsg_enable, \
                    d.input_properties_d.ipsg_enable
  phvwr         p.control_metadata_mdest_flow_miss_action, \
                    d.input_properties_d.mdest_flow_miss_action
  phvwr         p.control_metadata_flow_miss_qos_class_id, \
                  d.input_properties_d.flow_miss_qos_class_id
  phvwr         p.control_metadata_clear_promiscuous_repl, \
                  d.input_properties_d.clear_promiscuous_repl
  phvwr         p.rewrite_metadata_rewrite_index, \
                    d.input_properties_d.rewrite_index
  phvwr         p.rewrite_metadata_tunnel_rewrite_index, \
                    d.input_properties_d.tunnel_rewrite_index
  phvwr         p.rewrite_metadata_tunnel_vnid, \
                    d.input_properties_d.bounce_vnid
  phvwr         p.{control_metadata_mirror_on_drop_en, \
                   control_metadata_mirror_on_drop_session_id}, \
                    d.{input_properties_d.mirror_on_drop_en, \
                       input_properties_d.mirror_on_drop_session_id}
  phvwr         p.flow_lkp_metadata_lkp_dir, d.input_properties_d.dir
  phvwr         p.l4_metadata_profile_idx, d.input_properties_d.l4_profile_idx
  phvwr         p.control_metadata_if_label_check_en, d.input_properties_d.if_label_check_en
  phvwr         p.control_metadata_if_label_check_fail_drop, d.input_properties_d.if_label_check_fail_drop
  phvwr         p.control_metadata_src_if_label, d.input_properties_d.src_if_label
  phvwr         p.control_metadata_mseg_bm_bc_repls, d.input_properties_d.mseg_bm_bc_repls
  phvwr         p.control_metadata_mseg_bm_mc_repls, d.input_properties_d.mseg_bm_mc_repls
  seq           c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  phvwr.c1      p.flow_lkp_metadata_lkp_reg_mac_vrf, d.input_properties_d.reg_mac_vrf
  phvwr.!c1     p.flow_lkp_metadata_lkp_reg_mac_vrf, d.input_properties_d.vrf
  phvwr         p.control_metadata_flow_learn, d.input_properties_d.flow_learn
  phvwr.e       p.control_metadata_uuc_fl_pe_sup_en, d.input_properties_d.uuc_fl_pe_sup_en
  phvwr.f       p.control_metadata_has_prom_host_lifs, d.input_properties_d.has_prom_host_lifs


// input_properties:
//   // if table lookup is miss, return
//   nop.!c1.e
//   or            r1, d.input_properties_d.src_lport, \
//                     d.input_properties_d.dst_lport, 16
//   phvwrpair     p.{control_metadata_dst_lport,control_metadata_src_lport}, r1, \
//                     p.flow_lkp_metadata_lkp_vrf, d.input_properties_d.vrf
//   phvwr         p.flow_lkp_metadata_lkp_classic_vrf, d.input_properties_d.classic_vrf
//   or            r1, k.capri_intrinsic_lif_s3_e10, k.capri_intrinsic_lif_s0_e2, 8
//   phvwr         p.control_metadata_src_lif, r1
//   phvwr         p.control_metadata_flow_miss_idx, \
//                     d.input_properties_d.flow_miss_idx
//   phvwr         p.control_metadata_mdest_flow_miss_action, \
//                     d.input_properties_d.mdest_flow_miss_action
//   seq           c1, d.input_properties_d.nic_mode, NIC_MODE_CLASSIC
//   seq.!c1       c1, d.input_properties_d.mode_switch_en, 1
//   phvwr.c1      p.control_metadata_registered_mac_launch, 1
//   phvwr         p.control_metadata_flow_miss_qos_class_id, \
//                   d.input_properties_d.flow_miss_qos_class_id
//   phvwr         p.control_metadata_clear_promiscuous_repl, \
//                   d.input_properties_d.clear_promiscuous_repl
//   phvwr         p.control_metadata_allow_flood, d.input_properties_d.allow_flood
//   phvwr         p.flow_miss_metadata_tunnel_vnid, \
//                     d.input_properties_d.bounce_vnid
//   phvwr         p.{control_metadata_mirror_on_drop_en, \
//                    control_metadata_mirror_on_drop_session_id}, \
//                     d.{input_properties_d.mirror_on_drop_en, \
//                        input_properties_d.mirror_on_drop_session_id}
//   phvwr         p.control_metadata_nic_mode, d.input_properties_d.nic_mode
//   phvwr         p.control_metadata_mode_switch_en, d.input_properties_d.mode_switch_en
//   phvwr.e       p.flow_lkp_metadata_lkp_dir, d.input_properties_d.dir
//   phvwr.f       p.l4_metadata_profile_idx, d.input_properties_d.l4_profile_idx

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_properties_error:
  nop.e
  nop
