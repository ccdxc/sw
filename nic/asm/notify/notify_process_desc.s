
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_notify_process_desc_d d;

#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

%%

.param  notify_completion

.align
notify_process_desc:

  bcf             [c2 | c3 | c7], notify_process_desc_error
  nop

  // Setup DMA CMD PTR
  phvwr           p.p4_txdma_intr_dma_cmd_ptr, NOTIFY_DMA_CMD_START_OFFSET
  phvwr           p.notify_global_dma_cur_index, (NOTIFY_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | NOTIFY_DMA_CMD_START_INDEX

  // Write event data into PHV
  phvwr           p.notify_host_event_desc_data, d[511:0]

  // Post to event host in next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwr.e         p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
  phvwri.f        p.common_te0_phv_table_pc, notify_completion[38:6]

notify_process_desc_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
