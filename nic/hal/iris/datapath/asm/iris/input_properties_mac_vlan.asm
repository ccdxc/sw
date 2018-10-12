#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_properties_mac_vlan_k k;
struct input_properties_mac_vlan_d d;
struct phv_                        p;

%%

input_properties_mac_vlan:
  seq           c2, k.control_metadata_tm_iport, TM_PORT_DMA
  phvwr.c2      p.flow_lkp_metadata_lkp_inst, k.p4plus_to_p4_lkp_inst

  cmov          r1, c2, (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_TXDMA_INTRINSIC_HDR_SZ + P4PLUS_TO_P4_HDR_SZ), \
                        CAPRI_GLOBAL_INTRINSIC_HDR_SZ
  sub           r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5, \
                       capri_p4_intrinsic_frame_size_sbit6_ebit13}, r1

  seq           c3, k.recirc_header_valid, TRUE
  phvwrpair.c3  p.control_metadata_recirc_reason, k.recirc_header_reason[1:0], \
                p.qos_metadata_qos_class_id[4:0], k.capri_intrinsic_tm_iq[4:0]
  sub.c3        r1, r1, P4_RECIRC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ

  // if table lookup is miss, return
  nop.!c1.e
  phvwr         p.capri_p4_intrinsic_packet_len, r1

  seq           c1, d.input_properties_mac_vlan_d.src_lif_check_en, 1
  seq           c2, d.input_properties_mac_vlan_d.src_lif, \
                    k.{capri_intrinsic_lif_sbit0_ebit2, \
                       capri_intrinsic_lif_sbit3_ebit10}
  bcf           [c1&!c2], dejavu_check_failed
  or            r1, d.input_properties_mac_vlan_d.flow_miss_idx, \
                    k.{capri_intrinsic_lif_sbit0_ebit2, \
                       capri_intrinsic_lif_sbit3_ebit10}, 16
  phvwr         p.{control_metadata_src_lif,control_metadata_flow_miss_idx}, r1
  phvwr         p.flow_miss_metadata_tunnel_originate, \
                    d.input_properties_mac_vlan_d.tunnel_originate
  or            r1, d.input_properties_mac_vlan_d.tunnel_vnid, \
                    d.input_properties_mac_vlan_d.tunnel_rewrite_index, 30
  phvwr         p.{flow_miss_metadata_tunnel_rewrite_index,_flit_pad__922,\
                    flow_miss_metadata_tunnel_vnid}, r1
  or            r1, d.input_properties_mac_vlan_d.src_lport, \
                    d.input_properties_mac_vlan_d.dst_lport, 16
  phvwrpair     p.{control_metadata_dst_lport,control_metadata_src_lport}, r1, \
                    p.flow_lkp_metadata_lkp_vrf, \
                    d.input_properties_mac_vlan_d.vrf
  or            r1, d.input_properties_mac_vlan_d.mdest_flow_miss_action, \
                    d.input_properties_mac_vlan_d.ipsg_enable, 2
  or            r1, r1, d.input_properties_mac_vlan_d.rewrite_index, 3
  phvwr         p.{flow_miss_metadata_rewrite_index, \
                    control_metadata_ipsg_enable, \
                    control_metadata_mdest_flow_miss_action}, r1
  or            r1, d.input_properties_mac_vlan_d.allow_flood, \
                    d.input_properties_mac_vlan_d.flow_miss_qos_class_id, 2
  phvwrm        p.{control_metadata_flow_miss_qos_class_id, \
                    l3_metadata_inner_ip_frag, \
                    control_metadata_allow_flood}, r1, 0x7D
  phvwr         p.{control_metadata_mirror_on_drop_en, \
                    control_metadata_mirror_on_drop_session_id}, \
                    d.{input_properties_mac_vlan_d.mirror_on_drop_en, \
                    input_properties_mac_vlan_d.mirror_on_drop_session_id}
  phvwrpair.e   p.control_metadata_nic_mode, NIC_MODE_SMART, \
                    p.flow_lkp_metadata_lkp_dir, \
                    d.input_properties_mac_vlan_d.dir
  phvwr.f       p.l4_metadata_profile_idx, \
                    d.input_properties_mac_vlan_d.l4_profile_idx

dejavu_check_failed:
  phvwr.e       p.control_metadata_drop_reason[DROP_INPUT_MAPPING_DEJAVU], 1
  phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_properties_mac_vlan_error:
  nop.e
  nop
