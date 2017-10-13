#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ipsg_k k;
struct ipsg_d d;
struct phv_   p;

%%

ipsg_miss:
  smeqb       c1, k.flow_lkp_metadata_lkp_type, \
                FLOW_KEY_LOOKUP_TYPE_IP_MASK, FLOW_KEY_LOOKUP_TYPE_IP_MASK
  phvwr.c1    p.control_metadata_drop_reason[DROP_IPSG], 1
  nop.e
  phvwr.c1    p.capri_intrinsic_drop, 1

.align
ipsg_hit:
  sne         c1, k.control_metadata_src_lif, d.u.ipsg_hit_d.src_lif
  sne.!c1     c1, k.ethernet_srcAddr, d.u.ipsg_hit_d.mac
  sne.!c1     c1, k.{vlan_tag_vid_sbit0_ebit3, vlan_tag_vid_sbit4_ebit11}, d.u.ipsg_hit_d.vlan_id
  phvwr.c1    p.control_metadata_drop_reason[DROP_IPSG], 1
  nop.e
  phvwr.c1    p.capri_intrinsic_drop, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_drop:
  phvwr.e     p.control_metadata_drop_reason[DROP_IPSG], 1
  phvwr       p.capri_intrinsic_drop, 1
