#define BITS_PER_BYTE                   8
#define SIZE_IN_BITS(bytes)             ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)             ((bits) / BITS_PER_BYTE)

/**
 * Scanner Session control block for stage 0
 */
header_type scanner_session_cb_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)

        /*
         * NOTE: cb is programmed by nicmgr and would work best when
         * fields are aligned on whole byte boundary.
         */
        scan_resched_time               : 32;
        normal_tmo_cb_addr              : 64;
        accel_tmo_cb_addr               : 64;
        pad                             : 240;
        cb_activate                     : 16;  // must be last in CB
    }
}

#define SCANNER_SESSION_CB_DATA                                                 \
    rsvd, cosA, cosB, cos_sel, eval_last,                                       \
    host, total, pid, pi_0, ci_0,                                               \
    scan_resched_time, normal_tmo_cb_addr, accel_tmo_cb_addr,                   \
    pad, cb_activate                                                            \
    
#define SCANNER_SESSION_CB_PRAGMA                                               \
@pragma little_endian pi_0 ci_0                                                 \
    scan_resched_time normal_tmo_cb_addr accel_tmo_cb_addr cb_activate          \

#define SCANNER_SESSION_CB_USE(scratch)                                         \
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
    modify_field(scratch.scan_resched_time, scan_resched_time);                 \
    modify_field(scratch.normal_tmo_cb_addr, normal_tmo_cb_addr);               \
    modify_field(scratch.accel_tmo_cb_addr, accel_tmo_cb_addr);                 \
    modify_field(scratch.pad, pad);                                             \
    modify_field(scratch.cb_activate, cb_activate);                             \
    
header_type scanner_session_fsm_t {
    fields {
        fsm_state                       : 8; // must be first in CB
        pad0                            : 8;
        scan_burst_sz_shft              : 16;
        scan_burst_sz                   : 32;
        
        scan_id_base                    : 32;
        scan_id_next                    : 32;
        scan_range_sz                   : 32;
        
        expiry_scan_id_base             : 32;
        expiry_map_entries_scanned      : 32;
        total_entries_scanned           : 32;
        scan_addr_base                  : 64;
        
        pad1                            : 176;
        cb_activate                     : 16;  // must be last in CB
    }
}

#define SCANNER_SESSION_FSM_DATA                                                \
    fsm_state, pad0, scan_burst_sz_shft, scan_burst_sz,                         \
    scan_id_base, scan_id_next, scan_range_sz,                                  \
    expiry_scan_id_base, expiry_map_entries_scanned,                            \
    total_entries_scanned, scan_addr_base,                                      \
    pad1, cb_activate                                                           \
    
#define SCANNER_SESSION_FSM_PRAGMA                                              \
@pragma little_endian scan_burst_sz_shft scan_burst_sz                          \
    scan_id_base scan_id_next scan_range_sz                                     \
    expiry_scan_id_base expiry_map_entries_scanned total_entries_scanned        \
    scan_addr_base cb_activate

#define SCANNER_SESSION_FSM_USE(scratch)                                        \
    modify_field(scratch.fsm_state, fsm_state);                                 \
    modify_field(scratch.pad0, pad0);                                           \
    modify_field(scratch.scan_burst_sz_shft, scan_burst_sz_shft);               \
    modify_field(scratch.scan_burst_sz, scan_burst_sz);                         \
    modify_field(scratch.scan_id_base, scan_id_base);                           \
    modify_field(scratch.scan_id_next, scan_id_next);                           \
    modify_field(scratch.scan_range_sz, scan_range_sz);                         \
    modify_field(scratch.expiry_scan_id_base, expiry_scan_id_base);             \
    modify_field(scratch.expiry_map_entries_scanned, expiry_map_entries_scanned);\
    modify_field(scratch.total_entries_scanned, total_entries_scanned);         \
    modify_field(scratch.scan_addr_base, scan_addr_base);                       \
    modify_field(scratch.pad1, pad1);                                           \
    modify_field(scratch.cb_activate, cb_activate);                             \
    
header_type scanner_session_fsm_state_t {
    fields {
        state                           : 8;
    }
}

header_type scanner_session_summarize_t {
    fields {
        poller_qdepth_shft              : 8;
        pad0                            : 8;
        pad1                            : 16;
        poller_qstate_addr              : 64;
        
        expiry_map0                     : 64;
        expiry_map1                     : 64;
        expiry_map2                     : 64;
        expiry_map3                     : 64;
        
        pad2                            : 144;
        cb_activate                     : 16; // must be last in CB
    }
}

#define SCANNER_SESSION_SUMMARIZE_DATA                                          \
    poller_qdepth_shft, pad0, pad1, poller_qstate_addr,                         \
    expiry_map0, expiry_map1, expiry_map2, expiry_map3,                         \
    pad2, cb_activate                                                           \
    
#define SCANNER_SESSION_SUMMARIZE_PRAGMA                                        \
@pragma little_endian poller_qstate_addr                                        \
    expiry_map0 expiry_map1 expiry_map2 expiry_map3                             \
    cb_activate                                                                 \
    
#define SCANNER_SESSION_SUMMARIZE_USE(scratch)                                  \
    modify_field(scratch.poller_qdepth_shft, poller_qdepth_shft);               \
    modify_field(scratch.pad0, pad0);                                           \
    modify_field(scratch.pad1, pad1);                                           \
    modify_field(scratch.poller_qstate_addr, poller_qstate_addr);               \
    modify_field(scratch.expiry_map0, expiry_map0);                             \
    modify_field(scratch.expiry_map1, expiry_map1);                             \
    modify_field(scratch.expiry_map2, expiry_map2);                             \
    modify_field(scratch.expiry_map3, expiry_map3);                             \
    modify_field(scratch.pad2, pad2);                                           \
    modify_field(scratch.cb_activate, cb_activate);                             \
    
header_type scanner_session_metrics0_t {
    fields {

        // CAUTION: order of fields must match session_kivec9_t
        cb_cfg_discards                 : 64;
        scan_invocations                : 64;
        expired_entries                 : 64;
    }
}

#define SCANNER_SESSION_METRICS0_DATA                                           \
    cb_cfg_discards, scan_invocations, expired_entries
    
#define SCANNER_SESSION_METRICS0_PRAGMA                                         \
@pragma little_endian cb_cfg_discards scan_invocations expired_entries          \
    
#define SCANNER_SESSION_METRICS0_USE(scratch)                                   \
    modify_field(scratch.cb_cfg_discards, cb_cfg_discards);                     \
    modify_field(scratch.scan_invocations, scan_invocations);                   \
    modify_field(scratch.expired_entries, expired_entries);                     \
    
/*
 * Aging timeout values
 */
header_type age_tmo_cb_t {
    fields {
        cb_activate                     : 16;
        cb_select                       : 8;
        pad0                            : 8;
        tcp_syn_tmo                     : 32;
        tcp_est_tmo                     : 32;
        tcp_fin_tmo                     : 32;
        tcp_timewait_tmo                : 32;
        tcp_rst_tmo                     : 32;
        icmp_tmo                        : 32;
        udp_tmo                         : 32;
        udp_est_tmo                     : 32;
        other_tmo                       : 32;
        pad1                            : 192;
    }
}

/*
 * Note: timeout values must be defined as big endian due to loading
 * into PHV with bit truncation (as endian swap will not work correctly
 * when truncation is involved)..
 */
#define SCANNER_AGE_TMO_CB_PRAGMA                                               \
@pragma little_endian cb_activate                                               \

#define SCANNER_AGE_TMO_CB_DATA                                                 \
    cb_activate, cb_select, pad0,                                               \
    tcp_syn_tmo, tcp_est_tmo, tcp_fin_tmo,                                      \
    tcp_timewait_tmo, tcp_rst_tmo, icmp_tmo,                                    \
    udp_tmo, udp_est_tmo, other_tmo, pad1                                       \
    
#define SCANNER_AGE_TMO_CB_USE(scratch)                                         \
    modify_field(scratch.cb_activate, cb_activate);                             \
    modify_field(scratch.cb_select, cb_select);                                 \
    modify_field(scratch.pad0, pad0);                                           \
    modify_field(scratch.tcp_syn_tmo, tcp_syn_tmo);                             \
    modify_field(scratch.tcp_est_tmo, tcp_est_tmo);                             \
    modify_field(scratch.tcp_fin_tmo, tcp_fin_tmo);                             \
    modify_field(scratch.tcp_timewait_tmo, tcp_timewait_tmo);                   \
    modify_field(scratch.tcp_rst_tmo, tcp_rst_tmo);                             \
    modify_field(scratch.icmp_tmo, icmp_tmo);                                   \
    modify_field(scratch.udp_tmo, udp_tmo);                                     \
    modify_field(scratch.udp_est_tmo, udp_est_tmo);                             \
    modify_field(scratch.other_tmo, other_tmo);                                 \
    modify_field(scratch.pad1, pad1);                                           \
    
/*
 * Poller qstate (control block)
 * Note that there are no RxDMA/TxDMA programs for software pollers
 * (i.e., no stage0 program) so poller_cb_t must be a full definition
 * which includes the pc_offset field.
 */
header_type poller_cb_t {
    fields {
        pc_offset                       : 8;
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)
        pi_0_shadow                     : 16;
        qdepth_shft                     : 8;
        wring_base_addr                 : 64;
        num_qfulls                      : 64;
    }
}

#define POLLER_CB_PRAGMA                                                        \
@pragma little_endian pi_0 ci_0 pi_0_shadow wring_base_addr num_qfulls          \

#define POLLER_CB_DATA                                                          \
    pc_offset, rsvd, cosA, cosB, cos_sel, eval_last,                            \
    host, total, pid, pi_0, ci_0, pi_0_shadow,                                  \
    qdepth_shft, wring_base_addr, num_qfulls                                    \
    
#define POLLER_CB_USE(scratch)                                                  \
    modify_field(scratch.pc_offset, pc_offset);                                 \
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
    modify_field(scratch.wring_base_addr, wring_base_addr);                     \
    modify_field(scratch.num_qfulls, num_qfulls);                               \
    
/*
 * Data posted to a poller queue entry
 */
header_type poller_slot_data_t {
    fields {
        table_id_base                   : 32;
        scanner_qid                     : 32;
        expiry_map0                     : 64;
        expiry_map1                     : 64;
        expiry_map2                     : 64;
        expiry_map3                     : 64;
        scanner_qtype                   : 8;
        flags                           : 8;
    }
}

#define POLLER_SLOT_DATA_DATA                                                   \
    table_id_base, scanner_qid,                                                 \
    expiry_map0, expiry_map1, expiry_map2, expiry_map3,                         \
    scanner_qtype, flags, pad0                                                  \

    
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

