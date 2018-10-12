#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_properties_k k;
struct input_properties_d d;
struct phv_               p;

%%

input_properties:
  // if table lookup is miss, return
  nop.!c1.e
  or            r1, d.input_properties_d.src_lport, \
                    d.input_properties_d.dst_lport, 16
  phvwrpair     p.{control_metadata_dst_lport,control_metadata_src_lport}, r1, \
                    p.flow_lkp_metadata_lkp_vrf, d.input_properties_d.vrf
  or            r1, k.capri_intrinsic_lif_sbit3_ebit10, \
                    k.capri_intrinsic_lif_sbit0_ebit2, 8
  or            r1, d.input_properties_d.flow_miss_idx, r1, 16
  phvwr         p.{control_metadata_src_lif,control_metadata_flow_miss_idx}, r1
  or            r1, d.input_properties_d.clear_promiscuous_repl, \
                    d.input_properties_d.mdest_flow_miss_action, 1
  or            r1, r1, d.input_properties_d.ipsg_enable, 3
  phvwr         p.{control_metadata_ipsg_enable,\
                   control_metadata_mdest_flow_miss_action, \
                   control_metadata_clear_promiscuous_repl}, r1
  or            r1, d.input_properties_d.allow_flood, \
                    d.input_properties_d.flow_miss_qos_class_id, 2
  phvwrm        p.{control_metadata_flow_miss_qos_class_id, \
                   l3_metadata_inner_ip_frag, \
                   control_metadata_allow_flood}, r1, 0x7D
  phvwr         p.flow_miss_metadata_tunnel_vnid, d.input_properties_d.bounce_vnid
  phvwr         p.{control_metadata_mirror_on_drop_en, \
                   control_metadata_mirror_on_drop_session_id}, \
                    d.{input_properties_d.mirror_on_drop_en, \
                       input_properties_d.mirror_on_drop_session_id}
  phvwrpair.e   p.control_metadata_nic_mode, d.input_properties_d.nic_mode, \
                    p.flow_lkp_metadata_lkp_dir, d.input_properties_d.dir
  phvwr.f       p.l4_metadata_profile_idx, d.input_properties_d.l4_profile_idx

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_properties_error:
  nop.e
  nop
