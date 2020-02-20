#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_p4plus_app_k.h"
#include "CSUM_EGRESS.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct p4plus_app_k_ k;
struct phv_          p;

%%

.align
p4plus_app_default:
  slt         c1, k.capri_p4_intrinsic_packet_len, MIN_ETHER_FRAME_LEN
  phvwr.c1    p.capri_p4_intrinsic_packet_len, MIN_ETHER_FRAME_LEN
  seq         c1, k.tcp_valid, TRUE
  sle         c2, k.tcp_dataOffset, 5
  setcf.e     c1, [c1 & !c2]
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr.c1    p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0

.align
p4plus_app_classic_nic:
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr         p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0
  // r7 : packet_len
  or            r7, r0, k.capri_p4_intrinsic_packet_len
  phvwr         p.p4_to_p4plus_classic_nic_l2csum, TRUE

  seq           c1, k.vlan_tag_valid, TRUE
  seq           c2, k.control_metadata_vlan_strip, TRUE
  bcf           ![c1&c2], p4plus_app_classic_nic_no_vlan_strip
  add           r1, r0, k.vlan_tag_vid
  phvwr         p.p4_to_p4plus_classic_nic_vlan_valid, TRUE
  phvwr         p.ethernet_etherType, k.vlan_tag_etherType
  phvwrpair     p.{p4_to_p4plus_classic_nic_vlan_pcp, p4_to_p4plus_classic_nic_vlan_dei}, \
                    k.{vlan_tag_pcp, vlan_tag_dei}, \
                    p.p4_to_p4plus_classic_nic_vlan_vid, r1
  phvwr         p.vlan_tag_valid, FALSE
  sub           r7, r7, 4
  // l2 checksum (CHECKSUM_COMPLETE)
  b             p4plus_app_classic_nic_post_vlan_strip
  phvwrpair     p.ipv4_l2csum, k.ipv4_valid, p.ipv6_l2csum, k.ipv6_valid

p4plus_app_classic_nic_no_vlan_strip:
  // l2 checksum (CHECKSUM_COMPLETE)
  phvwr.c1      p.vlan_tag_l2csum, TRUE
  phvwrpair.!c1 p.ipv4_l2csum, k.ipv4_valid, p.ipv6_l2csum, k.ipv6_valid

p4plus_app_classic_nic_post_vlan_strip:
  sub           r6, r7, 14
  phvwrpair     p.p4_to_p4plus_classic_nic_valid, TRUE, \
                p.capri_rxdma_intrinsic_valid, TRUE
  phvwrpair     p.capri_deparser_len_udp_opt_l2_checksum_len, r6, \
                p.p4_to_p4plus_classic_nic_p4plus_app_id, \
                k.control_metadata_p4plus_app_id[3:0]
  phvwr         p.capri_rxdma_intrinsic_rx_splitter_offset, \
                (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                P4PLUS_CLASSIC_NIC_HDR_SZ)
  phvwrpair.e   p.capri_rxdma_intrinsic_qid, k.control_metadata_qid, \
                p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype[2:0]
  phvwr.f       p.p4_to_p4plus_classic_nic_packet_len, r7

.align
p4plus_app_tcp_proxy:
  smeqb         c7, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  balcf         r7, [c7], f_p4plus_cpu_pkt
  add           r6, r0, r0 // pass packet start offset = 0

  phvwrpair.!c7 p.vlan_tag_valid, FALSE, p.ethernet_valid, FALSE
  phvwrpair.!c7 p.ipv4_valid, FALSE, p.ipv6_valid, FALSE
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_options_blob_valid) == 12)
  phvwrpair.!c7 p.{tcp_option_eol_valid...tcp_options_blob_valid}, r0, p.tcp_valid, r0
  or            r3, r0, k.capri_p4_intrinsic_packet_len
  add           r3, r3, P4PLUS_CPU_PKT_SZ
  add.!c7       r3, r0, k.l4_metadata_tcp_data_len

  phvwr.!c7     p.capri_deparser_len_trunc_pkt_len, k.l4_metadata_tcp_data_len
  phvwr.!c7     p.{capri_intrinsic_payload,capri_deparser_len_trunc}, 0x1

  phvwrpair     p.p4_to_p4plus_tcp_proxy_sack_valid, TRUE, \
                    p.p4_to_p4plus_tcp_proxy_valid, TRUE
  phvwrpair     p.p4_to_p4plus_tcp_proxy_p4plus_app_id, \
                    k.control_metadata_p4plus_app_id[3:0], \
                    p.{p4_to_p4plus_tcp_proxy_table0_valid,p4_to_p4plus_tcp_proxy_table1_valid}, 0x3

  or            r1, k.tcp_option_one_sack_valid, k.tcp_option_two_sack_valid, 1
  or            r1, r1, k.tcp_option_three_sack_valid, 2
  or            r1, r1, k.tcp_option_four_sack_valid, 3
  indexn        r2, r1, [0xF, 0x7, 0x3, 0x1, 0x0], 0
  or            r2, r2, k.tcp_option_timestamp_valid, 3
  seq           c1, k.ipv4_valid, 1
  or.c1         r2, r2, k.ipv4_diffserv[1:0], 4
  or.!c1        r2, r2, k.ipv6_trafficClass_s4_e7[1:0]
  or            r3, r3, r2, 16
  phvwr         p.{p4_to_p4plus_tcp_proxy_ecn, \
                    p4_to_p4plus_tcp_proxy_timestamp_valid, \
                    p4_to_p4plus_tcp_proxy_num_sack_blocks, \
                    p4_to_p4plus_tcp_proxy_payload_len}, r3

  phvwr         p.capri_rxdma_intrinsic_valid, TRUE
  phvwr.e       p.capri_rxdma_intrinsic_rx_splitter_offset, \
                    (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                     CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_TCP_PROXY_HDR_SZ)
  phvwrpair     p.capri_rxdma_intrinsic_qid, k.control_metadata_qid, \
                    p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype[2:0]

.align
p4plus_app_cpu:
  phvwrpair   p.p4_to_p4plus_cpu_p4plus_app_id, \
                    k.control_metadata_p4plus_app_id[3:0], \
                    p.p4_to_p4plus_cpu_table0_valid, TRUE
  add         r6, r0, r0 // pass packet start offset = 0

p4plus_app_cpu_raw_redir_common:
  or            r1, r0, r0
  seq           c1, k.ipv4_valid, TRUE
  seq           c2, k.ipv6_valid, TRUE
  phvwr.c1      p.p4_to_p4plus_cpu_ip_proto, k.ipv4_protocol
  phvwr.c1      p.p4_to_p4plus_cpu_packet_type, CPU_PACKET_TYPE_IPV4
  phvwr.c2      p.p4_to_p4plus_cpu_ip_proto, k.ipv6_nextHdr
  phvwr.c2      p.p4_to_p4plus_cpu_packet_type, CPU_PACKET_TYPE_IPV6

  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_cpu_l4_icmp
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, k.tcp_srcPort
  b           p4plus_app_cpu_common
  phvwr       p.p4_to_p4plus_cpu_l4_dport, k.tcp_dstPort

p4plus_app_cpu_l4_icmp:
  seq         c1, k.icmp_valid, TRUE
  or.c1       r1, k.icmp_typeCode_s8_e15, \
              k.icmp_typeCode_s0_e7, 8
  bcf         [!c1], p4plus_app_cpu_l4_udp
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, r1
  b           p4plus_app_cpu_common
  nop

p4plus_app_cpu_l4_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_cpu_common
  phvwr.c1    p.p4_to_p4plus_cpu_l4_sport, k.udp_srcPort
  phvwr       p.p4_to_p4plus_cpu_l4_dport, k.udp_dstPort

p4plus_app_cpu_common:
  add         r2, r0, k.capri_p4_intrinsic_packet_len
  add         r2, P4PLUS_CPU_PKT_SZ, r2
  phvwr       p.p4_to_p4plus_cpu_packet_len, r2

  bal         r7, f_p4plus_cpu_pkt
  .assert(offsetof(p, p4_to_p4plus_cpu_ip_valid) - offsetof(p, capri_rxdma_intrinsic_valid) == 2)
  phvwr       p.{p4_to_p4plus_cpu_ip_valid, p4_to_p4plus_cpu_valid, \
                 capri_rxdma_intrinsic_valid}, 0x7
  phvwr.e     p.capri_rxdma_intrinsic_rx_splitter_offset, \
              (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
              P4PLUS_CPU_HDR_SZ)
  phvwrpair   p.capri_rxdma_intrinsic_qid, k.control_metadata_qid, \
                p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype[2:0]

.align
p4plus_app_ipsec:
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr       p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0
  phvwr       p.p4_to_p4plus_ipsec_valid, TRUE
  phvwr       p.p4_to_p4plus_ipsec_p4plus_app_id, k.control_metadata_p4plus_app_id
  phvwr       p.p4_to_p4plus_ipsec_table1_valid, TRUE
  phvwr       p.p4_to_p4plus_ipsec_seq_no, k.ipsec_metadata_seq_no
  add         r4, k.flow_lkp_metadata_lkp_dport,k.flow_lkp_metadata_lkp_sport, 16
  phvwr       p.p4_to_p4plus_ipsec_spi, r4
  phvwr       p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
              (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
              P4PLUS_IPSEC_HDR_SZ)
  phvwr      p.capri_rxdma_intrinsic_qid, k.control_metadata_qid
  phvwr       p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype
  seq         c1, k.vlan_tag_valid, TRUE
  cmov        r6, c1, 18, 14
  phvwr       p.p4_to_p4plus_ipsec_ipsec_payload_start, r6
  seq         c2, k.ipv6_valid, TRUE
  bcf         [c2], p4plus_app_ipsec_ipv6
  phvwr       p.p4_to_p4plus_ipsec_l4_protocol, k.ipv4_protocol
  sll         r2, k.ipv4_ihl, 2
  seq         c3, k.udp_valid, TRUE
  add.c3      r2, r2, 8
  phvwr       p.p4_to_p4plus_ipsec_ip_hdr_size, r2
  or          r1, k.ipv4_totalLen, k.ipv4_totalLen, 0
  add.e       r3, r6, r1
  phvwr       p.p4_to_p4plus_ipsec_ipsec_payload_end, r3

p4plus_app_ipsec_ipv6:
  phvwri      p.v6_generic_valid, 0
  seq         c3, k.udp_valid, TRUE
  phvwri.!c3  p.p4_to_p4plus_ipsec_ip_hdr_size, 40
  phvwri.c3   p.p4_to_p4plus_ipsec_ip_hdr_size, 48
  phvwr       p.p4_to_p4plus_ipsec_l4_protocol, k.ipv6_nextHdr
  add.e       r5, r6, k.ipv6_payloadLen
  phvwr       p.p4_to_p4plus_ipsec_ipsec_payload_end, r5

.align
p4plus_app_raw_redir:
  b           p4plus_app_cpu_raw_redir_common
  add         r6, r0, P4PLUS_RAW_REDIR_HDR_SZ // pass packet start offset

.align
p4plus_app_rdma:
  bbne          k.ipv4_valid, TRUE, p4plus_app_rdma_ipv6
  seq           c1, k.control_metadata_rdma_ud, TRUE
  phvwr.c1      p.p4_to_p4plus_roce_ipv4_valid, TRUE
  b             p4plus_app_rdma_common
  phvwr         p.p4_to_p4plus_roce_ecn, k.ipv4_diffserv[1:0]

p4plus_app_rdma_ipv6:
  phvwr.c1      p.p4_to_p4plus_roce_ipv6_valid, TRUE
  phvwr         p.p4_to_p4plus_roce_ecn, k.ipv6_trafficClass_s4_e7[1:0]

p4plus_app_rdma_common:
  phvwr         p.p4_to_p4plus_roce_p4plus_app_id, k.control_metadata_p4plus_app_id
  seq           c1, k.udp_opt_ocs_valid, FALSE
  seq           c2, k.capri_intrinsic_csum_err[csum_hdr_udp_opt_ocs], 0
  orcf          c1, [!c1 & c2]
  phvwrpair.c1  p.p4_to_p4plus_roce_roce_opt_ts_valid, k.udp_opt_timestamp_valid, \
                    p.p4_to_p4plus_roce_roce_opt_mss_valid, k.udp_opt_mss_valid
  phvwrpair     p.vlan_tag_valid, FALSE, p.ethernet_valid, FALSE
  phvwrpair.e   p.ipv4_valid, FALSE, p.ipv6_valid, FALSE
  .assert(offsetof(p, udp_opt_eol_valid) - offsetof(p, icrc_valid) == 7)
  phvwrpair     p.{udp_opt_eol_valid...icrc_valid}, 0, p.udp_valid, FALSE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_p4pt:
  phvwr       p.p4_to_p4plus_p4pt_valid, TRUE
  phvwr       p.p4_to_p4plus_p4pt_p4plus_app_id, k.control_metadata_p4plus_app_id
  phvwr       p.p4_to_p4plus_p4pt_p4pt_idx, k.flow_info_metadata_flow_index

  add         r1, r0, r0
  seq         c1, k.tcp_valid, TRUE
  slt         c2, k.l4_metadata_tcp_data_len, 64
  cmov.c1     r1, c2, k.l4_metadata_tcp_data_len, 64
  seq         c1, k.udp_valid, TRUE
  slt         c2, k.udp_len, 64
  cmov.c1     r1, c2, k.udp_len, 64

  phvwr       p.ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr       p.ipv4_valid, FALSE
  phvwr       p.ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_options_blob_valid) == 12)
  phvwr       p.{tcp_option_eol_valid...tcp_options_blob_valid}, r0
  phvwr       p.tcp_valid, r0

  phvwr       p.p4_to_p4plus_p4pt_payload_len, r1
  add         r1, r1, (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                       CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_P4PT_HDR_SZ)
  phvwr       p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, r1
  phvwr.e     p.capri_rxdma_intrinsic_qid, k.control_metadata_qid
  phvwr       p.capri_rxdma_intrinsic_qtype, k.control_metadata_qtype

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_mirror:
  phvwr       p.p4_to_p4plus_mirror_p4plus_app_id, \
                k.control_metadata_p4plus_app_id
  add         r1, r0, k.capri_p4_intrinsic_packet_len
  add         r2, r1, P4PLUS_MIRROR_PKT_SZ
  phvwrpair   p.p4_to_p4plus_mirror_payload_len, r2, \
                p.p4_to_p4plus_mirror_capture_len, r1
  seq         c1, k.capri_intrinsic_tm_iport, TM_PORT_EGRESS
  phvwr.c1    p.p4_to_p4plus_mirror_direction, 1
#ifndef CAPRI_IGNORE_TIMESTAMP
  phvwr       p.p4_to_p4plus_mirror_timestamp, r6
#endif
  phvwrpair.e p.p4_to_p4plus_mirror_valid, TRUE, \
                p.capri_rxdma_intrinsic_valid, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
                (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +\
                 P4PLUS_MIRROR_HDR_SZ)

// input r6 : packet start offset
// do not use c7 in this function
f_p4plus_cpu_pkt:
  phvwr       p.p4_to_p4plus_cpu_pkt_src_app_id, k.control_metadata_src_app_id
  phvwr       p.{p4_to_p4plus_cpu_tcp_pkt_valid,p4_to_p4plus_cpu_pkt_valid}, 0x3
  phvwr       p.p4_to_p4plus_cpu_pkt_src_lif, k.{control_metadata_src_lif}.hx
  or          r1, k.capri_intrinsic_lif_s3_e10, k.capri_intrinsic_lif_s0_e2, 8
  phvwr       p.p4_to_p4plus_cpu_pkt_lif, r1[15:0].hx
  phvwr       p.p4_to_p4plus_cpu_pkt_src_lport, k.{control_metadata_src_lport}.hx
  phvwr       p.p4_to_p4plus_cpu_pkt_qid, k.{control_metadata_qid}.wx
  phvwr       p.p4_to_p4plus_cpu_pkt_qtype, k.control_metadata_qtype
  phvwr       p.p4_to_p4plus_cpu_pkt_lkp_vrf, k.{flow_lkp_metadata_lkp_vrf}.hx
  phvwr       p.{p4_to_p4plus_cpu_pkt_lkp_dir, p4_to_p4plus_cpu_pkt_lkp_inst, \
                 p4_to_p4plus_cpu_pkt_lkp_type}, \
                  k.control_metadata_lkp_flags_egress[3:0]
  phvwr       p.p4_to_p4plus_cpu_pkt_flow_hash, k.{rewrite_metadata_entropy_hash}.wx
  seq         c1, k.control_metadata_lkp_flags_egress[CPU_LKP_FLAGS_LKP_DIR], FLOW_DIR_FROM_DMA
  phvwr.c1    p.p4_to_p4plus_cpu_pkt_src_tm_iq[4:0], k.control_metadata_src_tm_iq
  // r1 : offset
  // r2 : flags
  add         r1, r0, r6
  add         r2, r0, r0
  seq         c1, k.tunnel_metadata_tunnel_terminate_egress, TRUE
  or.c1       r2, r2, CPU_FLAGS_TUNNEL_TERMINATE
  seq         c1, k.ethernet_valid, TRUE
  phvwr.c1    p.p4_to_p4plus_cpu_pkt_l2_offset, r1.hx
  phvwr.!c1   p.p4_to_p4plus_cpu_pkt_l2_offset, -1
  seq         c2, k.vlan_tag_valid, TRUE
  or.c2       r2, r2, CPU_FLAGS_VLAN_VALID
  cmov        r3, c2, 18, 14
  add.c1      r1, r1, r3
  seq         c1, k.ipv4_valid, TRUE
  seq         c2, k.ipv6_valid, TRUE
  setcf       c3, [c1|c2]
  phvwr.c3    p.p4_to_p4plus_cpu_pkt_l3_offset, r1.hx
  phvwr.!c3   p.p4_to_p4plus_cpu_pkt_l3_offset, -1
  add.c1      r1, r1, k.ipv4_ihl, 2
  or.c1       r2, r2, CPU_FLAGS_IPV4_VALID
  add.c2      r1, r1, 40
  or.c2       r2, r2, CPU_FLAGS_IPV6_VALID
  seq         c2, k.ipv4_ihl, 5
  setcf       c1, [c1 & !c2]
  or.c1       r2, r2, CPU_FLAGS_IP_OPTIONS_PRESENT
  seq         c1, k.tcp_valid, TRUE
  b.c1        lb_cpu_pkt_tcp
  or          r6, k.udp_valid, k.esp_valid
  seq         c2, r6, 1
  seq         c3, k.icmp_valid, 1
  seq         c4, k.ah_valid, 1
  setcf       c5, [c2|c3|c4]
  phvwr.c5    p.p4_to_p4plus_cpu_pkt_l4_offset, r1.hx
  phvwr.!c5   p.p4_to_p4plus_cpu_pkt_l4_offset, -1
  add.c2      r1, r1, 8
  add.c3      r1, r1, 4
  add.c4      r1, r1, 12
  phvwr       p.p4_to_p4plus_cpu_pkt_payload_offset, r1.hx
  jr          r7
  phvwr       p.p4_to_p4plus_cpu_pkt_flags, r2

lb_cpu_pkt_tcp:
  phvwr       p.p4_to_p4plus_cpu_pkt_l4_offset, r1.hx
  add         r1, r1, k.tcp_dataOffset, 2
  phvwr       p.p4_to_p4plus_cpu_pkt_payload_offset, r1.hx
  sle         c3, k.tcp_dataOffset, 5
  or.!c3      r2, r2, CPU_FLAGS_TCP_OPTIONS_PRESENT
  phvwr       p.p4_to_p4plus_cpu_pkt_flags, r2
  add         r2, r0, r0
  seq         c1, k.tcp_option_ws_valid, TRUE
  or.c1       r2, r2, CPU_TCP_OPTIONS_WINDOW_SCALE
  seq         c1, k.tcp_option_mss_valid, TRUE
  or.c1       r2, r2, CPU_TCP_OPTIONS_MSS
  seq         c1, k.tcp_option_timestamp_valid, TRUE
  or.c1       r2, r2, CPU_TCP_OPTIONS_TIMESTAMP
  seq         c1, k.tcp_option_sack_perm_valid, TRUE
  or.c1       r2, r2, CPU_TCP_OPTIONS_SACK_PERMITTED
  phvwr       p.p4_to_p4plus_cpu_tcp_pkt_tcp_options, r2
  jr          r7
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr.!c3    p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_error:
  nop.e
  nop
