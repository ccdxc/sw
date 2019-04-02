
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

 .param          eth_rx_fetch_desc

.align
eth_rx_app_header:
  INIT_STATS(_r_stats)

  tblwr.l.f       d.rsvd1, 0
  // !!! No table updates after this point !!!

  // Save all required information from APP header
  phvwr           p.eth_rx_global_lif, k.p4_intr_global_lif
  phvwr           p.eth_rx_global_qstate_addr, k.p4_rxdma_intr_qstate_addr
  phvwr           p.eth_rx_global_l2_pkt_type, k.p4_to_p4plus_l2_pkt_type
  phvwr           p.eth_rx_global_pkt_type, k.p4_to_p4plus_pkt_type
  phvwr           p.eth_rx_global_pkt_len, k.p4_to_p4plus_packet_len

  // Build completion entry in the PHV

  // Packet type information
  phvwr           p.eth_rx_cq_desc_pkt_type, k.p4_to_p4plus_pkt_type

  // L2/Complete checksum offload
  sne             c1, k.p4_to_p4plus_pkt_type, PKT_TYPE_NON_IP
  SET_STAT(_r_stats, c1, oper_csum_complete)
  phvwr.c1        p.eth_rx_cq_desc_csum_calc, 1
  xor.c1          _r_csum, k.p4_to_p4plus_csum, -1
  phvwr.c1        p.eth_rx_cq_desc_csum, _r_csum

  // Checksum verification offload
  seq             c1, k.p4_to_p4plus_csum_ip_bad, 1
  SET_STAT(_r_stats, c1, oper_csum_ip_bad)
  seq             c1, k.p4_to_p4plus_csum_udp_bad, 1
  SET_STAT(_r_stats, c1, oper_csum_udp_bad)
  seq             c1, k.p4_to_p4plus_csum_tcp_bad, 1
  SET_STAT(_r_stats, c1, oper_csum_tcp_bad)
  phvwr           p.{eth_rx_cq_desc_csum_ip_bad...eth_rx_cq_desc_csum_tcp_ok}, k.{p4_to_p4plus_csum_ip_bad...p4_to_p4plus_csum_tcp_ok}

  // Vlan strip offload
  seq             c1, k.p4_to_p4plus_vlan_valid, 1
  SET_STAT(_r_stats, c1, oper_vlan_strip)
  phvwr.c1        p.eth_rx_cq_desc_vlan_strip, 1
  phvwr.c1        p.eth_rx_cq_desc_vlan_tci, k.{p4_to_p4plus_vlan_pcp...p4_to_p4plus_vlan_vid}.hx

  SAVE_STATS(_r_stats)

#ifndef APOLLO
  phvwr.e         p.eth_rx_cq_desc_len_lo, k.p4_to_p4plus_packet_len[7:0]
  phvwr.f         p.eth_rx_cq_desc_len_hi, k.p4_to_p4plus_packet_len[13:8]
#else
  phvwr           p.eth_rx_cq_desc_len_lo, k.p4_to_p4plus_packet_len[7:0]
  phvwr           p.eth_rx_cq_desc_len_hi, k.p4_to_p4plus_packet_len[13:8]

  // Launch qstate in next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwri          p.common_te0_phv_table_pc, eth_rx_fetch_desc[38:6]
  phvwr.e         p.common_te0_phv_table_addr, k.p4_rxdma_intr_qstate_addr
  phvwri.f        p.common_te0_phv_table_raw_table_size, LG2_RX_QSTATE_SIZE
#endif
