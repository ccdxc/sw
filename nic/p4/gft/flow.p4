/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
action rx_gft_hash_info(entry_valid, flow_index, policer_index,
                      hash1, hint1, hash2, hint2, hash3, hint3,
                      hash4, hint4, hash5, hint5, hash6, hint6,
                      hash7, hint7, hash8, hint8, hash9, hint9,
                      hash10, hint10, more_hash, more_hints) {
    if (entry_valid == TRUE) {
        // if register c1 indicates hit, take the results.
        modify_field(flow_action_metadata.flow_index, flow_index);
        modify_field(flow_action_metadata.policer_index, policer_index);

        // if register c1 indicates miss, compare hints and lookup
        // overflow hash table
        modify_field(flow_action_metadata.overflow_lkp, TRUE);
        modify_field(flow_lkp_metadata.overflow_hash, hint1);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.hash, hash1);
    modify_field(scratch_metadata.hash, hash2);
    modify_field(scratch_metadata.hash, hash3);
    modify_field(scratch_metadata.hash, hash4);
    modify_field(scratch_metadata.hash, hash5);
    modify_field(scratch_metadata.hash, hash6);
    modify_field(scratch_metadata.hash, hash7);
    modify_field(scratch_metadata.hash, hash8);
    modify_field(scratch_metadata.hash, hash9);
    modify_field(scratch_metadata.hash, hash10);
    modify_field(scratch_metadata.hint, hint1);
    modify_field(scratch_metadata.hint, hint2);
    modify_field(scratch_metadata.hint, hint3);
    modify_field(scratch_metadata.hint, hint4);
    modify_field(scratch_metadata.hint, hint5);
    modify_field(scratch_metadata.hint, hint6);
    modify_field(scratch_metadata.hint, hint7);
    modify_field(scratch_metadata.hint, hint8);
    modify_field(scratch_metadata.hint, hint9);
    modify_field(scratch_metadata.hint, hint10);
    modify_field(scratch_metadata.flag, more_hash);
    modify_field(scratch_metadata.hint, more_hints);
}

@pragma stage 2
@pragma hbm_table
table rx_gft_hash {
    reads {
        flow_lkp_metadata.ethernet_dst_1     : exact;
        flow_lkp_metadata.ethernet_src_1     : exact;
        flow_lkp_metadata.ethernet_type_1    : exact;
        flow_lkp_metadata.pad_1              : exact;
        flow_lkp_metadata.ctag_1             : exact;
        flow_lkp_metadata.ip_src_1           : exact;
        flow_lkp_metadata.ip_dst_1           : exact;
        flow_lkp_metadata.ip_dscp_1          : exact;
        flow_lkp_metadata.ip_proto_1         : exact;
        flow_lkp_metadata.ip_ttl_1           : exact;
        flow_lkp_metadata.l4_sport_1         : exact;
        flow_lkp_metadata.l4_dport_1         : exact;
        flow_lkp_metadata.tcp_flags_1        : exact;
        flow_lkp_metadata.tenant_id_1        : exact;
        flow_lkp_metadata.gre_proto_1        : exact;

        flow_lkp_metadata.ethernet_dst_2     : exact;
        flow_lkp_metadata.ethernet_src_2     : exact;
        flow_lkp_metadata.ethernet_type_2    : exact;
        flow_lkp_metadata.pad_2              : exact;
        flow_lkp_metadata.ctag_2             : exact;
        flow_lkp_metadata.ip_src_2           : exact;
        flow_lkp_metadata.ip_dst_2           : exact;
        flow_lkp_metadata.ip_dscp_2          : exact;
        flow_lkp_metadata.ip_proto_2         : exact;
        flow_lkp_metadata.ip_ttl_2           : exact;
        flow_lkp_metadata.l4_sport_2         : exact;
        flow_lkp_metadata.l4_dport_2         : exact;
        flow_lkp_metadata.tcp_flags_2        : exact;
        flow_lkp_metadata.tenant_id_2        : exact;
        flow_lkp_metadata.gre_proto_2        : exact;

        flow_lkp_metadata.ethernet_dst_3     : exact;
        flow_lkp_metadata.ethernet_src_3     : exact;
        flow_lkp_metadata.ethernet_type_3    : exact;
        flow_lkp_metadata.pad_3              : exact;
        flow_lkp_metadata.ctag_3             : exact;
        flow_lkp_metadata.ip_src_3           : exact;
        flow_lkp_metadata.ip_dst_3           : exact;
        flow_lkp_metadata.ip_dscp_3          : exact;
        flow_lkp_metadata.ip_proto_3         : exact;
        flow_lkp_metadata.ip_ttl_3           : exact;
        flow_lkp_metadata.l4_sport_3         : exact;
        flow_lkp_metadata.l4_dport_3         : exact;
        flow_lkp_metadata.tcp_flags_3        : exact;
        flow_lkp_metadata.tenant_id_3        : exact;
        flow_lkp_metadata.gre_proto_3        : exact;
    }
    actions {
        rx_gft_hash_info;
    }
    size : FLOW_HASH_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table rx_gft_hash
table rx_gft_hash_overflow {
    reads {
        flow_lkp_metadata.overflow_hash : exact;
    }
    actions {
        rx_gft_hash_info;
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

control rx_flow_lookup {
    apply(rx_gft_hash);
    if (flow_action_metadata.overflow_lkp == TRUE) {
        apply(rx_gft_hash_overflow);
    }
}

/******************************************************************************/
/* Tx pipeline                                                                */
/******************************************************************************/
action tx_gft_hash_info(entry_valid, flow_index, policer_index,
                      hash1, hint1, hash2, hint2, hash3, hint3,
                      hash4, hint4, hash5, hint5, hash6, hint6,
                      hash7, hint7, hash8, hint8, hash9, hint9,
                      hash10, hint10) {
    if (entry_valid == TRUE) {
        // if register c1 indicates hit, take the results.
        modify_field(flow_action_metadata.flow_index, flow_index);
        modify_field(flow_action_metadata.policer_index, policer_index);

        // if register c1 indicates miss, compare hints and lookup
        // overflow hash table
        modify_field(flow_action_metadata.overflow_lkp, TRUE);
        modify_field(flow_lkp_metadata.overflow_hash, hint1);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.hash, hash1);
    modify_field(scratch_metadata.hash, hash2);
    modify_field(scratch_metadata.hash, hash3);
    modify_field(scratch_metadata.hash, hash4);
    modify_field(scratch_metadata.hash, hash5);
    modify_field(scratch_metadata.hash, hash6);
    modify_field(scratch_metadata.hash, hash7);
    modify_field(scratch_metadata.hash, hash8);
    modify_field(scratch_metadata.hash, hash9);
    modify_field(scratch_metadata.hash, hash10);
    modify_field(scratch_metadata.hint, hint1);
    modify_field(scratch_metadata.hint, hint2);
    modify_field(scratch_metadata.hint, hint3);
    modify_field(scratch_metadata.hint, hint4);
    modify_field(scratch_metadata.hint, hint5);
    modify_field(scratch_metadata.hint, hint6);
    modify_field(scratch_metadata.hint, hint7);
    modify_field(scratch_metadata.hint, hint8);
    modify_field(scratch_metadata.hint, hint9);
    modify_field(scratch_metadata.hint, hint10);
}

@pragma stage 1
@pragma hbm_table
table tx_gft_hash {
    reads {
        flow_lkp_metadata.ethernet_dst_1     : exact;
        flow_lkp_metadata.ethernet_src_1     : exact;
        flow_lkp_metadata.ethernet_type_1    : exact;
        flow_lkp_metadata.pad_1              : exact;
        flow_lkp_metadata.ctag_1             : exact;
        flow_lkp_metadata.ip_src_1           : exact;
        flow_lkp_metadata.ip_dst_1           : exact;
        flow_lkp_metadata.ip_dscp_1          : exact;
        flow_lkp_metadata.ip_proto_1         : exact;
        flow_lkp_metadata.ip_ttl_1           : exact;
        flow_lkp_metadata.l4_sport_1         : exact;
        flow_lkp_metadata.l4_dport_1         : exact;
        flow_lkp_metadata.tcp_flags_1        : exact;
        flow_lkp_metadata.tx_pad             : exact;
    }
    actions {
        tx_gft_hash_info;
    }
    size : FLOW_HASH_TABLE_SIZE;
}

@pragma stage 2
@pragma hbm_table
@pragma overflow_table tx_gft_hash
table tx_gft_hash_overflow {
    reads {
        flow_lkp_metadata.overflow_hash : exact;
    }
    actions {
        tx_gft_hash_info;
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

control tx_flow_lookup {
    apply(tx_gft_hash);
    if (flow_action_metadata.overflow_lkp == TRUE) {
        apply(tx_gft_hash_overflow);
    }
}
