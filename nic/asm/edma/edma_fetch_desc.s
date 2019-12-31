
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_edma_fetch_desc_d d;

#define  _r_desc_addr       r1    // Descriptor address
#define  _r_cq_desc_addr    r2    // CQ descriptor address
#define  _r_dbval             r3    // Doorbell value
#define  _r_dbaddr            r4    // Doorbell address

%%

.param  edma_process_desc

.align
edma_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], edma_queue_disabled
  seq             c2, d.p_index0, d.c_index0
  bcf             [c2], edma_spurious_db
#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif
  // Compute descriptor fetch address
  add             _r_desc_addr, d.{ring_base}.dx, d.{c_index0}.hx, LG2_EDMA_CMD_DESC_SIZE

  // Compute completion entry address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{c_index0}.hx, LG2_EDMA_COMP_DESC_SIZE

  // Claim the descriptor
  phvwr           p.edma_comp_desc_comp_index, d.c_index0
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

  // Claim the completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Update color
  phvwr           p.edma_comp_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_EDMA_CMD_DESC_SIZE, p.common_te0_phv_table_addr, _r_desc_addr
  phvwri          p.common_te0_phv_table_pc, edma_process_desc[38:6]

  // Save data for next stages
  phvwr           p.edma_global_intr_enable, d.intr_enable
  phvwr           p.edma_t0_s2s_cq_desc_addr, _r_cq_desc_addr
  phvwr           p.edma_t0_s2s_intr_assert_index, d.{intr_assert_index}.hx
  phvwri          p.edma_t0_s2s_intr_assert_data, 0x01000000

edma_fetch_desc_done:
  // Eval doorbell when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [c3], edma_fetch_desc_eval_db
  nop.!c3.e
  nop

edma_spurious_db:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

edma_fetch_desc_eval_db:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      _r_dbaddr, _r_dbval
  nop

edma_queue_disabled:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

edma_fetch_drop:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
