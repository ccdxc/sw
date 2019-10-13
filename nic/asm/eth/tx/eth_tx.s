
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;

#define   _r_addr       r2  // Buffer address
#define   _r_stats      r3  // Stats
#define   _r_ptr        r5  // Current DMA byte offset in PHV
#define   _r_index      r6  // Current DMA command index in PHV

%%

.param eth_tx_completion
.param eth_tx_stats

.align
eth_tx_start:
  LOAD_STATS(_r_stats)

  bcf             [c2 | c3 | c7], eth_tx_error
  nop

  // Load DMA command pointer
  add             _r_index, r0, k.eth_tx_global_dma_cur_index

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_INTRINSIC(_r_ptr)
  DMA_CMD_NEXT(_r_index)

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_PKT(_r_addr, _r_ptr, to_s3)
  DMA_CMD_NEXT(_r_index)

eth_tx_done:
  SAVE_STATS(_r_stats)

  // Save DMA command index
  phvwr           p.eth_tx_global_dma_cur_index, _r_index

  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (TABLE_VALID_0 | TABLE_VALID_1)

  phvwri          p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri          p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  phvwri.e        p.common_te0_phv_table_pc, eth_tx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_tx_error:
  SET_STAT(_r_stats, _C_TRUE, desc_fetch_error)

  SAVE_STATS(_r_stats)

  // Don't drop the phv, because, we have claimed the completion entry.
  // Generate an error completion.
  phvwr           p.eth_tx_global_do_cq, 1
  phvwr           p.cq_desc_status, ETH_TX_DESC_ADDR_ERROR
  phvwr           p.eth_tx_global_drop, 1     // increment pkt drop counters

  // Reset the DMA command stack to discard existing DMA commands.
  phvwr           p.eth_tx_global_dma_cur_index, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (TABLE_VALID_0 | TABLE_VALID_1)

  phvwri          p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri          p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  phvwri.e        p.common_te0_phv_table_pc, eth_tx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
