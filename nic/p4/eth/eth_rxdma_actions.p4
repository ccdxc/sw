/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#include "../common-p4+/common_rxdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy eth_rx_app_header
#define rx_table_s0_t0_action eth_rx_app_header
// The following tables & their respective actions are defined in common-p4+
// Stage0 - table=eth_rx_rss_params
// Stage1 - table=eth_rx_rss_indir
#define rx_table_s1_t0_action eth_rx_rss_skip
#define rx_table_s2_t0_action eth_rx_fetch_desc
#define rx_table_s3_t0_action eth_rx_packet

#include "../common-p4+/common_rxdma.p4"
#include "eth_rxdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action eth_dummy_action_default() {}

action eth_rx_dummy(data0, data1, data2, data3, data4, data5, data6, data7) 
{
    SCRATCH_METADATA_INIT_7(scratch_metadata0)
}

action eth_rx_app_header(
    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
    p_index0, c_index0, p_index1, c_index1,
    enable, color, rsvd1,
    ring_base, ring_size, cq_ring_base, intr_assert_addr, rss_type)
{
    // --- For K+I struct generation

    // K: From Intrinsic & RXDMA headers
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // I: From APP header
    modify_field(p4_to_p4plus_scratch.p4plus_app_id, p4_to_p4plus.p4plus_app_id);
    modify_field(p4_to_p4plus_scratch.table0_valid, p4_to_p4plus.table0_valid);
    modify_field(p4_to_p4plus_scratch.table1_valid, p4_to_p4plus.table1_valid);
    modify_field(p4_to_p4plus_scratch.table2_valid, p4_to_p4plus.table2_valid);
    modify_field(p4_to_p4plus_scratch.table3_valid, p4_to_p4plus.table3_valid);

    modify_field(p4_to_p4plus_scratch.vlan_pcp, p4_to_p4plus.vlan_pcp);
    modify_field(p4_to_p4plus_scratch.vlan_dei, p4_to_p4plus.vlan_dei);
    modify_field(p4_to_p4plus_scratch.vlan_vid, p4_to_p4plus.vlan_vid);
    modify_field(p4_to_p4plus_scratch.vlan_valid, p4_to_p4plus.vlan_valid);
    modify_field(p4_to_p4plus_scratch.packet_len, p4_to_p4plus.packet_len);
    modify_field(p4_to_p4plus_scratch.csum, p4_to_p4plus.csum);
    modify_field(p4_to_p4plus_scratch.csum_ip_bad, p4_to_p4plus.csum_ip_bad);
    modify_field(p4_to_p4plus_scratch.csum_ip_ok, p4_to_p4plus.csum_ip_ok);
    modify_field(p4_to_p4plus_scratch.csum_udp_bad, p4_to_p4plus.csum_udp_bad);
    modify_field(p4_to_p4plus_scratch.csum_udp_ok, p4_to_p4plus.csum_udp_ok);
    modify_field(p4_to_p4plus_scratch.csum_tcp_bad, p4_to_p4plus.csum_tcp_bad);
    modify_field(p4_to_p4plus_scratch.csum_tcp_ok, p4_to_p4plus.csum_tcp_ok);

    modify_field(p4_to_p4plus_scratch.rss_flags, p4_to_p4plus.rss_flags);
    modify_field(p4_to_p4plus_scratch.l4_sport, p4_to_p4plus.l4_sport);
    modify_field(p4_to_p4plus_scratch.l4_dport, p4_to_p4plus.l4_dport);
    modify_field(p4_to_p4plus_scratch.ip_sa, p4_to_p4plus.ip_sa);
    modify_field(p4_to_p4plus_scratch.ip_da, p4_to_p4plus.ip_da);

    // --- For D-struct generation
    MODIFY_QSTATE_INTRINSIC(eth_rx_qstate)
    modify_field(eth_rx_qstate.enable, enable);
    modify_field(eth_rx_qstate.color, color);
    modify_field(eth_rx_qstate.rsvd1, rsvd1);
    modify_field(eth_rx_qstate.ring_base, ring_base);
    modify_field(eth_rx_qstate.ring_size, ring_size);
    modify_field(eth_rx_qstate.cq_ring_base, cq_ring_base);
    modify_field(eth_rx_qstate.intr_assert_addr, intr_assert_addr);
    modify_field(eth_rx_qstate.rss_type, rss_type);
}

action eth_rx_rss_skip()
{
    // --- For K+I struct generation

    MODIFY_ETH_RX_GLOBAL
    MODIFY_ETH_RX_T0_S2S

    // --- For D-struct generation
}

action eth_rx_fetch_desc(
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
    p_index0, c_index0, p_index1, c_index1,
    enable, color, rsvd1,
    ring_base, ring_size, cq_ring_base, intr_assert_addr, rss_type)
{
    // --- For K+I struct generation

    MODIFY_ETH_RX_GLOBAL
    MODIFY_ETH_RX_T0_S2S

    // --- For D-struct generation
    modify_field(eth_rx_qstate.pc, pc);
    MODIFY_QSTATE_INTRINSIC(eth_rx_qstate)
    modify_field(eth_rx_qstate.enable, enable);
    modify_field(eth_rx_qstate.color, color);
    modify_field(eth_rx_qstate.rsvd1, rsvd1);
    modify_field(eth_rx_qstate.ring_base, ring_base);
    modify_field(eth_rx_qstate.ring_size, ring_size);
    modify_field(eth_rx_qstate.cq_ring_base, cq_ring_base);
    modify_field(eth_rx_qstate.intr_assert_addr, intr_assert_addr);
    modify_field(eth_rx_qstate.rss_type, rss_type);
}

action eth_rx_packet(
    PARAM_RX_DESC()
)
{
    // --- For K+I struct generation

    MODIFY_ETH_RX_GLOBAL
    MODIFY_ETH_RX_T0_S2S

    // --- For D-struct generation
    MODIFY_RX_DESC()
}
