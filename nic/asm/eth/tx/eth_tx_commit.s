
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;

#define  _r_stats           r2    // Stats
#define  _r_dbval           r3    // Doorbell value
#define  _r_dbaddr          r4    // Doorbell address
#define  _r_cq_desc_addr    r5    // CQ descriptor address
#define  _r_sg_desc_addr    r6    // SG descriptor address

%%

.param  eth_tx_start
.param  eth_tx_sg_start
.param  eth_tx_tso_start
.param  eth_tx_stats

.align
eth_tx_commit:
  LOAD_STATS(_r_stats)

  bcf             [c2 | c3 | c7], eth_tx_commit_error
  nop

  // Do we need to generate a completion?
  bbne            k.eth_tx_global_cq_entry, 1, eth_tx_commit_no_cq_entry
  nop

eth_tx_commit_cq_entry:
  // Compute the completion descriptor address
  sll             r7, d.{comp_index}.hx, d.lg2_cq_desc_sz
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, r7
  phvwr           p.eth_tx_t0_s2s_cq_desc_addr, _r_cq_desc_addr

  // Claim the completion entry
  phvwr           p.eth_tx_cq_desc_comp_index, k.eth_tx_to_s2_my_ci
  tblmincri       d.{comp_index}.hx, d.ring_size, 1

  // Change color if end-of-ring
  phvwr           p.eth_tx_cq_desc_color, d.color
  seq             c7, d.comp_index, 0
  tblmincri.c7    d.color, 1, 1

  // Save intr information for eth_tx_completion action
  phvwr           p.eth_tx_t0_s2s_intr_assert_index, d.{intr_assert_index}.hx
  phvwri          p.eth_tx_t0_s2s_intr_assert_data, 0x01000000

eth_tx_commit_no_cq_entry:
  bbeq            k.eth_tx_t0_s2s_do_tso, 1, eth_tx_commit_tso
  nop
  bbeq            k.eth_tx_t0_s2s_do_sg, 1, eth_tx_commit_sg
  tblwr.f         d.rsvd, 0   // For eth_tx_commit_tx and eth_tx_commit_sg paths

eth_tx_commit_tx:
  // Launch eth_tx stage
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY, p.common_te0_phv_table_addr, 0
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_start[38:6]

eth_tx_commit_sg:
  // Compute the sg descriptor address
  sll             r7, k.{eth_tx_to_s2_my_ci}.hx, d.lg2_sg_desc_sz
  add             _r_sg_desc_addr, d.{sg_ring_base}.dx, r7
  phvwr           p.eth_tx_global_sg_desc_addr, _r_sg_desc_addr

  // Launch eth_tx_sg stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)
  phvwrpair.e     p.common_te1_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te1_phv_table_addr, _r_sg_desc_addr
  phvwri.f        p.common_te1_phv_table_pc, eth_tx_sg_start[38:6]

eth_tx_commit_tso:
  bbeq            k.eth_tx_global_tso_sot, 1, eth_tx_commit_tso_sot
  nop

eth_tx_commit_tso_cont:
eth_tx_commit_tso_eot:
  // update tso state in qstate
  tbladd          d.tso_ipid_delta, 1
  tbladd.f        d.tso_seq_delta, k.eth_tx_to_s2_tso_hdr_addr // mss
  b               eth_tx_commit_tso_done
  phvwr           p.{eth_tx_t2_s2s_tso_hdr_addr...eth_tx_t2_s2s_tso_seq_delta}, d.{tso_hdr_addr...tso_seq_delta}

eth_tx_commit_tso_sot:
  // init tso state in qstate
  tblwr           d.{tso_hdr_addr...tso_hdr_len}, k.{eth_tx_to_s2_tso_hdr_addr...eth_tx_to_s2_tso_hdr_len}
  tblwr.f         d.{tso_ipid_delta...tso_seq_delta}, 0
  b               eth_tx_commit_tso_done
  phvwr           p.{eth_tx_t2_s2s_tso_hdr_addr...eth_tx_t2_s2s_tso_seq_delta}, d.{tso_hdr_addr...tso_seq_delta}

eth_tx_commit_tso_done:
  bbeq            k.eth_tx_t0_s2s_do_sg, 1, eth_tx_commit_tso_sg
  nop

eth_tx_commit_tso_nonsg:
  // Launch eth_tx_tso stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 1)
  phvwrpair.e     p.common_te2_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY, p.common_te2_phv_table_addr, 0
  phvwri.f        p.common_te2_phv_table_pc, eth_tx_tso_start[38:6]

eth_tx_commit_tso_sg:
  // Compute the sg descriptor address
  sll             r7, k.{eth_tx_to_s2_my_ci}.hx, d.lg2_sg_desc_sz
  add             _r_sg_desc_addr, d.{sg_ring_base}.dx, r7
  phvwr           p.eth_tx_global_sg_desc_addr, _r_sg_desc_addr

  // Launch eth_tx_tso stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 1)
  phvwrpair.e     p.common_te2_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te2_phv_table_addr, _r_sg_desc_addr
  phvwri.f        p.common_te2_phv_table_pc, eth_tx_tso_start[38:6]

eth_tx_commit_error:
  SET_STAT(_r_stats, _C_TRUE, queue_error)

  SAVE_STATS(_r_stats)

  phvwr           p.eth_tx_global_drop, 1     // increment pkt drop counters
  phvwr           p.p4_intr_global_drop, 1

  // Launch eth_tx_stats action
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)
  phvwri.e        p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
