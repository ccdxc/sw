
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_nicmgr_post_adminq_d d;

#define   _r_cq_desc_addr     r1
#define   _r_ptr              r2        // Current DMA byte offset in PHV
#define   _r_db_data          r3
#define   _r_db_addr          r4
#define   _r_flit             r5        // Current DMA flit offset in PHV
#define   _r_index            r6        // Current DMA command offset in PHV flit

%%

.param  nicmgr_commit

.align
nicmgr_post_adminq:

  seq             c1, d.enable, 0
  bcf             [c1], nicmgr_post_adminq_queue_disabled
  nop

  // Load DMA command pointer
  add             _r_flit, r0, k.nicmgr_global_dma_cur_flit
  add             _r_index, r0, k.nicmgr_global_dma_cur_index

  // Compute completion descriptor address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, k.{nicmgr_t0_s2s_comp_index}, LG2_ADMINQ_COMP_DESC_SIZE

  // Update completion descriptor
  phvwr           p.adminq_comp_desc_comp_index, k.{nicmgr_t0_s2s_comp_index}.hx
  phvwr           p.adminq_comp_desc_color, d.color

  // Claim completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Update color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // DMA adminq completion
  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HOST_PHV2MEM_LIF(_r_ptr, !c0, _r_cq_desc_addr, CAPRI_PHV_START_OFFSET(adminq_comp_desc_status), CAPRI_PHV_END_OFFSET(adminq_comp_desc_rsvd2), k.nicmgr_t0_s2s_lif, r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  // Do we need to generate an interrupt?
  seq             c2, r0, d.intr_assert_addr
  bcf             [c2], nicmgr_post_adminq_done
  nop

  // DMA adminq interrupt
  phvwri          p.nicmgr_to_s2_intr_assert_data, 0x01000000
  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, !c0, d.{intr_assert_addr}.wx, CAPRI_PHV_START_OFFSET(nicmgr_to_s2_intr_assert_data), CAPRI_PHV_END_OFFSET(nicmgr_to_s2_intr_assert_data), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

nicmgr_post_adminq_done:
  // Save DMA command pointer
  phvwrpair       p.nicmgr_global_dma_cur_flit, _r_flit, p.nicmgr_global_dma_cur_index, _r_index

  // Setup nicmgr qstate lookup for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_NICMGR_QSTATE_SIZE, p.common_te0_phv_table_addr[33:0], k.nicmgr_t0_s2s_nicmgr_qstate_addr
  phvwri.f        p.common_te0_phv_table_pc, nicmgr_commit[38:6]

nicmgr_post_adminq_queue_disabled:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
