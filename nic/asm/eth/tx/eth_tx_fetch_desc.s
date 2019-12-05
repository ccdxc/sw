
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

#define  _r_ci                r1    // Current CI
#define  _r_desc_addr         r5    // Descriptor address
//#define  _r_stats           r6    // Stats

// DB macros use specific registers. Only used at the end, so no conflicts.
#define  _r_dbval       r3    // Doorbell value must be r3
#define  _r_dbaddr      r4    // Doorbell address must be r4

// conditions used througout
#define _c_tx           c1    // TX is nonempty
#define _c_eq           c2    // EQ is enabled
#define _c_arm          c3    // Request to arm cq, commit stage will arm


%%

.param  eth_tx_prep
.param  eth_tx_prep_arm

.align
eth_tx_fetch_desc:
#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif

  bbeq            d.enable, 0, eth_tx_queue_disabled
  // Use this phv to arm the cq, or do tx fetch?
  seq             _c_eq, d.eq_enable, 1 // BD Slot for branch not taken
  sne             _c_arm, d.p_index1, 0xffff
  bcf             [ _c_eq & _c_arm ], eth_tx_fetch_arm
  // If not arming, is there anything to transmit?
  sne             _c_tx, d.p_index0, d.c_index0 // BD Slot, for both
  bcf             [ !_c_tx ], eth_tx_queue_empty

  // Claim the descriptor
  add             _r_ci, r0, d.{c_index0}.hx
  tblmincri.f     d.{c_index0}.hx, d.ring_size, 1

  // Save data for next stages
  phvwr           p.{eth_tx_global_host_queue...eth_tx_global_cpu_queue}, d.{host_queue...cpu_queue}
  phvwr           p.eth_tx_global_lif, k.p4_intr_global_lif
  phvwr           p.eth_tx_to_s1_qstate_addr[33:0], k.p4_txdma_intr_qstate_addr
  phvwr           p.eth_tx_to_s2_my_ci, _r_ci
  phvwr           p.eth_tx_to_s2_qid, k.p4_txdma_intr_qid

  // Compute the descriptor fetch address
  sll             r7, _r_ci, d.lg2_desc_sz
  add             _r_desc_addr, d.{ring_base}.dx, r7

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_0
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwri          p.common_te0_phv_table_pc, eth_tx_prep[38:6]
  phvwr.e         p.common_te0_phv_table_addr, _r_desc_addr
  phvwr.f         p.common_te0_phv_table_raw_table_size, d.lg2_desc_sz

eth_tx_queue_disabled:
  // SET_STAT(_r_stats, _C_TRUE, queue_disabled)

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  // SAVE_STATS(_r_stats)

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

eth_tx_queue_empty:
  // SET_STAT(_r_stats, _C_TRUE, queue_empty)

  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  // SAVE_STATS(_r_stats)

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

eth_tx_fetch_arm:
  // We will use this phv to arm the cq, and clear the arm ring.
  // With the arm rings cleared, the next phv will proceed with normal tx.

  // Stash the arm index (as intr_index) in the phv for the commit stage
  phvwr           p.eth_tx_t0_s2s_intr_index, d.{p_index1}.hx // BD Slot, for both

  // Replace Arm Host PI to out-of-range index (will be equal to CI)
  tblwr.f         d.p_index1, 0xffff

  // XXX the tblwr to p_index1 must flush before the eq is posted,
  // otherwise we can miss a doorbell to arm after the event
  wrfence

  // Ring doorbell to re-eval if there is nothing to transmit after arming
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx.!_c_tx _r_dbaddr, _r_dbval

  // Setup globals needed for arm and event-only
  phvwr           p.eth_tx_global_host_queue, d.host_queue
  phvwr           p.eth_tx_global_cpu_queue, d.cpu_queue
  phvwr           p.eth_tx_global_lif, k.p4_intr_global_lif
  phvwr           p.eth_tx_to_s2_qid, k.p4_txdma_intr_qid

  // Setup same qstate table address for commit_arm, but next stage is prep_arm
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_0
  phvwr           p.common_te0_phv_table_addr, k.p4_txdma_intr_qstate_addr
  phvwri.e        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_prep_arm[38:6]
