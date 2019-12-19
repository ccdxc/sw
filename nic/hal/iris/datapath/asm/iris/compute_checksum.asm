#include "egress.h"
#include "EGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"
#include "EGRESS_compute_checksum_k.h"

struct compute_checksum_k_ k;
struct compute_checksum_d d;
struct phv_               p;

#define COMPUTE_IP_CSUM       k.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM]
#define COMPUTE_L4_CSUM       k.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM]
#define COMPUTE_INNER_IP_CSUM k.control_metadata_checksum_ctl[CHECKSUM_CTL_INNER_IP_CHECKSUM]
#define COMPUTE_INNER_L4_CSUM k.control_metadata_checksum_ctl[CHECKSUM_CTL_INNER_L4_CHECKSUM]

%%

nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum1:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr         p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair.e   p.ipv4_tcp_csum, COMPUTE_L4_CSUM, p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr         p.tcp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum2:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_ipv4_udp
  phvwr         p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwrpair.e   p.ipv4_udp_csum, COMPUTE_L4_CSUM, p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr         p.udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum3:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  phvwr.e       p.ipv4_csum, COMPUTE_IP_CSUM
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum4:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr         p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair.e   p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwrpair     p.tcp_csum, COMPUTE_INNER_L4_CSUM, p.ipv4_csum, COMPUTE_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum5:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv4_inner_udp
  phvwr         p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwrpair.e   p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair     p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum6:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  phvwr.e       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr         p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum7:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  phvwr         p.capri_deparser_len_l4_payload_len, k.udp_len
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr         p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair     p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum8:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv4_inner_udp
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwrpair     p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum9:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  phvwr         p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwrpair.e   p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair     p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum10:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr         p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair.e   p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr         p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum11:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv6_inner_udp
  phvwr         p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr.e       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair     p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum12:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, r1
  phvwr         p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum13:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv6_inner_udp
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr         p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum14:
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr.e       p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair     p.tcp_csum, COMPUTE_L4_CSUM, p.ipv6_tcp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum15:
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_ipv6_udp
  phvwr         p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr.e       p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwr         p.udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum16:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr         p.capri_deparser_len_inner_l4_payload_len, r1
  phvwr.e       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwrpair     p.tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum17:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv4_inner_udp
  phvwr         p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr.e       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwrpair     p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum18:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  phvwr.e       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum19:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair     p.udp_csum, COMPUTE_L4_CSUM, p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair.e   p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr         p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum20:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv4_inner_udp
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwrpair     p.udp_csum, COMPUTE_L4_CSUM, p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair.e   p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr         p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum21:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  phvwr         p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwr         p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum22:
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.tcp_csum, COMPUTE_INNER_L4_CSUM, \
                p.inner_ipv6_tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum23:
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv6_inner_udp
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, \
                    p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum24:
  add           r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr.e       p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair     p.tcp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum25:
  seq           c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf         r7, [c1], icrc_inner_ipv6_inner_udp
  phvwr         p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr.e       p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr         p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum26:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  sub           r1, k.inner_ipv4_totalLen, k.inner_ipv4_ihl, 2
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, r1
  phvwr         p.icmp_csum, COMPUTE_INNER_L4_CSUM
  phvwrpair.e   p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair     p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum27:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, \
                    k.inner_ipv6_payloadLen
  phvwr         p.inner_ipv6_icmp_csum, COMPUTE_INNER_L4_CSUM
  phvwrpair.e   p.icmp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwrpair     p.udp_csum, COMPUTE_L4_CSUM, p.ipv4_udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum28:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  sub           r1, k.inner_ipv4_totalLen, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair.e   p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr         p.icmp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum29:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  phvwr         p.capri_deparser_len_inner_l4_payload_len, \
                    k.inner_ipv6_payloadLen
  phvwr.e       p.inner_ipv6_icmp_csum, COMPUTE_INNER_L4_CSUM
  phvwrpair     p.icmp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.ipv4_csum, COMPUTE_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum30:
  add           r1, r0, k.ipv4_ihl, 2
  phvwr         p.capri_deparser_len_ipv4_hdr_len, r1
  sub           r1, k.ipv4_totalLen, k.ipv4_ihl, 2
  phvwr.e       p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair     p.icmp_csum, COMPUTE_L4_CSUM, p.ipv4_csum, COMPUTE_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum31:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  sub           r1, k.inner_ipv4_totalLen, k.inner_ipv4_ihl, 2
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.icmp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum32:
  phvwrpair     p.capri_deparser_len_l4_payload_len, k.udp_len, \
                    p.capri_deparser_len_inner_l4_payload_len, \
                    k.inner_ipv6_payloadLen
  phvwrpair.e   p.udp_csum, COMPUTE_L4_CSUM, p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair     p.icmp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_icmp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum33:
  add           r1, r0, k.inner_ipv4_ihl, 2
  phvwr         p.capri_deparser_len_inner_ipv4_hdr_len, r1
  sub           r1, k.inner_ipv4_totalLen, k.inner_ipv4_ihl, 2
  phvwr.e       p.capri_deparser_len_inner_l4_payload_len, r1
  phvwrpair     p.icmp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum34:
  phvwr.e       p.capri_deparser_len_inner_l4_payload_len, \
                    k.inner_ipv6_payloadLen
  phvwrpair     p.icmp_csum, COMPUTE_INNER_L4_CSUM, \
                    p.inner_ipv6_icmp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum35:
  phvwr.e       p.capri_deparser_len_inner_l4_payload_len, \
                    k.ipv6_payloadLen
  phvwrpair     p.icmp_csum, COMPUTE_L4_CSUM, \
                    p.ipv6_icmp_csum, COMPUTE_L4_CSUM

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum_error:
  nop.e
  nop

icrc_ipv4_udp:
  phvwrpair     p.udp_icrc, TRUE, p.ipv4_icrc, TRUE
  phvwrpair     p.udp_opt_ocs_csum, k.udp_opt_ocs_valid, p.roce_bth_icrc, TRUE
  sub           r6, k.ipv4_totalLen, k.control_metadata_udp_opt_bytes
  jr            r7
  phvwrpair     p.capri_deparser_len_icrc_payload_len, r6, \
                    p.capri_deparser_len_udp_opt_l2_checksum_len[7:0], \
                    k.control_metadata_udp_opt_bytes

icrc_inner_ipv4_inner_udp:
  phvwrpair     p.inner_udp_icrc, TRUE, p.inner_ipv4_icrc, TRUE
  phvwrpair     p.udp_opt_ocs_csum, k.udp_opt_ocs_valid, p.roce_bth_icrc, TRUE
  sub           r6, k.inner_ipv4_totalLen, k.control_metadata_udp_opt_bytes
  jr            r7
  phvwrpair     p.capri_deparser_len_icrc_payload_len, r6, \
                    p.capri_deparser_len_udp_opt_l2_checksum_len[7:0], \
                    k.control_metadata_udp_opt_bytes

icrc_ipv6_udp:
  phvwrpair     p.udp_icrc, TRUE, p.ipv6_icrc, TRUE
  phvwrpair     p.udp_opt_ocs_csum, k.udp_opt_ocs_valid, p.roce_bth_icrc, TRUE
  add           r6, k.ipv6_payloadLen, 40
  sub           r6, r6, k.control_metadata_udp_opt_bytes
  jr            r7
  phvwrpair     p.capri_deparser_len_icrc_payload_len, r6, \
                    p.capri_deparser_len_udp_opt_l2_checksum_len[7:0], \
                    k.control_metadata_udp_opt_bytes

icrc_inner_ipv6_inner_udp:
  phvwrpair     p.inner_udp_icrc, TRUE, p.inner_ipv6_icrc, TRUE
  phvwrpair     p.udp_opt_ocs_csum, k.udp_opt_ocs_valid, p.roce_bth_icrc, TRUE
  add           r6, k.inner_ipv6_payloadLen, 40
  sub           r6, r6, k.control_metadata_udp_opt_bytes
  jr            r7
  phvwrpair     p.capri_deparser_len_icrc_payload_len, r6, \
                    p.capri_deparser_len_udp_opt_l2_checksum_len[7:0], \
                    k.control_metadata_udp_opt_bytes
