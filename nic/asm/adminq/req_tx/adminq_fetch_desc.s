
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_adminq_fetch_desc_d d;

#define   _r_desc_addr        r1
#define   _r_db_data          r3
#define   _r_db_addr          r4

%%

.param  adminq_process_desc

.align
adminq_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], adminq_queue_disabled
  seq             c2, d.ci_fetch, d.p_index0
  bcf             [c2], adminq_spurious_db
#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif
  // Compute the descriptor address
  add             _r_desc_addr, d.{ring_base}.dx, d.{ci_fetch}.hx, LG2_ADMINQ_CMD_DESC_SIZE

  // Speculatively claim the descriptor
  phvwr           p.adminq_t0_s2s_comp_index, d.ci_fetch
  phvwr           p.adminq_to_s3_adminq_ci, d.ci_fetch
  tblmincri.f     d.{ci_fetch}.hx, d.{ring_size}.hx, 1

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_ADMINQ_CMD_DESC_SIZE, p.common_te0_phv_table_addr, _r_desc_addr
  phvwri          p.common_te0_phv_table_pc, adminq_process_desc[38:6]

  // Save data for next stages
  phvwr           p.adminq_t0_s2s_host_queue, d.host_queue
  phvwrpair       p.adminq_t0_s2s_lif, k.p4_intr_global_lif, p.adminq_t0_s2s_qtype, k.p4_txdma_intr_qtype
  phvwrpair.e     p.adminq_t0_s2s_qid, k.p4_txdma_intr_qid, p.adminq_t0_s2s_adminq_qstate_addr, k.p4_txdma_intr_qstate_addr
  phvwr.f         p.adminq_t0_s2s_nicmgr_qstate_addr, d.{nicmgr_qstate_addr}.dx

adminq_spurious_db:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_db_addr, _r_db_data
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

adminq_queue_disabled:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_db_addr, _r_db_data

adminq_fetch_drop:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
