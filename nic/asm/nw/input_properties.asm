#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_properties_k k;
struct input_properties_d d;
struct phv_               p;

%%

input_properties:
  // if table lookup is miss, return
  nop.!c1.e
  or          r1, k.capri_intrinsic_lif_sbit3_ebit10, k.capri_intrinsic_lif_sbit0_ebit2, 8
  phvwr       p.control_metadata_src_lif, r1
  phvwr       p.control_metadata_src_lport, d.input_properties_d.src_lport
  phvwr       p.flow_lkp_metadata_lkp_vrf, d.input_properties_d.vrf
  phvwr       p.flow_lkp_metadata_lkp_dir, d.input_properties_d.dir
  phvwr       p.control_metadata_flow_miss_action, d.input_properties_d.flow_miss_action
  phvwr       p.control_metadata_flow_miss_tm_oqueue, d.input_properties_d.flow_miss_tm_oqueue
  phvwr       p.control_metadata_flow_miss_idx, d.input_properties_d.flow_miss_idx
  phvwr       p.control_metadata_ipsg_enable, d.input_properties_d.ipsg_enable
  phvwr       p.control_metadata_lif_filter, d.input_properties_d.filter
  phvwr.e     p.qos_metadata_dscp, d.input_properties_d.dscp
  phvwr       p.l4_metadata_profile_idx, d.input_properties_d.l4_profile_idx
