
/*
 * Qstate Intrinsic
 */

#define FIELDS_QSTATE_INTRINSIC \
    pc : 8; \
    rsvd : 8; \
    cosA : 4; \
    cosB : 4; \
    cos_sel : 8; \
    eval_last : 8; \
    host : 4; \
    total : 4; \
    pid : 16;

#define PARAMS_QSTATE_INTRINSIC_PC \
    pc

#define PARAMS_QSTATE_INTRINSIC_NOPC \
    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid

#define PARAMS_QSTATE_INTRINSIC \
    PARAMS_QSTATE_INTRINSIC_PC, \
    PARAMS_QSTATE_INTRINSIC_NOPC

#define MODIFY_QSTATE_INTRINSIC_PC(qstate) \
    modify_field(qstate.pc, pc);

#define MODIFY_QSTATE_INTRINSIC_NOPC(qstate) \
    modify_field(qstate.rsvd, rsvd); \
    modify_field(qstate.cosA, cosA); \
    modify_field(qstate.cosB, cosB); \
    modify_field(qstate.cos_sel, cos_sel); \
    modify_field(qstate.eval_last, eval_last); \
    modify_field(qstate.host, host); \
    modify_field(qstate.total, total);  \
    modify_field(qstate.pid, pid);

#define MODIFY_QSTATE_INTRINSIC(qstate) \
    MODIFY_QSTATE_INTRINSIC_PC(qstate) \
    MODIFY_QSTATE_INTRINSIC_NOPC(qstate)

/*
 * Qstate Ring Intrinsic
 */

#define FIELDS_QSTATE_RING(n) \
    p_index##n : 16; \
    c_index##n : 16;

#define PARAMS_QSTATE_RING(n) \
    p_index##n, c_index##n

#define MODIFY_QSTATE_RING(qstate, n) \
    modify_field(qstate.p_index##n, p_index##n); \
    modify_field(qstate.c_index##n, c_index##n);

/*
 * RX/TXDMA common qstate fields
 *
 * The common fields MUST be exactly the same in eth_tx/eth_rx_qstate_d!
 *   Arming rx from txdma views the rx qstate using the tx type.
 */

#define FIELDS_ETH_TXRX_QSTATE_CFG \
    enable : 1; \
    debug : 1; \
    host_queue : 1; \
    cpu_queue : 1; \
    eq_enable : 1; \
    intr_enable : 1; \
    rsvd_cfg : 2;

#define PARAMS_ETH_TXRX_QSTATE_CFG \
    /* cfg */ enable, debug, host_queue, cpu_queue, eq_enable, intr_enable, rsvd_cfg

#define MODIFY_ETH_TXRX_QSTATE_CFG(qstate) \
    modify_field(qstate.enable, enable); \
    modify_field(qstate.debug, debug); \
    modify_field(qstate.host_queue, host_queue); \
    modify_field(qstate.cpu_queue, cpu_queue); \
    modify_field(qstate.eq_enable, eq_enable); \
    modify_field(qstate.intr_enable, intr_enable); \
    modify_field(qstate.rsvd_cfg, rsvd_cfg);

/* 192 bits (3 dwords, common size is dword aligned) */
#define FIELDS_ETH_TXRX_QSTATE_COMMON \
    FIELDS_QSTATE_INTRINSIC \
    FIELDS_QSTATE_RING(0) \
    FIELDS_QSTATE_RING(1) \
    FIELDS_QSTATE_RING(2) \
    FIELDS_ETH_TXRX_QSTATE_CFG \
    rsvd_db_cnt : 3; \
    ring_size : 5; \
    lif_index : 16;

#define PARAMS_ETH_TXRX_QSTATE_COMMON_NOPC \
    PARAMS_QSTATE_INTRINSIC_NOPC, \
    PARAMS_QSTATE_RING(0), \
    PARAMS_QSTATE_RING(1), \
    PARAMS_QSTATE_RING(2), \
    PARAMS_ETH_TXRX_QSTATE_CFG, \
    rsvd_db_cnt, ring_size, lif_index

#define MODIFY_ETH_TXRX_QSTATE_COMMON_NOPC(qstate) \
    MODIFY_QSTATE_INTRINSIC_NOPC(qstate) \
    MODIFY_QSTATE_RING(qstate, 0) \
    MODIFY_QSTATE_RING(qstate, 1) \
    MODIFY_QSTATE_RING(qstate, 2) \
    MODIFY_ETH_TXRX_QSTATE_CFG(qstate) \
    modify_field(qstate.rsvd_db_cnt, rsvd_db_cnt); \
    modify_field(qstate.ring_size, ring_size); \
    modify_field(qstate.lif_index, lif_index);

/*
 * RX/TXDMA SG
 */

#define PARAM_SG_ELEM(n) \
    addr##n, len##n, rsvd##n

#define MODIFY_SG_ELEM(n) \
    modify_field(eth_sg_desc.addr##n, addr##n); \
    modify_field(eth_sg_desc.len##n, len##n); \
    modify_field(eth_sg_desc.rsvd##n, rsvd##n);

/*
 * RX/TXDMA EQ
 */

#define PARAMS_ETH_EQ_QSTATE \
    eq_ring_base, eq_ring_size, eq_enable, intr_enable, rsvd_cfg, \
    eq_index, eq_gen, rsvd, intr_index

#define MODIFY_ETH_EQ_QSTATE \
    modify_field(eth_eq_qstate.eq_ring_base, eq_ring_base); \
    modify_field(eth_eq_qstate.eq_ring_size, eq_ring_size); \
    modify_field(eth_eq_qstate.eq_enable, eq_enable); \
    modify_field(eth_eq_qstate.intr_enable, intr_enable); \
    modify_field(eth_eq_qstate.rsvd_cfg, rsvd_cfg); \
    modify_field(eth_eq_qstate.eq_index, eq_index); \
    modify_field(eth_eq_qstate.eq_gen, eq_gen); \
    modify_field(eth_eq_qstate.rsvd, rsvd); \
    modify_field(eth_eq_qstate.intr_index, intr_index);

/*
 * RXDMA
 */

#define PARAMS_ETH_RX_QSTATE_NOPC \
    PARAMS_ETH_TXRX_QSTATE_COMMON_NOPC, \
    comp_index, \
    /* sta */ color, armed, rsvd_sta, \
    lg2_desc_sz, lg2_cq_desc_sz, lg2_sg_desc_sz, sg_max_elems, \
    __pad256, ring_base, cq_ring_base, sg_ring_base, intr_index_or_eq_addr

#define PARAMS_ETH_RX_QSTATE \
    PARAMS_QSTATE_INTRINSIC_PC, \
    PARAMS_ETH_RX_QSTATE_NOPC

#define MODIFY_ETH_RX_QSTATE_NOPC \
    MODIFY_ETH_TXRX_QSTATE_COMMON_NOPC(eth_rx_qstate) \
    modify_field(eth_rx_qstate.comp_index, comp_index); \
    modify_field(eth_rx_qstate.color, color); \
    modify_field(eth_rx_qstate.armed, armed); \
    modify_field(eth_rx_qstate.rsvd_sta, rsvd_sta); \
    modify_field(eth_rx_qstate.lg2_desc_sz, lg2_desc_sz); \
    modify_field(eth_rx_qstate.lg2_cq_desc_sz, lg2_cq_desc_sz); \
    modify_field(eth_rx_qstate.lg2_sg_desc_sz, lg2_sg_desc_sz); \
    modify_field(eth_rx_qstate.sg_max_elems, sg_max_elems); \
    modify_field(eth_rx_qstate.__pad256, __pad256); \
    modify_field(eth_rx_qstate.ring_base, ring_base); \
    modify_field(eth_rx_qstate.cq_ring_base, cq_ring_base); \
    modify_field(eth_rx_qstate.sg_ring_base, sg_ring_base); \
    modify_field(eth_rx_qstate.intr_index_or_eq_addr, intr_index_or_eq_addr);

#define MODIFY_ETH_RX_QSTATE \
    MODIFY_QSTATE_INTRINSIC_PC(eth_rx_qstate) \
    MODIFY_ETH_RX_QSTATE_NOPC

#define MODIFY_ETH_RX_GLOBAL \
    modify_field(eth_rx_global_scratch.dma_cur_index, eth_rx_global.dma_cur_index); \
    modify_field(eth_rx_global_scratch.sg_desc_addr, eth_rx_global.sg_desc_addr); \
    modify_field(eth_rx_global_scratch.host_queue, eth_rx_global.host_queue); \
    modify_field(eth_rx_global_scratch.cpu_queue, eth_rx_global.cpu_queue); \
    modify_field(eth_rx_global_scratch.do_eq, eth_rx_global.do_eq); \
    modify_field(eth_rx_global_scratch.do_intr, eth_rx_global.do_intr); \
    modify_field(eth_rx_global_scratch.lif, eth_rx_global.lif); \
    modify_field(eth_rx_global_scratch.stats, eth_rx_global.stats); \
    modify_field(eth_rx_global_scratch.drop, eth_rx_global.drop);

#define MODIFY_ETH_RX_T0_S2S \
    modify_field(eth_rx_t0_s2s_scratch.cq_desc_addr, eth_rx_t0_s2s.cq_desc_addr); \
    modify_field(eth_rx_t0_s2s_scratch.eq_desc_addr, eth_rx_t0_s2s.eq_desc_addr); \
    modify_field(eth_rx_t0_s2s_scratch.intr_index, eth_rx_t0_s2s.intr_index); \
    modify_field(eth_rx_t0_s2s_scratch.pkt_len, eth_rx_t0_s2s.pkt_len);

#define MODIFY_ETH_RX_T1_S2S \
    modify_field(eth_rx_t1_s2s_scratch.l2_pkt_type, eth_rx_t1_s2s.l2_pkt_type); \
    modify_field(eth_rx_t1_s2s_scratch.pkt_type, eth_rx_t1_s2s.pkt_type); \
    modify_field(eth_rx_t1_s2s_scratch.pkt_len, eth_rx_t1_s2s.pkt_len); \
    modify_field(eth_rx_t1_s2s_scratch.rem_sg_elems, eth_rx_t1_s2s.rem_sg_elems); \
    modify_field(eth_rx_t1_s2s_scratch.rem_pkt_bytes, eth_rx_t1_s2s.rem_pkt_bytes); \
    modify_field(eth_rx_t1_s2s_scratch.sg_max_elems, eth_rx_t1_s2s_scratch.sg_max_elems);

#define MODIFY_ETH_RX_TO_S1 \
    modify_field(eth_rx_to_s1_scratch.qstate_addr, eth_rx_to_s1.qstate_addr); \

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

#define PARAMS_ETH_TX_QSTATE_NOPC \
    PARAMS_ETH_TXRX_QSTATE_COMMON_NOPC, \
    comp_index, \
    /* sta */ color, armed, rsvd_sta, \
    lg2_desc_sz, lg2_cq_desc_sz, lg2_sg_desc_sz, \
    __pad256, ring_base, cq_ring_base, sg_ring_base, intr_index_or_eq_addr

#define PARAMS_ETH_TX_QSTATE \
    PARAMS_QSTATE_INTRINSIC_PC, \
    PARAMS_ETH_TX_QSTATE_NOPC

#define MODIFY_ETH_TX_QSTATE_NOPC \
    MODIFY_ETH_TXRX_QSTATE_COMMON_NOPC(eth_tx_qstate) \
    modify_field(eth_tx_qstate.comp_index, comp_index); \
    modify_field(eth_tx_qstate.color, color); \
    modify_field(eth_tx_qstate.armed, armed); \
    modify_field(eth_tx_qstate.rsvd_sta, rsvd_sta); \
    modify_field(eth_tx_qstate.lg2_desc_sz, lg2_desc_sz); \
    modify_field(eth_tx_qstate.lg2_cq_desc_sz, lg2_cq_desc_sz); \
    modify_field(eth_tx_qstate.lg2_sg_desc_sz, lg2_sg_desc_sz); \
    modify_field(eth_tx_qstate.__pad256, __pad256); \
    modify_field(eth_tx_qstate.ring_base, ring_base); \
    modify_field(eth_tx_qstate.cq_ring_base, cq_ring_base); \
    modify_field(eth_tx_qstate.sg_ring_base, sg_ring_base); \
    modify_field(eth_tx_qstate.intr_index_or_eq_addr, intr_index_or_eq_addr);

#define MODIFY_ETH_TX_QSTATE \
    MODIFY_QSTATE_INTRINSIC_PC(eth_tx_qstate) \
    MODIFY_ETH_TX_QSTATE_NOPC

#define PARAMS_ETH_TX2_QSTATE \
    tso_hdr_addr, tso_hdr_len, tso_hdr_rsvd, \
    tso_ipid_delta, tso_seq_delta, tso_rsvd

#define MODIFY_ETH_TX2_QSTATE \
    modify_field(eth_tx2_qstate.tso_hdr_addr, tso_hdr_addr); \
    modify_field(eth_tx2_qstate.tso_hdr_len, tso_hdr_len); \
    modify_field(eth_tx2_qstate.tso_hdr_rsvd, tso_hdr_rsvd); \
    modify_field(eth_tx2_qstate.tso_ipid_delta, tso_ipid_delta); \
    modify_field(eth_tx2_qstate.tso_seq_delta, tso_seq_delta); \
    modify_field(eth_tx2_qstate.tso_rsvd, tso_rsvd);

#define MODIFY_ETH_TX_GLOBAL \
    modify_field(eth_tx_global_scratch.dma_cur_index, eth_tx_global.dma_cur_index); \
    modify_field(eth_tx_global_scratch.sg_in_progress, eth_tx_global.sg_in_progress); \
    modify_field(eth_tx_global_scratch.num_sg_elems, eth_tx_global.num_sg_elems); \
    modify_field(eth_tx_global_scratch.tso_sot, eth_tx_global.tso_sot); \
    modify_field(eth_tx_global_scratch.tso_eot, eth_tx_global.tso_eot); \
    modify_field(eth_tx_global_scratch.host_queue, eth_tx_global.host_queue); \
    modify_field(eth_tx_global_scratch.cpu_queue, eth_tx_global.cpu_queue); \
    modify_field(eth_tx_global_scratch.do_cq, eth_tx_global.do_cq); \
    modify_field(eth_tx_global_scratch.do_eq, eth_tx_global.do_eq); \
    modify_field(eth_tx_global_scratch.do_intr, eth_tx_global.do_intr); \
    modify_field(eth_tx_global_scratch.lif, eth_tx_global.lif); \
    modify_field(eth_tx_global_scratch.stats, eth_tx_global.stats); \
    modify_field(eth_tx_global_scratch.drop, eth_tx_global.drop);

#define MODIFY_ETH_TX_T0_S2S \
    modify_field(eth_tx_t0_s2s_scratch.cq_desc_addr, eth_tx_t0_s2s.cq_desc_addr); \
    modify_field(eth_tx_t0_s2s_scratch.eq_desc_addr, eth_tx_t0_s2s.eq_desc_addr); \
    modify_field(eth_tx_t0_s2s_scratch.intr_index, eth_tx_t0_s2s.intr_index); \
    modify_field(eth_tx_t0_s2s_scratch.do_sg, eth_tx_t0_s2s.do_sg); \
    modify_field(eth_tx_t0_s2s_scratch.do_tso, eth_tx_t0_s2s.do_tso);

#define MODIFY_ETH_TX_T1_S2S \
    modify_field(eth_tx_t1_s2s_scratch.tso_hdr_addr, eth_tx_t1_s2s.tso_hdr_addr); \
    modify_field(eth_tx_t1_s2s_scratch.tso_hdr_len, eth_tx_t1_s2s.tso_hdr_len); \
    modify_field(eth_tx_t1_s2s_scratch.tso_hdr_rsvd, eth_tx_t1_s2s.tso_hdr_rsvd); \
    modify_field(eth_tx_t1_s2s_scratch.tso_ipid_delta, eth_tx_t1_s2s.tso_ipid_delta); \
    modify_field(eth_tx_t1_s2s_scratch.tso_seq_delta, eth_tx_t1_s2s.tso_seq_delta);

#define MODIFY_ETH_TX_TO_S1 \
    modify_field(eth_tx_to_s1_scratch.qstate_addr, eth_tx_to_s1.qstate_addr);

#define MODIFY_ETH_TX_TO_S2 \
    modify_field(eth_tx_to_s2_scratch.qid, eth_tx_to_s2.qid); \
    modify_field(eth_tx_to_s2_scratch.qtype, eth_tx_to_s2.qtype); \
    modify_field(eth_tx_to_s2_scratch.pad, eth_tx_to_s2.pad); \
    modify_field(eth_tx_to_s2_scratch.my_ci, eth_tx_to_s2.my_ci); \
    modify_field(eth_tx_to_s2_scratch.tso_hdr_addr, eth_tx_to_s2.tso_hdr_addr); \
    modify_field(eth_tx_to_s2_scratch.tso_hdr_len, eth_tx_to_s2.tso_hdr_len); \
    modify_field(eth_tx_to_s2_scratch.tso_hdr_rsvd, eth_tx_to_s2.tso_hdr_rsvd);

#define MODIFY_ETH_TX_TO_S3 \
    MODIFY_TX_DESC_KEY(to_s3,)

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
