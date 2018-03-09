#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"
#include "nw.h"

struct phv_ p;
struct tunnel_decap_k k;

%%

remove_tunnel_hdrs:
  K_DBG_WR(0x110)
  seq         c1, k.ipv4_valid, TRUE
  sub.c1      r1, k.ipv4_totalLen, 20
  sub.!c1     r1, r0, k.ipv6_payloadLen

  seq         c1, k.vxlan_valid, TRUE
  seq         c2, k.genv_valid, TRUE
  orcf        c3, [c1|c2]
  sub.c3      r1, r1, 16

  seq         c1, k.gre_valid, TRUE
  sub.c1      r1, r1, 4

  seq         c1, k.nvgre_valid, TRUE
  sub.c1      r1, r1, 4

  phvwr       p.capri_p4_intrinsic_packet_len, r1
  .assert(offsetof(p, vxlan_valid) - offsetof(p, vxlan_gpe_valid) == 2)
  phvwrmi     p.{vxlan_valid...vxlan_gpe_valid}, 0, 0x7
  .assert(offsetof(p, nvgre_valid) - offsetof(p, gre_valid) == 2)
  phvwrmi.e   p.{nvgre_valid...gre_valid}, 0, 0x5
  phvwr       p.{mpls_2_valid...mpls_0_valid}, 0

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_decap_error:
  nop.e
  nop
