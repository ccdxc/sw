
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s2_t0_k_ k;
struct rx_table_s2_t0_eth_rx_fetch_desc_d d;

#define  _r_desc_addr       r1    // Descriptor address
#define  _r_cq_desc_addr    r2    // CQ descriptor address

%%

.param  eth_rx

.align
eth_rx_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], eth_rx_queue_disabled
  seq             c2, d.p_index0, d.c_index0
  bcf             [c2], eth_rx_queue_nobuf
  nop

  // Compute descriptor fetch address
  add             _r_desc_addr, d.{ring_base}.dx, d.{c_index0}.hx, LG2_RX_DESC_SIZE

  // Compute completion entry address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{c_index0}.hx, LG2_RX_CMPL_DESC_SIZE

  // Claim the descriptor
  phvwr           p.eth_rx_cq_desc_comp_index, d.c_index0
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

  // Claim the completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Update color
  phvwr           p.eth_rx_cq_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri          p.common_te0_phv_table_pc, eth_rx[38:6]
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_RX_DESC_SIZE, p.common_te0_phv_table_addr, _r_desc_addr

  // Save completion and interrupt information
  phvwr           p.eth_rx_global_host_queue, d.host_queue
  phvwr           p.eth_rx_global_intr_enable, d.intr_enable
  phvwr           p.eth_rx_t0_s2s_cq_desc_addr, _r_cq_desc_addr
  phvwr.e         p.eth_rx_t0_s2s_intr_assert_index, d.{intr_assert_index}.hx
  phvwri.f        p.eth_rx_t0_s2s_intr_assert_data, 0x01000000

eth_rx_queue_nobuf:
eth_rx_queue_disabled:
  phvwri.e        p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0
