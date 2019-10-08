#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_ipsg_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct ipsg_k_ k;
struct ipsg_d  d;
struct phv_    p;

%%

ipsg_miss:
  seq         c1, k.control_metadata_ipsg_enable, TRUE
  bcf         [!c1], ipsg_disable
  smeqb       c1, k.flow_lkp_metadata_lkp_type, \
                FLOW_KEY_LOOKUP_TYPE_IP_MASK, FLOW_KEY_LOOKUP_TYPE_IP_MASK
  phvwr.c1    p.control_metadata_drop_reason[DROP_IPSG], 1
  nop.e
  phvwr.c1    p.capri_intrinsic_drop, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_hit:
  seq         c1, k.control_metadata_ipsg_enable, TRUE
  bcf         [!c1], ipsg_disable
  sne         c1, k.control_metadata_src_lif, d.u.ipsg_hit_d.src_lif
  sne.!c1     c1, k.ethernet_srcAddr, d.u.ipsg_hit_d.mac
  sne.!c1     c1, k.vlan_tag_vid, d.u.ipsg_hit_d.vlan_id
  phvwr.c1    p.control_metadata_drop_reason[DROP_IPSG], 1
  nop.e
  phvwr.c1    p.capri_intrinsic_drop, 1

ipsg_disable:
  nop.e
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_error:
  nop.e
  nop
