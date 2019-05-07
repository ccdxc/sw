
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
    modify_field(QSTATE.c_index0, c_index0);

#define PARAM_SG_ELEM(n) \
    addr##n, len##n, rsvd##n

#define MODIFY_SG_ELEM(n) \
    modify_field(eth_sg_desc.addr##n, addr##n); \
    modify_field(eth_sg_desc.len##n, len##n); \
    modify_field(eth_sg_desc.rsvd##n, rsvd##n);

/*
 * RXDMA
 */

#define MODIFY_ETH_RX_GLOBAL \
    modify_field(eth_rx_global_scratch.dma_cur_index, eth_rx_global.dma_cur_index); \
    modify_field(eth_rx_global_scratch.qstate_addr, eth_rx_global.qstate_addr); \
    modify_field(eth_rx_global_scratch.host_queue, eth_rx_global.host_queue); \
    modify_field(eth_rx_global_scratch.cpu_queue, eth_rx_global.cpu_queue); \
    modify_field(eth_rx_global_scratch.intr_enable, eth_rx_global.intr_enable); \
    modify_field(eth_rx_global_scratch.lif, eth_rx_global.lif); \
    modify_field(eth_rx_global_scratch.stats, eth_rx_global.stats); \
    modify_field(eth_rx_global_scratch.l2_pkt_type, eth_rx_global.l2_pkt_type); \
    modify_field(eth_rx_global_scratch.pkt_type, eth_rx_global.pkt_type); \
    modify_field(eth_rx_global_scratch.pkt_len, eth_rx_global.pkt_len); \
    modify_field(eth_rx_global_scratch.drop, eth_rx_global.drop);

#define MODIFY_ETH_RX_T0_S2S \
    modify_field(eth_rx_t0_s2s_scratch.cq_desc_addr, eth_rx_t0_s2s.cq_desc_addr); \
    modify_field(eth_rx_t0_s2s_scratch.intr_assert_index, eth_rx_t0_s2s.intr_assert_index); \
    modify_field(eth_rx_t0_s2s_scratch.intr_assert_data, eth_rx_t0_s2s.intr_assert_data); \

#define MODIFY_ETH_RX_T1_S2S \
    modify_field(eth_rx_t1_s2s_scratch.sg_desc_addr, eth_rx_t1_s2s.sg_desc_addr); \
    modify_field(eth_rx_t1_s2s_scratch.rem_sg_elems, eth_rx_t1_s2s.rem_sg_elems); \
    modify_field(eth_rx_t1_s2s_scratch.rem_pkt_bytes, eth_rx_t1_s2s.rem_pkt_bytes);

#define MODIFY_ETH_RX_TO_S3 \
    modify_field(eth_rx_to_s3_scratch.sg_desc_addr, eth_rx_to_s3.sg_desc_addr);

#define PARAMS_ETH_RX_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, \
    p_index0, c_index0, comp_index, \
    color, rsvd1, \
    enable, host_queue, cpu_queue, intr_enable, rsvd2, \
    ring_base, ring_size, cq_ring_base, intr_assert_index, \
    sg_ring_base

#define MODIFY_ETH_RX_QSTATE \
    MODIFY_QSTATE_INTRINSIC(eth_rx_qstate) \
    modify_field(eth_rx_qstate.comp_index, comp_index); \
    modify_field(eth_rx_qstate.color, color); \
    modify_field(eth_rx_qstate.rsvd1, rsvd1); \
    modify_field(eth_rx_qstate.enable, enable); \
    modify_field(eth_rx_qstate.host_queue, host_queue); \
    modify_field(eth_rx_qstate.cpu_queue, cpu_queue); \
    modify_field(eth_rx_qstate.intr_enable, intr_enable); \
    modify_field(eth_rx_qstate.rsvd2, rsvd2); \
    modify_field(eth_rx_qstate.ring_base, ring_base); \
    modify_field(eth_rx_qstate.ring_size, ring_size); \
    modify_field(eth_rx_qstate.cq_ring_base, cq_ring_base); \
    modify_field(eth_rx_qstate.intr_assert_index, intr_assert_index); \
    modify_field(eth_rx_qstate.sg_ring_base, sg_ring_base);

#define PARAM_RX_DESC(n) \
    opcode##n, rsvd##n, len##n, addr##n

#define MODIFY_RX_DESC(n) \
    modify_field(eth_rx_desc.opcode##n, opcode##n); \
    modify_field(eth_rx_desc.rsvd##n, rsvd##n); \
    modify_field(eth_rx_desc.len##n, len##n); \
    modify_field(eth_rx_desc.addr##n, addr##n); \

/*
 * TXDMA
 */

#define MODIFY_ETH_TX_GLOBAL \
    modify_field(eth_tx_global_scratch.dma_cur_index, eth_tx_global.dma_cur_index); \
    modify_field(eth_tx_global_scratch.sg_desc_addr, eth_tx_global.sg_desc_addr); \
    modify_field(eth_tx_global_scratch.sg_in_progress, eth_tx_global.sg_in_progress); \
    modify_field(eth_tx_global_scratch.num_sg_elems, eth_tx_global.num_sg_elems); \
    modify_field(eth_tx_global_scratch.tso_sot, eth_tx_global.tso_sot); \
    modify_field(eth_tx_global_scratch.tso_eot, eth_tx_global.tso_eot); \
    modify_field(eth_tx_global_scratch.host_queue, eth_tx_global.host_queue); \
    modify_field(eth_tx_global_scratch.cpu_queue, eth_tx_global.cpu_queue); \
    modify_field(eth_tx_global_scratch.cq_entry, eth_tx_global.cq_entry); \
    modify_field(eth_tx_global_scratch.intr_enable, eth_tx_global.intr_enable); \
    modify_field(eth_tx_global_scratch.lif, eth_tx_global.lif); \
    modify_field(eth_tx_global_scratch.stats, eth_tx_global.stats); \
    modify_field(eth_tx_global_scratch.drop, eth_tx_global.drop);

#define MODIFY_ETH_TX_T0_S2S \
    modify_field(eth_tx_t0_s2s_scratch.num_todo, eth_tx_t0_s2s.num_todo); \
    modify_field(eth_tx_t0_s2s_scratch.num_desc, eth_tx_t0_s2s.num_desc); \
    modify_field(eth_tx_t0_s2s_scratch.do_sg, eth_tx_t0_s2s.do_sg); \
    modify_field(eth_tx_t0_s2s_scratch.do_tso, eth_tx_t0_s2s.do_tso); \
    modify_field(eth_tx_t0_s2s_scratch.__pad, eth_tx_t0_s2s.__pad); \
    modify_field(eth_tx_t0_s2s_scratch.cq_desc_addr, eth_tx_t0_s2s.cq_desc_addr); \
    modify_field(eth_tx_t0_s2s_scratch.intr_assert_index, eth_tx_t0_s2s.intr_assert_index); \
    modify_field(eth_tx_t0_s2s_scratch.intr_assert_data, eth_tx_t0_s2s.intr_assert_data);

#define MODIFY_ETH_TX_T1_S2S

#define MODIFY_ETH_TX_T2_S2S \
    modify_field(eth_tx_t2_s2s_scratch.tso_hdr_addr, eth_tx_t2_s2s.tso_hdr_addr); \
    modify_field(eth_tx_t2_s2s_scratch.tso_hdr_len, eth_tx_t2_s2s.tso_hdr_len); \
    modify_field(eth_tx_t2_s2s_scratch.tso_ipid_delta, eth_tx_t2_s2s.tso_ipid_delta); \
    modify_field(eth_tx_t2_s2s_scratch.tso_seq_delta, eth_tx_t2_s2s.tso_seq_delta);

#define MODIFY_ETH_TX_TO_S1 \
    modify_field(eth_tx_to_s1_scratch.qstate_addr, eth_tx_to_s1.qstate_addr);

#define MODIFY_ETH_TX_TO_S2 \
    modify_field(eth_tx_to_s2_scratch.qtype, eth_tx_to_s2.qtype); \
    modify_field(eth_tx_to_s2_scratch.qid, eth_tx_to_s2.qid); \
    modify_field(eth_tx_to_s2_scratch.my_ci, eth_tx_to_s2.my_ci); \
    modify_field(eth_tx_to_s2_scratch.tso_hdr_addr, eth_tx_to_s2.tso_hdr_addr); \
    modify_field(eth_tx_to_s2_scratch.tso_hdr_len, eth_tx_to_s2.tso_hdr_len);

#define MODIFY_ETH_TX_TO_S3 \
    MODIFY_TX_DESC_KEY(to_s3, 0)

#define PARAMS_ETH_TX_QSTATE \
        pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, \
        p_index0, c_index0, comp_index, ci_fetch, ci_miss, \
        color, spec_miss, rsvd1, \
        enable, host_queue, cpu_queue, intr_enable, rsvd2, \
        ring_base, ring_size, cq_ring_base, intr_assert_index, \
        sg_ring_base, \
        tso_hdr_addr, tso_hdr_len, tso_ipid_delta, tso_seq_delta, \
        spurious_db_cnt

#define MODIFY_ETH_TX_QSTATE \
    MODIFY_QSTATE_INTRINSIC(eth_tx_qstate) \
    modify_field(eth_tx_qstate.comp_index, comp_index); \
    modify_field(eth_tx_qstate.ci_fetch, ci_fetch); \
    modify_field(eth_tx_qstate.ci_miss, ci_miss); \
    modify_field(eth_tx_qstate.color, color); \
    modify_field(eth_tx_qstate.spec_miss, spec_miss); \
    modify_field(eth_tx_qstate.rsvd1, rsvd1); \
    modify_field(eth_tx_qstate.enable, enable); \
    modify_field(eth_tx_qstate.host_queue, host_queue); \
    modify_field(eth_tx_qstate.cpu_queue, cpu_queue); \
    modify_field(eth_tx_qstate.intr_enable, intr_enable); \
    modify_field(eth_tx_qstate.rsvd2, rsvd2); \
    modify_field(eth_tx_qstate.ring_base, ring_base); \
    modify_field(eth_tx_qstate.ring_size, ring_size); \
    modify_field(eth_tx_qstate.cq_ring_base, cq_ring_base); \
    modify_field(eth_tx_qstate.intr_assert_index, intr_assert_index); \
    modify_field(eth_tx_qstate.sg_ring_base, sg_ring_base); \
    modify_field(eth_tx_qstate.tso_hdr_addr, tso_hdr_addr); \
    modify_field(eth_tx_qstate.tso_hdr_len, tso_hdr_len); \
    modify_field(eth_tx_qstate.tso_ipid_delta, tso_ipid_delta); \
    modify_field(eth_tx_qstate.tso_seq_delta, tso_seq_delta); \
    modify_field(eth_tx_qstate.spurious_db_cnt, spurious_db_cnt); \

#define PARAM_TX_DESC(n) \
    opcode##n, \
    csum_l4_or_eot##n, csum_l3_or_sot##n, encap##n, vlan_insert##n,\
    addr_hi##n, num_sg_elems##n, addr_lo##n, \
    len##n, vlan_tci##n, csum_start_or_hdr_len##n, csum_offset_or_mss##n

#define MODIFY_TX_DESC(n) \
    modify_field(eth_tx_desc.opcode##n, opcode##n); \
    modify_field(eth_tx_desc.csum_l4_or_eot##n, csum_l4_or_eot##n); \
    modify_field(eth_tx_desc.csum_l3_or_sot##n, csum_l3_or_sot##n); \
    modify_field(eth_tx_desc.encap##n, encap##n); \
    modify_field(eth_tx_desc.vlan_insert##n, vlan_insert##n); \
    modify_field(eth_tx_desc.addr_hi##n, addr_hi##n); \
    modify_field(eth_tx_desc.num_sg_elems##n, num_sg_elems##n); \
    modify_field(eth_tx_desc.addr_lo##n, addr_lo##n); \
    modify_field(eth_tx_desc.len##n, len##n); \
    modify_field(eth_tx_desc.vlan_tci##n, vlan_tci##n); \
    modify_field(eth_tx_desc.csum_start_or_hdr_len##n, csum_start_or_hdr_len##n); \
    modify_field(eth_tx_desc.csum_offset_or_mss##n, csum_offset_or_mss##n);

#define MODIFY_TX_DESC_KEY(hdr, n) \
    modify_field(eth_tx_##hdr##_scratch.opcode##n, eth_tx_##hdr.opcode##n); \
    modify_field(eth_tx_##hdr##_scratch.csum_l4_or_eot##n, eth_tx_##hdr.csum_l4_or_eot##n); \
    modify_field(eth_tx_##hdr##_scratch.csum_l3_or_sot##n, eth_tx_##hdr.csum_l3_or_sot##n); \
    modify_field(eth_tx_##hdr##_scratch.encap##n, eth_tx_##hdr.encap##n); \
    modify_field(eth_tx_##hdr##_scratch.vlan_insert##n, eth_tx_##hdr.vlan_insert##n); \
    modify_field(eth_tx_##hdr##_scratch.addr_hi##n, eth_tx_##hdr.addr_hi##n); \
    modify_field(eth_tx_##hdr##_scratch.num_sg_elems##n, eth_tx_##hdr.num_sg_elems##n); \
    modify_field(eth_tx_##hdr##_scratch.addr_lo##n, eth_tx_##hdr.addr_lo##n); \
    modify_field(eth_tx_##hdr##_scratch.len##n, eth_tx_##hdr.len##n); \
    modify_field(eth_tx_##hdr##_scratch.vlan_tci##n, eth_tx_##hdr.vlan_tci##n); \
    modify_field(eth_tx_##hdr##_scratch.csum_start_or_hdr_len##n, eth_tx_##hdr.csum_start_or_hdr_len##n); \
    modify_field(eth_tx_##hdr##_scratch.csum_offset_or_mss##n, eth_tx_##hdr.csum_offset_or_mss##n);
