/*****************************************************************************/
/* Policy (IPv6 and non-IP)                                                  */
/*****************************************************************************/
@pragma capi appdatafields session_index nexthop_group_index flow_role
@pragma capi hwfields_access_api
action flow_hash(entry_valid, session_index, nexthop_group_index, flow_role,
                 hash1, hint1, hash2, hint2, hash3, hint3, hash4, hint4,
                 more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(service_header.flow_done, TRUE);
        modify_field(control_metadata.session_index, session_index);
        if (nexthop_group_index != 0) {
            modify_field(txdma_to_p4e_header.nexthop_group_index,
                         nexthop_group_index);
            modify_field(control_metadata.fastpath, TRUE);
        }
        modify_field(control_metadata.flow_role, flow_role);
        modify_field(p4i_apollo_i2e.entropy_hash, scratch_metadata.flow_hash);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.flow_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.flow_hash,
                     scratch_metadata.flow_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash1)) {
            modify_field(scratch_metadata.flow_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash2)) {
            modify_field(scratch_metadata.flow_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash3)) {
            modify_field(scratch_metadata.flow_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash4)) {
            modify_field(scratch_metadata.flow_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.flow_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(control_metadata.flow_ohash_lkp, TRUE);
            modify_field(service_header.flow_ohash, scratch_metadata.flow_hint);
        } else {
            modify_field(service_header.flow_done, TRUE);
            modify_field(control_metadata.session_index, 0);
        }
    } else {
        modify_field(service_header.flow_done, TRUE);
        modify_field(control_metadata.session_index, 0);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.flow_hash, hash3);
    modify_field(scratch_metadata.flow_hash, hash4);
}

@pragma stage 2
@pragma hbm_table
@pragma capi_bitfields_struct
table flow {
    reads {
        key_metadata.lkp_id : exact;
        key_metadata.ktype  : exact;
        key_metadata.src    : exact;
        key_metadata.dst    : exact;
        key_metadata.proto  : exact;
        key_metadata.sport  : exact;
        key_metadata.dport  : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table flow
table flow_ohash {
    reads {
        service_header.flow_ohash   : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}

/*****************************************************************************/
/* Policy (IPv4)                                                             */
/*****************************************************************************/
@pragma capi appdatafields session_index nexthop_group_index flow_role
@pragma capi hwfields_access_api
action ipv4_flow_hash(entry_valid, session_index, nexthop_group_index,
                      flow_role, hash1, hint1, hash2, hint2,
                      more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(service_header.flow_done, TRUE);
        modify_field(control_metadata.session_index, session_index);
        if (nexthop_group_index != 0) {
            modify_field(txdma_to_p4e_header.nexthop_group_index,
                         nexthop_group_index);
            modify_field(control_metadata.fastpath, TRUE);
        }
        modify_field(control_metadata.flow_role, flow_role);
        modify_field(p4i_apollo_i2e.entropy_hash, scratch_metadata.flow_hash);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.flow_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.flow_hash,
                     scratch_metadata.flow_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash1)) {
            modify_field(scratch_metadata.flow_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash2)) {
            modify_field(scratch_metadata.flow_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.flow_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(control_metadata.flow_ohash_lkp, TRUE);
            modify_field(service_header.flow_ohash, scratch_metadata.flow_hint);
        } else {
            modify_field(service_header.flow_done, TRUE);
            modify_field(control_metadata.session_index, 0);
        }
    } else {
        modify_field(service_header.flow_done, TRUE);
        modify_field(control_metadata.session_index, 0);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
}

@pragma stage 2
@pragma hbm_table
@pragma capi_bitfields_struct
table ipv4_flow {
    reads {
        key_metadata.lkp_id     : exact;
        key_metadata.ipv4_src   : exact;
        key_metadata.ipv4_dst   : exact;
        key_metadata.proto      : exact;
        key_metadata.sport      : exact;
        key_metadata.dport      : exact;
    }
    actions {
        ipv4_flow_hash;
    }
    size : FLOW_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table ipv4_flow
table ipv4_flow_ohash {
    reads {
        service_header.flow_ohash   : exact;
    }
    actions {
        ipv4_flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}

action session_info(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                    iflow_tcp_win_sz, iflow_tcp_win_scale, rflow_tcp_state,
                    rflow_tcp_seq_num, rflow_tcp_ack_num, rflow_tcp_win_sz,
                    rflow_tcp_win_scale, drop) {
    if (control_metadata.session_index == 0) {
        if (p4_to_rxdma_header.sacl_base_addr == 0) {
            modify_field(p4_to_rxdma_header.sacl_bypass, TRUE);
        }
        // return
    }

    modify_field(p4_to_rxdma_header.sacl_bypass, TRUE);
    modify_field(scratch_metadata.session_stats_addr,
                 scratch_metadata.session_stats_addr +
                 (control_metadata.session_index * 8 * 4));
    modify_field(scratch_metadata.flag, drop);
    modify_field(scratch_metadata.in_bytes, capri_p4_intrinsic.packet_len);
    if (drop == TRUE) {
        modify_field(control_metadata.p4i_drop_reason, 1 << P4I_DROP_FLOW_HIT);
        drop_packet();
    }

    if (tcp.valid == TRUE) {
        modify_field(scratch_metadata.tcp_flags, tcp.flags);
        if (control_metadata.flow_role == TCP_FLOW_INITIATOR) {
            modify_field(scratch_metadata.tcp_state, iflow_tcp_state);
            modify_field(scratch_metadata.tcp_seq_num, iflow_tcp_seq_num);
            modify_field(scratch_metadata.tcp_ack_num, iflow_tcp_ack_num);
            modify_field(scratch_metadata.tcp_win_sz, iflow_tcp_win_sz);
            modify_field(scratch_metadata.tcp_win_scale, iflow_tcp_win_scale);
        } else {
            modify_field(scratch_metadata.tcp_state, rflow_tcp_state);
            modify_field(scratch_metadata.tcp_seq_num, rflow_tcp_seq_num);
            modify_field(scratch_metadata.tcp_ack_num, rflow_tcp_ack_num);
            modify_field(scratch_metadata.tcp_win_sz, rflow_tcp_win_sz);
            modify_field(scratch_metadata.tcp_win_scale, rflow_tcp_win_scale);
        }
    }
}

@pragma stage 4
@pragma hbm_table
@pragma table_write
@pragma index_table
table session {
    reads {
        control_metadata.session_index  : exact;
    }
    actions {
        session_info;
    }
    size : SESSION_TABLE_SIZE;
}

control flow_lookup {
    if (service_header.valid == FALSE) {
        if (control_metadata.skip_flow_lkp == FALSE) {
            if (key_metadata.ktype == KEY_TYPE_IPV4) {
                apply(ipv4_flow);
            } else {
                apply(flow);
            }
        }
    }
    if (control_metadata.flow_ohash_lkp == TRUE) {
        if (key_metadata.ktype == KEY_TYPE_IPV4) {
            apply(ipv4_flow_ohash);
        } else {
            apply(flow_ohash);
        }
    }
    apply(session);
}
