#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct validate_packet_k k;
struct phv_              p;

%%

validate_packet:
  seq         c1, k.tunnel_metadata_tunnel_terminate, TRUE
  bcf         [c1], validate_tunneled_packet

validate_native_packet:
  seq         c1, k.ethernet_srcAddr, r0
  seq         c2, k.ethernet_dstAddr, r0
  seq         c3, k.ethernet_srcAddr[40], 1
  sub         r1, r0, 1
  seq         c4, k.ethernet_srcAddr, r1[47:0]
  bcf         [c1|c2|c3|c4], malformed_packet
  seq         c1, k.ipv4_valid, TRUE
  seq         c2, k.ipv6_valid, TRUE

  .csbegin
  cswitch [c2,c1]
  nop
  .cscase 0
  nop.e
  nop
  .cscase 1
  sne         c1, k.ipv4_version, 4
  seq.!c1     c1, k.ipv4_ttl, 0
  nop.!c1.e
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
  .cscase 2
  sne         c1, k.ipv6_version, 6
  seq.!c1     c1, k.ipv6_hopLimit, 0
  nop.!c1.e
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
  .cscase 3
  nop.e
  nop
  .csend

validate_tunneled_packet:
  seq         c1, k.ethernet_srcAddr, r0
  seq.!c1     c1, k.ethernet_dstAddr, r0
  seq.!c1     c1, k.ethernet_srcAddr[40], 1
  sub         r1, r0, 1
  seq.!c1     c1, k.ethernet_srcAddr, r1[47:0]
  seq.!c1     c1, k.inner_ethernet_srcAddr, r0
  seq.!c1     c1, k.inner_ethernet_dstAddr, r0
  seq.!c1     c1, k.inner_ethernet_srcAddr[40], 1
  seq.!c1     c1, k.inner_ethernet_srcAddr, r1[47:0]
  b.c1        malformed_packet
  seq         c1, k.inner_ipv4_valid, TRUE
  seq         c2, k.inner_ipv6_valid, TRUE

  .csbegin
  cswitch [c2,c1]
  nop
  .cscase 0
  nop.e
  nop
  .cscase 1
  sne         c1, k.inner_ipv4_version, 4
  seq.!c1     c1, k.inner_ipv4_ttl, 0
  nop.!c1.e
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
  .cscase 2
  sne        c1, k.inner_ipv6_version, 6
  seq.!c1    c1, k.inner_ipv6_hopLimit, 0
  nop.!c1.e
  phvwr.c1.e  p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
  .cscase 3
  nop.e
  nop
  .csend

malformed_packet:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
