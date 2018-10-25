
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_eth_tx_prep_d d;

%%

.param  eth_tx_commit

#define  _c_do_sg         c2  // SG
#define  _c_do_tso        c3  // TSO
#define  _c_do_cq         c4  // Create CQ entry

#define  _r_num_desc      r1  // Number of descriptors consumed
#define  _r_op_x          r2  // Opcode processing register
#define  _r_op_y          r3  // Opcode processing register
#define  _r_tx_pktlen     r5  // Number of bytes that will be transferred
#define  _r_stats         r6  // Stats


.align
eth_tx_prep:

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
  phvwr           p.eth_tx_global_dma_cur_index, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

  seq             c1, r0, k.eth_tx_t0_s2s_num_todo
  bcf             [c1], eth_tx_prep_error
  nop

  // Launch commit stage
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_TX_QSTATE_SIZE, p.common_te0_phv_table_addr, k.eth_tx_to_s1_qstate_addr
  phvwri          p.common_te0_phv_table_pc, eth_tx_commit[38:6]

eth_tx_prep1:
  BUILD_APP_HEADER(0, _r_op_x, _r_op_y, _r_stats)
  addi            _r_num_desc, r0, 1
  add             _r_tx_pktlen, r0, d.{len0}.hx
  phvwr           p.to_stage_3_to_stage_data, d[511:384]

  // if we have finished processing all the descriptors then stop
  seq             c1, _r_num_desc, k.eth_tx_t0_s2s_num_todo
  // if the next descriptor is SG or TSO then stop
  sne             c2, d.num_sg_elems1, 0
  seq             c3, d.opcode1, TXQ_DESC_OPCODE_TSO
  bcf             [c1|c2|c3], eth_tx_prep_done
  nop

eth_tx_prep2:
  BUILD_APP_HEADER(1, _r_op_x, _r_op_y, _r_stats)
  addi            _r_num_desc, _r_num_desc, 1
  add             _r_tx_pktlen, _r_tx_pktlen, d.{len1}.hx
  phvwr           p.to_stage_4_to_stage_data, d[383:256]

  // if we have finished processing all the descriptors then stop
  seq             c1, _r_num_desc, k.eth_tx_t0_s2s_num_todo
  // if the next descriptor is SG or TSO then stop
  sne             c2, d.num_sg_elems2, 0
  seq             c3, d.opcode2, TXQ_DESC_OPCODE_TSO
  bcf             [c1|c2|c3], eth_tx_prep_done
  nop

eth_tx_prep3:
  BUILD_APP_HEADER(2, _r_op_x, _r_op_y, _r_stats)
  addi            _r_num_desc, _r_num_desc, 1
  add             _r_tx_pktlen, _r_tx_pktlen, d.{len2}.hx
  phvwr           p.to_stage_5_to_stage_data, d[255:128]

  // if we have finished processing all the descriptors then stop
  seq             c1, _r_num_desc, k.eth_tx_t0_s2s_num_todo
  // if the next descriptor is SG or TSO then stop
  sne             c2, d.num_sg_elems3, 0
  seq             c3, d.opcode3, TXQ_DESC_OPCODE_TSO
  bcf             [c1|c2|c3], eth_tx_prep_done
  nop

eth_tx_prep4:
  BUILD_APP_HEADER(3, _r_op_x, _r_op_y, _r_stats)
  addi            _r_num_desc, _r_num_desc, 1
  add             _r_tx_pktlen, _r_tx_pktlen, d.{len3}.hx
  phvwr           p.to_stage_6_to_stage_data, d[127:0]

eth_tx_prep_done:
  // Set number of sg elements to process
  sne             c1, d.num_sg_elems0, 0
  SET_STAT(_r_stats, c1, oper_sg)

  phvwr.c1        p.eth_tx_t0_s2s_do_sg, 1
  phvwr.c1        p.eth_tx_global_num_sg_elems, d.num_sg_elems0

  SAVE_STATS(_r_stats)

  // Set number of descriptors to process
  phvwr.e         p.eth_tx_t0_s2s_num_desc, _r_num_desc
  // Set number of bytes to tx (for rate limiter)
  phvwr.f         p.p4_intr_packet_len, _r_tx_pktlen

eth_tx_prep_error:
  SET_STAT(_r_stats, _C_TRUE, desc_fetch_error)
  SAVE_STATS(_r_stats)
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
