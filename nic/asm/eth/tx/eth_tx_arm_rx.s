
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

#include "defines.h"

// Rather than pollute the txdma phv in the normal case, for just this program
// an alternate phv is described here.  Use flit 1 as headers, and flit 2 as
// dma descriptors.
//
// This will break if there are changes to the intrinsic portion of the phv in
// flit zero.  Hopefully that is not very likley.
//
struct alt_phv_ {
    //----- flit 2 -----
    phv2pkt_dma_cmd_phv2pkt_pad : 41;
    phv2pkt_dma_cmd_phv_end_addr3 : 10;
    phv2pkt_dma_cmd_phv_start_addr3 : 10;
    phv2pkt_dma_cmd_phv_end_addr2 : 10;
    phv2pkt_dma_cmd_phv_start_addr2 : 10;
    phv2pkt_dma_cmd_phv_end_addr1 : 10;
    phv2pkt_dma_cmd_phv_start_addr1 : 10;
    phv2pkt_dma_cmd_phv_end_addr : 10;
    phv2pkt_dma_cmd_phv_start_addr : 10;
    phv2pkt_dma_cmd_cmdsize : 2;
    phv2pkt_dma_pkt_eop : 1;
    phv2pkt_dma_cmd_eop : 1;
    phv2pkt_dma_cmd_type : 3;
    _pad_flit_2_  : 384;
    //----- flit 1 -----
    pkt_p4_intr_global_tm_iport : 4;
    pkt_p4_intr_global_tm_oport : 4;
    pkt_p4_intr_global_tm_iq : 5;
    pkt_p4_intr_global_lif : 11;
    pkt_p4_intr_global_timestamp : 48;
    pkt_p4_intr_global_tm_span_session : 8;
    pkt_p4_intr_global_tm_replicate_ptr : 16;
    pkt_p4_intr_global_tm_replicate_en : 1;
    pkt_p4_intr_global_tm_cpu : 1;
    pkt_p4_intr_global_tm_q_depth : 14;
    pkt_p4_intr_global_drop : 1;
    pkt_p4_intr_global_bypass : 1;
    pkt_p4_intr_global_hw_error : 1;
    pkt_p4_intr_global_tm_oq : 5;
    pkt_p4_intr_global_debug_trace : 1;
    pkt_p4_intr_global_csum_err : 5;
    pkt_p4_intr_global_error_bits : 6;
    pkt_p4_intr_global_tm_instance_type : 4;
    pkt_p4_intr_crc_err : 1;
    pkt_p4_intr_len_err : 4;
    pkt_p4_intr_recirc_count : 3;
    pkt_p4_intr_parser_err : 1;
    pkt_p4_intr_p4_pad : 1;
    pkt_p4_intr_frame_size : 14;
    pkt_p4_intr_no_data : 1;
    pkt_p4_intr_recirc : 1;
    pkt_p4_intr_packet_len : 14;
    pkt_p4_rxdma_intr_qid : 24;
    pkt_p4_rxdma_intr_dma_cmd_ptr : 6;
    pkt_p4_rxdma_intr_qstate_addr : 34;
    pkt_p4_rxdma_intr_qtype : 3;
    pkt_p4_rxdma_intr_rx_splitter_offset : 10;
    pkt_p4_rxdma_intr_rxdma_rsv : 3;
    pkt_p4_to_p4plus_p4plus_app_id : 4;
    pkt_p4_to_p4plus_table0_valid : 1;
    pkt_p4_to_p4plus_table1_valid : 1;
    pkt_p4_to_p4plus_table2_valid : 1;
    pkt_p4_to_p4plus_table3_valid : 1;
    pkt_p4_to_p4plus_flow_id : 24;
    pkt_p4_to_p4plus_vlan_pcp : 3;
    pkt_p4_to_p4plus_vlan_dei : 1;
    pkt_p4_to_p4plus_vlan_vid : 12;
    pkt_p4_to_p4plus_packet_len : 16;
    pkt_p4_to_p4plus_csum : 16;
    pkt_p4_to_p4plus_csum_ip_bad : 1;
    pkt_p4_to_p4plus_csum_ip_ok : 1;
    pkt_p4_to_p4plus_csum_udp_bad : 1;
    pkt_p4_to_p4plus_csum_udp_ok : 1;
    pkt_p4_to_p4plus_csum_tcp_bad : 1;
    pkt_p4_to_p4plus_csum_tcp_ok : 1;
    pkt_p4_to_p4plus_vlan_valid : 1;
    pkt_p4_to_p4plus_pad : 1;
    pkt_p4_to_p4plus_l2_pkt_type : 2;
    pkt_p4_to_p4plus_pkt_type : 6;
    pkt_p4_to_p4plus_l4_sport : 16;
    pkt_p4_to_p4plus_l4_dport : 16;
    pkt_p4_to_p4plus_ip_sa : 128;
    //----- flit 0 -----
    p4_intr_global_tm_iport : 4;
    p4_intr_global_tm_oport : 4;
    p4_intr_global_tm_iq : 5;
    p4_intr_global_lif : 11;
    p4_intr_global_timestamp : 48;
    p4_intr_global_tm_span_session : 8;
    p4_intr_global_tm_replicate_ptr : 16;
    p4_intr_global_tm_replicate_en : 1;
    p4_intr_global_tm_cpu : 1;
    p4_intr_global_tm_q_depth : 14;
    p4_intr_global_drop : 1;
    p4_intr_global_bypass : 1;
    p4_intr_global_hw_error : 1;
    p4_intr_global_tm_oq : 5;
    p4_intr_global_debug_trace : 1;
    p4_intr_global_csum_err : 5;
    p4_intr_global_error_bits : 6;
    p4_intr_global_tm_instance_type : 4;
    p4_intr_crc_err : 1;
    p4_intr_len_err : 4;
    p4_intr_recirc_count : 3;
    p4_intr_parser_err : 1;
    p4_intr_p4_pad : 1;
    p4_intr_frame_size : 14;
    p4_intr_no_data : 1;
    p4_intr_recirc : 1;
    p4_intr_packet_len : 14;
    p4_txdma_intr_qid : 24;
    p4_txdma_intr_dma_cmd_ptr : 6;
    p4_txdma_intr_qstate_addr : 34;
    p4_txdma_intr_qtype : 3;
    p4_txdma_intr_txdma_rsv : 5;
    app_header_app_type : 4;
    app_header_table0_valid : 1;
    app_header_table1_valid : 1;
    app_header_table2_valid : 1;
    app_header_table3_valid : 1;
    app_header_app_data1 : 256;
};


// The p vector is the alt_phv_ defined above.
//
// The d vector is really rx qstate, but accessed here as tx qstate type.
// This is to avoid polluting eth txdma programs with rx qstate types.
//
// Txdma owns all ring intrinsics for rx qstate, so txdma can set ci=pi to stop
// phvs being scheduled.

struct alt_phv_ p;
struct tx_table_s0_t0_k_ k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

#define  _r_arm_index   r3    // host cq index when arming

// db macros use specific registers. only used at the end, so no conflicts.
#define  _r_dbval       r3    // doorbell value must be r3
#define  _r_dbaddr      r4    // doorbell address must be r4

// conditions used througout
#define _c_rx           c1    // rx is nonempty
#define _c_eq           c2    // eq is enabled
#define _c_arm          c3    // request to arm cq, commit stage will arm


%%

.align
eth_tx_arm_rx:
  bbeq            d.enable, 0, eth_tx_arm_rx_queue_disabled
  add             _r_arm_index, r0, d.{p_index1}.hx // BD Slot
  bbeq            d.eq_enable, 0, eth_tx_arm_rx_no_eq
  tblwr           d.c_index0, d.p_index0 // BD Slot
  sne             _c_arm, _r_arm_index, 0xffff // BD Slot
  tblwr._c_arm    d.p_index1, 0xffff
  bcf             [ !_c_arm ], eth_tx_arm_rx_no_eq
  tblwr.f         d.rsvd, 0

#ifdef PHV_DEBUG
  seq             c7, d.debug, 1
  phvwr.c7        p.p4_intr_global_debug_trace, 1
  trace.c7        0x1
#endif

eth_tx_arm_rx_eq:
  // Setup packet headers to invoke rxdma, with rss_type = None
  phvwrpair       p.pkt_p4_intr_global_tm_iport, TM_PORT_DMA, p.pkt_p4_intr_global_tm_oport, TM_PORT_DMA
  phvwr           p.pkt_p4_intr_global_lif, k.p4_intr_global_lif
#ifdef PHV_DEBUG
  phvwr           p.pkt_p4_intr_global_debug_trace, d.debug
#endif
  phvwrpair       p.pkt_p4_rxdma_intr_qid, k.p4_txdma_intr_qid, p.pkt_p4_rxdma_intr_qstate_addr, k.p4_txdma_intr_qstate_addr
  phvwrpair       p.pkt_p4_rxdma_intr_qtype, k.p4_txdma_intr_qtype, p.pkt_p4_rxdma_intr_rx_splitter_offset, CAPRI_PHV_FIELDS_SIZE(pkt_p4_intr_global_tm_iport, pkt_p4_to_p4plus_ip_sa)
  phvwri          p.pkt_p4_to_p4plus_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC

  // Tell eth_rx_app_header and eth_rx_rss_skip this is a request to arm the cq
  phvwr           p.pkt_p4_to_p4plus_pad, 1 // indicate it is not normal rx
  phvwr           p.pkt_p4_to_p4plus_csum, _r_arm_index // csum is arm_index

  // Send this "packet" to rxdma
  CAPRI_DMA_CMD_PHV2PKT_SETUP_STOP(phv2pkt_dma_cmd, pkt_p4_intr_global_tm_iport, pkt_p4_to_p4plus_ip_sa);
  phvwri          p.p4_txdma_intr_dma_cmd_ptr, (CAPRI_PHV_START_OFFSET(phv2pkt_dma_cmd_type) / 16)
  phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_DMA

  // XXX the tblwr to p_index1 must flush before the eq is posted,
  // otherwise we can miss a doorbell to arm after the event
  wrfence

  // Done with txdma, reeval the doobell, but don't drop the phv
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx.e      _r_dbaddr, _r_dbval
  phvwri.f        p.{app_header_table0_valid...app_header_table3_valid}, 0

eth_tx_arm_rx_no_eq:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1

eth_tx_arm_rx_queue_disabled:
  CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_CLEAR, k.p4_txdma_intr_qtype, k.p4_intr_global_lif)   // R4 = ADDR
  CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, 0, 0)   // R3 = DATA
  memwr.dx        _r_dbaddr, _r_dbval

  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
