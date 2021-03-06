
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_nicmgr_post_adminq_d d;

#define   _r_cq_desc_addr     r1
#define   _r_intr_addr        r2        // Interrupt Assert Address
#define   _r_db_data          r3
#define   _r_db_addr          r4
#define   _r_ptr              r5        // Current DMA byte offset in PHV
#define   _r_index            r6        // Current DMA command index in PHV

#define   _c_intr_enable      c3

%%

.param  nicmgr_commit

.align
nicmgr_post_adminq:

  bcf             [c2 | c3 | c7], nicmgr_post_adminq_error
  nop

  seq             c1, d.enable, 0
  bcf             [c1], nicmgr_post_adminq_queue_disabled
  nop

  // Load DMA command pointer
  add             _r_index, r0, k.nicmgr_global_dma_cur_index

nicmgr_post_adminq_completion:
  // Compute completion descriptor address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{comp_index}.hx, LG2_ADMINQ_COMP_DESC_SIZE

  // Claim completion entry
  phvwr           p.adminq_comp_desc_comp_index, d.comp_index
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Update color
  phvwr           p.adminq_comp_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // Do we need to generate an interrupt?
  seq             _c_intr_enable, d.intr_enable, 1

  // DMA adminq completion
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_PHV2MEM(_r_ptr, !_c_intr_enable, d.host_queue, _r_cq_desc_addr, CAPRI_PHV_START_OFFSET(adminq_comp_desc_status), CAPRI_PHV_END_OFFSET(adminq_comp_desc_rsvd2), r7)
  DMA_CMD_NEXT(_r_index)

  bcf             [!_c_intr_enable], nicmgr_post_adminq_done
  nop

nicmgr_post_adminq_interrupt:
  // Compute interrupt address
  addi            _r_intr_addr, r0, INTR_ASSERT_BASE
  add             _r_intr_addr, _r_intr_addr, d.{intr_assert_index}.hx, LG2_INTR_ASSERT_STRIDE

  // DMA adminq interrupt
  phvwri          p.nicmgr_to_s2_intr_assert_data, 0x01000000
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, _C_FALSE, _r_intr_addr, CAPRI_PHV_START_OFFSET(nicmgr_to_s2_intr_assert_data), CAPRI_PHV_END_OFFSET(nicmgr_to_s2_intr_assert_data), r7)
  DMA_CMD_NEXT(_r_index)

nicmgr_post_adminq_done:
  // Save DMA command pointer
  phvwr           p.nicmgr_global_dma_cur_index, _r_index

  // Setup nicmgr qstate lookup for next stage
  add             r7, r0, k.nicmgr_t0_s2s_nicmgr_qstate_addr
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_NICMGR_QSTATE_SIZE, p.common_te0_phv_table_addr, r7
  phvwri.f        p.common_te0_phv_table_pc, nicmgr_commit[38:6]

nicmgr_post_adminq_error:
nicmgr_post_adminq_queue_disabled:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
