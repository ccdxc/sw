#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_app_k k;
struct p4plus_app_d d;
struct phv_         p;

%%

nop:
  nop.e
  nop

.align
p4plus_app_classic_nic:
  seq         c1, k.inner_ipv4_valid, TRUE
  seq         c2, k.inner_ipv6_valid, TRUE
  setcf       c3, [c1|c2]
  bcf         [!c3], p4plus_app_classic_nic_native
  or          r1, k.p4_to_p4plus_classic_nic_flags, CLASSIC_NIC_FLAGS_FCS_OK

  phvwr.c1    p.p4_to_p4plus_classic_nic_ip_proto, k.inner_ipv4_protocol
  or.c1       r1, r1, CLASSIC_NIC_FLAGS_IPV4_VALID
  phvwr.c2    p.p4_to_p4plus_classic_nic_ip_proto, k.inner_ipv6_nextHdr
  or.c2       r1, r1, CLASSIC_NIC_FLAGS_IPV6_VALID
  phvwr.c3    p.p4_to_p4plus_classic_nic_inner_ip_valid, TRUE
  b           p4plus_app_classic_nic_l4
  or.c3       r1, r1, CLASSIC_NIC_FLAGS_TUNNELED

p4plus_app_classic_nic_native:
  phvwr       p.p4_to_p4plus_classic_nic_ip_valid, TRUE
  seq         c1, k.ipv4_valid, TRUE
  seq         c2, k.ipv6_valid, TRUE
  phvwr.c1    p.p4_to_p4plus_classic_nic_ip_proto, k.ipv4_protocol
  or.c1       r1, r1, CLASSIC_NIC_FLAGS_IPV4_VALID
  phvwr.c2    p.p4_to_p4plus_classic_nic_ip_proto, k.ipv6_nextHdr
  or.c2       r1, r1, CLASSIC_NIC_FLAGS_IPV6_VALID

p4plus_app_classic_nic_l4:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_l4_inner_udp
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_sport, k.tcp_srcPort
  phvwr       p.p4_to_p4plus_classic_nic_l4_dport, k.tcp_dstPort
  b           p4plus_app_classic_nic_common
  phvwr       p.p4_to_p4plus_classic_nic_l4_checksum, k.tcp_checksum

p4plus_app_classic_nic_l4_inner_udp:
  seq         c1, k.inner_udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_l4_udp
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_sport, k.inner_udp_srcPort
  phvwr       p.p4_to_p4plus_classic_nic_l4_dport, k.inner_udp_dstPort
  b           p4plus_app_classic_nic_common
  phvwr       p.p4_to_p4plus_classic_nic_l4_checksum, k.inner_udp_checksum

p4plus_app_classic_nic_l4_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_common
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_sport, k.udp_srcPort
  phvwr       p.p4_to_p4plus_classic_nic_l4_dport, k.udp_dstPort
  phvwr       p.p4_to_p4plus_classic_nic_l4_checksum, k.udp_checksum

p4plus_app_classic_nic_common:
  phvwr       p.p4_to_p4plus_classic_nic_flags, r1
  phvwr       p.p4_to_p4plus_classic_nic_valid, TRUE
  phvwr       p.p4_to_p4plus_classic_nic_p4plus_app_id, k.control_metadata_p4plus_app_id
  phvwr       p.p4_to_p4plus_classic_nic_packet_len, k.control_metadata_packet_len
  phvwr       p.capri_rxdma_p4_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
              (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
              P4PLUS_CLASSIC_NIC_HDR_SZ)
  phvwr.e     p.capri_rxdma_intrinsic_qid, k.control_metadata_qid
  phvwr       p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype

.align
p4plus_app_tcp_proxy:
  phvwr       p.ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr       p.ipv4_valid, FALSE
  phvwr       p.ipv6_valid, FALSE
  phvwr       p.tcp_valid, FALSE

  phvwr       p.p4_to_p4plus_tcp_proxy_valid, TRUE
  phvwr       p.p4_to_p4plus_tcp_proxy_sack_valid, TRUE
  phvwr       p.p4_to_p4plus_tcp_proxy_payload_len, k.l4_metadata_tcp_data_len
  phvwr       p.p4_to_p4plus_tcp_proxy_p4plus_app_id, k.control_metadata_p4plus_app_id

  or          r1, k.tcp_option_one_sack_valid, k.tcp_option_two_sack_valid, 1
  or          r1, r1, k.tcp_option_three_sack_valid, 2
  or          r1, r1, k.tcp_option_four_sack_valid, 3
  indexn      r2, r1, [0xF, 0x7, 0x3, 0x1, 0x0], 0
  phvwr       p.p4_to_p4plus_tcp_proxy_num_sack_blocks, r2

  phvwr       p.capri_rxdma_p4_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
              (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
              P4PLUS_TCP_PROXY_HDR_SZ)
  phvwr.e     p.capri_rxdma_intrinsic_qid, k.control_metadata_qid
  phvwr       p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype

.align
p4plus_app_cpu:
  or          r1, r0, r0
  seq         c1, k.ipv4_valid, TRUE
  seq         c2, k.ipv6_valid, TRUE
  seq         c3, k.inner_ipv4_valid, TRUE
  seq         c4, k.inner_ipv6_valid, TRUE
  seq         c5, k.inner_ethernet_valid, TRUE
  or.c1       r1, r1, CPU_FLAGS_IPV4_VALID
  or.c2       r1, r1, CPU_FLAGS_IPV6_VALID
  or.c3       r1, r1, CPU_FLAGS_INNER_IPV4_VALID
  or.c4       r1, r1, CPU_FLAGS_INNER_IPV6_VALID
  phvwr.c1    p.p4_to_p4plus_cpu_pkt_ip_proto_outer, k.ipv4_protocol
  phvwr.c2    p.p4_to_p4plus_cpu_pkt_ip_proto_outer, k.ipv6_nextHdr
  phvwr.c3    p.p4_to_p4plus_cpu_pkt_ip_proto_inner, k.inner_ipv4_protocol
  phvwr.c4    p.p4_to_p4plus_cpu_pkt_ip_proto_inner, k.inner_ipv6_nextHdr

  setcf       c6, [c3|c4|c5]
  bcf         [!c6], p4plus_app_cpu_native
  phvwr.c6    p.p4_to_p4plus_cpu_inner_ip_valid, TRUE

  phvwr.c3    p.p4_to_p4plus_cpu_ip_proto, k.inner_ipv4_protocol
  b           p4plus_app_cpu_l4_tcp
  phvwr.c4    p.p4_to_p4plus_cpu_ip_proto, k.inner_ipv6_nextHdr

p4plus_app_cpu_native:
  phvwr       p.p4_to_p4plus_cpu_ip_valid, TRUE
  phvwr.c1    p.p4_to_p4plus_cpu_ip_proto, k.ipv4_protocol
  phvwr.c2    p.p4_to_p4plus_cpu_ip_proto, k.ipv6_nextHdr

p4plus_app_cpu_l4_tcp:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_cpu_l4_icmp
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, k.tcp_srcPort
  b           p4plus_app_cpu_common
  phvwr       p.p4_to_p4plus_cpu_l4_dport, k.tcp_dstPort

p4plus_app_cpu_l4_icmp:
  seq         c1, k.icmp_valid, TRUE
  bcf         [!c1], p4plus_app_cpu_l4_inner_udp
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, k.icmp_typeCode
  b           p4plus_app_cpu_common
  nop

p4plus_app_cpu_l4_inner_udp:
  seq         c1, k.inner_udp_valid, TRUE
  bcf         [!c1], p4plus_app_cpu_l4_udp
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, k.inner_udp_srcPort
  b           p4plus_app_cpu_common
  phvwr       p.p4_to_p4plus_cpu_l4_dport, k.inner_udp_dstPort

p4plus_app_cpu_l4_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_cpu_common
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, k.udp_srcPort
  phvwr       p.p4_to_p4plus_cpu_l4_dport, k.udp_dstPort

p4plus_app_cpu_common:
  add         r2, k.control_metadata_packet_len, P4PLUS_CPU_PKT_SZ
  phvwr       p.p4_to_p4plus_cpu_packet_len, r2
  phvwr       p.p4_to_p4plus_cpu_pkt_flags, r1
  phvwr       p.p4_to_p4plus_cpu_valid, TRUE
  phvwr       p.p4_to_p4plus_cpu_pkt_valid, TRUE
  phvwr       p.capri_rxdma_p4_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
              (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
              P4PLUS_CPU_HDR_SZ)
  phvwr.e     p.capri_rxdma_intrinsic_qid, k.control_metadata_qid
  phvwr       p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype

.align
p4plus_app_ipsec:
  phvwr       p.p4_to_p4plus_ipsec_valid, TRUE
  phvwr       p.p4_to_p4plus_ipsec_p4plus_app_id, k.control_metadata_p4plus_app_id
  phvwr       p.p4_to_p4plus_ipsec_seq_no, k.ipsec_metadata_seq_no
  phvwr       p.p4_to_p4plus_ipsec_l4_protocol, k.ipv4_protocol
  sll         r2, k.ipv4_ihl, 2
  phvwr       p.p4_to_p4plus_ipsec_ip_hdr_size, r2 
  seq         c1, k.vlan_tag_valid, TRUE
  cmov        r6, c1, 18, 14
  phvwr       p.p4_to_p4plus_ipsec_ipsec_payload_start, r6 
  add         r3, r6, k.ipv4_totalLen
  phvwr       p.p4_to_p4plus_ipsec_ipsec_payload_end, r3
  phvwr       p.capri_rxdma_p4_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
              (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
              P4PLUS_IPSEC_HDR_SZ)
  phvwr.e     p.capri_rxdma_intrinsic_qid, k.control_metadata_qid
  phvwr       p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_rdma:
  phvwr       p.p4_to_p4plus_roce_p4plus_app_id, k.control_metadata_p4plus_app_id
  phvwr       p.ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr       p.ipv4_valid, FALSE
  phvwr.e     p.ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE
