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
  phvwr       p.p4_to_p4plus_header_valid, TRUE
  phvwr       p.p4_to_p4plus_tcp_proxy_p4plus_app_id, k.control_metadata_p4plus_app_id

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
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_ipsec:
  nop.e
  nop
