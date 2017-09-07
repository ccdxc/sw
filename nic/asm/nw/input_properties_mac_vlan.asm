#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_properties_mac_vlan_k k;
struct input_properties_mac_vlan_d d;
struct phv_                        p;

%%

input_properties_mac_vlan:
  seq         c2, k.recirc_header_valid, TRUE
  phvwr.c2    p.control_metadata_recirc_reason, k.recirc_header_reason

  seq         c2, k.capri_intrinsic_tm_iport, TM_PORT_DMA
  cmov        r1, c2, (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                       CAPRI_TXDMA_INTRINSIC_HDR_SZ + \
                       P4PLUS_TO_P4_HDR_SZ), CAPRI_GLOBAL_INTRINSIC_HDR_SZ
  sub         r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5, \
                     capri_p4_intrinsic_frame_size_sbit6_ebit13}, r1
  phvwr       p.control_metadata_packet_len, r1
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  // if table lookup is miss, return
  nop.!c1.e
  seq         c1, d.input_properties_mac_vlan_d.src_lif_check_en, 1
  or          r1, k.capri_intrinsic_lif_sbit3_ebit10, k.capri_intrinsic_lif_sbit0_ebit2, 8
  seq         c2, d.input_properties_mac_vlan_d.src_lif, r1
  bcf         [c1&!c2], dejavu_check_failed
  phvwr       p.control_metadata_src_lif, r1

  phvwr       p.control_metadata_src_lport, d.input_properties_mac_vlan_d.src_lport
  phvwr       p.flow_lkp_metadata_lkp_vrf, d.input_properties_mac_vlan_d.vrf
  phvwr       p.flow_lkp_metadata_lkp_dir, d.input_properties_mac_vlan_d.dir
  phvwr       p.control_metadata_flow_miss_action, d.input_properties_mac_vlan_d.flow_miss_action
  phvwr       p.control_metadata_flow_miss_tm_oqueue, d.input_properties_mac_vlan_d.flow_miss_tm_oqueue
  phvwr       p.control_metadata_flow_miss_idx, d.input_properties_mac_vlan_d.flow_miss_idx
  phvwr       p.control_metadata_ipsg_enable, d.input_properties_mac_vlan_d.ipsg_enable
  phvwr       p.control_metadata_lif_filter, d.input_properties_mac_vlan_d.filter
  phvwr.e     p.qos_metadata_dscp, d.input_properties_mac_vlan_d.dscp
  phvwr       p.l4_metadata_profile_idx, d.input_properties_mac_vlan_d.l4_profile_idx

dejavu_check_failed:
  phvwr.e     p.control_metadata_drop_reason[DROP_INPUT_MAPPING_DEJAVU], 1
  phvwr       p.capri_intrinsic_drop, 1
