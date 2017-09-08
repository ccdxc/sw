
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"
#include "capri-macros.h"

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

%%

.param  eth_tx_packet

.align
eth_tx_fetch_desc:
  seq             c1, d.enable, 0                                           // Queue is not enabled
  seq             c2, d.c_index0, d.p_index0                                // Queue is empty
  bcf             [c1 | c2], abort_tx
  nop

  // Setup Descriptor read for next stage
  phvwri          p.app_header_table0_valid, 1
  phvwri          p.app_header_table1_valid, 0
  phvwri          p.app_header_table2_valid, 0
  phvwri          p.app_header_table3_valid, 0

  phvwri          p.common_te0_phv_table_lock_en, 1

  addi            r2, r0, loword(eth_tx_packet)
  addui           r2, r2, hiword(eth_tx_packet)
  srl             r1, r2, CAPRI_MPU_PC_SHIFT
  phvwr           p.common_te0_phv_table_pc, r1

  add             r5, d.ring_base, d.c_index0, LG2_TX_DESC_SIZE
  phvwr           p.common_te0_phv_table_addr, r5.dx
  phvwr           p.common_te0_phv_table_raw_table_size, LG2_TX_DESC_SIZE

  //tblmincri.f     d.{c_index0}.hx, d.ring_size, 1

  add             r1, r0, k.p4_intr_global_lif_sbit0_ebit2
  add             r1, r1, k.p4_intr_global_lif_sbit3_ebit10, 3
  CAPRI_RING_DOORBELL_ADDR1(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 1, r1)   // R4 = ADDR
  add             r2, r0, d.{c_index0}.hx
  mincr           r2, d.ring_size, 1

  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, r2)   // R3 = DATA
  memwr.dx        r4, r3

  nop.e
  nop

.align
abort_tx:
  phvwr           p.p4_intr_global_drop, 1        // Drop this PHV
  nop.e
  nop
