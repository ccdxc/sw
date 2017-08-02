#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ipsg_k k;
struct ipsg_d d;
struct phv_   p;

%%

ipsg_miss:
  seq         c1, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  seq         c2, k.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  setcf       c3, [c1|c2]
  bcf         [c3], ipsg_drop
  nop.!c5.e
  nop

.align
ipsg_hit:
  sne         c1, k.control_metadata_src_lif, d.u.ipsg_hit_d.lif
  sne         c2, k.ethernet_srcAddr, d.u.ipsg_hit_d.mac
  sne         c3, k.vlan_tag_valid, d.u.ipsg_hit_d.vlan_valid
  sne         c4, k.vlan_tag_vid, d.u.ipsg_hit_d.vlan_id
  setcf       c5, [c1|c2|c3|c4]
  bcf         [c5], ipsg_drop
  nop.!c5.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_drop:
  phvwr.e     p.control_metadata_drop_reason[DROP_IPSG], 1
  phvwr       p.capri_intrinsic_drop, 1

