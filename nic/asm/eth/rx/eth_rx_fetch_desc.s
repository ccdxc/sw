
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s2_t0_k_ k;
struct rx_table_s2_t0_eth_rx_fetch_desc_d d;

#define  _c_queue_empty       c1

#define  _r_ci                r1    // Current ci
#define  _r_comp_index        r2    // Current completion index
#define  _r_color             r3    // Current color
#define  _r_desc_addr         r4    // Descriptor address
#define  _r_stats             r5    // Stats

%%

.param  eth_rx
.param  eth_rx_stats

.align
eth_rx_fetch_desc:
  // LOAD_STATS(_r_stats)

  bcf             [c2 | c3 | c7], eth_rx_queue_error
  nop

  bbeq            d.enable, 0, eth_rx_queue_disabled
  seq             _c_queue_empty, d.p_index0, d.c_index0
  bcf             [_c_queue_empty], eth_rx_queue_empty
  nop

#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif

  // Claim the descriptor
  add             _r_ci, r0, d.c_index0
  tblmincri       d.{c_index0}.hx, d.ring_size, 1

  // Claim the completion entry
  add             _r_comp_index, r0, d.comp_index
  tblmincri       d.{comp_index}.hx, d.ring_size, 1

  // Change color if end-of-ring
  add             _r_color, r0, d.color
  seq             c7, d.comp_index, 0
  tblmincri.c7    d.color, 1, 1

  tblwr.f         d.rsvd, 0

  // Compute descriptor fetch address
  sll             r7, _r_ci.hx, d.lg2_desc_sz
  add             _r_desc_addr, d.{ring_base}.dx, r7

  // Compute the sg descriptor address
  sne             c7, d.sg_ring_base, 0
  bcf             [!c7], eth_rx_cq_entry
  sll             r7, _r_ci.hx, d.lg2_sg_desc_sz
  add             r7, d.{sg_ring_base}.dx, r7
  phvwr           p.eth_rx_global_sg_desc_addr, r7
  phvwr           p.eth_rx_t1_s2s_rem_sg_elems, d.sg_max_elems
  phvwr           p.eth_rx_t1_s2s_sg_max_elems, d.sg_max_elems

eth_rx_cq_entry:
  // Compute completion descriptor address
  sll             r7, _r_comp_index.hx, d.lg2_cq_desc_sz
  add             r7, d.{cq_ring_base}.dx, r7
  phvwr           p.eth_rx_t0_s2s_cq_desc_addr, r7

  // Update completion descriptor
  phvwr           p.eth_rx_cq_desc_comp_index, _r_ci
  phvwr           p.eth_rx_cq_desc_color, _r_color

  // SAVE_STATS(_r_stats)

  // Save data for next stages
  phvwr           p.{eth_rx_global_host_queue...eth_rx_global_intr_enable}, d.{host_queue...intr_enable}
  phvwr           p.eth_rx_t0_s2s_intr_assert_index, d.{intr_assert_index}.hx
  phvwri          p.eth_rx_t0_s2s_intr_assert_data, 0x01000000

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri          p.common_te0_phv_table_pc, eth_rx[38:6]
  phvwr.e         p.common_te0_phv_table_addr, _r_desc_addr
  phvwr.f         p.common_te0_phv_table_raw_table_size, d.lg2_desc_sz

eth_rx_queue_error:
  // SET_STAT(_r_stats, _C_TRUE, queue_error)
eth_rx_queue_empty:
  // SET_STAT(_r_stats, _c_queue_empty, queue_empty)
eth_rx_queue_disabled:
  // SET_STAT(_r_stats, _c_queue_disabled, queue_disabled)

  // SAVE_STATS(_r_stats)

  phvwr           p.eth_rx_global_drop, 1     // increment pkt drop counters
  phvwr           p.p4_intr_global_drop, 1

  // Launch eth_rx_stats action
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)
  phvwri.e        p.common_te1_phv_table_pc, eth_rx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
