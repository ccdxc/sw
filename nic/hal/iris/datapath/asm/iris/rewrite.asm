#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_rewrite_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct rewrite_k_ k;
struct rewrite_d  d;
struct phv_       p;

%%

nop:
  nop.e
  nop

.align
rewrite:
  seq         c1, k.vlan_tag_valid, 1
  phvwr.c1    p.ethernet_etherType, k.vlan_tag_etherType
  phvwr.c1    p.vlan_tag_valid, 0
  sub.c1      r1, k.capri_p4_intrinsic_packet_len, 4
  phvwr.c1    p.capri_p4_intrinsic_packet_len, r1

  smeqb       c1, k.rewrite_metadata_flags, REWRITE_FLAGS_MAC_SA, REWRITE_FLAGS_MAC_SA
  phvwr.c1    p.ethernet_srcAddr, d.u.rewrite_d.mac_sa
  smeqb       c1, k.rewrite_metadata_flags, REWRITE_FLAGS_MAC_DA, REWRITE_FLAGS_MAC_DA
  phvwr.c1    p.ethernet_dstAddr, d.u.rewrite_d.mac_da

  add         r6, r0, k.ipv4_valid
  or          r6, r6, k.ipv6_valid, 1
  smeqb       c1, k.rewrite_metadata_flags, REWRITE_FLAGS_TTL_DEC, REWRITE_FLAGS_TTL_DEC
  seq         c2, k.qos_metadata_dscp_en, 1
  setcf       c3, [c1 | c2]
  .brbegin
  br          r6[1:0]
  phvwr.c3    p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE
  .brcase 0
  nop.e
  nop
  .brcase 1
  phvwr.c2    p.ipv4_diffserv, k.qos_metadata_dscp
  sub.e       r7, k.ipv4_ttl, 1
  phvwr.c1    p.ipv4_ttl, r7
  .brcase 2
  phvwr.c2    p.ipv6_trafficClass, k.qos_metadata_dscp
  sub.e       r7, k.ipv6_hopLimit, 1
  phvwr.c1    p.ipv6_hopLimit, r7
  .brcase 3
  nop.e
  nop
  .brend

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rewrite_error:
  nop.e
  nop
