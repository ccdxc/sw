#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_input_properties_mac_vlan_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_properties_mac_vlan_k_ k;
struct input_properties_mac_vlan_d  d;
struct phv_                         p;

%%

input_properties_mac_vlan:
  seq           c2, k.control_metadata_tm_iport, TM_PORT_DMA
  phvwr.c2      p.flow_lkp_metadata_lkp_inst, k.p4plus_to_p4_lkp_inst

  cmov          r1, c2, (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_TXDMA_INTRINSIC_HDR_SZ + P4PLUS_TO_P4_HDR_SZ), \
                        CAPRI_GLOBAL_INTRINSIC_HDR_SZ
  sub           r1, k.capri_p4_intrinsic_frame_size, r1

  seq           c3, k.recirc_header_valid, TRUE
  phvwr.c3      p.control_metadata_recirc_reason, k.recirc_header_reason[1:0]
  phvwr.c3      p.qos_metadata_qos_class_id[4:0], k.capri_intrinsic_tm_iq[4:0]
  sub.c3        r1, r1, P4_RECIRC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ

  // if table lookup is miss, return
  nop.!c1.e
  phvwr         p.capri_p4_intrinsic_packet_len, r1

  // if skip_flow_update, return
  seq           c1, d.input_properties_mac_vlan_d.skip_flow_update, TRUE
  phvwr.c1.e    p.control_metadata_skip_flow_update, d.input_properties_mac_vlan_d.skip_flow_update
  nop

  phvwr         p.flow_miss_metadata_tunnel_originate, \
                    d.input_properties_mac_vlan_d.tunnel_originate
  phvwr         p.control_metadata_ep_learn_en, \
                    d.input_properties_mac_vlan_d.ep_learn_en

  // input_properties call
  phvwr         p.control_metadata_dst_lport, d.input_properties_mac_vlan_d.dst_lport
  phvwr         p.control_metadata_src_lport, d.input_properties_mac_vlan_d.src_lport
  phvwr         p.flow_lkp_metadata_lkp_vrf, d.input_properties_mac_vlan_d.vrf
  phvwr         p.control_metadata_src_lif, k.capri_intrinsic_lif
  phvwr         p.capri_intrinsic_tm_replicate_ptr, \
                    d.input_properties_mac_vlan_d.flow_miss_idx
  phvwr         p.control_metadata_ipsg_enable, \
                    d.input_properties_mac_vlan_d.ipsg_enable
  phvwr         p.control_metadata_mdest_flow_miss_action, \
                    d.input_properties_mac_vlan_d.mdest_flow_miss_action
  phvwr         p.control_metadata_flow_miss_qos_class_id, \
                    d.input_properties_mac_vlan_d.flow_miss_qos_class_id
  phvwr         p.control_metadata_clear_promiscuous_repl, 0
  phvwr         p.rewrite_metadata_rewrite_index, \
                    d.input_properties_mac_vlan_d.rewrite_index
  phvwr         p.rewrite_metadata_tunnel_rewrite_index, \
                    d.input_properties_mac_vlan_d.tunnel_rewrite_index
  phvwr         p.rewrite_metadata_tunnel_vnid, \
                    d.input_properties_mac_vlan_d.tunnel_vnid
  phvwr         p.{control_metadata_mirror_on_drop_en, \
                    control_metadata_mirror_on_drop_session_id}, \
                    d.{input_properties_mac_vlan_d.mirror_on_drop_en, \
                    input_properties_mac_vlan_d.mirror_on_drop_session_id}
  phvwr         p.flow_lkp_metadata_lkp_dir, \
                    d.input_properties_mac_vlan_d.dir
  phvwr         p.l4_metadata_profile_idx, \
                    d.input_properties_mac_vlan_d.l4_profile_idx
  phvwr         p.control_metadata_if_label_check_en, 1
  phvwr         p.control_metadata_if_label_check_fail_drop, 0
  phvwr         p.control_metadata_src_if_label, d.input_properties_mac_vlan_d.src_if_label
  phvwr         p.control_metadata_mseg_bm_bc_repls, 1
  phvwr         p.control_metadata_mseg_bm_mc_repls, 1
  phvwr         p.flow_lkp_metadata_lkp_reg_mac_vrf, d.input_properties_mac_vlan_d.vrf
  phvwr.e       p.control_metadata_flow_learn, 1
  phvwr.f       p.control_metadata_uuc_fl_pe_sup_en, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_properties_mac_vlan_error:
  nop.e
  nop
