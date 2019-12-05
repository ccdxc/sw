
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s2_t0_k_ k;
struct rx_table_s2_t0_eth_rx_fetch_desc_d d;

#define  _c_queue_empty       c1
#define  _c_queue_disabled    c2

#define  _r_ci                r1    // Current ci and comp index
#define  _r_color             r3    // Current color
#define  _r_desc_addr         r4    // Descriptor address
#define  _r_tbl_valid         r6    // Next stage table valid bits

%%

.param eth_rx
.param eth_rx_event
.param eth_rx_stats

.align
eth_rx_fetch_desc:
#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif

  bcf             [c2 | c3 | c7], eth_rx_queue_error

  seq             _c_queue_disabled, d.enable, 0
  bcf             [_c_queue_disabled], eth_rx_queue_disabled
  seq             _c_queue_empty, d.p_index0, d.comp_index
  bcf             [_c_queue_empty], eth_rx_queue_empty
  ori             _r_tbl_valid, r0, 0 // BD Slot

  // Claim the descriptor and completion entry
  add             _r_ci, r0, d.{comp_index}.hx
  tblmincri       d.{comp_index}.hx, d.ring_size, 1

  // Change color if end-of-ring
  add             _r_color, r0, d.color
  seq             c7, d.comp_index, 0
  tblmincri.c7    d.color, 1, 1

  // Post an event if armed and posting a completion.
  bbeq            d.armed, 1, eth_rx_fetch_desc_eq
  // BD Slot: either way completion will disarm
  tblwr.f         d.armed, 0
  // Table is flushed, no tblwr/etc past this point.

eth_rx_fetch_desc_addr:
  // Compute descriptor fetch address
  sll             r7, _r_ci, d.lg2_desc_sz
  add             _r_desc_addr, d.{ring_base}.dx, r7

  // Compute the sg descriptor address
  sne             c7, d.sg_ring_base, 0
  bcf             [!c7], eth_rx_cq_entry
  sll             r7, _r_ci, d.lg2_sg_desc_sz
  add             r7, d.{sg_ring_base}.dx, r7
  phvwr           p.eth_rx_global_sg_desc_addr, r7
  phvwr           p.eth_rx_t1_s2s_rem_sg_elems, d.sg_max_elems
  phvwr           p.eth_rx_t1_s2s_sg_max_elems, d.sg_max_elems

eth_rx_cq_entry:
  // Compute completion descriptor address
  sll             r7, _r_ci, d.lg2_cq_desc_sz
  add             r7, d.{cq_ring_base}.dx, r7
  phvwr           p.eth_rx_t0_s2s_cq_desc_addr, r7

  // Save interrupt information
  seq             c7, d.intr_enable, 1
  phvwr.c7        p.eth_rx_global_do_intr, d.intr_enable
  phvwr.c7        p.eth_rx_t0_s2s_intr_index, d.{intr_index_or_eq_addr}.dx

  // Update the completion descriptor
  phvwr           p.cq_desc_comp_index, _r_ci.hx
  phvwr           p.cq_desc_color, _r_color

  // Save data for next stages
  phvwr           p.{eth_rx_global_host_queue...eth_rx_global_cpu_queue}, d.{host_queue...cpu_queue}

  // Setup Descriptor read for next stage
  ori             _r_tbl_valid, _r_tbl_valid, TABLE_VALID_0
  phvwr           p.{app_header_table0_valid...app_header_table3_valid}, _r_tbl_valid
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri          p.common_te0_phv_table_pc, eth_rx[38:6]
  phvwr.e         p.common_te0_phv_table_addr, _r_desc_addr
  phvwr.f         p.common_te0_phv_table_raw_table_size, d.lg2_desc_sz

eth_rx_fetch_desc_eq:
  // Will post an eq desc to host
  phvwri          p.eth_rx_global_do_eq, 1

  // Init fields in eq desc, except gen_color to be set by eth_rx_event action
  phvwri          p.eq_desc_code, EQ_CODE_RX_COMP_HX
  add             r7, r0, k.eth_rx_t0_s2s_cq_desc_addr // s0..s2 use cq_desc_addr for qid
  phvwrpair       p.eq_desc_lif_index, d.lif_index, p.eq_desc_qid, r7.wx

  // Launch eth_rx_event action in parallel with the rx action
  ori             _r_tbl_valid, _r_tbl_valid, TABLE_VALID_2
  phvwri          p.common_te2_phv_table_lock_en, 1
  phvwri          p.common_te2_phv_table_pc, eth_rx_event[38:6]
  phvwr           p.common_te2_phv_table_addr, d.{intr_index_or_eq_addr}.dx
  b               eth_rx_fetch_desc_addr
  phvwri          p.common_te2_phv_table_raw_table_size, LG2_EQ_QSTATE_SIZE

eth_rx_queue_error:
  phvwri          p.eth_rx_global_stats[STAT_queue_error], 1
eth_rx_queue_empty:
  phvwri._c_queue_empty     p.eth_rx_global_stats[STAT_queue_empty], 1
eth_rx_queue_disabled:
  phvwri._c_queue_disabled  p.eth_rx_global_stats[STAT_queue_disabled], 1

  phvwr           p.eth_rx_global_drop, 1   // increment pkt drop counters
  phvwr           p.p4_intr_global_drop, 1

  // Launch stats action
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_1

  phvwri.e        p.common_te1_phv_table_pc, eth_rx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
