/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#include "../common-p4+/common_txdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy eth_tx_fetch_desc
#define tx_table_s0_t0_action eth_tx_fetch_desc
#define tx_table_s1_t0_action eth_tx_packet


#include "../common-p4+/common_txdma.p4"
#include "eth_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action eth_tx_dummy(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata0)
}

action eth_tx_fetch_desc(
        pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
        p_index0, c_index0, p_index1, c_index1,
        enable, ring_base, ring_size, cq_ring_base, intr_assert_addr, color)
{
    // For K+I struct generation
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // For D-struct generation
    MODIFY_QSTATE_INTRINSIC(eth_tx_qstate)
    modify_field(eth_tx_qstate.enable, enable);
    modify_field(eth_tx_qstate.ring_base, ring_base);
    modify_field(eth_tx_qstate.ring_size, ring_size);
    modify_field(eth_tx_qstate.cq_ring_base, cq_ring_base);
    modify_field(eth_tx_qstate.intr_assert_addr, intr_assert_addr);
    modify_field(eth_tx_qstate.color, color);
}

action eth_tx_packet(addr,
        /* addr_lo, addr_hi, rsvd, num_sg_elems, opcode */
        len, vlan_tci, hdr_len, rsvd2, V, C, O, mss_or_csumoff, rsvd3_or_rsvd4)
{
    // For K+I struct generation

    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T0_S2S

    // For D-struct generation
    modify_field(eth_tx_desc.addr, addr);
    modify_field(eth_tx_desc.len, len);
    modify_field(eth_tx_desc.vlan_tci, vlan_tci);
    modify_field(eth_tx_desc.hdr_len, hdr_len);
    modify_field(eth_tx_desc.rsvd2, rsvd2);
    modify_field(eth_tx_desc.V, V);
    modify_field(eth_tx_desc.C, C);
    modify_field(eth_tx_desc.O, O);
    modify_field(eth_tx_desc.mss_or_csumoff, mss_or_csumoff);
    modify_field(eth_tx_desc.rsvd3_or_rsvd4, rsvd3_or_rsvd4);
}
