
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_adminq_post_nicmgr_d d;

#define   _r_desc_addr        r1
#define   _r_cq_desc_addr     r2
#define   _r_intr_addr        r3
#define   _r_db_data          r3
#define   _r_db_addr          r4
#define   _r_ptr              r5        // Current DMA byte offset in PHV
#define   _r_index            r6        // Current DMA command index in PHV

#define   _c_intr_enable      c3

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
  add             _r_index, r0, k.adminq_global_dma_cur_index

adminq_post_nicmgr_request:
  // Compute the descriptor address
  add             _r_desc_addr, d.{ring_base}.dx, d.{c_index0}.hx, LG2_NICMGR_REQ_DESC_SIZE

  // Claim the descriptor
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

  // DMA nicmgr request descriptor
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HBM_PHV2MEM(_r_ptr, _C_FALSE, _r_desc_addr, CAPRI_PHV_START_OFFSET(nicmgr_req_desc_lif), CAPRI_PHV_END_OFFSET(nicmgr_req_desc_adminq_cmd_desc), r7)
  DMA_CMD_NEXT(_r_index)

adminq_post_nicmgr_completion:
  // Compute the completion descriptor address
  add            _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{comp_index}.hx, LG2_NICMGR_REQ_COMP_DESC_SIZE

  // Claim the completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Change color if end-of-ring
  phvwr           p.nicmgr_req_comp_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // Do we need to generate an interrupt?
  seq             _c_intr_enable, d.intr_enable, 1

  // DMA nicmgr request completion
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, !_c_intr_enable, _r_cq_desc_addr, CAPRI_PHV_START_OFFSET(nicmgr_req_comp_desc_color), CAPRI_PHV_END_OFFSET(nicmgr_req_comp_desc_rsvd), r7)
  DMA_CMD_NEXT(_r_index)

  bcf             [!c3], adminq_post_nicmgr_done
  nop

adminq_post_nicmgr_interrupt:
  // Compute interrupt address
  addi            _r_intr_addr, r0, INTR_ASSERT_BASE
  add             _r_intr_addr, _r_intr_addr, d.{intr_assert_index}.hx, LG2_INTR_ASSERT_STRIDE

  // DMA nicmgr request interrupt
  phvwri          p.adminq_to_s2_intr_assert_data, 0x01000000
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, _C_TRUE, _r_intr_addr, CAPRI_PHV_START_OFFSET(adminq_to_s2_intr_assert_data), CAPRI_PHV_END_OFFSET(adminq_to_s2_intr_assert_data), r7)
  DMA_CMD_NEXT(_r_index)

adminq_post_nicmgr_done:
  // Save DMA command pointer
  phvwr           p.adminq_global_dma_cur_index, _r_index

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
