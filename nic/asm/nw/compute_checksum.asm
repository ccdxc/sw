#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct compute_checksum_k k;
struct compute_checksum_d d;
struct phv_               p;

%%

nop:
  nop.e
  nop

// Since p4plus_app and compute checksum table run in same stage
// pkt bound towards host (RDMA/Proxy case) without L3/L4 Hdr
// should be handle correctly such that csum bits are NOT set
// before deparser reconstructs packet.
// Disabling TCAM table population for now. John to fix it later.

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum1:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum2:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum3:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum4:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum5:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum6:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum7:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum8:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum9:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum10:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum11:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum12:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum13:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum14:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum15:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum16:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum17:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum18:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum19:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum20:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum21:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum22:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum23:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum24:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum25:
  nop.e
  nop


/*
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum1:
  phvwr       p.ipv4_csum, TRUE
  phvwr.e     p.ipv4_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum2:
  phvwr       p.ipv4_csum, TRUE
  phvwr.e     p.ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum3:
  phvwr.e     p.ipv4_csum, TRUE
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum4:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum5:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_udp_csum, TRUE
  phvwr       p.inner_udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum6:
  phvwr.e     p.ipv4_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum7:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr       p.ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr.e     p.inner_ipv4_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum8:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr       p.ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr.e     p.inner_ipv4_udp_csum, TRUE
  phvwr       p.inner_udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum9:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum10:
  phvwr       p.ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum11:
  phvwr       p.ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_udp_csum, TRUE
  phvwr       p.inner_udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum12:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr.e     p.inner_ipv6_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum13:
  phvwr       p.ipv4_csum, TRUE
  phvwr       p.ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr.e     p.inner_ipv6_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum14:
  phvwr.e     p.ipv6_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum15:
  phvwr.e     p.ipv6_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum16:
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum17:
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum18:
  phvwr.e     p.inner_ipv4_csum, TRUE
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum19:
  phvwr       p.ipv6_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum20:
  phvwr       p.ipv6_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE
  phvwr.e     p.inner_ipv4_udp_csum, TRUE
  phvwr       p.inner_udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum21:
  phvwr       p.ipv6_udp_csum, TRUE
  phvwr.e     p.udp_csum, TRUE
  phvwr       p.inner_ipv4_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum22:
  phvwr       p.ipv6_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr.e     p.inner_ipv6_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum23:
  phvwr       p.ipv6_udp_csum, TRUE
  phvwr       p.udp_csum, TRUE
  phvwr.e     p.inner_ipv6_udp_csum, TRUE
  phvwr       p.inner_udp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum24:
  phvwr.e     p.inner_ipv6_tcp_csum, TRUE
  phvwr       p.tcp_csum, TRUE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum25:
  phvwr.e     p.inner_ipv6_udp_csum, TRUE
  phvwr       p.inner_udp_csum, TRUE

*/
