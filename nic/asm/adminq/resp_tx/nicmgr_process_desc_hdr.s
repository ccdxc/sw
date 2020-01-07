
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_nicmgr_process_desc_hdr_d d;

%%

.param  nicmgr_post_adminq

.align
nicmgr_process_desc_hdr:

  bcf             [c2 | c3 | c7], nicmgr_process_desc_hdr_error
  nop

  // Setup DMA CMD PTR
  phvwr           p.p4_txdma_intr_dma_cmd_ptr, NICMGR_DMA_CMD_START_OFFSET
  phvwr           p.nicmgr_global_dma_cur_index, (NICMGR_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | NICMGR_DMA_CMD_START_INDEX

  // Save data for next stages
  phvwr           p.nicmgr_t0_s2s_lif, d.{lif}.hx
  phvwr           p.nicmgr_t0_s2s_qtype, d.qtype
  phvwr           p.nicmgr_t0_s2s_qid, d.{qid}.wx
  phvwr           p.nicmgr_t0_s2s_comp_index, d.{comp_index}.hx

  // Setup adminq qstate lookup for next stage
  add             r7, r0, d.{adminq_qstate_addr}.dx
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_ADMINQ_QSTATE_SIZE, p.common_te0_phv_table_addr, r7
  phvwri.f        p.common_te0_phv_table_pc, nicmgr_post_adminq[38:6]

nicmgr_process_desc_hdr_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
