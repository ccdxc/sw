#include "include/defines.h"
#include "include/parser.p4"
#include "include/headers.p4"
#include "include/i2e_metadata.p4"
#include "include/table_sizes.h"
#include "../include/intrinsic.p4"

#include "l4.p4"
#include "nat.p4"
#include "apps.p4"
#include "copp.p4"
#include "ddos.p4"
#include "flow.p4"
#include "ipsg.p4"
#include "nacl.p4"
#include "roce.p4"
#include "stats.p4"
#include "tunnel.p4"
#include "classic.p4"
#include "policer.p4"
#include "replica.p4"
#include "rewrite.p4"
#include "checksum.p4"
#include "validate.p4"
#include "input_mapping.p4"
#include "output_mapping.p4"

header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len   : 16;
        l4_payload_len  : 16;
        inner_l4_payload_len  : 16;
        icrc_payload_len: 16;
    }
}

header_type capri_gso_csum_phv_loc_t {
    fields {
        // For packet in TxDir (to uplink), this field
        // captures checksum value computed from gso_offset
        // to EOP.
        gso_checksum    : 16;
    }
}

header_type l3_metadata_t {
    fields {
        ip_option_seen           : 1;
        inner_ip_option_seen     : 1;
        ip_frag                  : 1;
        inner_ip_frag            : 1;
        ipv6_ulp                 : 8;
        inner_ipv6_ulp           : 8;
        ipv6_options_len         : 8;
        inner_ipv6_options_len   : 8;
    }
}

header_type control_metadata_t {
    fields {
        drop_reason                    : 40;
        qid                            : 24;
        qtype                          : 8;
        tm_iport                       : 4;
        egress_mirror_session_id       : 8;
        egress_tm_oqueue               : 5;
        normalization_cpu_reason       : 8;
        ingress_bypass                 : 1;
        egress_bypass                  : 1;
        ipsg_enable                    : 1;
        recirc_reason                  : 2;
        uplink                         : 1;
        from_cpu                       : 1;
        cpu_copy                       : 1;
        src_lif                        : 11;
        flow_miss_action               : 2;
        flow_miss_tm_oqueue            : 5;
        flow_miss_idx                  : 16;
        p4plus_app_id                  : 8;
        rdma_enabled                   : 1;
        src_lport                      : 11;
        dst_lport                      : 11;
        flow_miss                      : 1;
        flow_miss_ingress              : 1;  // NCC workaround for predication
        flow_miss_egress               : 1;  // NCC workaround for predication
        lkp_flags_egress               : 8;
        vlan_strip                     : 1;
        span_copy                      : 1;
        nic_mode                       : 1;
        parse_tcp_option_error         : 1;
        allow_flood                    : 1;
        checksum_ctl                   : 8;
        checksum_results               : 8;

        egress_ddos_src_vf_policer_drop   : 1;
        egress_ddos_service_policer_drop  : 1;
        egress_ddos_src_only_policer_drop : 1;
        egress_ddos_src_dst_policer_drop  : 1;
    }
}

header_type entry_inactive_t {
    fields {
        input_mapping    : 1;
        input_properties : 1;
        input_mac_vlan   : 1;
        registered_macs  : 1;
        ipsg             : 1;
        nacl             : 1;
        drop_stats       : 1;
        ddos_src_vf      : 1;
        ddos_src_dst     : 1;
        ddos_service     : 1;
        compute_checksum : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        cond_processed             : 1;
        flow_packets               : 4;
        flow_bytes                 : 18;
        flow_start_timestamp       : 32;       // when flow started
        flow_last_seen_timestamp   : 32;       // when was the flow last seen
        tx_drop_count              : 16;
        policer_packets            : 4;
        policer_bytes              : 18;
        ip_sa                      : 128;
        entropy_hash               : 32;
        egress_port                : 4;
        force_flow_hit             : 1;
        qid_en                     : 1;
        log_en                     : 1;
        ingress_mirror_en          : 1;
        egress_mirror_en           : 1;
        rewrite_en                 : 1;
        tunnel_rewrite_en          : 1;
        marking_overwrite          : 1;
        dst_lport_en               : 1;
        egress_policer_en          : 1;
        entry_valid                : 1;
        export_en                  : 4;
        export_id                  : 4;
        oport_en                   : 1;
        stats_idx                  : 5;
        drop_reason                : 32;
        stats_packets              : 4;
        stats_bytes                : 18;
		flag                       : 1;
        ethtype                    : 16;
        num_labels                 : 2;
        drop_count                 : 8;
        flow_agg_index             : 16;

        src_lif_check_en           : 1;
        vlan_strip                 : 1;

        // flow hash metadata
        flow_hash1                 : 11;
        flow_hash2                 : 11;
        flow_hash3                 : 11;
        flow_hash4                 : 11;
        flow_hash5                 : 11;
        flow_hash6                 : 11;
        flow_hint1                 : 14;
        flow_hint2                 : 14;
        flow_hint3                 : 14;
        flow_hint4                 : 14;
        flow_hint5                 : 14;
        flow_hint6                 : 14;
        more_hashs                 : 1;
        more_hints                 : 14;

        // flow key
        lkp_inst                   : 1;
        lkp_dir                    : 1;
        lkp_type                   : 4;
        lkp_vrf                    : 16;
        lkp_src                    : 128;
        lkp_dst                    : 128;
        lkp_proto                  : 8;
        lkp_sport                  : 16;
        lkp_dport                  : 16;

        // scratch state to perform TCP state checking
        expected_seq_num           : 32;   // expected TCP seq# on this flow
        adjusted_seq_num           : 32;   // delta adjust seq# of this flow
        rcvr_win_sz                : 32;   // receiver's window size
        tcp_seq_num_hi             : 32;   // seq# of last byte of the window
        adjusted_ack_num           : 32;   // delta adjusted ack# of this flow.
        b2b_expected_seq_num       : 32;   // when back2back traffic is coming in one direction.

        // DDoS specific fields.
        ddos_src_vf_base_policer_idx :8;
        ddos_service_base_policer_idx :8;
        ddos_src_dst_base_policer_idx :9;

        ddos_src_vf_policer_saved_color : 2;
        ddos_src_vf_policer_dropped_packets : 22;
        ddos_service_policer_saved_color : 2;
        ddos_service_policer_dropped_packets : 22;
        ddos_src_dst_policer_saved_color : 2;
        ddos_src_dst_policer_dropped_packets : 22;

        // RTT
        flow_rtt_seq_check_enabled    : 1;
        flow_rtt_in_progress          : 1;
        flow_rtt_seq_no               : 32;
        flow_rtt                      : 34; // Max 16 sec assuming nano sec granularity
        flow_rtt_timestamp            : 48;

        // Microburst detection
        burst_start_timestamp : 48;
        burst_max_timestamp   : 48;
        micro_burst_cycles    : 32; // enough ??
        allowed_bytes         : 40;
        max_allowed_bytes     : 40;
        burst_exceed_bytes    : 40;
        burst_exceed_count    : 32;

        // ipsg
        src_lif               : 11;
        mac                   : 48;
        vlan_valid            : 1;
        vlan_id               : 12;

        // flow state
        // initiator flow's TCP state
        iflow_tcp_seq_num             : 32;           // TCP seq#
        iflow_tcp_ack_num             : 32;           // TCP ack#
        iflow_tcp_win_sz              : 16;           // TCP window size
        iflow_tcp_win_scale           : 4;            // TCP window scale
        iflow_tcp_state               : 4;            // flow state
        iflow_exceptions_seen         : 14;           // list of exceptions seen
        iflow_tcp_ws_option_sent      : 1;
        iflow_tcp_ts_option_sent      : 1;
        iflow_tcp_sack_perm_option_sent : 1;

        // responder flow's TCP state
        rflow_tcp_seq_num             : 32;           // TCP seq#
        rflow_tcp_ack_num             : 32;           // TCP ack#
        rflow_tcp_win_sz              : 16;           // TCP window size
        rflow_tcp_win_scale           : 4;            // TCP window scale
        rflow_tcp_state               : 4;            // flow_state
        rflow_tcp_mss                 : 16;
        rflow_exceptions_seen         : 14;           // list of exceptions seen

        syn_cookie_delta              : 32 (signed);  // TCP seq/ack# adjustment

        // icmp code and type needed for ip normalization
        icmp_code                     : 8;
        icmp_type                     : 8;

        classic_nic_header_flags      : 16;
        cpu_flags                     : 8;
        cpu_tcp_options               : 8;
        packet_len                    : 16;
        offset                        : 16;

        // policer
        policer_valid                 : 1;
        policer_pkt_rate              : 1;
        policer_rlimit_en             : 1;
        policer_rlimit_prof           : 2;
        policer_color_aware           : 1;
        policer_rsvd                  : 1;
        policer_axi_wr_pend           : 1;
        policer_burst                 : 40;
        policer_rate                  : 40;
        policer_tbkt                  : 40;

        size16                        : 16;
        size8                         : 8;
    }
}

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t capri_deparser_len;
#ifdef GSO_CSUM
@pragma gso_csum_header
@pragma dont_trim
metadata capri_gso_csum_phv_loc_t   capri_gso_csum;
#endif
metadata l3_metadata_t l3_metadata;
metadata control_metadata_t control_metadata;
metadata entry_inactive_t entry_inactive;
// scratch_metadata : no phvs will be allocated for this. These fields
// should  only be used in action routines as temporary/local variables
@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata;

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

action permit_packet() {
    modify_field(capri_intrinsic.drop, FALSE);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    if (control_metadata.ingress_bypass == FALSE) {
        process_input_mapping();
        process_p4plus_to_p4();
        process_l4_profile();
        process_ipsg();
        process_normalization();
        process_flow_table();
        process_registered_macs();
        process_nacl();
        process_session_state();
        process_stats();
        process_ddos_ingress();
    }
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    if (control_metadata.egress_bypass == FALSE) {
        process_replica();
        process_output_mapping();
        process_roce();
        process_rewrites();
        process_egress_policer();
        process_ddos_egress();
    }
    process_tx_stats();
    process_checksum_computation();
}
