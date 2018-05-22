
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;

%%

.param  eth_tx_start
.param  eth_tx_sg_start

#define     _r_num_desc         r1
#define     _r_cq_desc_addr     r5
#define     _r_sg_desc_addr     r6

.align
eth_tx_commit:

  // Have the PHVs ahead of us caught up with their work?
  seq             c1, d.{c_index0}.hx, k.{eth_tx_to_s2_my_ci}.hx
  bcf             [!c1], eth_tx_commit_abort
  add             _r_num_desc, r0, k.eth_tx_t0_s2s_num_desc

  // Compute the completion descriptor address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{comp_index}.hx, LG2_TX_CMPL_DESC_SIZE

  // Claim the completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Change color if end-of-ring
  phvwr           p.eth_tx_cq_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // Compute the sg descriptor address
  add             _r_sg_desc_addr, d.{sg_ring_base}.dx, d.{c_index0}.hx, LG2_TX_SG_DESC_SIZE

  // Claim the descriptor
  tblmincr        d.{c_index0}.hx, d.{ring_size}.hx, _r_num_desc

  // Update ci_fetch if we misspeculated
  sne             c2, k.eth_tx_t0_s2s_num_todo, k.eth_tx_t0_s2s_num_desc
  tblwr.c2        d.ci_fetch, d.c_index0

  // comp_index = (ci - 1)
  add             r7, r0, d.{c_index0}.hx
  mincr           r7, d.{ring_size}.hx, -1
  phvwr           p.eth_tx_cq_desc_comp_index, r7.hx

  // Eval doorbell when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [!c3], eth_tx_commit_done
  nop

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.eth_tx_to_s2_qtype, k.eth_tx_to_s2_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.eth_tx_to_s2_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

eth_tx_commit_done:
  // Save data for next stages
  phvwr           p.eth_tx_t0_s2s_cq_desc_addr, _r_cq_desc_addr
  phvwr           p.eth_tx_t0_s2s_intr_assert_addr, d.{intr_assert_addr}.wx
  phvwri          p.eth_tx_t0_s2s_intr_assert_data, 0x01000000

  bbeq            k.eth_tx_t0_s2s_sg_start, 1, eth_tx_commit_sg
  nop

eth_tx_commit_tx:
  // Launch eth_tx stage
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY, p.common_te0_phv_table_addr, 0
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_start[38:6]

eth_tx_commit_sg:
  // Launch eth_tx_sg stage
  phvwr           p.eth_tx_t1_s2s_sg_desc_addr, _r_sg_desc_addr 
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)
  phvwrpair.e     p.common_te1_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te1_phv_table_addr, _r_sg_desc_addr
  phvwri.f        p.common_te1_phv_table_pc, eth_tx_sg_start[38:6]

eth_tx_commit_abort:
  tblwr.f         d.ci_fetch, d.c_index0

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
