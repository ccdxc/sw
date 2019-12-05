
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;

#define  _r_stats           r2    // Stats

%%

.param  eth_tx_event
.param  eth_tx_stats

.align
eth_tx_commit_arm:
  bcf             [c2 | c3 | c7], eth_tx_commit_arm_error
  LOAD_STATS(_r_stats)

  //SET_STAT(_r_stats, _C_TRUE, arm)

  // Is the host caught up?
  seq             c7, k.eth_tx_t0_s2s_intr_index, d.{comp_index}.hx
  // No: immediately post an event only, otherwise arm the qstate
  b.!c7           eth_tx_commit_arm_event
  // Posting an event disarms the qstate (flush the table either way)
  tblwr.!c7.f     d.armed, 0 // BD Slot, for branch taken
  tblwr.c7.f      d.armed, 1

  // If armed, then we're done.
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

eth_tx_commit_arm_event:
  // Will post an eq desc to host (do_intr here indicates it is event-only)
  phvwri          p.eth_tx_global_do_eq, 1
  phvwri          p.eth_tx_global_do_intr, 1

  // Init fields in eq desc, except gen_color to be set by eth_tx_event action
  phvwri          p.eq_desc_code, EQ_CODE_TX_COMP_HX
  add             r7, r0, k.eth_tx_to_s2_qid
  phvwrpair       p.eq_desc_lif_index, d.lif_index, p.eq_desc_qid, r7.wx
  phvwri          p.eq_desc_intr_data, 0x01000000

  // Launch eth_tx_event action
  phvwr           p.app_header_table2_valid, 0x1
  phvwr           p.common_te2_phv_table_addr, d.{intr_index_or_eq_addr}.dx
  phvwri          p.common_te2_phv_table_raw_table_size, LG2_EQ_QSTATE_SIZE
  phvwri.e        p.common_te2_phv_table_lock_en, 1
  phvwri.f        p.common_te2_phv_table_pc, eth_tx_event[38:6]

eth_tx_commit_arm_error:
  SET_STAT(_r_stats, _C_TRUE, queue_error)

  SAVE_STATS(_r_stats)

  phvwr           p.eth_tx_global_drop, 1     // increment error counters
  phvwr           p.p4_intr_global_drop, 1

  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_1
  phvwri.e        p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
