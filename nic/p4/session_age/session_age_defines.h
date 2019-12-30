#define BITS_PER_BYTE                   8
#define SIZE_IN_BITS(bytes)             ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)             ((bits) / BITS_PER_BYTE)


/*
 * Poller qstate (control block)
 */
header_type poller_cb_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)
        pi_0_shadow                     : 16;
        qdepth_shft                     : 8;
        qbase_addr                      : 64;
        num_qfulls                      : 64;
    }
}

#define POLLER_CB_PRAGMA                                                        \
@pragma little_endian pi_0 ci_0 pi_0_shadow qbase_addr num_qfulls               \

#define POLLER_CB_DATA                                                          \
    rsvd, cosA, cosB, cos_sel, eval_last,                                       \
    host, total, pid, pi_0, ci_0, pi_0_shadow,                                  \
    qdepth_shft, qbase_addr, num_qfulls                                         \
    
#define POLLER_CB_USE(scratch)                                                  \
    modify_field(scratch.rsvd, rsvd);                                           \
    modify_field(scratch.cosA, cosA);                                           \
    modify_field(scratch.cosB, cosB);                                           \
    modify_field(scratch.cos_sel, cos_sel);                                     \
    modify_field(scratch.eval_last, eval_last);                                 \
    modify_field(scratch.host, host);                                           \
    modify_field(scratch.total, total);                                         \
    modify_field(scratch.pid, pid);                                             \
    modify_field(scratch.pi_0, pi_0);                                           \
    modify_field(scratch.ci_0, ci_0);                                           \
    modify_field(scratch.pi_0_shadow, pi_0_shadow);                             \
    modify_field(scratch.qdepth_shft, qdepth_shft);                             \
    modify_field(scratch.qbase_addr, qbase_addr);                               \
    modify_field(scratch.num_qfulls, num_qfulls);                               \
    
/*
 * Data posted to a poller queue entry
 */
header_type poller_slot_data_t {
    fields {
        session_id_base                 : 32;
        scanner_qid                     : 32;
        expiry_map0                     : 64;
        expiry_map1                     : 64;
        expiry_map2                     : 64;
        expiry_map3                     : 64;
        flags                           : 8;
    }
}

#define POLLER_SLOT_DATA_DATA                                                   \
    session_id_base, scanner_qid,                                               \
    expiry_map0, expiry_map1, expiry_map2, expiry_map3,                         \
    flags, pad0                                                                 \

    
/*
 * Session info data -
 * TODO: how to get this structure from P4 code
 */
#if defined(ARTEMIS)

header_type session_info_d {
    fields {
        iflow_tcp_state                 : 4;
        iflow_tcp_seq_num               : 32;
        iflow_tcp_ack_num               : 32;
        iflow_tcp_win_sz                : 16;
        iflow_tcp_win_scale             : 4;
        rflow_tcp_state                 : 4;
        rflow_tcp_seq_num               : 32;
        rflow_tcp_ack_num               : 32;
        rflow_tcp_win_sz                : 16;
        rflow_tcp_win_scale             : 4;
        tx_dst_ip                       : 128;
        tx_dst_l4port                   : 16;
        nexthop_idx                     : 20;
        tx_rewrite_flags                : 8;
        rx_rewrite_flags                : 8;
        tx_policer_idx                  : 12;
        rx_policer_idx                  : 12;
        meter_idx                       : 16;
        timestamp                       : 48;
        drop                            : 1;
        valid_flag                      : 1;
        flow_type                       : 3;    /* temporary until provided by P4 */
        pad_to_512b                     : 63;
    }
};

#define SESSION_INFO_DATA                                                       \
    iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,                      \
    iflow_tcp_win_sz, iflow_tcp_win_scale, rflow_tcp_state,                     \
    rflow_tcp_seq_num, rflow_tcp_ack_num, rflow_tcp_win_sz,                     \
    rflow_tcp_win_scale, tx_dst_ip, tx_dst_l4port, nexthop_idx,                 \
    tx_rewrite_flags, rx_rewrite_flags, tx_policer_idx, rx_policer_idx,         \
    meter_idx, timestamp, drop, valid_flag,                                     \
    flow_type, pad_to_512b                                                      \

#define SESSION_INFO_USE(scratch)                                               \
    modify_field(scratch.iflow_tcp_state, iflow_tcp_state);                     \
    modify_field(scratch.iflow_tcp_seq_num, iflow_tcp_seq_num);                 \
    modify_field(scratch.iflow_tcp_ack_num, iflow_tcp_ack_num);                 \
    modify_field(scratch.iflow_tcp_win_sz, iflow_tcp_win_sz);                   \
    modify_field(scratch.iflow_tcp_win_scale, iflow_tcp_win_scale);             \
    modify_field(scratch.rflow_tcp_state, rflow_tcp_state);                     \
    modify_field(scratch.rflow_tcp_seq_num, rflow_tcp_seq_num);                 \
    modify_field(scratch.rflow_tcp_ack_num, rflow_tcp_ack_num);                 \
    modify_field(scratch.rflow_tcp_win_sz, rflow_tcp_win_sz);                   \
    modify_field(scratch.rflow_tcp_win_scale, rflow_tcp_win_scale);             \
    modify_field(scratch.tx_dst_ip, tx_dst_ip);                                 \
    modify_field(scratch.tx_dst_l4port, tx_dst_l4port);                         \
    modify_field(scratch.nexthop_idx, nexthop_idx);                             \
    modify_field(scratch.tx_rewrite_flags, tx_rewrite_flags);                   \
    modify_field(scratch.rx_rewrite_flags, rx_rewrite_flags);                   \
    modify_field(scratch.tx_policer_idx, tx_policer_idx);                       \
    modify_field(scratch.rx_policer_idx, rx_policer_idx);                       \
    modify_field(scratch.meter_idx, meter_idx);                                 \
    modify_field(scratch.timestamp, timestamp);                                 \
    modify_field(scratch.drop, drop);                                           \
    modify_field(scratch.valid_flag, valid_flag);                               \
    modify_field(scratch.flow_type, flow_type);                                 \
    modify_field(scratch.pad_to_512b, pad_to_512b);                             \

#else //if defined(ATHENA)

header_type session_info_d {
    fields {
        valid_flag                      : 1;
        timestamp                       : 48;
        config1_epoch                   : 32;
        config1_idx                     : 16;
        config2_epoch                   : 32;
        config2_idx                     : 16;
        config_substrate_src_ip         : 32;
        throttle_pps                    : 16;
        throttle_bw                     : 16;
        counterset1                     : 24;
        counterset2                     : 24;
        histogram                       : 24;
        pop_hdr_flag                    : 1;
        flow_type                       : 3;    /* temporary until provided by P4 */
        pad_to_512b                     : 227;
    }
}

#define SESSION_INFO_DATA                                                       \
    valid_flag, timestamp,                                                      \
    config1_epoch, config1_idx, config2_epoch, config2_idx,                     \
    config_substrate_src_ip, throttle_pps, throttle_bw,                         \
    counterset1, counterset2, histogram, pop_hdr_flag,                          \
    flow_type, pad_to_512b                                                      \

#define SESSION_INFO_USE(scratch)                                               \
    modify_field(scratch.valid_flag, valid_flag);                               \
    modify_field(scratch.timestamp, timestamp);                                 \
    modify_field(scratch.config1_epoch, config1_epoch);                         \
    modify_field(scratch.config1_idx, config1_idx);                             \
    modify_field(scratch.config2_epoch, config2_epoch);                         \
    modify_field(scratch.config2_idx, config2_idx);                             \
    modify_field(scratch.config_substrate_src_ip, config_substrate_src_ip);     \
    modify_field(scratch.throttle_pps, throttle_pps);                           \
    modify_field(scratch.throttle_bw, throttle_bw);                             \
    modify_field(scratch.counterset1, counterset1);                             \
    modify_field(scratch.counterset2, counterset2);                             \
    modify_field(scratch.histogram, histogram);                                 \
    modify_field(scratch.pop_hdr_flag, pop_hdr_flag);                           \
    modify_field(scratch.flow_type, flow_type);                                 \
    modify_field(scratch.pad_to_512b, pad_to_512b);                             \

#endif

/*
 * Common ring producer index
 */
header_type ring_pi_t {
    fields {
        pi                              : 16;
    }
}

