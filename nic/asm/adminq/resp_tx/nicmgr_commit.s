
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;
struct tx_table_s3_t0_nicmgr_commit_d d;

#define   _r_cq_desc_addr     r1
#define   _r_intr_addr        r2
#define   _r_db_data          r3
#define   _r_db_addr          r4
#define   _r_ptr              r5        // Current DMA byte offset in PHV
#define   _r_index            r6        // Current DMA command index in PHV

#define   _c_spec_hit         c1
#define   _c_intr_enable      c2

%%

.align
nicmgr_commit:

  bcf             [c2 | c3 | c7], nicmgr_commit_error
  nop

  seq             _c_spec_hit, d.{c_index0}.hx, k.{nicmgr_to_s3_nicmgr_ci}.hx
  bcf             [!_c_spec_hit], nicmgr_commit_abort
  nop

  // Load DMA command pointer
  add             _r_index, r0, k.nicmgr_global_dma_cur_index

  // Claim the descriptor
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

nicmgr_commit_completion:
  // Compute completion entry address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{comp_index}.hx, LG2_NICMGR_RESP_COMP_DESC_SIZE

  // Claim the completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Update color
  phvwr           p.nicmgr_resp_comp_desc_color, d.color
  seq             c7, d.comp_index, 0
  tblmincri.c7    d.color, 1, 1

  // Do we need to generate an interrupt?
  seq             _c_intr_enable, d.intr_enable, 1

  // DMA nicmgr response completion
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, !_c_intr_enable, _r_cq_desc_addr, CAPRI_PHV_START_OFFSET(nicmgr_resp_comp_desc_color), CAPRI_PHV_END_OFFSET(nicmgr_resp_comp_desc_color), r7)
  DMA_CMD_NEXT(_r_index)

  bcf             [!_c_intr_enable], nicmgr_commit_done
  nop

nicmgr_commit_interrupt:
  addi            _r_intr_addr, r0, INTR_ASSERT_BASE
  add             _r_intr_addr, _r_intr_addr, d.{intr_assert_index}.hx, LG2_INTR_ASSERT_STRIDE

  // DMA nicmgr response interrupt
  phvwri          p.nicmgr_to_s2_intr_assert_data, 0x01000000
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HBM_PHV2MEM_WF(_r_ptr, _C_TRUE, _r_intr_addr, CAPRI_PHV_START_OFFSET(nicmgr_to_s2_intr_assert_data), CAPRI_PHV_END_OFFSET(nicmgr_to_s2_intr_assert_data), r7)
  DMA_CMD_NEXT(_r_index)

nicmgr_commit_done:
  // End of pipeline - Make sure no more tables will be launched
  phvwri.e.f      p.{app_header_table0_valid...app_header_table3_valid}, 0
  nop

nicmgr_commit_abort:
nicmgr_commit_error:
  // TODO: Don't commit CI
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
