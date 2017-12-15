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
  phvwr         p.control_metadata_nic_mode, r5[0]
  phvwrpair     p.control_metadata_src_lif[10:8], k.capri_intrinsic_lif_sbit0_ebit2, \
                    p.control_metadata_src_lif[7:0], k.capri_intrinsic_lif_sbit3_ebit10
  phvwrpair     p.control_metadata_dst_lport[10:0], d.input_properties_d.dst_lport, \
                    p.control_metadata_src_lport[10:0], d.input_properties_d.src_lport
  phvwr         p.flow_lkp_metadata_lkp_vrf, d.input_properties_d.vrf
  phvwrpair     p.{control_metadata_flow_miss_action,control_metadata_flow_miss_qos_class_id}, \
                    d.{input_properties_d.flow_miss_action,input_properties_d.flow_miss_qos_class_id}, \
                    p.control_metadata_flow_miss_idx, d.input_properties_d.flow_miss_idx
  phvwrpair     p.control_metadata_ipsg_enable, d.input_properties_d.ipsg_enable, \
                    p.flow_lkp_metadata_lkp_dir, d.input_properties_d.dir
  phvwr         p.flow_miss_metadata_tunnel_vnid, d.input_properties_d.bounce_vnid
  phvwr.e       p.control_metadata_allow_flood, d.input_properties_d.allow_flood
  phvwr         p.l4_metadata_profile_idx, d.input_properties_d.l4_profile_idx
