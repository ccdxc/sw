
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;

#define  _r_comp_index      r1    // Current completion index
#define  _r_color           r2    // Current color
#define  _r_stats           r3    // Stats
#define  _r_tbl_addr        r4    // Table address
#define  _r_tbl_size        r5    // Table size
#define  _r_tbl_valid       r6    // Next stage table valid bits

// DB macros use specific registers. Only used at the end, so no conflicts.
#define  _r_dbval           r3    // Doorbell value must be r3
#define  _r_dbaddr          r4    // Doorbell address must be r4

#define  _c_cq              c1    // Will post a completion
#define  _c_eq              c2    // Will post an event
#define  _c_sg              c3    // Need to fetch sgl


%%

.param  eth_tx_start
.param  eth_tx_sg_start
.param  eth_tx_tso_start
.param  eth_tx_event
.param  eth_tx_stats

.align
eth_tx_commit:
  bcf             [c2 | c3 | c7], eth_tx_commit_error
  ori             _r_tbl_valid, r0, 0 // BD Slot

  // Do we need to generate a completion?
  bbne            k.eth_tx_global_do_cq, 1, eth_tx_commit_pkt
  seq             _c_sg, k.eth_tx_t0_s2s_do_sg, 1   // BD slot

  // Claim the completion entry
  add             _r_comp_index, r0, d.{comp_index}.hx
  tblmincri       d.{comp_index}.hx, d.ring_size, 1

  // Change color if end-of-ring
  add             _r_color, r0, d.color
  seq             c7, d.comp_index, 0
  tblmincri.c7    d.color, 1, 1

  // Post an event if armed and posting a completion.
  bbeq            d.armed, 1, eth_tx_commit_eq
  // BD Slot: either way completion will disarm
  tblwr.f         d.armed, 0
  // Table is flushed, no tblwr/etc past this point.

eth_tx_cq_entry:
  // If eq is not enabled then fire the interrupt if interrupts are enabled
  seq             _c_eq, d.eq_enable, 1
  phvwr.!_c_eq    p.eth_tx_global_do_intr, d.intr_enable
  phvwr.!_c_eq    p.eth_tx_t0_s2s_intr_index, d.{intr_index_or_eq_addr}.dx

  // Compute the completion descriptor address
  sll             r7, _r_comp_index, d.lg2_cq_desc_sz
  add             r7, r7, d.{cq_ring_base}.dx
  phvwr           p.eth_tx_t0_s2s_cq_desc_addr, r7

  // Update the completion descriptor
  phvwr           p.cq_desc_comp_index, k.{eth_tx_to_s2_my_ci}.hx
  phvwr           p.cq_desc_color, _r_color

eth_tx_commit_pkt:
  bbeq            k.eth_tx_t0_s2s_do_tso, 1, eth_tx_commit_pkt_tso
  nop

eth_tx_commit_pkt_nontso:
  // Compute the sg descriptor address
  sll             _r_tbl_addr, k.eth_tx_to_s2_my_ci, d.lg2_sg_desc_sz
  add             _r_tbl_addr, _r_tbl_addr, d.{sg_ring_base}.dx
  cmov            _r_tbl_size, _c_sg, LG2_TX_SG_MAX_READ_SIZE, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  // Launch eth_tx stage
  ori             _r_tbl_valid, _r_tbl_valid, TABLE_VALID_0
  phvwr           p.{app_header_table0_valid...app_header_table3_valid}, _r_tbl_valid
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, _r_tbl_size, p.common_te0_phv_table_addr, _r_tbl_addr
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_start[38:6]

eth_tx_commit_pkt_tso:
  tblwr.f         d.rsvd, 0

  // Compute the sg descriptor address
  sll             _r_tbl_addr, k.eth_tx_to_s2_my_ci, d.lg2_sg_desc_sz
  add             _r_tbl_addr, _r_tbl_addr, d.{sg_ring_base}.dx
  cmov            _r_tbl_size, _c_sg, LG2_TX_SG_MAX_READ_SIZE, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  // Launch eth_tx_tso stage
  ori             _r_tbl_valid, _r_tbl_valid, TABLE_VALID_1
  phvwr           p.{app_header_table0_valid...app_header_table3_valid}, _r_tbl_valid
  phvwri          p.common_te1_phv_table_lock_en, 0
  phvwrpair.e     p.common_te1_phv_table_raw_table_size, _r_tbl_size, p.common_te1_phv_table_addr, _r_tbl_addr
  phvwri.f        p.common_te1_phv_table_pc, eth_tx_tso_start[38:6]

eth_tx_commit_eq:
  // Will post an eq desc to host
  phvwri          p.eth_tx_global_do_eq, 1

  // Init fields in eq desc, except gen_color to be set by eth_tx_event action
  phvwri          p.eq_desc_code, EQ_CODE_TX_COMP_HX
  add             r7, r0, k.eth_tx_to_s2_qid
  phvwrpair       p.eq_desc_lif_index, d.lif_index, p.eq_desc_qid, r7.wx

  // Launch eth_tx_event action in parallel with the tx action
  ori             _r_tbl_valid, _r_tbl_valid, TABLE_VALID_2
  phvwri          p.common_te2_phv_table_lock_en, 1
  phvwri          p.common_te2_phv_table_pc, eth_tx_event[38:6]
  phvwr           p.common_te2_phv_table_addr, d.{intr_index_or_eq_addr}.dx
  b               eth_tx_cq_entry
  phvwri          p.common_te2_phv_table_raw_table_size, LG2_EQ_QSTATE_SIZE

eth_tx_commit_error:
  LOAD_STATS(_r_stats)
  SET_STAT(_r_stats, _C_TRUE, queue_error)
  SAVE_STATS(_r_stats)

  phvwr           p.eth_tx_global_drop, 1     // increment pkt drop counters
  phvwr           p.p4_intr_global_drop, 1

  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_1
  phvwri.e        p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
