
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_notify_fetch_desc_d d;

#define  _r_desc_addr         r1    // Descriptor address
#define  _r_host_desc_addr    r2    // Host descriptor address
#define  _r_dbval             r3    // Doorbell value
#define  _r_dbaddr            r4    // Doorbell address

%%

.param  notify_process_desc

.align
notify_fetch_desc:
  seq             c1, d.enable, 0
  bcf             [c1], notify_queue_disabled
  seq             c2, d.p_index0, d.c_index0
  bcf             [c2], notify_spurious_db
#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif
  // Compute descriptor fetch address
  add             _r_desc_addr, d.{ring_base}.dx, d.{c_index0}.hx, LG2_NOTIFY_EVENT_DESC_SIZE

  // Compute host descriptor address
  add             _r_host_desc_addr, d.{host_ring_base}.dx, d.{host_pindex}.hx, LG2_NOTIFY_EVENT_DESC_SIZE

  // Claim the host & notify descriptors
  tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1
  tblmincri       d.{host_pindex}.hx, d.{host_ring_size}.hx, 1

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
  phvwri          p.common_te0_phv_table_lock_en, 0
  phvwrpair       p.common_te0_phv_table_raw_table_size, LG2_NOTIFY_EVENT_DESC_SIZE, p.common_te0_phv_table_addr, _r_desc_addr
  phvwri          p.common_te0_phv_table_pc, notify_process_desc[38:6]

  // Save data for next stages
  phvwr           p.notify_global_host_queue, d.host_queue
  phvwr           p.notify_global_intr_enable, d.intr_enable
  phvwr           p.notify_t0_s2s_host_desc_addr, _r_host_desc_addr
  phvwr.e         p.notify_t0_s2s_intr_assert_index, d.{host_intr_assert_index}.hx
  phvwri.f        p.notify_t0_s2s_intr_assert_data, 0x01000000

notify_spurious_db:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

notify_queue_disabled:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
