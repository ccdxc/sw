
#include "defines.h"
#include "capri-macros.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_adminq_fetch_desc_d d;

%%

.param  adminq_operation

adminq_fetch_desc:
  seq             c1, d.enable, 0                                            // Queue is not enabled
  add             r1, r0, d.{p_index0}.hx                                    // Queue is full
  mincr           r1, d.{ring_size}.hx, 1
  seq             c2, r1, d.{c_index0}.hx
  bcf             [c1 | c2], return
  nop

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 1

  LOAD_NEXT_PC(p, adminq_operation, 0)

  add             r5, d.{ring_base}.dx, d.{c_index0}.hx, ADMINQ_DESC_SIZE
  phvwr           p.common_te0_phv_table_addr, r5
  phvwri          p.common_te0_phv_table_raw_table_size, ADMINQ_DESC_SIZE
  add             r1, d.{ring_size}.hx, ADMINQ_DESC_SIZE
  addi            r2, r0, 1
  sllv            r2, r2, r1
  add             r3, r2, d.{ring_base}.dx
  phvwr           p.adminq_to_s1_cq_ring_base, r3

  or              r1, k.p4_intr_global_lif_sbit3_ebit10, k.p4_intr_global_lif_sbit0_ebit2, 8
  CAPRI_RING_DOORBELL_ADDR_HOST(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, r1)   // R4 = ADDR

  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1
  tblmincri       d.{p_index1}.hx, d.{ring_size}.hx, 1
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        r4, r3

  nop.e
  nop

return:
  phvwri          p.p4_intr_global_drop, 1        // Drop this PHV
  nop.e
  nop
