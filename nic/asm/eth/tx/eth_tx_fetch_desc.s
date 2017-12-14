
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

%%

.param  eth_tx_packet

.align
eth_tx_fetch_desc:
  seq             c1, d.enable, 0
  seq             c2, d.c_index0, d.p_index0
  bcf             [c1 | c2], abort_tx
  nop

  phvwri          p.app_header_app_type, P4PLUS_APPTYPE_CLASSIC_NIC

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwri          p.common_te0_phv_table_pc, eth_tx_packet[38:6]
  add             r5, d.{ring_base}.dx, d.{c_index0}.hx, LG2_TX_DESC_SIZE
  phvwr           p.common_te0_phv_table_addr, r5
  phvwri          p.common_te0_phv_table_raw_table_size, LG2_TX_DESC_SIZE

  // Completion descriptor address
  add             r1, d.{cq_ring_base}.dx, d.{p_index1}.hx, LG2_TX_CMPL_DESC_SIZE
  phvwr           p.eth_tx_to_s1_cq_desc_addr, r1

  // Completion descriptor
  phvwr           p.eth_tx_cq_desc_completion_index, d.c_index0
  phvwr           p.eth_tx_cq_desc_queue_id, k.{p4_txdma_intr_qid}.hx
  phvwr           p.eth_tx_cq_desc_color, d.color

  // Claim the descriptor
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1

  // Claim the completion entry
  tblmincri       d.{p_index1}.hx, d.{ring_size}.hx, 1

  seq             c1, d.p_index1, 0
  tblmincri.c1    d.color, 1, 1

  // Update doorbell
  or              r1, k.p4_intr_global_lif_sbit3_ebit10, k.p4_intr_global_lif_sbit0_ebit2, 8
  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, r1)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

  nop.e
  nop

.align
abort_tx:
  // Update doorbell
  or              r1, k.p4_intr_global_lif_sbit3_ebit10, k.p4_intr_global_lif_sbit0_ebit2, 8
  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, r1)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

  phvwr.e         p.p4_intr_global_drop, 1        // Drop this PHV
  nop
