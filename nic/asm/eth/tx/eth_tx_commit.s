
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;

%%

.align
eth_tx_commit:

  // We cannot commit the work done in this PHV if another PHV
  // ahead of it has already claimed this descriptor
  seq             c1, d.{c_index0}.hx, k.{eth_tx_t0_s2s_c_index0}.hx
  bcf             [!c1], eth_tx_commit_abort
  add             r1, r0, k.eth_tx_global_num_desc

  // Compute the completion descriptor address
  add             r3, d.{cq_ring_base}.dx, d.{comp_index}.hx, LG2_TX_CMPL_DESC_SIZE

  // Claim the completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Claim the descriptor
  tblmincr        d.{c_index0}.hx, d.{ring_size}.hx, r1
  add             r7, r0, d.{c_index0}.hx
  mincr           r7, d.{ring_size}.hx, -1
  phvwr           p.eth_tx_cq_desc_comp_index, r7.hx

  // Change color if end-of-ring
  phvwr           p.eth_tx_cq_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // Commit 
  tblwr.f         d.{spec_index}.hx, d.{c_index0}.hx

  phvwr           p.eth_tx_t3_s2s_cq_desc_addr, r3
  phvwr           p.eth_tx_t3_s2s_intr_assert_addr, d.{intr_assert_addr}.wx
  phvwri.f        p.eth_tx_t3_s2s_intr_assert_data, 0x01000000

  // Eval the doorbell only when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [c3], eth_tx_commit_eval_db     // pi == ci
  nop.!c3.e                               		    // pi != ci
  nop

eth_tx_commit_eval_db:

  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.eth_tx_t0_s2s_qtype, k.eth_tx_t0_s2s_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.eth_tx_t0_s2s_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      r4, r3
  nop

eth_tx_commit_abort:

  phvwr.e         p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0
