
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;
struct tx_table_s3_t0_nicmgr_commit_d d;

#define   _r_cq_desc_addr     r1
#define   _r_ptr              r2        // Current DMA byte offset in PHV
#define   _r_db_data          r3
#define   _r_db_addr          r4
#define   _r_flit             r5        // Current DMA flit offset in PHV
#define   _r_index            r6        // Current DMA command offset in PHV flit

#define NICMGR_DB_ADDR    (0x8800000 + \
                          (0x5 /* UPD = DB_IDX_UPD_CIDX_SET | DB_SCHED_UPD_EVAL */ << 17) + \
                          (0x1 /* LIF */ << 6) + \
                          (0x1 /* QTYPE */ << 3))

%%

.align
nicmgr_commit:

  seq             c1, d.{c_index0}.hx, k.{nicmgr_to_s3_nicmgr_ci}.hx
  bcf             [!c1], nicmgr_commit_abort
  nop

  // Load DMA command pointer
  add             _r_flit, r0, k.nicmgr_global_dma_cur_flit
  add             _r_index, r0, k.nicmgr_global_dma_cur_index

// TODO: Remove this section when nicmgr becomes interrupt driven
nicmgr_commit_ci_db:
  // DMA nicmgr c-index update
  add             _r_db_data, r0, d.{c_index0}.hx
  mincr           _r_db_data, d.{ring_size}.hx, 1

  addi            _r_db_addr, r0, NICMGR_DB_ADDR
  phvwr           p.nicmgr_to_s2_nicmgr_db_data, _r_db_data.dx

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HBM_PHV2MEM(_r_ptr, c0, _r_db_addr, CAPRI_PHV_START_OFFSET(nicmgr_to_s2_nicmgr_db_data), CAPRI_PHV_END_OFFSET(nicmgr_to_s2_nicmgr_db_data), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  // Save DMA command pointer
  phvwrpair.e     p.nicmgr_global_dma_cur_flit, _r_flit, p.nicmgr_global_dma_cur_index, _r_index

#if 0
nicmgr_commit_ci_update:
  // Claim the descriptor
  tblwr.f         d.c_index0, d.ci_fetch

  // Eval doorbell when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [!c3], nicmgr_commit_done
  nop.!c3.e

nicmgr_commit_eval_db:

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.nicmgr_t0_s2s_qtype, k.adminq_t0_s2s_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.nicmgr_t0_s2s_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      _r_db_addr, _r_db_data
#endif

nicmgr_commit_done:
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

nicmgr_commit_abort:
  tblwr.f         d.ci_fetch, d.c_index0

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
