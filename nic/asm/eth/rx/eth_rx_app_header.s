
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_common_p4plus_stage0_app_header_table_k.h"

#include "defines.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k k;
struct common_p4plus_stage0_app_header_table_eth_rx_app_header_d d;

%%

.align
eth_rx_app_header:

  tblwr.l.f       d.rsvd1, 0
  // !!! No table updates after this point !!!

  // Save all required information from APP header
  phvwr           p.eth_rx_global_qstate_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1, p4_rxdma_intr_qstate_addr_sbit2_ebit33}
  phvwr           p.eth_rx_t0_s2s_packet_len, k.p4_to_p4plus_packet_len

  // Build completion entry in the PHV
  sne             c1, k.p4_to_p4plus_pkt_type, PKT_TYPE_NON_IP
  phvwr.c1        p.eth_rx_cq_desc_csum_calc, 1
  xor.c1          r1, k.p4_to_p4plus_csum, -1
  phvwr.c1        p.eth_rx_cq_desc_csum, r1
  phvwr           p.{eth_rx_cq_desc_csum_ip_bad...eth_rx_cq_desc_csum_tcp_ok}, k.{p4_to_p4plus_csum_ip_bad...p4_to_p4plus_csum_tcp_ok}
  phvwr           p.eth_rx_cq_desc_vlan_strip, k.p4_to_p4plus_vlan_valid
  phvwr           p.eth_rx_cq_desc_vlan_tci, k.{p4_to_p4plus_vlan_pcp...p4_to_p4plus_vlan_vid_sbit4_ebit11}.hx
  phvwr.e         p.eth_rx_cq_desc_len_lo, k.p4_to_p4plus_packet_len[7:0]
  phvwr.f         p.eth_rx_cq_desc_len_hi, k.p4_to_p4plus_packet_len[13:8]
