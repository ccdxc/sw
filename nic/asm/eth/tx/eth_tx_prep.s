
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_eth_tx_prep_d d;

%%

.param  eth_tx_commit

#define  _r_num_rem       r1  // Number of descriptors remaining to prep
#define  _r_num_desc      r2  // Number of descriptors to process

.align
eth_tx_prep:

#if 0
  DEBUG_DESCR(0)
  DEBUG_DESCR(1)
  DEBUG_DESCR(2)
  DEBUG_DESCR(3)
#endif

  bcf             [c2 | c3 | c7], eth_tx_prep_error
  nop

  // Set intrinsics
#ifndef GFT
  phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
  phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif

  // Setup DMA CMD PTR
  phvwr           p.p4_txdma_intr_dma_cmd_ptr, ETH_DMA_CMD_START_OFFSET
  phvwrpair       p.eth_tx_global_dma_cur_flit, ETH_DMA_CMD_START_FLIT, p.eth_tx_global_dma_cur_index, ETH_DMA_CMD_START_INDEX

  add             _r_num_rem, r0, k.eth_tx_t0_s2s_num_todo
  beq             _r_num_rem, r0, eth_tx_prep_error
  addi            _r_num_desc, r0, 0

  sne             c1, d.num_sg_elems0, 0
  bcf             [c1], eth_tx_prep1
  subi            _r_num_rem, _r_num_rem, 1
  beq             _r_num_rem, r0, eth_tx_prep1
  

  sne             c2, d.num_sg_elems1, 0
  bcf             [c2], eth_tx_prep1
  subi            _r_num_rem, _r_num_rem, 1
  beq             _r_num_rem, r0, eth_tx_prep2

  sne             c3, d.num_sg_elems2, 0
  bcf             [c3], eth_tx_prep2
  subi            _r_num_rem, _r_num_rem, 1
  beq             _r_num_rem, r0, eth_tx_prep3

  sne             c4, d.num_sg_elems3, 0
  bcf             [c4], eth_tx_prep3
  nop

eth_tx_prep4:
  BUILD_APP_HEADER(3, r6, r7)
  addi            _r_num_desc, _r_num_desc, 1
  phvwr           p.to_stage_6_to_stage_data, d[127:0]

eth_tx_prep3:
  BUILD_APP_HEADER(2, r6, r7)
  addi            _r_num_desc, _r_num_desc, 1
  phvwr           p.to_stage_5_to_stage_data, d[255:128]

eth_tx_prep2:
  BUILD_APP_HEADER(1, r6, r7)
  addi            _r_num_desc, _r_num_desc, 1
  phvwr           p.to_stage_4_to_stage_data, d[383:256]

eth_tx_prep1:
  BUILD_APP_HEADER(0, r6, r7)
  addi            _r_num_desc, _r_num_desc, 1
  phvwr           p.to_stage_3_to_stage_data, d[511:384]

eth_tx_prep_done:
  // Set number of descriptors to process
  phvwr           p.eth_tx_t0_s2s_num_desc, _r_num_desc

  // Set number of sg elements to process
  phvwr.c1        p.eth_tx_t0_s2s_sg_start, 1
  phvwr.c1        p.eth_tx_t1_s2s_num_sg_elems, d.num_sg_elems0

  // Launch commit stage
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_TX_QSTATE_SIZE, p.common_te0_phv_table_addr, k.eth_tx_to_s1_qstate_addr
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_commit[38:6]

eth_tx_prep_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
