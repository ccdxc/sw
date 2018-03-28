
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t3_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s3_t3_k_ k;

#define   _r_ptr        r4        // Current DMA byte offset in PHV
#define   _r_flit       r5        // Current DMA flit offset in PHV
#define   _r_index      r6        // Current DMA command offset in PHV flit

%%

.align
eth_tx_completion:

  add             _r_flit, r0, k.eth_tx_global_dma_cur_flit
  add             _r_index, r0, k.eth_tx_global_dma_cur_index

  // End of pipeline - Make sure no more tables will be launched
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, 0

  // Do we need to generate an interrupt?
  seq             c1, r0, k.eth_tx_t3_s2s_intr_assert_addr

  // DMA Completion
  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HOST_PHV2MEM(_r_ptr, c1, k.eth_tx_t3_s2s_cq_desc_addr, CAPRI_PHV_START_OFFSET(eth_tx_cq_desc_status), CAPRI_PHV_END_OFFSET(eth_tx_cq_desc_rsvd4), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  nop.e.c1
  nop

eth_tx_interrupt:

  // DMA Interrupt
  DMA_CMD_PTR(_r_ptr, _r_flit, _r_index, r7)
  DMA_HBM_PHV2MEM(_r_ptr, c0, k.eth_tx_t3_s2s_intr_assert_addr, CAPRI_PHV_START_OFFSET(eth_tx_t3_s2s_intr_assert_data), CAPRI_PHV_END_OFFSET(eth_tx_t3_s2s_intr_assert_data), r7)
  DMA_CMD_NEXT(_r_flit, _r_index, c7)

  nop.e
  nop
