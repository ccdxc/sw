
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_adminq_post_nicmgr_d d;

#define   _r_desc_addr        r1
#define   _r_ptr              r2        // Current DMA byte offset in PHV
#define   _r_db_data          r3
#define   _r_db_addr          r4
#define   _r_flit             r5        // Current DMA flit offset in PHV
#define   _r_index            r6        // Current DMA command offset in PHV flit

#define NICMGR_DB_ADDR    (0x8800000 + \
                          (0x4 /* UPD = DB_IDX_UPD_CIDX_SET | DB_SCHED_UPD_NOP */ << 17) + \
                          (0x1 /* LIF */ << 6) + \
                          (0x0 /* QTYPE */ << 3))

%%

.param  adminq_commit

.align
adminq_post_nicmgr:

  bcf             [c2 | c3 | c7], adminq_post_nicmgr_error
  nop

  seq             c1, d.enable, 0
  bcf             [c1], adminq_post_nicmgr_queue_disabled
  seq             c2, d.c_index0, d.p_index0
  bcf             [c2], adminq_post_nicmgr_queue_full
  nop

  // Load DMA command pointer
  add             _r_flit, r0, k.adminq_global_dma_cur_flit
  add             _r_index, r0, k.adminq_global_dma_cur_index

  // Compute the descriptor address
  add             _r_desc_addr, d.{ring_base}.dx, d.{c_index0}.hx, LG2_NICMGR_REQ_DESC_SIZE

  // DMA nicmgr request descriptor
  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HBM_PHV2MEM(_r_ptr, !c0, _r_desc_addr, CAPRI_PHV_START_OFFSET(nicmgr_req_desc_lif), CAPRI_PHV_END_OFFSET(nicmgr_req_desc_adminq_cmd_desc), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  // Do we need to generate an interrupt?
  seq             c3, r0, d.intr_assert_addr
  bcf             [c3], adminq_post_nicmgr_ci_db
  nop

  // DMA nicmgr request interrupt
  phvwri          p.adminq_to_s2_intr_assert_data, 0x01000000
  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HBM_PHV2MEM(_r_ptr, !c0, d.intr_assert_addr, CAPRI_PHV_START_OFFSET(adminq_to_s2_intr_assert_data), CAPRI_PHV_END_OFFSET(adminq_to_s2_intr_assert_data), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

// TODO: Remove this section when nicmgr becomes interrupt driven
adminq_post_nicmgr_ci_db:
  // DMA nicmgr c-index update
  add             _r_db_data, r0, d.{c_index0}.hx
  mincr           _r_db_data, d.{ring_size}.hx, 1

  addi            _r_db_addr, r0, NICMGR_DB_ADDR
  phvwr           p.adminq_to_s2_nicmgr_db_data, _r_db_data.dx

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, c0, _r_db_addr, CAPRI_PHV_START_OFFSET(adminq_to_s2_nicmgr_db_data), CAPRI_PHV_END_OFFSET(adminq_to_s2_nicmgr_db_data), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

#if 0
adminq_post_nicmgr_ci_update:
  // Claim the descriptor
  tblwr.f         d.c_index0, d.ci_fetch

  // Eval doorbell when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [!c3], nicmgr_commit_done
  nop.!c3.e

adminq_post_nicmgr_evaldb:

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.adminq_t0_s2s_qtype, k.adminq_t0_s2s_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.adminq_t0_s2s_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      r4, r3
#endif

adminq_post_nicmgr_done:
  // Save DMA command pointer
  phvwrpair       p.adminq_global_dma_cur_flit, _r_flit, p.adminq_global_dma_cur_index, _r_index

  // Setup adminq commit for next stage
  add             r7, r0, k.adminq_t0_s2s_adminq_qstate_addr
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_ADMINQ_QSTATE_SIZE, p.common_te0_phv_table_addr, r7
  phvwri.f        p.common_te0_phv_table_pc, adminq_commit[38:6]

adminq_post_nicmgr_error:
adminq_post_nicmgr_queue_disabled:
adminq_post_nicmgr_queue_full:
  // TODO: Don't commit adminq ci
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
