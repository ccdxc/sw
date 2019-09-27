
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

//#define   _r_stats      r4        // Stats

%%

.param eth_rx_completion
.param eth_rx_stats

.align
eth_rx_sg_start:
  // LOAD_STATS(_r_stats)

  bcf             [c2 | c3 | c7], eth_rx_sg_addr_error
  nop

  // Load DMA command pointer
  add             _r_index, r0, k.eth_rx_global_dma_cur_index

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

  // Did we run out of sg elements?
  seq             c1, _r_rem_sg, r0
  sne             c2, _r_rem_bytes, r0
  setcf           c3, [c1 & c2]
  bcf             [c3], eth_rx_sg_data_error
  nop
  //SET_STAT(_r_stats, c3, desc_data_error)

  // Calculate the next SG descriptor address
  add             _r_addr, k.eth_rx_global_sg_desc_addr, 1, LG2_RX_SG_MAX_READ_SIZE

  // Update the remaining number of pky bytes & SG descriptor address
  phvwr           p.eth_rx_global_sg_desc_addr, _r_addr
  phvwr           p.eth_rx_t1_s2s_rem_sg_elems, _r_rem_sg
  phvwr           p.eth_rx_t1_s2s_rem_pkt_bytes, _r_rem_bytes

  // Save DMA command pointer
  phvwr           p.eth_rx_global_dma_cur_index, _r_index

  // Launch eth_rx_sg stage
  phvwr.e         p.common_te1_phv_table_addr, _r_addr
  phvwr.f         p.common_te1_phv_table_raw_table_size, LG2_RX_SG_MAX_READ_SIZE

eth_rx_sg_done:   // We are done with SG

  // Update completion descriptor
  sub             r7, k.eth_rx_t1_s2s_sg_max_elems, _r_rem_sg
  phvwr           p.cq_desc_num_sg_elems, r7

  // Save DMA command pointer
  phvwr           p.eth_rx_global_dma_cur_index, _r_index

  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, ((1 << 3) | (1 << 2))

  // Launch eth_rx_stats action
  phvwri          p.common_te1_phv_table_pc, eth_rx_stats[38:6]
  phvwri          p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  // Launch eth_completion action
  phvwri.e        p.common_te0_phv_table_pc, eth_rx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_rx_sg_addr_error:
  //SET_STAT(_r_stats, _C_TRUE, desc_fetch_error)

eth_rx_sg_data_error:
  //SAVE_STATS(_r_stats)

  // Don't drop the phv, because, we have claimed the completion entry.
  // Generate an error completion.
  phvwr           p.cq_desc_status, ETH_RX_DESC_DATA_ERROR
  phvwr           p.eth_rx_global_drop, 1     // increment pkt drop counters

  // Reset the DMA command stack to discard existing DMA commands.
  addi            _r_index, r0, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_CMD_RESET(_r_ptr, _C_TRUE)
  DMA_SKIP_TO_EOP(_r_ptr, _C_FALSE)
  DMA_CMD_NEXT(_r_index)

  // Save DMA command pointer
  phvwr           p.eth_rx_global_dma_cur_index, _r_index

  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, ((1 << 3) | (1 << 2))

  // Launch eth_rx_stats action
  phvwri          p.common_te1_phv_table_pc, eth_rx_stats[38:6]
  phvwri          p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

  // Launch eth_completion action
  phvwri.e        p.common_te0_phv_table_pc, eth_rx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
