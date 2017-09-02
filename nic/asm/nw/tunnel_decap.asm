#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;
struct tunnel_decap_k k;

%%

remove_tunnel_hdrs:
  seq         c1, k.ipv4_valid, TRUE
  sub.c1      r1, k.ipv4_totalLen, 34
  sub.!c1     r1, k.ipv6_payloadLen, 14

  seq         c1, k.vxlan_valid, TRUE
  seq         c2, k.genv_valid, TRUE
  orcf        c3, [c1|c2]
  sub.c3      r1, r1, 16
  
  seq         c1, k.gre_valid, TRUE
  sub.c1      r1, r1, 4

  seq         c1, k.nvgre_valid, TRUE
  sub.c1      r1, r1, 4

  phvwr       p.l3_metadata_payload_length, r1
  phvwr.e     p.{vxlan_valid...gre_valid}, 0
  phvwr       p.{mpls_2_valid...mpls_0_valid}, 0
