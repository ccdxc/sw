
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k k;
struct common_p4plus_stage0_app_header_table_eth_rx_fetch_desc_d d;

%%

.param  eth_rx_packet

.align
eth_rx_fetch_desc:
  seq             c1, d.enable, 0                                            // Queue is not enabled
  add             r1, r0, d.c_index0                                         // Queue is full
  mincr           r1, d.ring_size, 1
  seq             c2, r1, d.p_index0
  bcf             [c1 | c2], abort_rx
  nop

  phvwr           p.eth_rx_global_packet_len, k.p4_to_p4plus_packet_len

  // Setup Descriptor read for next stage
  phvwri          p.app_header_table0_valid, 1
  phvwri          p.app_header_table1_valid, 0
  phvwri          p.app_header_table2_valid, 0
  phvwri          p.app_header_table3_valid, 0

  phvwri          p.common_te0_phv_table_lock_en, 1

  addi            r2, r0, loword(eth_rx_packet);
  addui           r2, r2, hiword(eth_rx_packet);
  srl             r1, r2, CAPRI_MPU_PC_SHIFT;
  phvwr           p.common_te0_phv_table_pc, r1

  add             r5, d.ring_base, d.c_index0, LG2_RX_DESC_SIZE
  phvwr           p.common_te0_phv_table_addr, r5.dx
  phvwr           p.common_te0_phv_table_raw_table_size, LG2_RX_DESC_SIZE

  tblmincri.f     d.{c_index0}.hx, d.ring_size, 1

  nop.e
  nop

.align
abort_rx:
  phvwr           p.p4_intr_global_drop, 1
  nop.e
  nop
