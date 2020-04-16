action offloads() {
    // update IP length
    if (p4plus_to_p4.update_ip_len == TRUE) {
        if (ipv4_1.valid == TRUE) {
            modify_field(ipv4_1.totalLen,
                (capri_p4_intrinsic.frame_size - offset_metadata.l3_1));
        }
        if (ipv6_1.valid == TRUE) {
            modify_field(ipv6_1.payloadLen,
                (capri_p4_intrinsic.frame_size - (offset_metadata.l3_1 + 40)));
        }
        if (udp_1.valid == TRUE) {
            modify_field(udp_1.len,
                (capri_p4_intrinsic.frame_size - offset_metadata.l4_1));
        }
        if (ipv4_2.valid == TRUE) {
            modify_field(ipv4_2.totalLen,
                (capri_p4_intrinsic.frame_size - offset_metadata.l3_2));
        }
        if (ipv6_2.valid == TRUE) {
            modify_field(ipv6_2.payloadLen,
                (capri_p4_intrinsic.frame_size - (offset_metadata.l3_2 + 40)));
        }
        if (udp_2.valid == TRUE) {
            modify_field(udp_2.len,
                (capri_p4_intrinsic.frame_size - offset_metadata.l4_2));
        }
    }

    // update IP id
    if (p4plus_to_p4.update_ip_id == TRUE) {
        add(ipv4_1.identification, ipv4_1.identification,
            p4plus_to_p4.ip_id_delta);
        modify_field(scratch_metadata.update_ip_chksum, TRUE);
    }

    // update TCP sequence number
    if (p4plus_to_p4.update_tcp_seq_no == TRUE) {
        add(tcp.seqNo, tcp.seqNo, p4plus_to_p4.tcp_seq_delta);
        modify_field(scratch_metadata.update_l4_chksum, TRUE);
    }

    // tso
    if (p4plus_to_p4.tso_valid == TRUE) {
        if (p4plus_to_p4.tso_first_segment != TRUE) {
            // reset CWR bit
            modify_field(tcp.flags, 0, TCP_FLAG_CWR);
            modify_field(scratch_metadata.update_l4_chksum, TRUE);
        }
        if (p4plus_to_p4.tso_last_segment != TRUE) {
            // reset FIN and PSH bits
            modify_field(tcp.flags, 0, (TCP_FLAG_FIN|TCP_FLAG_PSH));
            modify_field(scratch_metadata.update_l4_chksum, TRUE);
        }
    }

    // insert vlan tag
    if (p4plus_to_p4.insert_vlan_tag == TRUE) {
        add_header(ctag_1);
        modify_field(ctag_1.pcp, p4plus_to_p4_vlan.pcp);
        modify_field(ctag_1.dei, p4plus_to_p4_vlan.dei);
        modify_field(ctag_1.vid, p4plus_to_p4_vlan.vid);
        modify_field(ctag_1.etherType, ethernet_1.etherType);
        modify_field(ethernet_1.etherType, ETHERTYPE_VLAN);
        add(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 4);
    }

    // copy gso valid to capri_gso_csum.gso
    modify_field(scratch_metadata.flag, p4plus_to_p4.gso_valid);

    // outer checksums (computed in egress pipeline)
    if ((p4plus_to_p4.compute_ip_csum == 1) or
        (p4plus_to_p4.compute_l4_csum == 1)) {
        modify_field(p4i_i2e.update_checksum, TRUE);
    }

    // inner checksums (computed in ingress pipeline)
    if (p4plus_to_p4.compute_inner_ip_csum == 1) {
        modify_field(capri_deparser_len.ipv4_2_hdr_len, ipv4_2.ihl << 2);
    }
    if (p4plus_to_p4.compute_inner_l4_csum == 1) {
        modify_field(capri_deparser_len.l4_payload_len,
                     (capri_p4_intrinsic.frame_size - offset_metadata.l4_2));
        // checksum bits to be set in ASM based on header valid bits
        modify_field(scratch_metadata.flag, ipv4_2.valid);
        modify_field(scratch_metadata.flag, ipv6_2.valid);
        modify_field(scratch_metadata.flag, udp_2.valid);
        modify_field(scratch_metadata.flag, tcp.valid);
    }
}

@pragma stage 3
table offloads {
    actions {
        offloads;
    }
    default_action : offloads;
}

control offloads {
    if ((p4plus_to_p4.valid == TRUE) and (arm_to_p4i.valid == FALSE)) {
        apply(offloads);
    }
}
