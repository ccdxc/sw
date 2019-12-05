
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;
struct tx_table_s3_t0_eth_tx_d d;

#define   _r_addr       r1        // Current buffer/descriptor address
#define   _r_num_sg     r2        // Remaining number of SG elements
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

%%

.param eth_tx_completion
.param eth_tx_stats

.align
eth_tx_start:
  bcf             [c2 | c3 | c7], eth_tx_error

  // Load DMA command pointer
  add             _r_index, r0, k.eth_tx_global_dma_cur_index   // BD slot

  // Are we in the middle of SG?
  bbeq            k.eth_tx_global_sg_in_progress, 1, eth_tx_sg_continue
  add             _r_num_sg, r0, k.eth_tx_global_num_sg_elems

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_INTRINSIC(_r_ptr)
  DMA_CMD_NEXT(_r_index)

eth_tx:
  bbeq            k.eth_tx_t0_s2s_do_sg, 1, eth_tx_sg_start
  nop

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_PKT(_r_addr, _r_ptr, to_s3)
  DMA_CMD_NEXT(_r_index)

  b               eth_tx_done
  nop

eth_tx_sg_start:

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_HDR(_r_addr, _r_ptr, to_s3)
  DMA_CMD_NEXT(_r_index)

eth_tx_sg_continue:

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(0, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  bcf             [c1], eth_tx_done

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(1, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  bcf             [c1], eth_tx_done

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(2, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  bcf             [c1], eth_tx_done

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(3, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  bcf             [c1], eth_tx_done

eth_tx_sg_next:     // Continue SG in next stage
  // Calculate the address of next 4 sges
  add             r7, k.common_te0_phv_table_addr, 1, LG2_TX_SG_MAX_READ_SIZE
  phvwr           p.common_te0_phv_table_addr, r7

  // Save state
  phvwrpair.e     p.eth_tx_global_sg_in_progress, 1, p.eth_tx_global_num_sg_elems, _r_num_sg

  // Save DMA command pointer
  phvwr.f         p.eth_tx_global_dma_cur_index, _r_index

eth_tx_done:
  // Save DMA command pointer
  phvwr           p.eth_tx_global_dma_cur_index, _r_index

  phvwri          p.{app_header_table0_valid...app_header_table1_valid}, 0x3

  phvwri          p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri          p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  phvwri.e        p.common_te0_phv_table_pc, eth_tx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_tx_error:
  phvwri          p.eth_tx_global_stats[STAT_desc_fetch_error], 1

  // Don't drop the phv, because, we have claimed the completion entry.
  // Generate an error completion.
  phvwr           p.eth_tx_global_do_cq, 1
  phvwr           p.cq_desc_status, ETH_TX_DESC_ADDR_ERROR
  phvwr           p.eth_tx_global_drop, 1     // increment pkt drop counters

  // Reset the DMA command stack to discard existing DMA commands.
  phvwr           p.eth_tx_global_dma_cur_index, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

  phvwri          p.{app_header_table0_valid...app_header_table1_valid}, 0x3

  phvwri          p.common_te1_phv_table_pc, eth_tx_stats[38:6]
  phvwri          p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  phvwri.e        p.common_te0_phv_table_pc, eth_tx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
