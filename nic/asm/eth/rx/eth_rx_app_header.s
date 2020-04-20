
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_common_p4plus_stage0_app_header_table_k.h"

#include "defines.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k_ k;
struct common_p4plus_stage0_app_header_table_eth_rx_app_header_d d;

#define _r_csum       r1
#define _r_stats      r2

%%

.align
eth_rx_packet_len:
#ifdef IRIS
  phvwr           p.eth_rx_t1_s2s_pkt_len, r1
  phvwr.e         p.eth_rx_t0_s2s_pkt_len, r1
  phvwr.f         p.cq_desc_len, r1.hx
#else
  nop.e
  nop
#endif

.align
eth_rx_app_header:
  tblwr.f         d.rsvd, 0
  // !!! No table updates after this point !!!

  // Save required information from APP header
  phvwr           p.eth_rx_global_lif, k.p4_intr_global_lif
  phvwr           p.eth_rx_t0_s2s_cq_desc_addr, k.p4_rxdma_intr_qid // s0..s2 use cq_desc_addr for qid
  // Is this something other than normal rx?
  bbeq            k.p4_to_p4plus_pad, 1, eth_rx_app_header_other
  phvwr           p.eth_rx_to_s1_qstate_addr, k.p4_rxdma_intr_qstate_addr // BD Slot
  // Save required information from APP header for normal packet processing
  phvwr           p.eth_rx_t1_s2s_l2_pkt_type, k.p4_to_p4plus_l2_pkt_type
  phvwr           p.eth_rx_t1_s2s_pkt_type, k.p4_to_p4plus_pkt_type

#ifndef IRIS
  phvwr           p.eth_rx_t1_s2s_pkt_len, k.p4_to_p4plus_packet_len
  phvwr           p.eth_rx_t0_s2s_pkt_len, k.p4_to_p4plus_packet_len
#endif

  // Build completion entry in the PHV

#ifndef IRIS
  // Packet length information
  phvwr           p.cq_desc_len, k.{p4_to_p4plus_packet_len}.hx
#endif

  // Packet type information
  phvwr           p.cq_desc_pkt_type, k.p4_to_p4plus_pkt_type

  // L2/Complete checksum offload
  sne             c1, k.p4_to_p4plus_pkt_type, PKT_TYPE_NON_IP
  // SET_STAT(_r_stats, c1, oper_csum_complete)
  phvwr.c1        p.cq_desc_csum_calc, 1
  xor.c1          _r_csum, k.p4_to_p4plus_csum, -1
  phvwr.c1        p.cq_desc_csum, _r_csum

  // Checksum verification offload
  // seq             c1, k.p4_to_p4plus_csum_ip_bad, 1
  // SET_STAT(_r_stats, c1, oper_csum_ip_bad)
  // seq             c1, k.p4_to_p4plus_csum_udp_bad, 1
  // SET_STAT(_r_stats, c1, oper_csum_udp_bad)
  // seq             c1, k.p4_to_p4plus_csum_tcp_bad, 1
  // SET_STAT(_r_stats, c1, oper_csum_tcp_bad)
  phvwr           p.{cq_desc_csum_ip_bad...cq_desc_csum_tcp_ok}, k.{p4_to_p4plus_csum_ip_bad...p4_to_p4plus_csum_tcp_ok}

  // Vlan strip offload
  seq             c1, k.p4_to_p4plus_vlan_valid, 1
  // SET_STAT(_r_stats, c1, oper_vlan_strip)
  phvwr.c1        p.cq_desc_vlan_strip, 1
  phvwr.c1        p.cq_desc_vlan_tci, k.{p4_to_p4plus_vlan_pcp...p4_to_p4plus_vlan_vid}.hx

  // SAVE_STATS(_r_stats)

  nop.e
  nop

eth_rx_app_header_other:
  // If there are other needs besides request to arm cq, we can branch here.

eth_rx_app_header_arm:
  // See eth_rx_rss_skip: from there we will launch arm instead of fetch
  // Using do_eq, indicate this is a request to arm
  phvwri.e        p.eth_rx_global_do_eq, 1
  // Arm index comes from txdma as csum, now pass it along as intr_index
  phvwr.f         p.eth_rx_t0_s2s_intr_index, k.p4_to_p4plus_csum
