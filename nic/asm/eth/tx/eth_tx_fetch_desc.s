
#include "INGRESS_p.h"
#include "ingress.h"
#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

%%

.param  eth_tx_prep

.align
eth_tx_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], eth_tx_queue_disabled
  seq             c2, d.c_index0, d.p_index0
  bcf             [c2], eth_tx_spurious_db
  seq             c3, d.spurious_db_cnt, MAX_SPURIOUS_DB

  /* R1 = What is the ring size? */
  sll             r1, 1, d.{ring_size}.hx     // R1 = ring_size

  /* R2 = How many max descriptors should we process? */

  // How many descriptors are posted?
  // num_posted = (pi < ci) ? (ring_size - (ci - pi)) : (pi - ci)
  slt             c1, d.{p_index0}.hx, d.{c_index0}.hx
  sub.c1          r2, d.{c_index0}.hx, d.{p_index0}.hx
  sub.c1          r2, r1, r2
  sub.!c1         r2, d.{p_index0}.hx, d.{c_index0}.hx

  // We should only process upto MAX_DESC_PER_PHV
  // num_todo = max(num_posted, max_per_phv)
  sle             c2, r2, MAX_DESC_PER_PHV
  addi.!c2        r2, r0, MAX_DESC_PER_PHV

  // Are we at the end of the ring?
  // end_of_ring = ring_size <  (ci + num_todo)
  // num_todo = end_of_ring ? 1 : num_todo
  add             r6, d.{c_index0}.hx, r2
  slt             c3, r1, r6
  addi.c3         r2, r0, 1
  phvwr           p.eth_tx_t0_s2s_num_desc, r2

  // Compute the descriptor fetch address
  add             r3, d.{ring_base}.dx, d.{c_index0}.hx, LG2_TX_DESC_SIZE

  // Compute the sg descriptor address
  add             r4, d.{sg_ring_base}.dx, d.{c_index0}.hx, LG2_TX_SG_DESC_SIZE

  // Compute the completion descriptor address
  add             r5, d.{cq_ring_base}.dx, d.{p_index1}.hx, LG2_TX_CMPL_DESC_SIZE

  // Claim the completion entry
  tblmincri       d.{p_index1}.hx, d.{ring_size}.hx, 1

  // Change color if end-of-ring
  phvwr           p.eth_tx_cq_desc_color, d.color
  seq             c1, d.p_index1, 0
  tblmincri.c1    d.color, 1, 1

  // Claim the descriptor
  phvwr           p.eth_tx_cq_desc_comp_index, d.c_index0
  tblmincr.f      d.{c_index0}.hx, d.{ring_size}.hx, r2
  // !!! No table updates after this point !!!

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  //phvwri          p.common_te0_phv_table_lock_en, 1
  phvwri          p.common_te0_phv_table_pc, eth_tx_prep[38:6]  
  phvwr           p.common_te0_phv_table_addr, r3

  // Compute the size of descriptor read
  // read_size = (num_todo == 1) ? 16 : 64
  // NOTE: num_todo == 1 when end_of_ring or num_posted == 1
  seq             c4, r2, 1
  addi.c4         r6, r0, LG2_TX_DESC_SIZE
  addi.!c4        r6, r0, LG2_TX_DESC_SIZE + 2
  phvwr           p.common_te0_phv_table_raw_table_size, r6

  phvwr           p.eth_tx_t0_s2s_sg_desc_addr, r4

  phvwr           p.eth_tx_t1_s2s_cq_desc_addr, r5
  phvwr           p.eth_tx_t1_s2s_intr_assert_addr, d.{intr_assert_addr}.wx
  phvwri          p.eth_tx_t1_s2s_intr_assert_data, 0x01000000

  // Eval the doorbell only when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [c3], eth_tx_eval_db    // pi == ci
  nop.!c3.e                               // pi != ci
  nop

eth_tx_spurious_db:
  tblmincri.f     d.spurious_db_cnt, LG2_MAX_SPURIOUS_DB, 1
  // !!! No table updates after this point !!!

  bcf             [c3], eth_tx_eval_db        // spurious_db_cnt == max
  phvwr.!c3.e     p.p4_intr_global_drop, 1    // spurious_db_cnt != max
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, 0

eth_tx_eval_db:

  or              r1, k.p4_intr_global_lif_sbit3_ebit10, k.p4_intr_global_lif_sbit0_ebit2, 8
  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, r1)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

  nop.e
  nop

eth_tx_queue_disabled:
  tblwr.l.f       d.rsvd1, 0
  // !!! No table updates after this point !!!

  or              r1, k.p4_intr_global_lif_sbit3_ebit10, k.p4_intr_global_lif_sbit0_ebit2, 8
  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, r1)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

  phvwr.e         p.p4_intr_global_drop, 1
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, 0
