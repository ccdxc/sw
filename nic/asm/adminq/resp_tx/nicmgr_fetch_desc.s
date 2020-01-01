
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "../defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_nicmgr_fetch_desc_d d;

#define   _r_desc_addr        r1
#define   _r_db_data          r3
#define   _r_db_addr          r4

%%

.param  nicmgr_process_desc_hdr
.param  nicmgr_process_desc_data

.align
nicmgr_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], nicmgr_queue_disabled
  seq             c2, d.ci_fetch, d.p_index0
  bcf             [c2], nicmgr_spurious_db
#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif
  // Compute the descriptor fetch address
  add             _r_desc_addr, d.{ring_base}.dx, d.{ci_fetch}.hx, LG2_NICMGR_RESP_DESC_SIZE

  // Speculatively claim the descriptor
  phvwr           p.nicmgr_to_s3_nicmgr_ci, d.ci_fetch
  tblmincri.f     d.{ci_fetch}.hx, d.{ring_size}.hx, 1

  // Setup Descriptor Header & Data read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (3 << 2)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_NICMGR_RESP_DESC_HDR_SIZE, p.common_te0_phv_table_addr, _r_desc_addr
  phvwri          p.common_te0_phv_table_pc, nicmgr_process_desc_hdr[38:6]

  add             _r_desc_addr, _r_desc_addr, 1, LG2_NICMGR_RESP_DESC_HDR_SIZE
  phvwri          p.common_te1_phv_table_lock_en, 1
  phvwrpair       p.common_te1_phv_table_raw_table_size, LG2_NICMGR_RESP_DESC_DATA_SIZE, p.common_te1_phv_table_addr, _r_desc_addr
  phvwri.e        p.common_te1_phv_table_pc, nicmgr_process_desc_data[38:6]

  // Save data for next stages
  phvwr.f         p.nicmgr_t0_s2s_nicmgr_qstate_addr, k.p4_txdma_intr_qstate_addr

nicmgr_spurious_db:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_db_addr, _r_db_data
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

nicmgr_queue_disabled:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_db_addr, _r_db_data
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
