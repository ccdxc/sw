/*****************************************************************************/
/* Flow table processing                                                     */
/*****************************************************************************/
header_type flow_lkp_metadata_t {
    fields {
        lkp_inst         : 1;
        lkp_type         : 4;
        ipv4_hlen        : 4;          // For Normalization
        lkp_vrf          : 16;
        lkp_src          : 128;
        lkp_dst          : 128;
        lkp_sport        : 16;
        lkp_dport        : 16;

        lkp_srcMacAddr   : 48;         // For NACL Key
        lkp_dstMacAddr   : 48;         // For NACL Key

        ipv4_flags       : 3;          // For Normalization
        lkp_proto        : 8;
        ip_ttl           : 8;          // used to check if TTL == 0
        pkt_type         : 2;
        lkp_dir          : 1;          // To Prevent uplink spoofing workload packets.
    }
}

header_type flow_info_metadata_t {
    fields {
        flow_role                : 1;  // initiator flow (0) or responder flow (1)
        flow_ttl                 : 8;  // Initial ttl seen
        flow_ttl_change_detected : 1;  // records that flow ttl change is seen
        flow_index               : 20; // flow info table index and for  P4+ pipeline
        session_state_index      : 20; // index to iflow and rflow state
    }
}

header_type flow_miss_metadata_t {
    fields {
        rewrite_index         : 12;
        tunnel_rewrite_index  : 10;
        tunnel_vnid           : 24;
        tunnel_originate      : 1;
    }
}

metadata flow_lkp_metadata_t flow_lkp_metadata;
metadata flow_info_metadata_t flow_info_metadata;
metadata flow_miss_metadata_t flow_miss_metadata;

// entry 0 of flow_info table will be programmed as the miss entry
action flow_miss() {
    validate_flow_key();

    if (flow_lkp_metadata.lkp_vrf == 0) {
        modify_field(control_metadata.drop_reason, DROP_INPUT_PROPERTIES_MISS);
        drop_packet();
    }

    if ((flow_lkp_metadata.lkp_proto == IP_PROTO_TCP) and
        ((tcp.flags & TCP_FLAG_SYN) != TCP_FLAG_SYN) and
        (l4_metadata.tcp_non_syn_first_pkt_drop == ACT_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NON_SYN_FIRST_PKT);
        drop_packet();
    }

    modify_field (capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    modify_field(capri_intrinsic.tm_oq, control_metadata.flow_miss_tm_oqueue);

    if ((control_metadata.flow_miss_action == FLOW_MISS_ACTION_CPU) or
        ((control_metadata.flow_miss_action == FLOW_MISS_ACTION_FLOOD) and
        (flow_lkp_metadata.pkt_type != PACKET_TYPE_MULTICAST))) {
        modify_field(control_metadata.flow_miss, TRUE);
        modify_field(control_metadata.flow_miss_ingress, TRUE);
        modify_field(control_metadata.dst_lport, CPU_LPORT);
    }

    if (control_metadata.flow_miss_action == FLOW_MISS_ACTION_DROP) {
        modify_field(control_metadata.drop_reason, DROP_FLOW_MISS);
        drop_packet();
    }

    if ((control_metadata.flow_miss_action == FLOW_MISS_ACTION_FLOOD) and
        ((flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) or
         (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST))) {
        if (control_metadata.allow_flood == TRUE) {
            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
            modify_field(capri_intrinsic.tm_replicate_ptr,
                         control_metadata.flow_miss_idx);
	    modify_field(flow_miss_metadata.rewrite_index,
                         flow_miss_metadata.rewrite_index);
	    modify_field(flow_miss_metadata.tunnel_rewrite_index,
                         flow_miss_metadata.tunnel_rewrite_index);
	    modify_field(flow_miss_metadata.tunnel_vnid,
                         flow_miss_metadata.tunnel_vnid);
	    modify_field(flow_miss_metadata.tunnel_originate,
                         flow_miss_metadata.tunnel_originate);
        } else {
            modify_field(control_metadata.drop_reason, DROP_FLOW_MISS);
            drop_packet();
        }
    }

    if (control_metadata.flow_miss_action == FLOW_MISS_ACTION_REDIRECT) {
        modify_field(rewrite_metadata.tunnel_rewrite_index,
                     control_metadata.flow_miss_idx);
    }
}

action flow_hit_drop(flow_index, start_timestamp) {
    modify_field(flow_info_metadata.flow_index, flow_index);

    /* set the drop flag */
    modify_field(control_metadata.drop_reason, DROP_FLOW_HIT);
    drop_packet();

    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.flow_start_timestamp, start_timestamp);
}

// Srini: Topic came up while discussing with Sarat on VM Move case.
// We should mention that we won't support more than one mirror session
// per flow.
// Is p4+ expecting a flow_index per flow or per session ?
// We should have a flag here which enables/disables connection tracking.
// Change all timestamps to be 48 bit.
action flow_info(dst_lport, multicast_en, multicast_ptr, qtype,
                 ingress_policer_index, egress_policer_index,
                 ingress_mirror_session_id, egress_mirror_session_id,
                 rewrite_index, tunnel_rewrite_index, tunnel_vnid,
                 tunnel_originate, nat_ip, nat_l4_port, twice_nat_idx,
                 cos_en, cos, dscp_en, dscp, qid_en, log_en, rewrite_flags,
                 flow_conn_track, flow_ttl, flow_role,
                 session_state_index, start_timestamp,
                 ingress_tm_oqueue, egress_tm_oqueue,
                 expected_src_lif_check_en, expected_src_lif,
                 export_id1, export_id2, export_id3, export_id4) {
    /* expected src lif check */
    if ((expected_src_lif_check_en == TRUE) and
        (p4plus_to_p4.p4plus_app_id != P4PLUS_APPTYPE_CPU) and
        (expected_src_lif != control_metadata.src_lif)) {
        modify_field(control_metadata.drop_reason, DROP_SRC_LIF_MISMATCH);
        drop_packet();
    }

    /* egress port/vf */
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    modify_field(capri_intrinsic.tm_replicate_en, multicast_en);
    modify_field(capri_intrinsic.tm_replicate_ptr, multicast_ptr);
    modify_field(control_metadata.dst_lport, dst_lport);

    /* Output queue selection */
    modify_field(capri_intrinsic.tm_oq, ingress_tm_oqueue);
    modify_field(control_metadata.egress_tm_oqueue, egress_tm_oqueue);

    /* qid */
    if (qid_en == TRUE) {
        modify_field(control_metadata.qid, tunnel_vnid);
        modify_field(control_metadata.qtype, qtype);
    }

    /* mirror session id */
    modify_field(capri_intrinsic.tm_span_session, ingress_mirror_session_id);
    modify_field(control_metadata.egress_mirror_session_id,
                 egress_mirror_session_id);

    /* logging - need to create a copy */
    if (log_en == TRUE) {
        modify_field(capri_intrinsic.tm_cpu, TRUE);
    }

    /* policer indicies */
    modify_field(policer_metadata.ingress_policer_index, ingress_policer_index);
    modify_field(policer_metadata.egress_policer_index, egress_policer_index);

    /* flow info */
    modify_field(flow_info_metadata.session_state_index, session_state_index);
    modify_field(flow_info_metadata.flow_ttl, flow_ttl);
    modify_field(flow_info_metadata.flow_role, flow_role);

    // ttl change detected
    if ((l4_metadata.ip_ttl_change_detect_en == 1) and
        (flow_ttl != flow_lkp_metadata.ip_ttl)) {
        modify_field(flow_info_metadata.flow_ttl_change_detected, 1);
    }
    // Flow Connection Tracking enabled
    modify_field(l4_metadata.flow_conn_track, flow_conn_track);

    /* rewrite index */
    modify_field(rewrite_metadata.rewrite_index, rewrite_index);
    modify_field(rewrite_metadata.flags, rewrite_flags);

    /* NAT rewrite data */
    modify_field(nat_metadata.nat_ip, nat_ip);
    modify_field(nat_metadata.nat_l4_port, nat_l4_port);
    modify_field(nat_metadata.twice_nat_idx, twice_nat_idx);

    /* tunnel info */
    modify_field(tunnel_metadata.tunnel_originate, tunnel_originate);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(rewrite_metadata.tunnel_vnid, tunnel_vnid);

    /* qos rewrite data */
    modify_field(qos_metadata.cos_en, cos_en);
    modify_field(qos_metadata.dscp_en, dscp_en);
    modify_field(qos_metadata.cos, cos);
    modify_field(qos_metadata.dscp, dscp);

    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.flow_start_timestamp, start_timestamp);
    modify_field(scratch_metadata.qid_en, qid_en);
    modify_field(scratch_metadata.log_en, log_en);
    modify_field(scratch_metadata.flag, expected_src_lif_check_en);
    modify_field(scratch_metadata.src_lif, expected_src_lif);
    modify_field(scratch_metadata.export_id, export_id1);
    modify_field(scratch_metadata.export_id, export_id2);
    modify_field(scratch_metadata.export_id, export_id3);
    modify_field(scratch_metadata.export_id, export_id4);

    /* promote size of data fields to multiple of bytes */
    modify_field(scratch_metadata.size16, twice_nat_idx);
    modify_field(scratch_metadata.size8, cos_en);
    modify_field(scratch_metadata.size8, cos);
    modify_field(scratch_metadata.size8, dscp_en);
}

action recirc_packet(recirc_reason) {
    modify_field(control_metadata.ingress_bypass, 1);
    add_header(capri_p4_intrinsic);
    add_header(recirc_header);
    modify_field(recirc_header.src_tm_iport, control_metadata.tm_iport);
    modify_field(recirc_header.reason, recirc_reason);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
    modify_field(capri_intrinsic.tm_oq, TM_P4_IG_RECIRC_QUEUE);
}

action flow_hit_from_vm_bounce(src_lif) {
    remove_header(ethernet);
    remove_header(ipv4);
    remove_header(udp);
    remove_header(vxlan_gpe);
    recirc_packet(RECIRC_VM_BOUNCE);
    modify_field(capri_intrinsic.lif, src_lif);
}

action flow_hit_to_vm_bounce(dst_lport, tm_oqueue) {
    remove_header(ethernet);
    remove_header(ipv4);
    remove_header(udp);
    remove_header(vxlan_gpe);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.tm_oq, tm_oqueue);
}

@pragma stage 2
@pragma hbm_table
table flow_info {
    reads {
        flow_info_metadata.flow_index : exact;
    }
    actions {
        nop;
        flow_info;
        flow_miss;
        flow_hit_drop;
        flow_hit_from_vm_bounce;
        flow_hit_to_vm_bounce;
    }
    default_action : flow_miss;
    size : FLOW_TABLE_SIZE;
}

action flow_hash_info(entry_valid, export_en,
                      flow_index, hash1, hint1, hash2, hint2, hash3, hint3,
                      hash4, hint4, hash5, hint5, hash6, hint6,
                      more_hashs, more_hints) {
    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.entry_valid, entry_valid);
    modify_field(scratch_metadata.export_en, export_en);
    modify_field(scratch_metadata.flow_hash1, hash1);
    modify_field(scratch_metadata.flow_hash2, hash2);
    modify_field(scratch_metadata.flow_hash3, hash3);
    modify_field(scratch_metadata.flow_hash4, hash4);
    modify_field(scratch_metadata.flow_hash5, hash5);
    modify_field(scratch_metadata.flow_hash6, hash6);
    modify_field(scratch_metadata.flow_hint1, hint1);
    modify_field(scratch_metadata.flow_hint2, hint2);
    modify_field(scratch_metadata.flow_hint3, hint3);
    modify_field(scratch_metadata.flow_hint4, hint4);
    modify_field(scratch_metadata.flow_hint5, hint5);
    modify_field(scratch_metadata.flow_hint6, hint6);
    modify_field(scratch_metadata.more_hashs, more_hashs);
    modify_field(scratch_metadata.more_hints, more_hints);

    // If register C1 indicates hit, take flow_index. Otherwise walk
    // through the hints to pick the overflow entry
    modify_field(flow_info_metadata.flow_index, flow_index);
    modify_field(rewrite_metadata.entropy_hash, scratch_metadata.entropy_hash);

    // pack fields into control_metadata.lkp_flags_egress to transfer to egress
    modify_field(scratch_metadata.cpu_flags, flow_lkp_metadata.lkp_type);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_dir << 2);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_inst << 1);
    modify_field(control_metadata.lkp_flags_egress, scratch_metadata.cpu_flags);

    // no further matches
    if ((hash1 == 0) and (hint1 == 0)) {
        modify_field(control_metadata.flow_miss, TRUE);
        modify_field(control_metadata.flow_miss_ingress, TRUE);
        modify_field(flow_info_metadata.flow_index, 0);
    }

    // resolve hint and pick next hash for recirc path
    if ((control_metadata.flow_miss == TRUE) and
        (hash1 == scratch_metadata.flow_hash1)) {
        recirc_packet(RECIRC_FLOW_HASH_OVERFLOW);
        // do not assign it to larger variable.. can result in K+D violation
        //modify_field(recirc_header.overflow_entry_index, hint1);
    }
}

@pragma stage 1
@pragma hbm_table
@pragma hash_type 0
@pragma include_k_in_d
table flow_hash {
    reads {
        flow_lkp_metadata.lkp_inst  : exact;
        flow_lkp_metadata.lkp_dir   : exact;
        flow_lkp_metadata.lkp_type  : exact;
        flow_lkp_metadata.lkp_vrf   : exact;
        flow_lkp_metadata.lkp_src   : exact;
        flow_lkp_metadata.lkp_dst   : exact;
        flow_lkp_metadata.lkp_proto : exact;
        flow_lkp_metadata.lkp_sport : exact;
        flow_lkp_metadata.lkp_dport : exact;
    }
    actions {
        flow_hash_info;
    }
    default_action : flow_hash_info;
    size : FLOW_HASH_TABLE_SIZE;
}

action flow_hash_overflow(lkp_inst, lkp_dir, lkp_type, lkp_vrf, lkp_src,
                          lkp_dst, lkp_proto, lkp_sport, lkp_dport,
                          entry_valid, export_en, flow_index,
                          hash1, hint1, hash2, hint2, hash3, hint3,
                          hash4, hint4, hash5, hint5, hash6, hint6,
                          more_hashs, more_hints) {
    // remove the recirc header
    remove_header(recirc_header);

    // check if the key matches the entry data
    if ((lkp_inst == flow_lkp_metadata.lkp_inst) and
        (lkp_dir == flow_lkp_metadata.lkp_dir) and
        (lkp_type == flow_lkp_metadata.lkp_type) and
        (lkp_vrf == flow_lkp_metadata.lkp_vrf) and
        (lkp_src == flow_lkp_metadata.lkp_src) and
        (lkp_dst == flow_lkp_metadata.lkp_dst) and
        (lkp_proto == flow_lkp_metadata.lkp_proto) and
        (lkp_sport == flow_lkp_metadata.lkp_sport) and
        (lkp_dport == flow_lkp_metadata.lkp_dport)) {
        modify_field(flow_info_metadata.flow_index, flow_index);
        modify_field(rewrite_metadata.entropy_hash,
                     scratch_metadata.entropy_hash);
    } else {
        modify_field(control_metadata.flow_miss, TRUE);
        modify_field(control_metadata.flow_miss_ingress, TRUE);
        modify_field(flow_info_metadata.flow_index, 0);
    }

    // pack fields into control_metadata.lkp_flags_egress to transfer to egress
    modify_field(scratch_metadata.cpu_flags, 0);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_dir << 7);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_inst << 6);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_type << 5);
    modify_field(control_metadata.lkp_flags_egress, scratch_metadata.cpu_flags);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.lkp_inst, lkp_inst);
    modify_field(scratch_metadata.lkp_dir, lkp_dir);
    modify_field(scratch_metadata.lkp_type, lkp_type);
    modify_field(scratch_metadata.lkp_vrf, lkp_vrf);
    modify_field(scratch_metadata.lkp_src, lkp_src);
    modify_field(scratch_metadata.lkp_dst, lkp_dst);
    modify_field(scratch_metadata.lkp_proto, lkp_proto);
    modify_field(scratch_metadata.lkp_sport, lkp_sport);
    modify_field(scratch_metadata.lkp_dport, lkp_dport);
    modify_field(scratch_metadata.entry_valid, entry_valid);
    modify_field(scratch_metadata.export_en, export_en);
    modify_field(scratch_metadata.flow_hash1, hash1);
    modify_field(scratch_metadata.flow_hash2, hash2);
    modify_field(scratch_metadata.flow_hash3, hash3);
    modify_field(scratch_metadata.flow_hash4, hash4);
    modify_field(scratch_metadata.flow_hash5, hash5);
    modify_field(scratch_metadata.flow_hash6, hash6);
    modify_field(scratch_metadata.flow_hint1, hint1);
    modify_field(scratch_metadata.flow_hint2, hint2);
    modify_field(scratch_metadata.flow_hint3, hint3);
    modify_field(scratch_metadata.flow_hint4, hint4);
    modify_field(scratch_metadata.flow_hint5, hint5);
    modify_field(scratch_metadata.flow_hint6, hint6);
    modify_field(scratch_metadata.more_hashs, more_hashs);
    modify_field(scratch_metadata.more_hints, more_hints);

    // resolve hint and pick next hash for recirc path
    if ((control_metadata.flow_miss == TRUE) and
        (hash1 == scratch_metadata.flow_hash1)) {
        recirc_packet(RECIRC_FLOW_HASH_OVERFLOW);
        // modify_field(recirc_header.overflow_entry_index, hint1);
    }
}

@pragma stage 1
@pragma hbm_table
@pragma overflow_table flow_hash
table flow_hash_overflow {
    reads {
        recirc_header.overflow_entry_index : exact;
    }
    actions {
        flow_hash_info; // Use the same function as hash table.. need to fix the asm code
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

control process_flow_table {
    // NCC-predication (currently) does not allow sourcing condition bits from headers
    // If a header can be placed in first flit, this problem can be fixed XXX
    if (control_metadata.nic_mode == NIC_MODE_SMART) {
        if (valid(recirc_header) and
            (control_metadata.recirc_reason == RECIRC_FLOW_HASH_OVERFLOW)) {
            apply(flow_hash_overflow);
        } else {
            apply(flow_hash);
        }
    }
    if (control_metadata.nic_mode == NIC_MODE_SMART) {
        apply(flow_info);
    }
}
