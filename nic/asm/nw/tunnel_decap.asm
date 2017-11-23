#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;
struct tunnel_decap_k k;

%%

remove_tunnel_hdrs:
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
  .assert(offsetof(p, vxlan_valid) - offsetof(p, gre_valid) == 14)
  phvwrmi.e   p.{vxlan_valid...gre_valid}, 0, 0x6005
  phvwr       p.{mpls_2_valid...mpls_0_valid}, 0
