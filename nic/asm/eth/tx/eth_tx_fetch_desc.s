
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

%%

.param  eth_tx_prep

.align
eth_tx_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], eth_tx_queue_disabled
  seq             c2, d.ci_fetch, d.p_index0
  bcf             [c2], eth_tx_spurious_db
  seq             c3, d.spurious_db_cnt, 0

  /* R1 = What is the ring size? */
  sll             r1, 1, d.{ring_size}.hx

  // Drop the PHV if we are speculating too far ahead
  sub             r7, d.{ci_fetch}.hx, d.{c_index0}.hx
  mincr           r7, d.{ring_size}.hx, 0
  slt             c1, r7, MAX_DESC_SPEC
  bcf             [!c1], eth_tx_fetch_drop

  /* R2 = How many max descriptors should we process? */

  // How many descriptors are posted?
  // num_posted = (pi < ci) ? (ring_size - (ci - pi)) : (pi - ci)
  sub             r2, d.{p_index0}.hx, d.{ci_fetch}.hx
  mincr           r2, d.{ring_size}.hx, 0

  // We should only process upto MAX_DESC_PER_PHV
  // num_todo = max(num_posted, max_per_phv)
  sle             c2, r2, MAX_DESC_PER_PHV
  addi.!c2        r2, r0, MAX_DESC_PER_PHV

  // Are we at the end of the ring?
  // end_of_ring = ring_size < (ci + num_todo)
  // num_todo = end_of_ring ? 1 : num_todo
  add             r6, d.{ci_fetch}.hx, r2
  slt             c3, r1, r6
  cmov            r2, c3, 1, r2

  // Compute the size of descriptor read
  // read_size = (num_todo == 1) ? 16 : 64
  seq             c4, r2, 1
  cmov            r6, c4, LG2_TX_DESC_SIZE, LG2_TX_DESC_SIZE + 2

  // Compute the descriptor fetch address
  add             r3, d.{ring_base}.dx, d.{ci_fetch}.hx, LG2_TX_DESC_SIZE

  // Speculatively claim the descriptors
  phvwr           p.eth_tx_to_s2_my_ci, d.ci_fetch
  tblmincr.f      d.{ci_fetch}.hx, d.{ring_size}.hx, r2

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_pc, eth_tx_prep[38:6]  
  phvwrpair       p.common_te0_phv_table_raw_table_size, r6, p.common_te0_phv_table_addr, r3

  // Save data for next stages
  phvwr           p.eth_tx_t0_s2s_num_todo, r2
  phvwr           p.eth_tx_to_s1_qstate_addr[33:0], k.p4_txdma_intr_qstate_addr
  phvwrpair.e     p.eth_tx_to_s2_lif, k.p4_intr_global_lif, p.eth_tx_to_s2_qtype, k.p4_txdma_intr_qtype
  phvwr.f         p.eth_tx_to_s2_qid, k.p4_txdma_intr_qid

eth_tx_spurious_db:

  bcf             [c3], eth_tx_eval_db
  tblmincri.f     d.spurious_db_cnt, LG2_MAX_SPURIOUS_DB, 1

  phvwri.e        p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

eth_tx_eval_db:

  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

  phvwri.e        p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

eth_tx_queue_disabled:

  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

eth_tx_fetch_drop:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
