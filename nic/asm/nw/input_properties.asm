#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_properties_k k;
struct input_properties_d d;
struct phv_               p;

%%

input_properties:
  or          r1, k.capri_intrinsic_lif_sbit3_ebit10, k.capri_intrinsic_lif_sbit0_ebit2, 8
  phvwr       p.control_metadata_src_lif, r1
  phvwr       p.flow_lkp_metadata_lkp_vrf, d.u.input_properties_d.vrf
  phvwr       p.flow_lkp_metadata_lkp_dir, d.u.input_properties_d.dir
  phvwr       p.control_metadata_flow_miss_action, d.u.input_properties_d.flow_miss_action
  phvwr       p.control_metadata_flow_miss_tm_oqueue, d.u.input_properties_d.flow_miss_tm_oqueue
  phvwr       p.control_metadata_flow_miss_idx, d.u.input_properties_d.flow_miss_idx
  phvwr       p.control_metadata_ipsg_enable, d.u.input_properties_d.ipsg_enable
  phvwr       p.control_metadata_lif_filter, d.u.input_properties_d.filter
  phvwr       p.qos_metadata_dscp, d.u.input_properties_d.dscp
  phvwr       p.l4_metadata_profile_idx, d.u.input_properties_d.l4_profile_idx

  phvwr       p.capri_intrinsic_lif, d.u.input_properties_d.dst_lif

  seq         c1, k.capri_intrinsic_tm_iport, TM_PORT_DMA
  cmov        r1, c1, 15, 20
  sub.e       r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5, capri_p4_intrinsic_frame_size_sbit6_ebit13}, r1
  phvwr       p.control_metadata_packet_len, r1

