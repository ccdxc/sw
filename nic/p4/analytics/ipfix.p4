#include "../iris/include/table_sizes.h"
#include "../iris/include/defines.h"
#include "../include/intrinsic.p4"

#define THRESHOLD 10

header_type ipfix_metadata_t {
    fields {
        skip_record : 1;
        lookup_flow_hash : 1;
        flow_state_index_valid : 1;
        flow_hash_index : 21;
        flow_hash_overflow_index : 14;
        flow_index : 20;
        flow_state_index : 20;
    }
}

header_type nonip_record_t {
    fields {
        mac_sa : 48;
        mac_da : 48;
        ether_type : 16;
    }
}

header_type ipv4_record_t {
    fields {
        ipv4_sa : 32;
        ipv4_da : 32;
    }
}

header_type ipv6_record_t {
    fields {
        ipv6_sa : 128;
        ipv6_da : 128;
    }
}

header_type common_record_t {
    fields {
        vrf : 16;
        flow_index : 32;
        egress_interface : 16;
        egress_interface_type : 8;
        permit_bytes : 64;
        permit_packets : 64;
        drop_bytes : 64;
        drop_packets : 64;
        drop_vector : 32;
        micro_burst_exceed_bytes : 32;
        micro_burst_exceed_count : 32;
        start_timestamp : 32;
        last_seen_timestamp : 32;
    }
}

header_type ip_record_t {
    fields {
        flow_state_index : 32;
        proto : 8;
        role : 8;
        sport : 16;
        dport : 16;
        icmp_type_code : 16;
        tcp_seq_num : 32;
        tcp_ack_num : 32;
        tcp_win_size : 16;
        tcp_mss : 16;
        tcp_win_scale : 8;
        tcp_state : 8;
        tcp_exceptions : 32;
        tcp_rtt : 32;
        ttl : 8;
    }
}

header_type scratch_metadata_t {
    fields {
        lkp_type                       : 4;
        lkp_vrf                        : 16;
        lkp_src                        : 128;
        lkp_dst                        : 128;
        lkp_proto                      : 8;
        lkp_sport                      : 16;
        lkp_dport                      : 16;
        flag                           : 1;
        hash                           : 11;
        hint                           : 14;

        lif                            : LIF_BIT_WIDTH;
        ingress_policer_index          : 11;
        egress_policer_index           : 11;
        mirror_session_id              : 8;
        rewrite_index                  : 12;
        tunnel_rewrite_index           : 10;
        tunnel_vnid                    : 24;
        entropy_hash                   : 16;
        nat_ip                         : 128;
        nat_l4_port                    : 16;
        twice_nat_idx                  : 13;
        cos                            : 3;
        dscp                           : 8;
        flow_mss                       : 16;
        flow_ws                        : 16;
        flow_ts_negotiated             : 1;
        flow_ttl                       : 8;
        flow_role                      : 1;
        flow_index                     : 20;
        flow_state_index               : 20;
        start_timestamp                : 48;

        seq_delta                      : 32 (signed);
        rtt_seq_no                     : 32;
        rtt_timestamp                  : 48;

        last_seen_timestamp            : 48;
        permit_packet                  : 4;
        permit_bytes                   : 18;
        drop_packets                   : 4;
        drop_bytes                     : 18;
        drop_reason                    : 32;
        burst_start_timestamp          : 48;
        burst_max_timestamp            : 48;
        micro_burst_cycles             : 32;
        allowed_bytes                  : 40;
        max_allowed_bytes              : 40;
        burst_exceed_bytes             : 40;
        burst_exceed_count             : 32;

        last_export_timestamp          : 48;
    }
}

header cap_phv_intr_global_t capri_intrinsic;
metadata ipfix_metadata_t ipfix_metadata;
metadata nonip_record_t nonip_record;
metadata ipv4_record_t ipv4_record;
metadata ipv6_record_t ipv6_record;
metadata ip_record_t ip_record;
metadata common_record_t common_record;

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata;

// keep in sync with flow_hash_info of nic.p4
action flow_hash_info(lkp_type, lkp_vrf, lkp_src, lkp_dst, lkp_proto,
                      lkp_sport, lkp_dport, entry_valid, export_en, flow_index,
                      hash1, hint1, hash2, hint2, hash3, hint3,
                      hash4, hint4, hash5, hint5, hash6, hint6) {
    if ((entry_valid != TRUE) or (export_en != TRUE)) {
        modify_field(ipfix_metadata.skip_record, TRUE);
    }

    modify_field(ipfix_metadata.flow_index,flow_index);

    modify_field(common_record.vrf, lkp_vrf);
    modify_field(common_record.flow_index, flow_index);
    if (lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC) {
        modify_field(nonip_record.mac_sa, lkp_src);
        modify_field(nonip_record.mac_da, lkp_dst);
        modify_field(nonip_record.ether_type, lkp_dport);
    } else {
        if (lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4) {
            modify_field(ipv4_record.ipv4_sa, lkp_src);
            modify_field(ipv4_record.ipv4_da, lkp_dst);
            modify_field(ip_record.proto, lkp_proto);
            if (lkp_proto == IP_PROTO_ICMP) {
                modify_field(ip_record.icmp_type_code,lkp_sport);
            } else {
                modify_field(ip_record.sport, lkp_sport);
                modify_field(ip_record.dport, lkp_dport);
            }
        } else {
            if (lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6) {
                modify_field(ipv6_record.ipv6_sa, lkp_src);
                modify_field(ipv6_record.ipv6_da, lkp_dst);
                modify_field(ip_record.proto, lkp_proto);
                if (lkp_proto == IP_PROTO_ICMP) {
                    modify_field(ip_record.icmp_type_code, lkp_sport);
                } else {
                    modify_field(ip_record.sport, lkp_sport);
                    modify_field(ip_record.dport, lkp_dport);
                }
            } else {
                modify_field(ipfix_metadata.skip_record, TRUE);
            }
        }
    }

    // dummy ops
    modify_field(scratch_metadata.lkp_type, lkp_type);
    modify_field(scratch_metadata.lkp_vrf, lkp_vrf);
    modify_field(scratch_metadata.lkp_src, lkp_src);
    modify_field(scratch_metadata.lkp_dst, lkp_dst);
    modify_field(scratch_metadata.lkp_proto, lkp_proto);
    modify_field(scratch_metadata.lkp_sport, lkp_sport);
    modify_field(scratch_metadata.lkp_dport, lkp_dport);
    modify_field(scratch_metadata.lkp_dport, lkp_dport);
    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flag, export_en);
    modify_field(scratch_metadata.hash, hash1);
    modify_field(scratch_metadata.hash, hash2);
    modify_field(scratch_metadata.hash, hash3);
    modify_field(scratch_metadata.hash, hash4);
    modify_field(scratch_metadata.hash, hash5);
    modify_field(scratch_metadata.hash, hash6);
    modify_field(scratch_metadata.hint, hint1);
    modify_field(scratch_metadata.hint, hint2);
    modify_field(scratch_metadata.hint, hint3);
    modify_field(scratch_metadata.hint, hint4);
    modify_field(scratch_metadata.hint, hint5);
    modify_field(scratch_metadata.hint, hint6);
}

table ipfix_flow_hash {
    reads {
        ipfix_metadata.flow_hash_index : exact;
    }
    actions {
        flow_hash_info;
    }
    size : FLOW_HASH_TABLE_SIZE;
}

table ipfix_flow_hash_overflow {
    reads {
        ipfix_metadata.flow_hash_overflow_index : exact;
    }
    actions {
        flow_hash_info;
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

// keep in sync with flow_info of nic.p4
action flow_info(lif, multicast_en,
                 ingress_policer_index, egress_policer_index, mirror_session_id,
                 rewrite_index, tunnel_rewrite_index, tunnel_vnid, entropy_hash,
                 tunnel_originate, nat_ip, nat_l4_port, twice_nat_idx,
                 cos_en, cos, dscp_en, dscp, qid_en, log_en,
                 mac_sa_rewrite, mac_da_rewrite, ttl_dec,
                 flow_mss, flow_ws, flow_ts_negotiated, flow_ttl, flow_role,
                 flow_index, flow_state_index, start_timestamp) {
    modify_field(common_record.start_timestamp, start_timestamp);
    modify_field(common_record.egress_interface, lif);
    if (multicast_en == TRUE) {
        modify_field(common_record.egress_interface_type, 1);
    }

    modify_field(ip_record.tcp_mss, flow_mss);
    modify_field(ip_record.ttl, flow_ttl);
    modify_field(ip_record.role, flow_role);

    if (flow_state_index != 0) {
        modify_field(ipfix_metadata.flow_state_index_valid, TRUE);
        modify_field(ip_record.flow_state_index, flow_state_index);
        modify_field(ipfix_metadata.flow_state_index, flow_state_index);
    }

    // dummy ops
    modify_field(scratch_metadata.lif, lif);
    modify_field(scratch_metadata.flag, multicast_en);
    modify_field(scratch_metadata.ingress_policer_index, ingress_policer_index);
    modify_field(scratch_metadata.egress_policer_index, egress_policer_index);
    modify_field(scratch_metadata.mirror_session_id, mirror_session_id);
    modify_field(scratch_metadata.rewrite_index, rewrite_index);
    modify_field(scratch_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(scratch_metadata.tunnel_vnid, tunnel_vnid);
    modify_field(scratch_metadata.entropy_hash, entropy_hash);
    modify_field(scratch_metadata.flag, tunnel_originate);
    modify_field(scratch_metadata.nat_ip, nat_ip);
    modify_field(scratch_metadata.nat_l4_port, nat_l4_port);
    modify_field(scratch_metadata.twice_nat_idx, twice_nat_idx);
    modify_field(scratch_metadata.flag, cos_en);
    modify_field(scratch_metadata.cos, cos);
    modify_field(scratch_metadata.flag, dscp_en);
    modify_field(scratch_metadata.dscp, dscp);
    modify_field(scratch_metadata.flag, qid_en);
    modify_field(scratch_metadata.flag, log_en);
    modify_field(scratch_metadata.flag, mac_sa_rewrite);
    modify_field(scratch_metadata.flag, mac_da_rewrite);
    modify_field(scratch_metadata.flag, ttl_dec);
    modify_field(scratch_metadata.flow_mss, flow_mss);
    modify_field(scratch_metadata.flow_ws, flow_ws);
    modify_field(scratch_metadata.flow_ts_negotiated, flow_ts_negotiated);
    modify_field(scratch_metadata.flow_ttl, flow_ttl);
    modify_field(scratch_metadata.flow_role, flow_role);
    modify_field(scratch_metadata.flow_index, flow_index);
    modify_field(scratch_metadata.flow_state_index, flow_state_index);
    modify_field(scratch_metadata.start_timestamp, start_timestamp);
}

table ipfix_flow_info {
    reads {
        ipfix_metadata.flow_index : exact;
    }
    actions {
        flow_info;
    }
    size : FLOW_TABLE_SIZE;
}

// keep in sync with validate_tcp_flow_state of nic.p4
action flow_state_info(iflow_tcp_seq_num, iflow_tcp_seq_delta,
                       iflow_tcp_ack_num, iflow_tcp_ack_delta,
                       iflow_tcp_win_sz, iflow_tcp_win_scale,
                       iflow_tcp_state,
                       rflow_tcp_seq_num, rflow_tcp_seq_delta,
                       rflow_tcp_ack_num, rflow_tcp_ack_delta,
                       rflow_tcp_win_sz, rflow_tcp_win_scale,
                       rflow_tcp_state,
                       exceptions_seen, flow_rtt_seq_check_enabled,
                       rtt, rtt_seq_no, rtt_timestamp) {
    modify_field(ip_record.tcp_exceptions, exceptions_seen);
    modify_field(ip_record.tcp_rtt, rtt);
    if (ip_record.role == TCP_FLOW_INITIATOR) {
        modify_field(ip_record.tcp_seq_num, iflow_tcp_seq_num);
        modify_field(ip_record.tcp_ack_num, iflow_tcp_ack_num);
        modify_field(ip_record.tcp_win_size, iflow_tcp_win_sz);
        modify_field(ip_record.tcp_win_scale, iflow_tcp_win_scale);
        modify_field(ip_record.tcp_state, iflow_tcp_state);
    } else {
        modify_field(ip_record.tcp_seq_num, rflow_tcp_seq_num);
        modify_field(ip_record.tcp_ack_num, rflow_tcp_ack_num);
        modify_field(ip_record.tcp_win_size, rflow_tcp_win_sz);
        modify_field(ip_record.tcp_win_scale, rflow_tcp_win_scale);
        modify_field(ip_record.tcp_state, rflow_tcp_state);
    }

    // dummy ops
    modify_field(scratch_metadata.seq_delta, iflow_tcp_seq_delta);
    modify_field(scratch_metadata.seq_delta, iflow_tcp_ack_delta);
    modify_field(scratch_metadata.seq_delta, rflow_tcp_seq_delta);
    modify_field(scratch_metadata.seq_delta, rflow_tcp_ack_delta);
    modify_field(scratch_metadata.flag, flow_rtt_seq_check_enabled);
    modify_field(scratch_metadata.rtt_seq_no, rtt_seq_no);
    modify_field(scratch_metadata.rtt_timestamp, rtt_timestamp);
}

table ipfix_flow_state_info {
    reads {
        ipfix_metadata.flow_state_index : exact;
    }
    actions {
        flow_state_info;
    }
    size : FLOW_STATE_TABLE_SIZE;
}

// keep in sync with flow_stats of nic.p4
action flow_stats_info(last_seen_timestamp, permit_packet, permit_bytes,
                       drop_packets, drop_bytes, drop_reason,
                       burst_start_timestamp, burst_max_timestamp,
                       micro_burst_cycles, allowed_bytes,
                       max_allowed_bytes, burst_exceed_bytes,
                       burst_exceed_count) {
    modify_field(common_record.last_seen_timestamp, last_seen_timestamp);
    modify_field(common_record.drop_vector, drop_reason);
    modify_field(common_record.micro_burst_exceed_bytes, burst_exceed_bytes);
    modify_field(common_record.micro_burst_exceed_count, burst_exceed_count);

    // dummy ops
    modify_field(scratch_metadata.last_seen_timestamp, last_seen_timestamp);
    modify_field(scratch_metadata.permit_packet, permit_packet);
    modify_field(scratch_metadata.permit_bytes, permit_bytes);
    modify_field(scratch_metadata.drop_packets, drop_packets);
    modify_field(scratch_metadata.drop_bytes, drop_bytes);
    modify_field(scratch_metadata.drop_reason, drop_reason);
    modify_field(scratch_metadata.burst_start_timestamp, burst_start_timestamp);
    modify_field(scratch_metadata.burst_max_timestamp, burst_max_timestamp);
    modify_field(scratch_metadata.micro_burst_cycles, micro_burst_cycles);
    modify_field(scratch_metadata.allowed_bytes, allowed_bytes);
    modify_field(scratch_metadata.max_allowed_bytes, max_allowed_bytes);
    modify_field(scratch_metadata.burst_exceed_bytes, burst_exceed_bytes);
    modify_field(scratch_metadata.burst_exceed_count, burst_exceed_count);
}

table ipfix_flow_stats_info {
    reads {
        ipfix_metadata.flow_index : exact;
    }
    actions {
        flow_stats_info;
    }
    size : FLOW_TABLE_SIZE;
}

action flow_counter_info(permit_bytes, permit_packets,
                         drop_bytes, drop_packets) {
    modify_field(common_record.permit_bytes, permit_bytes);
    modify_field(common_record.permit_packets, permit_packets);
    modify_field(common_record.drop_bytes, drop_bytes);
    modify_field(common_record.drop_packets, drop_packets);
}

table ipfix_flow_counters_info {
    reads {
        ipfix_metadata.flow_index : exact;
    }
    actions {
        flow_counter_info;
    }
    size : FLOW_TABLE_SIZE;
}

action flow_export(flow_expired, export_complete, last_export_timestamp) {
    if (export_complete == TRUE) {
        modify_field(ipfix_metadata.skip_record, TRUE);
    }

    if ((flow_expired == TRUE) and (export_complete == FALSE)) {
        // modify_field(export_complete, TRUE);
    }

    if ((flow_expired == FALSE) and
        ((last_export_timestamp + THRESHOLD) > capri_intrinsic.timestamp)) {
        modify_field(ipfix_metadata.skip_record, TRUE);
    }

    if (ipfix_metadata.skip_record == FALSE) {
        // modify_field(last_export_timestamp, capri_intrinsic.timestamp);
    }

    // phv2mem instructions

    // dummy ops
    modify_field(scratch_metadata.flag, flow_expired);
    modify_field(scratch_metadata.flag, export_complete);
    modify_field(scratch_metadata.last_export_timestamp, last_export_timestamp);
}

table ipfix_export {
    reads {
        ipfix_metadata.flow_index : exact;
    }
    actions {
        flow_export;
    }
    size : FLOW_TABLE_SIZE;
}

action ipfix_cleanup() {
    if (ipfix_metadata.lookup_flow_hash == TRUE) {
        if (ipfix_metadata.flow_hash_index == 0x1FFFFF) {
            // ring flow_hash_overflow door bell
        } else {
            // increment flow_hash_index
            // re-schedule self again
        }
    } else {
        if (ipfix_metadata.flow_hash_overflow_index == 0x3FFF) {
            // ring flow_hash door bell
        } else {
            // increment flow_hash_overflow_index
            // re-schedule self again
        }
    }
}

table ipfix_cleanup {
    actions {
        ipfix_cleanup;
    }
    size : 1;
}

control ingress {
    if (ipfix_metadata.lookup_flow_hash == TRUE) {
        apply(ipfix_flow_hash);
    } else {
        apply(ipfix_flow_hash_overflow);
    }
    if (ipfix_metadata.skip_record == FALSE) {
        apply(ipfix_flow_info);
    }
    if (ipfix_metadata.flow_state_index_valid == TRUE) {
        apply(ipfix_flow_state_info);
    }
    if (ipfix_metadata.skip_record == FALSE) {
        apply(ipfix_flow_stats_info);
    }
    if (ipfix_metadata.skip_record == FALSE) {
        apply(ipfix_flow_counters_info);
    }
    if (ipfix_metadata.skip_record == FALSE) {
        apply(ipfix_export);
    }
    apply(ipfix_cleanup);
}

parser start {
    extract(capri_intrinsic);
    return ingress;
}
