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
        scan_resched_ticks              : 32;
        normal_tmo_cb_addr              : 64;
        accel_tmo_cb_addr               : 64;
        resched_uses_slow_timer         : 8;
        pad                             : 232;
        cb_activate                     : 16;  // must be last in CB
    }
}

#define SCANNER_SESSION_CB_DATA                                                 \
    rsvd, cosA, cosB, cos_sel, eval_last,                                       \
    host, total, pid, pi_0, ci_0,                                               \
    scan_resched_ticks, normal_tmo_cb_addr, accel_tmo_cb_addr,                  \
    resched_uses_slow_timer, pad, cb_activate                                   \
    
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
    modify_field(scratch.scan_resched_ticks, scan_resched_ticks);               \
    modify_field(scratch.normal_tmo_cb_addr, normal_tmo_cb_addr);               \
    modify_field(scratch.accel_tmo_cb_addr, accel_tmo_cb_addr);                 \
    modify_field(scratch.resched_uses_slow_timer, resched_uses_slow_timer);     \
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
        range_start_ts                  : 64;
        expiry_map_bit_pos              : 16;
        
        pad1                            : 96;
        cb_activate                     : 16;  // must be last in CB
    }
}

#define SCANNER_SESSION_FSM_DATA                                                \
    fsm_state, pad0, scan_burst_sz_shft, scan_burst_sz,                         \
    scan_id_base, scan_id_next, scan_range_sz,                                  \
    expiry_scan_id_base, expiry_map_entries_scanned,                            \
    total_entries_scanned, scan_addr_base, range_start_ts,                      \
    expiry_map_bit_pos, pad1, cb_activate                                       \
    
#define SCANNER_SESSION_FSM_PRAGMA                                              \
@pragma little_endian scan_burst_sz_shft scan_burst_sz                          \
    scan_id_base scan_id_next scan_range_sz                                     \
    expiry_scan_id_base expiry_map_entries_scanned total_entries_scanned        \
    scan_addr_base range_start_ts expiry_map_bit_pos cb_activate

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
    modify_field(scratch.range_start_ts, range_start_ts);                       \
    modify_field(scratch.expiry_map_bit_pos, expiry_map_bit_pos);               \
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
        range_has_posted                : 8;
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
    poller_qdepth_shft, range_has_posted, pad1, poller_qstate_addr,             \
    expiry_map0, expiry_map1, expiry_map2, expiry_map3,                         \
    pad2, cb_activate                                                           \
    
#define SCANNER_SESSION_SUMMARIZE_PRAGMA                                        \
@pragma little_endian poller_qstate_addr                                        \
    expiry_map0 expiry_map1 expiry_map2 expiry_map3                             \
    cb_activate                                                                 \
    
#define SCANNER_SESSION_SUMMARIZE_USE(scratch)                                  \
    modify_field(scratch.poller_qdepth_shft, poller_qdepth_shft);               \
    modify_field(scratch.range_has_posted, range_has_posted);                   \
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

        cb_cfg_discards                 : 64;
        scan_invocations                : 64;
        expired_entries                 : 64;
        min_range_elapsed_ticks         : 64;
        max_range_elapsed_ticks         : 64;
    }
}

#define SCANNER_SESSION_METRICS0_DATA                                           \
    cb_cfg_discards, scan_invocations, expired_entries,                         \
    min_range_elapsed_ticks, max_range_elapsed_ticks
    
#define SCANNER_SESSION_METRICS0_PRAGMA                                         \
@pragma little_endian cb_cfg_discards scan_invocations expired_entries          \
    min_range_elapsed_ticks max_range_elapsed_ticks
    
#define SCANNER_SESSION_METRICS0_USE(scratch)                                   \
    modify_field(scratch.cb_cfg_discards, cb_cfg_discards);                     \
    modify_field(scratch.scan_invocations, scan_invocations);                   \
    modify_field(scratch.expired_entries, expired_entries);                     \
    modify_field(scratch.min_range_elapsed_ticks, min_range_elapsed_ticks);     \
    modify_field(scratch.max_range_elapsed_ticks, max_range_elapsed_ticks);     \
    
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
        others_tmo                      : 32;
        session_tmo                     : 32;
        force_session_expired_ts        : 8;    // for debugging on SIM platform
        force_conntrack_expired_ts      : 8;
        pad1                            : 144;
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
    tcp_syn_tmo, tcp_est_tmo, tcp_fin_tmo, tcp_timewait_tmo,                    \
    tcp_rst_tmo, icmp_tmo, udp_tmo, udp_est_tmo,                                \
    others_tmo, session_tmo,                                                    \
    force_session_expired_ts, force_conntrack_expired_ts, pad1                  \
    
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
    modify_field(scratch.others_tmo, others_tmo);                               \
    modify_field(scratch.session_tmo, session_tmo);                             \
    modify_field(scratch.force_session_expired_ts, force_session_expired_ts);   \
    modify_field(scratch.force_conntrack_expired_ts, force_conntrack_expired_ts);\
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
#define session_info_d      session_info_common_d

header_type session_info_common_d {
    fields {
        valid_flag                      : 1;
        conntrack_id                    : 22;
        timestamp                       : 16;
        h2s_throttle_pps_id             : 13;
        h2s_throttle_bw_id              : 13;
        h2s_vnic_statistics_id          : 9;
        h2s_vnic_statistics_mask        : 16;
        h2s_vnic_histogram_id           : 9;
        s2h_throttle_pps_id             : 13;
        s2h_throttle_bw_id              : 13;
        s2h_vnic_statistics_id          : 9;
        s2h_vnic_statistics_mask        : 16;
        s2h_vnic_histogram_id           : 9;
        pad_to_512b                     : 353;
    }
}

#define SESSION_INFO_DATA                                                       \
    valid_flag, conntrack_id, timestamp,                                        \
    h2s_throttle_pps_id, h2s_throttle_bw_id, h2s_vnic_statistics_id,            \
    h2s_vnic_statistics_mask, h2s_vnic_histogram_id, s2h_throttle_pps_id,       \
    s2h_throttle_bw_id, s2h_vnic_statistics_id, s2h_vnic_statistics_mask,       \
    s2h_vnic_histogram_id, pad_to_512b                                          \

#define SESSION_INFO_USE(scratch)                                               \
    modify_field(scratch.valid_flag, valid_flag);                               \
    modify_field(scratch.conntrack_id, conntrack_id);                           \
    modify_field(scratch.timestamp, timestamp);                                 \
    modify_field(scratch.h2s_throttle_pps_id, h2s_throttle_pps_id);             \
    modify_field(scratch.h2s_throttle_bw_id, h2s_throttle_bw_id);               \
    modify_field(scratch.h2s_vnic_statistics_id, h2s_vnic_statistics_id);       \
    modify_field(scratch.h2s_vnic_statistics_mask, h2s_vnic_statistics_mask);   \
    modify_field(scratch.h2s_vnic_histogram_id, h2s_vnic_histogram_id);         \
    modify_field(scratch.s2h_throttle_pps_id, s2h_throttle_pps_id);             \
    modify_field(scratch.s2h_throttle_bw_id, s2h_throttle_bw_id);               \
    modify_field(scratch.s2h_vnic_statistics_id, s2h_vnic_statistics_id);       \
    modify_field(scratch.s2h_vnic_statistics_mask, s2h_vnic_statistics_mask);   \
    modify_field(scratch.s2h_vnic_histogram_id, s2h_vnic_histogram_id);         \
    modify_field(scratch.pad_to_512b, pad_to_512b);                             \

header_type conntrack_info_d {
    fields {
        valid_flag                      : 1;
        flow_type                       : 2;
        flow_state                      : 4;
        timestamp                       : 48;
        pad_to_512b                     : 457;
    }
}

#define CONNTRACK_INFO_DATA                                                     \
    valid_flag, flow_type, flow_state, timestamp,                               \
    pad_to_512b                                                                 \

#define CONNTRACK_INFO_USE(scratch)                                             \
    modify_field(scratch.valid_flag, valid_flag);                               \
    modify_field(scratch.flow_type, flow_type);                                 \
    modify_field(scratch.flow_state, flow_state);                               \
    modify_field(scratch.timestamp, timestamp);                                 \
    modify_field(scratch.pad_to_512b, pad_to_512b);                             \

/*
 * Common ring producer index
 */
header_type ring_pi_t {
    fields {
        pi                              : 16;
    }
}

/*
 * Indication whether some non-zero expiry maps have been posted for a range
 */
header_type poller_range_has_posted_t {
    fields {
        posted                          : 8;
    }
}

