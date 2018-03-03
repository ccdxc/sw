
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_eth_tx_prep_d d;

%%

.param  eth_tx_start

.align
eth_tx_prep:

#if 0
  DEBUG_DESCR(0)
  DEBUG_DESCR(1)
  DEBUG_DESCR(2)
  DEBUG_DESCR(3)
#endif

  // Set intrinsics
#ifndef GFT
  phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
  phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif

  // Setup DMA CMD PTR
  phvwr           p.p4_txdma_intr_dma_cmd_ptr, ETH_DMA_CMD_START_OFFSET
  phvwrpair       p.eth_tx_global_dma_cur_flit, ETH_DMA_CMD_START_FLIT, p.eth_tx_global_dma_cur_index, 0

  add             r1, r0, k.eth_tx_t0_s2s_num_desc
  beq             r1, r0, eth_tx_prep_error
  nop

  sub             r1, r1, 1   // Make num_desc 0-based

.brbegin
  br              r1[1:0]
  nop

  .brcase 0
    b               eth_tx_prep1
    nop

  .brcase 1
    b               eth_tx_prep2
    nop

  .brcase 2
    b               eth_tx_prep3
    nop

  .brcase 3
    b               eth_tx_prep4
    nop

.brend

eth_tx_prep4:
  phvwr           p.to_stage_5_to_stage_data, d[127:0]

eth_tx_prep3:
  phvwr           p.to_stage_4_to_stage_data, d[255:128]

eth_tx_prep2:
  phvwr           p.to_stage_3_to_stage_data, d[383:256]

eth_tx_prep1:
  phvwr           p.to_stage_2_to_stage_data, d[511:384]

eth_tx_prep_done:
  // Start TX
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  seq             c1, d.num_sg_elems0, 0
  phvwrpair.c1    p.common_te0_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY, p.common_te0_phv_table_addr, 0
  phvwrpair.!c1   p.common_te0_phv_table_raw_table_size, LG2_TX_SG_MAX_READ_SIZE, p.common_te0_phv_table_addr, k.eth_tx_t0_s2s_sg_desc_addr
  phvwri.e        p.common_te0_phv_table_pc, eth_tx_start[38:6]
  nop

eth_tx_prep_error:
  illegal
