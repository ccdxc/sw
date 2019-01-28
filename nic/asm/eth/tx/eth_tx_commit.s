
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;

#define  _r_num_desc        r1    // Number of descriptors processed
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

  // Have the PHVs ahead of us caught up with their work?
  seq             c1, d.{c_index0}.hx, k.{eth_tx_to_s2_my_ci}.hx
  bcf             [!c1], eth_tx_commit_abort
  add             _r_num_desc, r0, k.eth_tx_t0_s2s_num_desc
  seq             c1, _r_num_desc, 0
  bcf             [c1], eth_tx_commit_abort
  nop

  // Claim the descriptor(s)
  tblmincr        d.{c_index0}.hx, d.{ring_size}.hx, _r_num_desc

  // Indicate ci_miss to (stage 0) if this PHV misspeculated
  sne             c2, k.eth_tx_t0_s2s_num_todo, k.eth_tx_t0_s2s_num_desc
  tblwr.!c2       d.spec_miss, 0
  tblwr.c2        d.spec_miss, 1
  tblwr.c2        d.{ci_miss}.hx, d.{c_index0}.hx

  // Do we need to generate a completion?
  seq             c1, k.eth_tx_global_cq_entry, 0
  bcf             [c1], eth_tx_commit_no_cq_entry
  phvwr           p.eth_tx_global_host_queue, d.host_queue

eth_tx_commit_cq_entry:

  // Compute the completion descriptor address
  add             _r_cq_desc_addr, d.{cq_ring_base}.dx, d.{comp_index}.hx, LG2_TX_CMPL_DESC_SIZE
  phvwr           p.eth_tx_t0_s2s_cq_desc_addr, _r_cq_desc_addr

  // Claim a completion entry
  tblmincri       d.{comp_index}.hx, d.{ring_size}.hx, 1

  // Change color if end-of-ring
  phvwr           p.eth_tx_cq_desc_color, d.color
  seq             c1, d.comp_index, 0
  tblmincri.c1    d.color, 1, 1

  // comp_index = (ci - 1). Note - set this after incrementing ci
  add             r7, r0, d.{c_index0}.hx
  mincr           r7, d.{ring_size}.hx, -1
  phvwr           p.eth_tx_cq_desc_comp_index, r7.hx

  // Save cq & intr information for eth_tx_completion action
  phvwr           p.eth_tx_t0_s2s_intr_assert_index, d.{intr_assert_index}.hx
  phvwri          p.eth_tx_t0_s2s_intr_assert_data, 0x01000000
  phvwr           p.eth_tx_global_intr_enable, d.intr_enable

eth_tx_commit_no_cq_entry:
  bbeq            k.eth_tx_t0_s2s_do_tso, 1, eth_tx_commit_tso
  nop
  bbeq            k.eth_tx_t0_s2s_do_sg, 1, eth_tx_commit_sg
  tblwr.f         d.rsvd, 0   // For eth_tx_commit_tx and eth_tx_commit_sg paths

eth_tx_commit_tx:
  // Launch eth_tx stage
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwrpair       p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY, p.common_te0_phv_table_addr, 0
  b               eth_tx_commit_done
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_start[38:6]

eth_tx_commit_sg:
  // Compute the sg descriptor address
  add             _r_sg_desc_addr, d.{sg_ring_base}.dx, k.{eth_tx_to_s2_my_ci}.hx, LG2_TX_SG_DESC_SIZE
  phvwr           p.eth_tx_global_sg_desc_addr, _r_sg_desc_addr
  // Launch eth_tx_sg stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)
  phvwrpair       p.common_te1_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te1_phv_table_addr, _r_sg_desc_addr
  b               eth_tx_commit_done
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
  tblwr           d.tso_hdr_addr, k.eth_tx_to_s2_tso_hdr_addr
  tblwr           d.tso_hdr_len, k.eth_tx_to_s2_tso_hdr_len
  tblwr.f         d.{tso_ipid_delta...tso_seq_delta}, 0
  b               eth_tx_commit_tso_done
  phvwr           p.{eth_tx_t2_s2s_tso_hdr_addr...eth_tx_t2_s2s_tso_hdr_len}, k.{eth_tx_to_s2_tso_hdr_addr...eth_tx_to_s2_tso_hdr_len}

eth_tx_commit_tso_done:
  bbeq            k.eth_tx_t0_s2s_do_sg, 1, eth_tx_commit_tso_sg
  nop

eth_tx_commit_tso_nonsg:
  // Launch eth_tx_tso stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 1)
  phvwrpair       p.common_te2_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY, p.common_te2_phv_table_addr, 0
  b               eth_tx_commit_done
  phvwri.f        p.common_te2_phv_table_pc, eth_tx_tso_start[38:6]

eth_tx_commit_tso_sg:
  // Compute the sg descriptor address
  add             _r_sg_desc_addr, d.{sg_ring_base}.dx, k.{eth_tx_to_s2_my_ci}.hx, LG2_TX_SG_DESC_SIZE
  phvwr           p.eth_tx_global_sg_desc_addr, _r_sg_desc_addr
  // Launch eth_tx_tso stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 1)
  phvwrpair       p.common_te2_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te2_phv_table_addr, _r_sg_desc_addr
  phvwri.f        p.common_te2_phv_table_pc, eth_tx_tso_start[38:6]

eth_tx_commit_done:
  // SAVE_STATS(_r_stats)

  // Eval doorbell when pi == ci
  seq             c3, d.{p_index0}.hx, d.{c_index0}.hx
  bcf             [c3], eth_tx_commit_eval_db
  nop.!c3.e
  nop

eth_tx_commit_eval_db:
  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.eth_tx_to_s2_qtype, k.eth_tx_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.eth_tx_to_s2_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      _r_dbaddr, _r_dbval
  nop

eth_tx_commit_abort:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

eth_tx_commit_error:
  SET_STAT(_r_stats, _C_TRUE, queue_error)

  SAVE_STATS(_r_stats)

  phvwr           p.eth_tx_global_drop, 1     // increment pkt drop counters
  phvwr           p.p4_intr_global_drop, 1

  // Launch eth_tx_stats action
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, 1
  phvwri.e        p.common_te3_phv_table_pc, eth_tx_stats[38:6]
  phvwri.f        p.common_te3_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
