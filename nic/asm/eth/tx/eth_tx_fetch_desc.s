
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

#define  _c_queue_disabled    c1
#define  _c_queue_empty       c2

#define  _r_ci                r1    // Current CI
#define  _r_stats             r2    // Stats
#define  _r_dbval             r3    // Doorbell value
#define  _r_dbaddr            r4    // Doorbell address
#define  _r_desc_addr         r5    // Descriptor address

%%

.param  eth_tx_prep

.align
eth_tx_fetch_desc:
  // INIT_STATS(_r_stats)

  bbeq            d.enable, 0, eth_tx_queue_disabled
  seq             _c_queue_empty, d.p_index0, d.c_index0
  bcf             [_c_queue_empty], eth_tx_queue_empty
  nop

#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif

  // Claim the descriptor
  add             _r_ci, r0, d.c_index0
  tblmincri.f     d.{c_index0}.hx, d.ring_size, 1

  // Save data for next stages
  phvwr           p.{eth_tx_global_host_queue...eth_tx_global_intr_enable}, d.{host_queue...intr_enable}
  phvwr           p.eth_tx_to_s1_qstate_addr[33:0], k.p4_txdma_intr_qstate_addr
  phvwr           p.eth_tx_global_lif, k.p4_intr_global_lif
  phvwr           p.eth_tx_to_s2_my_ci, _r_ci

  // Compute the descriptor fetch address
  sll             r7, _r_ci.hx, d.lg2_desc_sz
  add             _r_desc_addr, d.{ring_base}.dx, r7

  // SAVE_STATS(_r_stats)

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri          p.common_te0_phv_table_pc, eth_tx_prep[38:6]
  phvwr.e         p.common_te0_phv_table_addr, _r_desc_addr
  phvwr.f         p.common_te0_phv_table_raw_table_size, d.lg2_desc_sz

eth_tx_queue_disabled:
  // SET_STAT(_r_stats, _C_TRUE, queue_disabled)

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  // SAVE_STATS(_r_stats)

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

eth_tx_queue_empty:
  // SET_STAT(_r_stats, _C_TRUE, queue_empty)

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  // SAVE_STATS(_r_stats)

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
