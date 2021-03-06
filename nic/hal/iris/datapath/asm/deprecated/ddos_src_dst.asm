#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct ddos_src_dst_k   k;
struct ddos_src_dst_d   d;
struct phv_             p;

%%

nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ddos_src_dst_hit:
  smeqb     c1, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf       [c1], ddos_type_syn

  seq       c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_ICMP
  bcf       [c1], ddos_type_icmp

  seq       c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_ICMPV6
  bcf       [c1], ddos_type_icmp

  seq       c1, k.flow_lkp_metadata_lkp_proto, IP_PROTO_UDP
  bcf       [c1], ddos_type_udp

  add       r1, DDOS_TYPE_ANY, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

ddos_type_syn:
  add       r1, DDOS_TYPE_SYN, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

ddos_type_icmp:
  add       r1, DDOS_TYPE_ICMP, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

ddos_type_udp:
  add       r1, DDOS_TYPE_UDP, d.u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx, 2
  phvwr.e   p.ddos_metadata_ddos_src_dst_policer_idx, r1
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ddos_src_dst_error:
  nop.e
  nop

