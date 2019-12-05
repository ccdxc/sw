
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s4_t1_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s4_t1_k_ k;
struct rx_table_s4_t1_eth_rx_sg_d d;

#define   _r_addr       r1        // Current buffer/descriptor address
#define   _r_rem_sg     r2        // Remaining number of sg elements
#define   _r_rem_bytes  r3        // Remaining bytes
#define   _r_len        r4
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

%%

.param eth_rx_completion
.param eth_rx_stats

.align
eth_rx_sg_start:
  bcf             [c2 | c3 | c7], eth_rx_sg_addr_error

  // Load DMA command pointer
  add             _r_index, r0, k.eth_rx_global_dma_cur_index   // BD slot

  // Load current SG status
  add             _r_rem_sg, r0, k.eth_rx_t1_s2s_rem_sg_elems
  add             _r_rem_bytes, r0, k.eth_rx_t1_s2s_rem_pkt_bytes

  // Make sure there are bytes remaining to DMA
  seq             c1, _r_rem_bytes, r0
  bcf             [c1], eth_rx_sg_done
  nop

eth_rx_sg_continue:

  // dma_len = min(rem_bytes, buf_len)
  sle             c1, _r_rem_bytes, d.{len0}.hx
  cmov            _r_len, c1, _r_rem_bytes, d.{len0}.hx
  beq             _r_len, r0, eth_rx_sg_data_error

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(0, _r_addr, _r_len, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  sub             _r_rem_bytes, _r_rem_bytes, _r_len
  sub             _r_rem_sg, _r_rem_sg, 1

  bcf             [c1], eth_rx_sg_done

  // dma_len = min(rem_bytes, buf_len)
  sle             c1, _r_rem_bytes, d.{len1}.hx
  cmov            _r_len, c1, _r_rem_bytes, d.{len1}.hx
  beq             _r_len, r0, eth_rx_sg_data_error

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(1, _r_addr, _r_len, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  sub             _r_rem_bytes, _r_rem_bytes, _r_len
  sub             _r_rem_sg, _r_rem_sg, 1

  bcf             [c1], eth_rx_sg_done

  // dma_len = min(rem_bytes, buf_len)
  sle             c1, _r_rem_bytes, d.{len2}.hx
  cmov            _r_len, c1, _r_rem_bytes, d.{len2}.hx
  beq             _r_len, r0, eth_rx_sg_data_error

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(2, _r_addr, _r_len, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  sub             _r_rem_bytes, _r_rem_bytes, _r_len
  sub             _r_rem_sg, _r_rem_sg, 1

  bcf             [c1], eth_rx_sg_done

  // dma_len = min(rem_bytes, buf_len)
  sle             c1, _r_rem_bytes, d.{len3}.hx
  cmov            _r_len, c1, _r_rem_bytes, d.{len3}.hx
  beq             _r_len, r0, eth_rx_sg_data_error

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_FRAG(3, _r_addr, _r_len, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  sub             _r_rem_bytes, _r_rem_bytes, _r_len
  sub             _r_rem_sg, _r_rem_sg, 1

  bcf             [c1], eth_rx_sg_done
  nop

eth_rx_sg_next:   // Continue SG in next stage
  // Calculate the address of next 4 sges
  add             r7, k.common_te1_phv_table_addr, 1, LG2_RX_SG_MAX_READ_SIZE
  phvwr           p.common_te1_phv_table_addr, r7

  // Save state
  phvwrpair.e     p.eth_rx_t1_s2s_rem_pkt_bytes, _r_rem_bytes, p.eth_rx_t1_s2s_rem_sg_elems, _r_rem_sg

  // Save DMA command pointer
  phvwr.f         p.eth_rx_global_dma_cur_index, _r_index

eth_rx_sg_done:   // We are done with SG
  // Did we run out of sg elements?
  seq             c1, _r_rem_sg, r0
  // Do we still have bytes left?
  sne             c2, _r_rem_bytes, r0
  setcf           c3, [c1 & c2]
  bcf             [c3], eth_rx_sg_data_error
  nop

  // Update completion descriptor
  sub             r7, k.eth_rx_t1_s2s_sg_max_elems, _r_rem_sg
  phvwr           p.cq_desc_num_sg_elems, r7

  // Save DMA command pointer
  phvwr           p.eth_rx_global_dma_cur_index, _r_index

  // Launch stats & completion action
  phvwri          p.{app_header_table0_valid...app_header_table1_valid}, 0x3

  phvwri          p.common_te0_phv_table_pc, eth_rx_completion[38:6]
  phvwri          p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  phvwri.e        p.common_te1_phv_table_pc, eth_rx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_rx_sg_addr_error:
  phvwri          p.eth_rx_global_stats[STAT_desc_fetch_error], 1

  b               eth_rx_sg_error
  phvwri          p.cq_desc_status, ETH_RX_DESC_ADDR_ERROR

eth_rx_sg_data_error:
  phvwri          p.eth_rx_global_stats[STAT_desc_data_error], 1

  b               eth_rx_sg_error
  phvwr           p.cq_desc_status, ETH_RX_DESC_DATA_ERROR

eth_rx_sg_error:
  // Reset the DMA command stack to discard existing DMA commands
  addi            _r_index, r0, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

  // Discard the packet data but not the phv
  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_CMD_RESET(_r_ptr, _C_TRUE)
  DMA_SKIP_TO_EOP(_r_ptr, _C_FALSE)
  DMA_CMD_NEXT(_r_index)

  // Increment pkt drop counters but don't drop the phv because we have
  // claimed a completion entry
  phvwr           p.eth_rx_global_drop, 1

  // Save DMA command pointer
  phvwr           p.eth_rx_global_dma_cur_index, _r_index

  // Launch stats & completion action
  phvwri          p.{app_header_table0_valid...app_header_table1_valid}, 0x3

  phvwri          p.common_te0_phv_table_pc, eth_rx_completion[38:6]
  phvwri          p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  phvwri.e        p.common_te1_phv_table_pc, eth_rx_stats[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
