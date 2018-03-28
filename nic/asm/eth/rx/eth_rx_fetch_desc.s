
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s2_t0_k_ k;
struct rx_table_s2_t0_eth_rx_fetch_desc_d d;

%%

.param  eth_rx_packet

.align
eth_rx_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], eth_rx_queue_disabled
  seq             c2, d.p_index0, d.c_index0
  bcf             [c2], eth_rx_queue_nobuf
  nop

  // Compute descriptor fetch address
  add             r1, d.{ring_base}.dx, d.{c_index0}.hx, LG2_RX_DESC_SIZE

  // Compute completion entry address
  add             r2, d.{cq_ring_base}.dx, d.{p_index1}.hx, LG2_RX_CMPL_DESC_SIZE

  // Claim the descriptor
  phvwr           p.eth_rx_cq_desc_comp_index, d.c_index0
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

  // Claim the completion entry
  tblmincri       d.{p_index1}.hx, d.{ring_size}.hx, 1

  // Update color
  phvwr           p.eth_rx_cq_desc_color, d.color
  seq             c1, d.p_index1, 0
  tblmincri.c1    d.color, 1, 1

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_pc, eth_rx_packet[38:6]
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_RX_DESC_SIZE, p.common_te0_phv_table_addr, r1

  // Save completion and interrupt information
  phvwr           p.eth_rx_t0_s2s_cq_desc_addr, r2
  phvwr.e         p.eth_rx_t0_s2s_intr_assert_addr, d.{intr_assert_addr}.wx
  phvwri.f        p.eth_rx_t0_s2s_intr_assert_data, 0x01000000

eth_rx_queue_nobuf:
eth_rx_queue_disabled:
  phvwri.e        p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0
