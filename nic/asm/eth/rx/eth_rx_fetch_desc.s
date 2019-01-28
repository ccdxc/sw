
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s2_t0_k_ k;
struct rx_table_s2_t0_eth_rx_fetch_desc_d d;

#define  _c_queue_disabled    c1
#define  _c_queue_empty       c2

#define  _r_desc_addr         r1    // Descriptor address
#define  _r_cq_desc_addr      r2    // CQ descriptor address
#define  _r_stats             r3    // Stats

%%

.param  eth_rx
.param  eth_rx_stats

.align
eth_rx_fetch_desc:
  LOAD_STATS(_r_stats)

  bcf             [c2 | c3 | c7], eth_rx_queue_error
  nop

  seq             _c_queue_disabled, d.enable, 0
  bcf             [_c_queue_disabled], eth_rx_queue_disabled
  seq             _c_queue_empty, d.p_index0, d.c_index0
  bcf             [_c_queue_empty], eth_rx_queue_empty
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
  seq             c7, d.comp_index, 0
  tblmincri.c7    d.color, 1, 1

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri          p.common_te0_phv_table_pc, eth_rx[38:6]
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_RX_DESC_SIZE, p.common_te0_phv_table_addr, _r_desc_addr

  SAVE_STATS(_r_stats)

  // Save completion and interrupt information
  phvwr           p.eth_rx_global_host_queue, d.host_queue
  phvwr           p.eth_rx_global_intr_enable, d.intr_enable
  phvwr           p.eth_rx_t0_s2s_cq_desc_addr, _r_cq_desc_addr
  phvwr.e         p.eth_rx_t0_s2s_intr_assert_index, d.{intr_assert_index}.hx
  phvwri.f        p.eth_rx_t0_s2s_intr_assert_data, 0x01000000

eth_rx_queue_error:
  SET_STAT(_r_stats, _C_TRUE, queue_error)
eth_rx_queue_empty:
  SET_STAT(_r_stats, _c_queue_empty, queue_empty)
eth_rx_queue_disabled:
  SET_STAT(_r_stats, _c_queue_disabled, queue_disabled)

  SAVE_STATS(_r_stats)

  phvwr           p.eth_rx_global_drop, 1     // increment pkt drop counters
  phvwr           p.p4_intr_global_drop, 1

  // Launch eth_rx_stats action
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, 1
  phvwri.e        p.common_te3_phv_table_pc, eth_rx_stats[38:6]
  phvwri.f        p.common_te3_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
