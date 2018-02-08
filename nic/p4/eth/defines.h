
// HACK: Commented out as workaround for NCC action_id bug
// modify_field(QSTATE.pc, pc); \

#define MODIFY_QSTATE_INTRINSIC(QSTATE) \
    modify_field(QSTATE.rsvd, rsvd); \
    modify_field(QSTATE.cosA, cosA); \
    modify_field(QSTATE.cosB, cosB); \
    modify_field(QSTATE.cos_sel, cos_sel); \
    modify_field(QSTATE.eval_last, eval_last); \
    modify_field(QSTATE.host, host); \
    modify_field(QSTATE.total, total);  \
    modify_field(QSTATE.pid, pid); \
    modify_field(QSTATE.p_index0, p_index0); \
    modify_field(QSTATE.c_index0, c_index0); \
    modify_field(QSTATE.p_index1, p_index1); \
    modify_field(QSTATE.c_index1, c_index1);

/*
 * RXDMA
 */

#define MODIFY_ETH_RX_GLOBAL \
    modify_field(eth_rx_global_scratch.qstate_addr, eth_rx_global.qstate_addr);

#define MODIFY_ETH_RX_T0_S2S \
    modify_field(eth_rx_t0_s2s_scratch.packet_len, eth_rx_t0_s2s.packet_len); \
    modify_field(eth_rx_t0_s2s_scratch.__pad, eth_rx_t0_s2s.__pad); \
    modify_field(eth_rx_t0_s2s_scratch.cq_desc_addr, eth_rx_t0_s2s.cq_desc_addr); \
    modify_field(eth_rx_t0_s2s_scratch.intr_assert_addr, eth_rx_t0_s2s.intr_assert_addr); \
    modify_field(eth_rx_t0_s2s_scratch.intr_assert_data, eth_rx_t0_s2s.intr_assert_data);

#define PARAM_RX_DESC(n) \
    addr_lo##n, rsvd0##n, addr_hi##n, rsvd1##n, \
    len##n, rsvd2##n, opcode##n, rsvd3##n, rsvd4##n

#define MODIFY_RX_DESC(n) \
    modify_field(eth_rx_desc.addr_lo##n, addr_lo##n); \
    modify_field(eth_rx_desc.addr_hi##n, addr_hi##n); \
    modify_field(eth_rx_desc.rsvd0##n, rsvd0##n); \
    modify_field(eth_rx_desc.rsvd1##n, rsvd1##n); \
    modify_field(eth_rx_desc.len##n, len##n); \
    modify_field(eth_rx_desc.opcode##n, opcode##n); \
    modify_field(eth_rx_desc.rsvd2##n, rsvd2##n); \
    modify_field(eth_rx_desc.rsvd3##n, rsvd3##n); \
    modify_field(eth_rx_desc.rsvd4##n, rsvd4##n);

/*
 * TXDMA
 */

#define MODIFY_ETH_TX_GLOBAL \
    modify_field(eth_tx_global_scratch.lif, eth_tx_global.lif); \
    modify_field(eth_tx_global_scratch.qtype, eth_tx_global.qtype); \
    modify_field(eth_tx_global_scratch.qid, eth_tx_global.qid); \
    modify_field(eth_tx_global_scratch.dma_cur_flit, eth_tx_global.dma_cur_flit); \
    modify_field(eth_tx_global_scratch.dma_cur_index, eth_tx_global.dma_cur_index);

#define MODIFY_ETH_TX_T0_S2S \
    modify_field(eth_tx_t0_s2s_scratch.num_desc, eth_tx_t0_s2s.num_desc); \
    modify_field(eth_tx_t0_s2s_scratch.sg_desc_addr, eth_tx_t0_s2s.sg_desc_addr); \
    modify_field(eth_tx_t0_s2s_scratch.sg_in_progress, eth_tx_t0_s2s.sg_in_progress);

#define MODIFY_ETH_TX_T1_S2S \
    modify_field(eth_tx_t1_s2s_scratch.cq_desc_addr, eth_tx_t1_s2s.cq_desc_addr); \
    modify_field(eth_tx_t1_s2s_scratch.intr_assert_addr, eth_tx_t1_s2s.intr_assert_addr); \
    modify_field(eth_tx_t1_s2s_scratch.intr_assert_data, eth_tx_t1_s2s.intr_assert_data);

#define MODIFY_ETH_TX_TO_S2 \
    MODIFY_TX_DESC_KEY(to_s2, 0)

#define PARAM_TX_DESC(n) \
    addr_lo##n, rsvd##n, addr_hi##n, opcode##n, num_sg_elems##n, \
    len##n, vlan_tci##n, \
    hdr_len_lo##n, \
    csum##n, cq_entry##n, vlan_insert##n, rsvd2##n, hdr_len_hi##n, \
    mss_or_csumoff_lo##n, rsvd3_or_rsvd4##n, mss_or_csumoff_hi##n

#define MODIFY_TX_DESC(n) \
    modify_field(eth_tx_desc.addr_lo##n, addr_lo##n); \
    modify_field(eth_tx_desc.rsvd##n, rsvd##n); \
    modify_field(eth_tx_desc.addr_hi##n, addr_hi##n); \
    modify_field(eth_tx_desc.num_sg_elems##n, num_sg_elems##n); \
    modify_field(eth_tx_desc.opcode##n, opcode##n); \
    modify_field(eth_tx_desc.len##n, len##n); \
    modify_field(eth_tx_desc.vlan_tci##n, vlan_tci##n); \
    modify_field(eth_tx_desc.hdr_len_lo##n, hdr_len_lo##n); \
    modify_field(eth_tx_desc.hdr_len_hi##n, hdr_len_hi##n); \
    modify_field(eth_tx_desc.rsvd2##n, rsvd2##n); \
    modify_field(eth_tx_desc.vlan_insert##n, vlan_insert##n); \
    modify_field(eth_tx_desc.cq_entry##n, cq_entry##n); \
    modify_field(eth_tx_desc.csum##n, csum##n); \
    modify_field(eth_tx_desc.mss_or_csumoff_lo##n, mss_or_csumoff_lo##n); \
    modify_field(eth_tx_desc.rsvd3_or_rsvd4##n, rsvd3_or_rsvd4##n); \
    modify_field(eth_tx_desc.mss_or_csumoff_hi##n, mss_or_csumoff_hi##n);

#define PARAM_TX_SG_ELEM(n) \
    addr_lo##n, rsvd##n, addr_hi##n, rsvd1##n, len##n, rsvd2##n

#define MODIFY_TX_SG_ELEM(n) \
    modify_field(eth_tx_sg_desc.addr_lo##n, addr_lo##n); \
    modify_field(eth_tx_sg_desc.rsvd##n, rsvd##n); \
    modify_field(eth_tx_sg_desc.addr_hi##n, addr_hi##n); \
    modify_field(eth_tx_sg_desc.rsvd1##n, rsvd1##n); \
    modify_field(eth_tx_sg_desc.len##n, len##n); \
    modify_field(eth_tx_sg_desc.rsvd2##n, rsvd2##n);

#define MODIFY_TX_DESC_KEY(hdr, n) \
    modify_field(eth_tx_##hdr##_scratch.addr_lo##n, eth_tx_##hdr.addr_lo##n); \
    modify_field(eth_tx_##hdr##_scratch.rsvd##n, eth_tx_##hdr.rsvd##n); \
    modify_field(eth_tx_##hdr##_scratch.addr_hi##n, eth_tx_##hdr.addr_hi##n); \
    modify_field(eth_tx_##hdr##_scratch.num_sg_elems##n, eth_tx_##hdr.num_sg_elems##n); \
    modify_field(eth_tx_##hdr##_scratch.opcode##n, eth_tx_##hdr.opcode##n); \
    modify_field(eth_tx_##hdr##_scratch.len##n, eth_tx_##hdr.len##n); \
    modify_field(eth_tx_##hdr##_scratch.vlan_tci##n, eth_tx_##hdr.vlan_tci##n); \
    modify_field(eth_tx_##hdr##_scratch.hdr_len_lo##n, eth_tx_##hdr.hdr_len_lo##n); \
    modify_field(eth_tx_##hdr##_scratch.hdr_len_hi##n, eth_tx_##hdr.hdr_len_hi##n); \
    modify_field(eth_tx_##hdr##_scratch.rsvd2##n, eth_tx_##hdr.rsvd2##n); \
    modify_field(eth_tx_##hdr##_scratch.vlan_insert##n, eth_tx_##hdr.vlan_insert##n); \
    modify_field(eth_tx_##hdr##_scratch.cq_entry##n, eth_tx_##hdr.cq_entry##n); \
    modify_field(eth_tx_##hdr##_scratch.csum##n, eth_tx_##hdr.csum##n); \
    modify_field(eth_tx_##hdr##_scratch.mss_or_csumoff_lo##n, eth_tx_##hdr.mss_or_csumoff_lo##n); \
    modify_field(eth_tx_##hdr##_scratch.rsvd3_or_rsvd4##n, eth_tx_##hdr.rsvd3_or_rsvd4##n); \
    modify_field(eth_tx_##hdr##_scratch.mss_or_csumoff_hi##n, eth_tx_##hdr.mss_or_csumoff_hi##n);
