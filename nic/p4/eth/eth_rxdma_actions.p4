/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#include "../common-p4+/common_rxdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy eth_rx_fetch_desc

#define rx_table_s0_t0_action eth_rx_fetch_desc
#define rx_table_s1_t0_action eth_rx_packet
#define rx_table_s2_t0_action eth_rx_cq_create_desc
#define rx_table_s3_t0_action eth_rx_cq_post_desc

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

action eth_rx_fetch_desc(
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
    p_index0, c_index0, p_index1, c_index1, p_index2, c_index2, p_index3, c_index3, 
    p_index4, c_index4, p_index5, c_index5, p_index6, c_index6, p_index7, c_index7, 
    enable, ring_base, ring_size, cq_qstate_addr)
{
    // For K+I struct generation

    // From Intrinsic & RXDMA headers
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // From APP header
    modify_field(p4_to_p4plus_scratch.p4plus_app_id, p4_to_p4plus.p4plus_app_id);
    modify_field(p4_to_p4plus_scratch.table0_valid, p4_to_p4plus.table0_valid);
    modify_field(p4_to_p4plus_scratch.table1_valid, p4_to_p4plus.table1_valid);
    modify_field(p4_to_p4plus_scratch.table2_valid, p4_to_p4plus.table2_valid);
    modify_field(p4_to_p4plus_scratch.table3_valid, p4_to_p4plus.table3_valid);
    modify_field(p4_to_p4plus_scratch.flags, p4_to_p4plus.flags);
    modify_field(p4_to_p4plus_scratch.vlan_pcp, p4_to_p4plus.vlan_pcp);
    modify_field(p4_to_p4plus_scratch.vlan_dei, p4_to_p4plus.vlan_dei);
    modify_field(p4_to_p4plus_scratch.vlan_vid, p4_to_p4plus.vlan_vid);
    modify_field(p4_to_p4plus_scratch.l4_checksum, p4_to_p4plus.l4_checksum);
    modify_field(p4_to_p4plus_scratch.ip_proto, p4_to_p4plus.ip_proto);
    modify_field(p4_to_p4plus_scratch.l4_sport, p4_to_p4plus.l4_sport);
    modify_field(p4_to_p4plus_scratch.l4_dport, p4_to_p4plus.l4_dport);
    modify_field(p4_to_p4plus_scratch.packet_len, p4_to_p4plus.packet_len);

    // For D-struct generation
    MODIFY_QSTATE_INTRINSIC(eth_rx_qstate)
    modify_field(eth_rx_qstate.enable, enable);
    modify_field(eth_rx_qstate.ring_base, ring_base);
    modify_field(eth_rx_qstate.ring_size, ring_size);
    modify_field(eth_rx_qstate.cq_qstate_addr, cq_qstate_addr);
}

action eth_rx_packet(addr, len, rsvd0)
{
    // For K+I struct generation
    modify_field(eth_rx_global_scratch.packet_len, eth_rx_global.packet_len);

    // For D-struct generation
    modify_field(eth_rx_desc.addr, addr);
    modify_field(eth_rx_desc.len, len);
    modify_field(eth_rx_desc.rsvd0, rsvd0);
}

action eth_rx_cq_create_desc()
{
}

action eth_rx_cq_post_desc()
{
}
