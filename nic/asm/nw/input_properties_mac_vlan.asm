#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_properties_mac_vlan_k k;
struct input_properties_mac_vlan_d d;
struct phv_                        p;

%%

input_properties_mac_vlan:
  seq           c2, k.control_metadata_tm_iport, TM_PORT_DMA
  phvwr.c2      p.flow_lkp_metadata_lkp_inst, \
                  k.p4plus_to_p4_flags[P4PLUS_TO_P4_FLAGS_LKP_INST_BIT_POS]

  cmov          r1, c2, (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_TXDMA_INTRINSIC_HDR_SZ + P4PLUS_TO_P4_HDR_SZ), \
                        CAPRI_GLOBAL_INTRINSIC_HDR_SZ
  sub           r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5, \
                       capri_p4_intrinsic_frame_size_sbit6_ebit13}, r1

  seq           c3, k.recirc_header_valid, TRUE
  phvwr.c3      p.control_metadata_recirc_reason, k.recirc_header_reason
  sub.c3        r1, r1, P4_RECIRC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ

  // if table lookup is miss, return
  nop.!c1.e
  phvwr         p.capri_p4_intrinsic_packet_len, r1

  seq           c1, d.input_properties_mac_vlan_d.src_lif_check_en, 1
  seq           c2, d.input_properties_mac_vlan_d.src_lif, \
                    k.{capri_intrinsic_lif_sbit0_ebit2, \
                       capri_intrinsic_lif_sbit3_ebit10}
  bcf           [c1&!c2], dejavu_check_failed
  phvwr         p.control_metadata_src_lif, \
                    k.{capri_intrinsic_lif_sbit0_ebit2, \
                       capri_intrinsic_lif_sbit3_ebit10}

  phvwr         p.flow_miss_metadata_tunnel_originate, d.input_properties_mac_vlan_d.tunnel_originate
  phvwr         p.flow_miss_metadata_tunnel_vnid, d.input_properties_mac_vlan_d.tunnel_vnid
  phvwrpair     p.flow_miss_metadata_rewrite_index, \
                    d.input_properties_mac_vlan_d.rewrite_index, \
                    p.flow_miss_metadata_tunnel_rewrite_index, \
                    d.input_properties_mac_vlan_d.tunnel_rewrite_index

  phvwrpair     p.control_metadata_dst_lport[10:0], \
                    d.input_properties_mac_vlan_d.dst_lport, \
                    p.control_metadata_src_lport[10:0], \
                    d.input_properties_mac_vlan_d.src_lport
  phvwr         p.flow_lkp_metadata_lkp_vrf, d.input_properties_mac_vlan_d.vrf
  phvwrpair     p.{control_metadata_flow_miss_action, \
                    control_metadata_flow_miss_qos_class_id}, \
                    d.{input_properties_mac_vlan_d.flow_miss_action, \
                    input_properties_mac_vlan_d.flow_miss_qos_class_id}, \
                    p.control_metadata_flow_miss_idx, \
                    d.input_properties_mac_vlan_d.flow_miss_idx
  phvwrpair     p.control_metadata_ipsg_enable, \
                    d.input_properties_mac_vlan_d.ipsg_enable, \
                    p.flow_lkp_metadata_lkp_dir, \
                    d.input_properties_mac_vlan_d.dir
  phvwr.e       p.control_metadata_allow_flood, \
                    d.input_properties_mac_vlan_d.allow_flood
  phvwr         p.l4_metadata_profile_idx, d.input_properties_mac_vlan_d.l4_profile_idx

dejavu_check_failed:
  phvwr.e       p.control_metadata_drop_reason[DROP_INPUT_MAPPING_DEJAVU], 1
  phvwr         p.capri_intrinsic_drop, 1

