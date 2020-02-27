/*****************************************************************************/
/* Flow table processing                                                     */
/*****************************************************************************/
header_type flow_lkp_metadata_t {
    fields {
        lkp_inst         : 1;
        lkp_type         : 4;
        ipv4_hlen        : 4;          // For Normalization
        lkp_vrf          : 16;
        lkp_reg_mac_vrf  : 16;
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
        flow_index               : 20; // flow info table index and for P4+ pipeline
        session_state_index      : 19; // index to iflow and rflow state
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

    modify_field(control_metadata.flow_miss_ingress, TRUE);
    modify_field(control_metadata.i2e_flags, (1 << P4_I2E_FLAGS_FLOW_MISS),
                 (1 << P4_I2E_FLAGS_FLOW_MISS));
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
        if (control_metadata.registered_mac_miss == TRUE) {
            // Miss
            if (control_metadata.flow_learn == FALSE) {
                // Prom Mgmt & Prom MSeg/BM
                modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                add_to_field(capri_intrinsic.tm_replicate_ptr, 8);
            } else {
                if (l4_metadata.policy_enf_cfg_en == TRUE) {
                    // Sup Redirect
                    // modify_field(control_metadata.dst_lport, CPU_LPORT);
                    // modify_field(control_metadata.cpu_copy, TRUE);
                    // modify_field(rewrite_metadata.tunnel_rewrite_index, 0);
                    // modify_field(rewrite_metadata.rewrite_index, 0);
                    // modify_field(qos_metadata.qos_class_id, control_metadata.flow_miss_qos_class_id);
                    // Prom Mgmt
                    modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                    add_to_field(capri_intrinsic.tm_replicate_ptr, 2);
                    if (control_metadata.uuc_fl_pe_sup_en == TRUE) {
                        // Sup Copy
                        modify_field(capri_intrinsic.tm_cpu, TRUE);
                    } else {
                        // Do we need to explicitly DROP ?
                    }
                } else {
                    if (l4_metadata.flow_learn_cfg_en == TRUE) {
                        // Prom Mgmt & Prom MSeg/BM
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 8);
                        if (control_metadata.uuc_fl_pe_sup_en == TRUE) {
                            // Sup Copy
                            modify_field(capri_intrinsic.tm_cpu, TRUE);
                        }
                    } else {
                        // Prom Mgmt & Prom MSeg/BM
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 8);
                    }
                }
            }
        } else {
            // Reg Mac Hit
            if (control_metadata.flow_learn == TRUE) {
                if (l4_metadata.policy_enf_cfg_en == TRUE) {
                    // Sup Redirect
                    modify_field(control_metadata.dst_lport, CPU_LPORT);
                    modify_field(control_metadata.cpu_copy, TRUE);
                    modify_field(rewrite_metadata.tunnel_rewrite_index, 0);
                    modify_field(rewrite_metadata.rewrite_index, 0);
                    modify_field(qos_metadata.qos_class_id, control_metadata.flow_miss_qos_class_id);
                } else {
                    if (l4_metadata.flow_learn_cfg_en == TRUE) {
                        // Sup Copy
                        modify_field(capri_intrinsic.tm_cpu, TRUE);
                    }
                }
            }
        }
    } else {
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
            if (control_metadata.registered_mac_miss == FALSE) {
                // Reg. Mac Hit
                if ((control_metadata.flow_learn == FALSE) or
                    (l4_metadata.ip_bm_mc_policy_enf_cfg_en == FALSE) or
                    (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC)) {
                    // Mcast Address's Mgmt & MSeg/BM Repls. tm_repl_ptr is from reg_mac
                    modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                    add_to_field(capri_intrinsic.tm_replicate_ptr, 2);
                } else {
                    if (l4_metadata.policy_enf_cfg_en == TRUE) {
                        // Sup copy. Mgmt Repls. tm_repl_ptr is from reg_mac
                        modify_field(capri_intrinsic.tm_cpu, TRUE);
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                    } else {
                        if (l4_metadata.flow_learn_cfg_en == TRUE) {
                            // Sup copy. Mcast Address's Mgmt & MSeg/BM Repls. tm_repl_ptr is from reg_mac
                            modify_field(capri_intrinsic.tm_cpu, TRUE);
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 2);
                        } else {
                            // No Sup copy. Mcast Address's Mgmt & MSeg/BM Repls. tm_repl_ptr is from reg_mac
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 2);
                        }
                    }
                }
            } else {
                // Reg. Mac Miss
                if ((control_metadata.flow_learn == FALSE) or
                    (l4_metadata.ip_bm_mc_policy_enf_cfg_en == FALSE) or
                    (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC)) {
                    modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                    add_to_field(capri_intrinsic.tm_replicate_ptr, 7);
                } else {
                    if (l4_metadata.policy_enf_cfg_en == TRUE) {
                        // Sup Copy. ALL_MC Mgmt
                        modify_field(capri_intrinsic.tm_cpu, TRUE);
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 1);
                    } else {
                        if (l4_metadata.flow_learn_cfg_en == TRUE) {
                            // Sup copy. ALL_MC Mgmt & MSeg/BM Repls
                            modify_field(capri_intrinsic.tm_cpu, TRUE);
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 7);
                        } else {
                            // No Sup copy. ALL_MC Mgmt & MSeg/BM Repls
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 7);
                        }
                    }
                }
            }
        } else {
            // PACKET_TYPE_BROADCAST
            if ((control_metadata.flow_learn == FALSE) or
                (l4_metadata.ip_bm_mc_policy_enf_cfg_en == FALSE) or
                (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC)) {
                modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                add_to_field(capri_intrinsic.tm_replicate_ptr, 6);
            } else {
                if (l4_metadata.policy_enf_cfg_en == TRUE) {
                    // Sup copy. Mgmt Repls.
                    modify_field(capri_intrinsic.tm_cpu, TRUE);
                    modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                } else {
                    if (l4_metadata.flow_learn_cfg_en == TRUE) {
                        // Sup Copy. BC Mgmt & MSeg/BM BC Repls
                        modify_field(capri_intrinsic.tm_cpu, TRUE);
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 6);
                    } else {
                        // BC Mgmt
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 6);
                    }
                }
            }
        }
    }
}

action flow_hit_drop(start_timestamp, mirror_on_drop_overwrite,
                     mirror_on_drop_en, mirror_on_drop_session_id) {
    /* mirror on drop */
    if (mirror_on_drop_overwrite == TRUE) {
        modify_field(control_metadata.mirror_on_drop_en, mirror_on_drop_en);
        modify_field(control_metadata.mirror_on_drop_session_id,
                     mirror_on_drop_session_id);
    }

    /* set the drop flag */
    modify_field(control_metadata.drop_reason, DROP_FLOW_HIT);
    drop_packet();

    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.flag, mirror_on_drop_overwrite);
    modify_field(scratch_metadata.flow_start_timestamp, start_timestamp);
}

action flow_info(flow_only_policy, dst_lport_en, dst_lport,
                 multicast_ptr, multicast_en, ingress_mirror_overwrite,
                 ingress_mirror_session_id, egress_mirror_session_id,
                 mirror_on_drop_overwrite, mirror_on_drop_en,
                 mirror_on_drop_session_id,
                 rewrite_index, tunnel_en, tunnel_rewrite_index, tunnel_vnid,
                 tunnel_originate, qid_en, log_en, rewrite_flags,
                 flow_conn_track, flow_ttl, flow_role,
                 session_state_index, start_timestamp,
                 qos_class_en, qos_class_id,
                 export_id1, export_id2, export_id3, export_id4) {

    if (multicast_en == TRUE) {
        modify_field(capri_intrinsic.tm_replicate_en, multicast_en);
        modify_field(capri_intrinsic.tm_replicate_ptr, multicast_ptr);
    } else {
        if (flow_only_policy == TRUE) {
            if (control_metadata.skip_flow_update == TRUE) {
                modify_field(l4_metadata.flow_conn_track, FALSE);
                // return;
            }
            if (control_metadata.from_cpu == TRUE) {
                if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
                    if (control_metadata.registered_mac_miss == TRUE) {
                        // Prom MSeg/BM Repls
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 5);
                    }
                } else {
                    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
                        if (control_metadata.registered_mac_miss == TRUE) {
                            // MSeg Repls. tm_repl_ptr from inp_props
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 4);
                        } else {
                            // MSeg/BM Repls. tm_repl_ptr from reg_mac
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 1);
                        }
                    } else {
                        // MSeg/BM Repls
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 3);
                    }
                }
            } else {
                if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
                    if (control_metadata.registered_mac_miss == TRUE) {
                        // Prom Mgmt + Prom MSeg/BM Repls
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 8);
                    }
                } else {
                    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
                        if (control_metadata.registered_mac_miss == TRUE) {
                            // MC Mgmt + MSeg Repls. tm_repl_ptr from inp_props 
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 7);
                        } else {
                            // MC Mgmt + MSeg/BM Mcast Repls. tm_repl_ptr from reg_mac
                            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                            add_to_field(capri_intrinsic.tm_replicate_ptr, 2);
                        }
                    } else {
                        // BC Mgmt + MSeg/BM Repls
                        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                        add_to_field(capri_intrinsic.tm_replicate_ptr, 6);
                    }
                }
            }
        }
    }

    if (dst_lport_en == TRUE) {
        modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
        modify_field(control_metadata.dst_lport, dst_lport);
    }

    if (qos_class_en == TRUE) {
        modify_field(qos_metadata.qos_class_id, qos_class_id);
    }

    /* mirror session id */
    bit_or(capri_intrinsic.tm_span_session, capri_intrinsic.tm_span_session,
           ingress_mirror_session_id);

    modify_field(control_metadata.egress_mirror_session_id,
                 egress_mirror_session_id);
    if (mirror_on_drop_overwrite == TRUE) {
        modify_field(control_metadata.mirror_on_drop_en, mirror_on_drop_en);
        modify_field(control_metadata.mirror_on_drop_session_id,
                     mirror_on_drop_session_id);
    }

    /* logging - need to create a copy */
    if (log_en == TRUE) {
        modify_field(capri_intrinsic.tm_cpu, TRUE);
    }

    /* flow info */
    modify_field(flow_info_metadata.session_state_index, session_state_index);
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

    /* tunnel info */
    if (tunnel_en == TRUE) {
        modify_field(tunnel_metadata.tunnel_originate, tunnel_originate);
        modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
        modify_field(rewrite_metadata.tunnel_vnid, tunnel_vnid);
    }

    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.flag, capri_intrinsic.tm_replicate_en);
    modify_field(scratch_metadata.tm_replicate_ptr, capri_intrinsic.tm_replicate_ptr);
    modify_field(scratch_metadata.flag, flow_only_policy);
    modify_field(scratch_metadata.flag, dst_lport_en);
    modify_field(scratch_metadata.flag, ingress_mirror_overwrite);
    modify_field(scratch_metadata.flag, mirror_on_drop_overwrite);
    modify_field(scratch_metadata.flag, tunnel_en);
    modify_field(scratch_metadata.flow_start_timestamp, start_timestamp);
    modify_field(scratch_metadata.qid_en, qid_en);
    modify_field(scratch_metadata.qos_class_en, qos_class_en);
    modify_field(scratch_metadata.log_en, log_en);
    modify_field(scratch_metadata.ttl, flow_ttl);
    modify_field(scratch_metadata.export_id, export_id1);
    modify_field(scratch_metadata.export_id, export_id2);
    modify_field(scratch_metadata.export_id, export_id3);
    modify_field(scratch_metadata.export_id, export_id4);
}

// We can add new parameters as we need. For now if none of the
// rewrite fields are set that measn the packet will not be
// modified.
action flow_info_from_cpu() {
    if (p4plus_to_p4.dst_lport_valid == TRUE) {
        modify_field(control_metadata.dst_lport, p4plus_to_p4.dst_lport);
    }
}

action recirc_packet(recirc_reason) {
    modify_field(control_metadata.ingress_bypass, 1);
    add_header(capri_p4_intrinsic);
    add_header(recirc_header);
    modify_field(recirc_header.src_tm_iport, control_metadata.tm_iport);
    modify_field(recirc_header.reason, recirc_reason);
    /* At this stage, the qos_class_id will be same as the original
     * tm_oq that the pkt entered with. It is possible that later
     * after a flow-hit, in the flow_info table the qos_class_id
     * gets a new value. But at this point, it is safe to use
     */
    modify_field(capri_intrinsic.tm_iq, qos_metadata.qos_class_id);

    /*
     * reset individual options (tcp and ip) validity bits - the blob headers
     * are still valid. this is implemented in ASM code
     */
}

action flow_hit_from_vm_bounce(src_lif) {
    remove_header(ethernet);
    remove_header(ipv4);
    //remove_header(udp);
    remove_header(vxlan_gpe);
    recirc_packet(RECIRC_VM_BOUNCE);
    modify_field(capri_intrinsic.lif, src_lif);
}

action flow_hit_to_vm_bounce(dst_lport, qos_class_id) {
    remove_header(ethernet);
    remove_header(ipv4);
    //remove_header(udp);
    remove_header(vxlan_gpe);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(qos_metadata.qos_class_id, qos_class_id);
}

@pragma stage 3
@pragma hbm_table
@pragma numthreads 2
@pragma capi_bitfields_struct
table flow_info {
    reads {
        flow_info_metadata.flow_index : exact;
    }
    actions {
        nop;
        flow_info;
        flow_info_from_cpu;
        flow_miss;
        flow_hit_drop;
        flow_hit_from_vm_bounce;
        flow_hit_to_vm_bounce;
    }
    default_action : flow_miss;
    size : FLOW_TABLE_SIZE;
}

@pragma capi appdatafields flow_index export_en
@pragma capi hwfields_access_api
action flow_hash_info(entry_valid, export_en,
                      flow_index, pad4, hash1, hint1, hash2, hint2,
                      hash3, hint3, hash4, hint4, hash5, hint5,
                      more_hashes, more_hints) {
    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.entry_valid, entry_valid);
    modify_field(scratch_metadata.export_en, export_en);
    modify_field(scratch_metadata.pad4, pad4);
    modify_field(scratch_metadata.flow_hash1, hash1);
    modify_field(scratch_metadata.flow_hash2, hash2);
    modify_field(scratch_metadata.flow_hash3, hash3);
    modify_field(scratch_metadata.flow_hash4, hash4);
    modify_field(scratch_metadata.flow_hash5, hash5);
    modify_field(scratch_metadata.flow_hint1, hint1);
    modify_field(scratch_metadata.flow_hint2, hint2);
    modify_field(scratch_metadata.flow_hint3, hint3);
    modify_field(scratch_metadata.flow_hint4, hint4);
    modify_field(scratch_metadata.flow_hint5, hint5);
    modify_field(scratch_metadata.more_hashes, more_hashes);
    modify_field(scratch_metadata.more_hints, more_hints);

    modify_field(rewrite_metadata.entropy_hash, scratch_metadata.entropy_hash);
    // if (control_metadata.nic_mode != NIC_MODE_SMART) {
    //     //return;
    // }

    // If register C1 indicates hit, take flow_index. Otherwise walk
    // through the hints to pick the overflow entry
    modify_field(flow_info_metadata.flow_index, flow_index);

    // pack fields into control_metadata.lkp_flags_egress to transfer to egress
    modify_field(scratch_metadata.cpu_flags, flow_lkp_metadata.lkp_type);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_dir << 2);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           flow_lkp_metadata.lkp_inst << 1);
    modify_field(control_metadata.lkp_flags_egress, scratch_metadata.cpu_flags);

    // no further matches
    if ((hash1 == 0) and (hint1 == 0)) {
        modify_field(control_metadata.i2e_flags, (1 << P4_I2E_FLAGS_FLOW_MISS),
                     (1 << P4_I2E_FLAGS_FLOW_MISS));
        modify_field(control_metadata.flow_miss_ingress, TRUE);
        modify_field(flow_info_metadata.flow_index, 0);
    }

    // resolve hint and pick next hash for recirc path
    if (hash1 == scratch_metadata.flow_hash1) {
        recirc_packet(RECIRC_FLOW_HASH_OVERFLOW);
        // do not assign it to larger variable.. can result in K+D violation
        //modify_field(recirc_header.overflow_entry_index, hint1);
    }
}

@pragma stage 2
@pragma hbm_table
@pragma hash_type 0
@pragma include_k_in_d
@pragma capi_bitfields_struct
table flow_hash {
    reads {
        flow_lkp_metadata.lkp_inst  : exact;
        // flow_lkp_metadata.lkp_dir   : exact;
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

@pragma stage 2
@pragma hbm_table
@pragma overflow_table flow_hash
table flow_hash_overflow {
    reads {
        recirc_header.overflow_entry_index : exact;
    }
    actions {
        flow_hash_info; // Use the same function as hash table
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

control process_flow_table {
    if (control_metadata.skip_flow_lkp == FALSE) {
        if (valid(recirc_header) and
            (control_metadata.recirc_reason == RECIRC_FLOW_HASH_OVERFLOW)) {
            apply(flow_hash_overflow);
        } else {
            apply(flow_hash);
        }
    }
    apply(flow_info);
}
