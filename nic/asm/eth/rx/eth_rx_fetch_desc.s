
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
  seq             c2, d.p_index0, d.c_index0
  bcf             [c1 | c2], abort_rx
  nop

  phvwr           p.eth_rx_global_packet_len, k.p4_to_p4plus_packet_len

  // Setup Descriptor read for next stage
  phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)

  phvwri          p.common_te0_phv_table_lock_en, 1

  addi            r2, r0, loword(eth_rx_packet)
  addui           r2, r2, hiword(eth_rx_packet)
  srl             r1, r2, CAPRI_MPU_PC_SHIFT
  phvwr           p.common_te0_phv_table_pc, r1

  add             r5, d.{ring_base}.dx, d.{c_index0}.hx, LG2_RX_DESC_SIZE
  phvwr           p.common_te0_phv_table_addr, r5
  phvwri          p.common_te0_phv_table_raw_table_size, LG2_RX_DESC_SIZE

  // Completion descriptor address
  add             r1, d.{cq_ring_base}.dx, d.{p_index1}.hx, LG2_RX_CMPL_DESC_SIZE
  phvwr           p.eth_rx_to_s1_cq_desc_addr, r1

  // Completion descriptor
  phvwr           p.eth_rx_cq_desc_completion_index, d.c_index0
  phvwr           p.eth_rx_cq_desc_queue_id, k.{p4_rxdma_intr_qid}.hx
  phvwr           p.eth_rx_cq_desc_bytes_written, k.{p4_to_p4plus_packet_len}.hx
  phvwr           p.eth_rx_cq_desc_checksum, k.{p4_to_p4plus_l4_checksum}.wx
  or              r1, k.p4_to_p4plus_vlan_vid_sbit4_ebit11, k.p4_to_p4plus_vlan_vid_sbit0_ebit3, 8
  or              r1, r1, k.p4_to_p4plus_vlan_dei, 12
  or              r1, r1, k.p4_to_p4plus_vlan_pcp, 13
  phvwr           p.eth_rx_cq_desc_vlan_tag, r1.hx
  phvwr           p.eth_rx_cq_desc_flags, k.{p4_to_p4plus_flags}.hx
  phvwr           p.eth_rx_cq_desc_color, d.color

  // Claim the descriptor
  //tblmincri       d.{c_index0}.hx, d.{ring_size}.hx, 1
  add             r1, r0, d.{c_index0}.hx
  mincr           r1, d.{ring_size}.hx, 1
  tblwr           d.c_index0, r1.hx

  // Claim the completion entry
  //tblmincri       d.{p_index1}.hx, d.{ring_size}.hx, 1
  add             r1, r0, d.{p_index1}.hx
  mincr           r1, d.{ring_size}.hx, 1
  tblwr           d.p_index1, r1.hx

  seq             c1, d.p_index1, 0
  tblmincri.c1    d.color, 1, 1

  nop.e
  nop

.align
abort_rx:
  phvwr           p.p4_intr_global_drop, 1
  nop.e
  nop

