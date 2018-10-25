
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "nic/p4/common/defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;

#define   _r_num_desc   r2        // Remaining number of descriptors
#define   _r_ptr        r5        // Current DMA byte offset in PHV
#define   _r_index      r6        // Current DMA command index in PHV

%%

.param eth_tx_completion

.align
eth_tx_start:

  bcf             [c2 | c3 | c7], eth_tx_error
  nop

  // Load DMA command pointer
  add             _r_index, r0, k.eth_tx_global_dma_cur_index

  // Do we need to process any more descriptors ?
  seq             c1, r0, k.eth_tx_t0_s2s_num_desc
  bcf             [c1], eth_tx_done
  nop

  add             _r_num_desc, r0, k.eth_tx_t0_s2s_num_desc

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_INTRINSIC(0, _r_ptr)
  DMA_CMD_NEXT(_r_index)

  DMA_CMD_PTR(_r_ptr, _r_index, r7)
  DMA_PKT(0, r3, _r_ptr, to_s3)
  DMA_CMD_NEXT(_r_index)

  subi            _r_num_desc, _r_num_desc, 1
  beq             _r_num_desc, r0, eth_tx_done

  // Save DMA command index
  phvwr           p.eth_tx_global_dma_cur_index, _r_index

eth_tx_continue:

  // Update the remaining number of descriptors
  phvwr.e         p.eth_tx_t0_s2s_num_desc, _r_num_desc

  // Launch eth_tx stage
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_tx_done:

  // Update the remaining number of descriptors
  phvwr           p.eth_tx_t0_s2s_num_desc, _r_num_desc

  // Launch eth_tx_completion stage
  phvwri.e        p.common_te0_phv_table_pc, eth_tx_completion[38:6]
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_tx_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
