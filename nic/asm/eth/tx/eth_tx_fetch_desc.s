
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

// NOTE: Be careful while using aliased registers, don't overlap their usage
// in any codepath.

#define  _r_ringsz      r1    // Ring size
#define  _r_numtodo     r2    // Number of descriptors to process
#define  _r_stats       r3    // Stats              (Alias)
#define  _r_dbval       r3    // Doorbell value     (Alias)
#define  _r_dbaddr      r4    // Doorbell address
#define  _r_tbladdr     r5    // Descriptor address
#define  _r_tblsz       r6    // Descriptor read size

%%

.param  eth_tx_prep

.align
eth_tx_fetch_desc:
  INIT_STATS(_r_stats)

  seq             c1, d.enable, 0
  bcf             [c1], eth_tx_queue_disabled
  seq             c2, d.p_index0, d.c_index0
  bcf             [c2], eth_tx_spurious_db
  seq             c3, d.spurious_db_cnt, 0

  // if some PHV ahead of this PHV miss speculated then reset ci_fetch
  seq             c1, d.spec_miss, 1

  // spec_miss is also intentionally cleared from stage 2. The update
  // here will not cause a conflict with it because stage 2 will not
  // update spec_miss until all PHVs between the spec_miss PHV and the
  // spec_miss ack PHV have both reached stage 2. when spec_miss is set
  // the current PHV is the spec_miss ack PHV. we are clearing spec_miss
  // here because we don't want other PHVs behind us to reset the ci_fetch
  tblwr.c1        d.spec_miss, 0
  tblwr.c1        d.{ci_fetch}.hx, d.{ci_miss}.hx

  // Drop the PHV if other PHV's ahead of this PHV are claiming to finish
  // the work
  seq             c1, d.p_index0, d.ci_fetch
  bcf             [c1], eth_tx_fetch_drop

  sll             _r_ringsz, 1, d.{ring_size}.hx

  // Drop the PHV if we are speculating too far ahead
  sub             r7, d.{ci_fetch}.hx, d.{c_index0}.hx
  mincr           r7, d.{ring_size}.hx, 0
  slt             c1, r7, MAX_DESC_SPEC
  bcf             [!c1], eth_tx_fetch_drop

  // How many descriptors are posted?
  // num_posted = (pi < ci) ? (ring_size - (ci - pi)) : (pi - ci)
  sub             _r_numtodo, d.{p_index0}.hx, d.{ci_fetch}.hx
  mincr           _r_numtodo, d.{ring_size}.hx, 0

  // We should only process upto MAX_DESC_PER_PHV
  // num_todo = max(num_posted, max_per_phv)
  sle             c2, _r_numtodo, MAX_DESC_PER_PHV
  addi.!c2        _r_numtodo, r0, MAX_DESC_PER_PHV

  // Are we at the end of the ring?
  // end_of_ring = ring_size < (ci + num_todo)
  // num_todo = end_of_ring ? 1 : num_todo
  add             r7, d.{ci_fetch}.hx, _r_numtodo
  slt             c3, _r_ringsz, r7
  cmov            _r_numtodo, c3, 1, _r_numtodo

  // Compute the descriptor fetch address
  add             _r_tbladdr, d.{ring_base}.dx, d.{ci_fetch}.hx, LG2_TX_DESC_SIZE

  // Save data for next stages
  phvwr           p.eth_tx_t0_s2s_num_todo, _r_numtodo
  phvwr           p.eth_tx_to_s1_qstate_addr[33:0], k.p4_txdma_intr_qstate_addr
  phvwr           p.eth_tx_global_lif, k.p4_intr_global_lif
  phvwr           p.eth_tx_to_s2_qtype, k.p4_txdma_intr_qtype
  phvwr           p.eth_tx_to_s2_qid, k.p4_txdma_intr_qid
  phvwr           p.eth_tx_to_s2_my_ci, d.ci_fetch

  // Speculatively claim the descriptors
  tblmincr.f      d.{ci_fetch}.hx, d.{ring_size}.hx, _r_numtodo

  // Compute the size of descriptor read
  // read_size = (num_todo == 1) ? 16 : 64
  seq             c4, _r_numtodo, 1
  cmov            _r_tblsz, c4, LG2_TX_DESC_SIZE, LG2_TX_DESC_SIZE + 2

  // SAVE_STATS(_r_stats)

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, _r_tblsz, p.common_te0_phv_table_addr, _r_tbladdr
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_prep[38:6]

eth_tx_spurious_db:

  bcf             [c3], eth_tx_eval_db
  tblmincri.f     d.spurious_db_cnt, LG2_MAX_SPURIOUS_DB, 1

  // SAVE_STATS(_r_stats)

  phvwri.e        p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

eth_tx_eval_db:

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  // SAVE_STATS(_r_stats)

  phvwri.e        p.p4_intr_global_drop, 1
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

eth_tx_queue_disabled:
  SET_STAT(_r_stats, _C_TRUE, queue_disabled)

  SAVE_STATS(_r_stats)

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

eth_tx_fetch_drop:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
