
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_adminq_process_desc_d d;

%%

.param  adminq_post_nicmgr

.align
adminq_process_desc:

  bcf             [c2 | c3 | c7], adminq_process_desc_error
  nop

  phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_EGRESS

  // Setup DMA CMD PTR
  phvwr           p.p4_txdma_intr_dma_cmd_ptr, ADMINQ_DMA_CMD_START_OFFSET
  phvwr           p.adminq_global_dma_cur_index, (ADMINQ_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ADMINQ_DMA_CMD_START_INDEX

  // Build nicmgr request descriptor
  phvwr           p.nicmgr_req_desc_lif, k.{adminq_t0_s2s_lif}.hx
  phvwr           p.nicmgr_req_desc_qtype, k.adminq_t0_s2s_qtype
  phvwr           p.nicmgr_req_desc_qid, k.{adminq_t0_s2s_qid}.wx
  phvwr           p.nicmgr_req_desc_adminq_qstate_addr, k.{adminq_t0_s2s_adminq_qstate_addr}.dx
  phvwr           p.nicmgr_req_desc_adminq_cmd_desc, d[511:0]

  // Setup nicmgr qstate lookup in next stage
  add             r7, r0, k.adminq_t0_s2s_nicmgr_qstate_addr
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_NICMGR_QSTATE_SIZE, p.common_te0_phv_table_addr, r7
  phvwri.f        p.common_te0_phv_table_pc, adminq_post_nicmgr[38:6]

adminq_process_desc_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
