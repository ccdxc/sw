
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_d d;

%%

.param eth_tx_completion

.align
eth_tx_start:

  add             r5, r0, k.eth_tx_global_dma_cur_flit
  add             r6, r0, k.eth_tx_global_dma_cur_index

  // Are we in the middle of SG?
  seq             c1, k.eth_tx_t0_s2s_sg_in_progress, 1
  bcf             [c1], eth_tx_sg_continue
  add             r2, r0, k.eth_tx_t0_s2s_num_sg_elems

  // Start new packet
  BUILD_APP_HEADER(0)

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_INTRINSIC(r4, 0, r7)
  DMA_CMD_NEXT(r5, r6, c7)

  // Is there an SG list provided with this descriptor?
  sne             c1, k.eth_tx_to_s2_num_sg_elems0, 0
  bcf             [c1], eth_tx_sg_start
  // Indicate SG in progress
  phvwri.c1       p.eth_tx_t0_s2s_sg_in_progress, 1

eth_tx_packet:

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_PKT(0, c0, r3, r4)
  DMA_CMD_NEXT(r5, r6, c7)

eth_tx_continue:

  // Do we need to process any more descriptors ?
  sub             r2, k.eth_tx_t0_s2s_num_desc, 1
  beq             r2, r0, eth_tx_done
  phvwr           p.eth_tx_t0_s2s_num_desc, r2

  // Save DMA command pointers
  phvwrpair       p.eth_tx_global_dma_cur_flit, r5, p.eth_tx_global_dma_cur_index, r6
  // Launch eth_tx stage
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri.f        p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_tx_done:

  // Save DMA command pointer
  phvwrpair       p.eth_tx_global_dma_cur_flit, r5, p.eth_tx_global_dma_cur_index, r6
  // Launch eth_completion stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)
  phvwri.e        p.common_te1_phv_table_pc, eth_tx_completion[38:6]
  phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_tx_sg_start:

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_HDR(0, !c0, r3, r4)
  DMA_CMD_NEXT(r5, r6, c7)

  add             r2, r0, k.eth_tx_to_s2_num_sg_elems0

eth_tx_sg_continue:

  // C2 = No more frags .. Packet EOP

  subi            r2, r2, 1
  seq             c1, r2, 0

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_FRAG(0, c1, r3, r4)
  DMA_CMD_NEXT(r5, r6, c7)

  bcf             [c1], eth_tx_sg_done

  subi            r2, r2, 1
  seq             c1, r2, 0

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_FRAG(1, c1, r3, r4)
  DMA_CMD_NEXT(r5, r6, c7)

  bcf             [c1], eth_tx_sg_done

  subi            r2, r2, 1
  seq             c1, r2, 0

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_FRAG(2, c1, r3, r4)
  DMA_CMD_NEXT(r5, r6, c7)

  bcf             [c1], eth_tx_sg_done

  subi            r2, r2, 1
  seq             c1, r2, 0

  DMA_CMD_PTR(r4, r5, r6, r7)
  DMA_FRAG(3, c1, r3, r4)
  DMA_CMD_NEXT(r5, r6, c7)

eth_tx_sg_done:
  bcf             [c1], eth_tx_continue
  phvwri.c1       p.eth_tx_t0_s2s_sg_in_progress, 0     // We are done with SG

  // Fetch & process more SG elements in subsequent stages

  // Update the remaining number of SG elements
  phvwr           p.eth_tx_t0_s2s_num_sg_elems, r2
  // Set the next SG descriptor address
  add             r2, k.eth_tx_t0_s2s_sg_desc_addr, 1, LG2_TX_SG_MAX_READ_SIZE
  phvwr           p.eth_tx_t0_s2s_sg_desc_addr, r2

  // Save DMA command pointer
  phvwrpair       p.eth_tx_global_dma_cur_flit, r5, p.eth_tx_global_dma_cur_index, r6
  // Launch eth_tx stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwr.e         p.common_te0_phv_table_addr, r2
  phvwri.f        p.common_te0_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE
