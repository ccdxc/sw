#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"
#include "nw.h"

struct ipsg_k k;
struct ipsg_d d;
struct phv_   p;

%%

ipsg_miss:
  K_DBG_WR(0x70)
  DBG_WR(0x78, 0x78)
  bbne        k.control_metadata_ipsg_enable, TRUE, f_normalization
  smeqb       c1, k.flow_lkp_metadata_lkp_type, \
                FLOW_KEY_LOOKUP_TYPE_IP_MASK, FLOW_KEY_LOOKUP_TYPE_IP_MASK
  phvwr.c1    p.control_metadata_drop_reason[DROP_IPSG], 1
  b           f_normalization
  phvwr.c1    p.capri_intrinsic_drop, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_hit:
  K_DBG_WR(0x70)
  DBG_WR(0x79, 0x79)
  bbne        k.control_metadata_ipsg_enable, TRUE, f_normalization
  sne         c1, k.control_metadata_src_lif, d.u.ipsg_hit_d.src_lif
  sne.!c1     c1, k.ethernet_srcAddr, d.u.ipsg_hit_d.mac
  sne.!c1     c1, k.{vlan_tag_vid_sbit0_ebit3, vlan_tag_vid_sbit4_ebit11}, d.u.ipsg_hit_d.vlan_id
  phvwr.c1    p.control_metadata_drop_reason[DROP_IPSG], 1
  DBG_WR(0x7a, r1)
  b           f_normalization
  phvwr.c1    p.capri_intrinsic_drop, 1

f_normalization:
  bbeq        k.l4_metadata_tcp_normalization_en, TRUE, tcp_stateless_normalization
  bbeq        k.l4_metadata_icmp_normalization_en, TRUE, icmp_normalization
  nop
  nop.e
  nop

#include "icmp_normalization.asm"
#include "tcp_stateless_normalization.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipsg_error:
  nop.e
  nop
