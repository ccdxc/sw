
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t1_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s2_t1_k_ k;
struct tx_table_s2_t1_eth_tx_d d;

#define   _r_num_desc   r2        // Remaining number of descriptors
#define   _r_ptr        r4        // Current DMA byte offset in PHV
#define   _r_flit       r5        // Current DMA flit offset in PHV
#define   _r_index      r6        // Current DMA command offset in PHV flit

%%

.param eth_tx_completion

.align
eth_tx_start:

  bcf             [c2 | c3 | c7], eth_tx_error
  nop

  // Load DMA command pointer
  add             _r_flit, r0, k.eth_tx_global_dma_cur_flit
  add             _r_index, r0, k.eth_tx_global_dma_cur_index

  // Do we need to process any more descriptors ?
  seq             c1, r0, k.eth_tx_global_num_desc
  bcf             [c1], eth_tx_done
  nop

  // TODO: r2, r3, c1 are overwitten!!
  BUILD_APP_HEADER(0)

  add             _r_num_desc, r0, k.eth_tx_global_num_desc

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_INTRINSIC(0, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_PKT(0, r3, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  subi            _r_num_desc, _r_num_desc, 1
  beq             _r_num_desc, r0, eth_tx_done

  // Save DMA command pointers
  phvwrpair       p.eth_tx_global_dma_cur_flit, _r_flit, p.eth_tx_global_dma_cur_index, _r_index

eth_tx_continue:

  // Update the remaining number of descriptors
  phvwr           p.eth_tx_global_num_desc, _r_num_desc

  // Launch eth_tx stage
  phvwri.e        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)

eth_tx_done:

  // Update the remaining number of descriptors
  phvwr           p.eth_tx_global_num_desc, _r_num_desc

  // Launch eth_completion stage
  phvwri          p.common_te3_phv_table_pc, eth_tx_completion[38:6]
  phvwri.e        p.common_te3_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 1

eth_tx_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
