#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_tunnel_decap_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct phv_ p;
//struct tunnel_decap_k k;
struct tunnel_decap_k_ k;

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

  seq         c1, k.mpls_0_valid, TRUE
  add.c1      r1, r1, (14 - 8 - 4)

  phvwr.e     p.capri_p4_intrinsic_packet_len, r1
  phvwr.f     p.{gre_valid,vxlan_valid,genv_valid, \
                vxlan_gpe_valid,mpls_0_valid}, 0

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_decap_error:
  nop.e
  nop
