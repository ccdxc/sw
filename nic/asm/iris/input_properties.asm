#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"
#include "nw.h"

struct input_properties_k k;
struct input_properties_d d;
struct phv_               p;

%%

input_properties:
  // if table lookup is miss, return
  K_DBG_WR(0x00)
  DBG_WR(0x08, r1)
  nop.!c1.e
  phvwr         p.control_metadata_nic_mode, r5[0]
  phvwr         p.flow_lkp_metadata_lkp_dir, d.input_properties_d.dir
  phvwr         p.flow_lkp_metadata_lkp_vrf, d.input_properties_d.vrf
  phvwrpair     p.control_metadata_dst_lport[10:0], d.input_properties_d.dst_lport, \
                    p.control_metadata_src_lport[10:0], d.input_properties_d.src_lport
  phvwrpair     p.control_metadata_src_lif[10:8], k.capri_intrinsic_lif_sbit0_ebit2, \
                    p.control_metadata_src_lif[7:0], k.capri_intrinsic_lif_sbit3_ebit10
  phvwrpair     p.control_metadata_flow_miss_action, \
                    d.input_properties_d.flow_miss_action, \
                    p.control_metadata_flow_miss_qos_class_id, \
                    d.input_properties_d.flow_miss_qos_class_id
  phvwrpair     p.control_metadata_ipsg_enable, d.input_properties_d.ipsg_enable, \
                    p.control_metadata_allow_flood, d.input_properties_d.allow_flood
  phvwrpair.e   p.flow_miss_metadata_tunnel_vnid, \
                    d.input_properties_d.bounce_vnid, \
                    p.control_metadata_flow_miss_idx, \
                    d.input_properties_d.flow_miss_idx
  phvwr.f       p.l4_metadata_profile_idx, d.input_properties_d.l4_profile_idx

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_properties_error:
  nop.e
  nop
