
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t1_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s2_t1_k_ k;
struct tx_table_s2_t1_eth_tx_d d;

#define   _r_addr       r1        // Current buffer/descriptor address
#define   _r_num_sg     r2        // Remaining number of SG elements
#define   _r_ptr        r4        // Current DMA byte offset in PHV
#define   _r_flit       r5        // Current DMA flit offset in PHV
#define   _r_index      r6        // Current DMA command offset in PHV flit

%%

.param eth_tx_completion

.align
eth_tx_sg_start:

  bcf             [c2 | c3 | c7], eth_tx_sg_error
  nop

  // Load DMA command pointer
  add             _r_flit, r0, k.eth_tx_global_dma_cur_flit
  add             _r_index, r0, k.eth_tx_global_dma_cur_index

  // Are we in the middle of SG?
  seq             c1, k.eth_tx_t1_s2s_sg_in_progress, 1
  bcf             [c1], eth_tx_sg_continue
  add             _r_num_sg, r0, k.eth_tx_t1_s2s_num_sg_elems

  BUILD_APP_HEADER(0)

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_INTRINSIC(0, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HDR(0, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

eth_tx_sg_continue:

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_FRAG(0, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  bcf             [c1], eth_tx_sg_done

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_FRAG(1, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  bcf             [c1], eth_tx_sg_done

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_FRAG(2, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  bcf             [c1], eth_tx_sg_done

  subi            _r_num_sg, _r_num_sg, 1
  seq             c1, _r_num_sg, 0

  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_FRAG(3, c1, _r_addr, _r_ptr)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  bcf             [c1], eth_tx_sg_done
  nop

eth_tx_sg_next:   // Continue SG in next stage
  phvwri          p.eth_tx_t1_s2s_sg_in_progress, 1

  // Calculate the next SG descriptor address
  add             _r_addr, k.eth_tx_t1_s2s_sg_desc_addr, 1, LG2_TX_SG_MAX_READ_SIZE

  // Update the remaining number of SG elements & SG descriptor address
  phvwrpair       p.eth_tx_t1_s2s_sg_desc_addr, _r_addr, p.eth_tx_t1_s2s_num_sg_elems, _r_num_sg

  // Save DMA command pointer
  phvwrpair       p.eth_tx_global_dma_cur_flit, _r_flit, p.eth_tx_global_dma_cur_index, _r_index

  // Launch eth_tx_sg stage
  phvwrpair.e     p.common_te1_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te1_phv_table_addr, _r_addr
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, (1 << 2)

eth_tx_sg_done:   // We are done with SG
  phvwri          p.eth_tx_t1_s2s_sg_in_progress, 0

  // Save DMA command pointer
  phvwrpair       p.eth_tx_global_dma_cur_flit, _r_flit, p.eth_tx_global_dma_cur_index, _r_index

  // Launch eth_completion stage
  phvwri          p.common_te3_phv_table_pc, eth_tx_completion[38:6]
  phvwri.e        p.common_te3_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 1

eth_tx_sg_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
