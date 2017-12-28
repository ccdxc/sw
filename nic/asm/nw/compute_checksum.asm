#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct compute_checksum_k k;
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
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr.e     p.ipv4_tcp_csum, COMPUTE_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum2:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_ipv4_udp
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr.e     p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum3:
  phvwr.e     p.ipv4_csum, COMPUTE_IP_CSUM
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum4:
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum5:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv4_inner_udp
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum6:
  phvwr.e     p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum7:
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr       p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM
  phvwr.e     p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum8:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv4_inner_udp
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr       p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM
  phvwr.e     p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum9:
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum10:
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr.e     p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum11:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv6_inner_udp
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr.e     p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum12:
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, r1
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM
  phvwr.e     p.inner_ipv6_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum13:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv6_inner_udp
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr       p.ipv4_csum, COMPUTE_IP_CSUM
  phvwr       p.ipv4_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM
  phvwr.e     p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum14:
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr.e     p.ipv6_tcp_csum, COMPUTE_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum15:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_ipv6_udp
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr.e     p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum16:
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum17:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv4_inner_udp
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum18:
  phvwr.e     p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum19:
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwr       p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.inner_ipv4_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum20:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv4_inner_udp
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr       p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwr       p.udp_csum, COMPUTE_L4_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM
  phvwr.e     p.inner_ipv4_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum21:
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwr.e     p.udp_csum, COMPUTE_L4_CSUM
  phvwr       p.inner_ipv4_csum, COMPUTE_INNER_IP_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum22:
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, \
                r1
  phvwrpair.e p.udp_csum, COMPUTE_L4_CSUM, \
                p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair   p.tcp_csum, COMPUTE_INNER_L4_CSUM, \
                p.inner_ipv6_tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum23:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv6_inner_udp
  phvwr       p.capri_deparser_len_l4_payload_len, k.udp_len
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwrpair.e p.udp_csum, COMPUTE_L4_CSUM, \
                p.ipv6_udp_csum, COMPUTE_L4_CSUM
  phvwrpair   p.inner_udp_csum, COMPUTE_INNER_L4_CSUM, \
                p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum24:
  add         r1, k.l4_metadata_tcp_data_len, k.tcp_dataOffset, 2
  phvwr       p.capri_deparser_len_inner_l4_payload_len, r1
  phvwr.e     p.inner_ipv6_tcp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.tcp_csum, COMPUTE_INNER_L4_CSUM

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
compute_checksum25:
  seq         c1, k.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  balcf       r7, [c1], icrc_inner_ipv6_inner_udp
  phvwr       p.capri_deparser_len_inner_l4_payload_len, k.inner_udp_len
  phvwr.e     p.inner_ipv6_udp_csum, COMPUTE_INNER_L4_CSUM
  phvwr       p.inner_udp_csum, COMPUTE_INNER_L4_CSUM

icrc_ipv4_udp:
  phvwr       p.ipv4_icrc, TRUE
  phvwr       p.udp_icrc, TRUE
  phvwr       p.roce_bth_icrc, TRUE
  phvwr       p.udp_opt_ocs_csum, k.udp_opt_ocs_valid
  sub         r6, k.ipv4_totalLen, k.control_metadata_udp_opt_bytes
  phvwr       p.capri_deparser_len_udp_opt_l2_checksum_len, k.control_metadata_udp_opt_bytes
  jr          r7
  phvwr       p.capri_deparser_len_icrc_payload_len, r6

icrc_inner_ipv4_inner_udp:
  phvwr       p.inner_ipv4_icrc, TRUE
  phvwr       p.inner_udp_icrc, TRUE
  phvwr       p.roce_bth_icrc, TRUE
  phvwr       p.udp_opt_ocs_csum, k.udp_opt_ocs_valid
  sub         r6, k.inner_ipv4_totalLen, k.control_metadata_udp_opt_bytes
  phvwr       p.capri_deparser_len_udp_opt_l2_checksum_len, k.control_metadata_udp_opt_bytes
  jr          r7
  phvwr       p.capri_deparser_len_icrc_payload_len, r6

icrc_ipv6_udp:
  phvwr       p.ipv6_icrc, TRUE
  phvwr       p.udp_icrc, TRUE
  phvwr       p.roce_bth_icrc, TRUE
  phvwr       p.udp_opt_ocs_csum, k.udp_opt_ocs_valid
  add         r6, k.ipv6_payloadLen, 40
  sub         r6, r6, k.control_metadata_udp_opt_bytes
  phvwr       p.capri_deparser_len_udp_opt_l2_checksum_len, k.control_metadata_udp_opt_bytes
  jr          r7
  phvwr       p.capri_deparser_len_icrc_payload_len, r6

icrc_inner_ipv6_inner_udp:
  phvwr       p.inner_ipv6_icrc, TRUE
  phvwr       p.inner_udp_icrc, TRUE
  phvwr       p.roce_bth_icrc, TRUE
  phvwr       p.udp_opt_ocs_csum, k.udp_opt_ocs_valid
  add         r6, k.inner_ipv6_payloadLen, 40
  sub         r6, r6, k.control_metadata_udp_opt_bytes
  phvwr       p.capri_deparser_len_udp_opt_l2_checksum_len, k.control_metadata_udp_opt_bytes
  jr          r7
  phvwr       p.capri_deparser_len_icrc_payload_len, r6
