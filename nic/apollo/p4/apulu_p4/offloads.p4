action offloads() {
    // update IP length
    if (p4plus_to_p4.update_ip_len == TRUE) {
        if (ctag_1.valid == TRUE) {
            subtract(scratch_metadata.packet_len,
                     capri_p4_intrinsic.packet_len, 18);
        } else {
            subtract(scratch_metadata.packet_len,
                     capri_p4_intrinsic.packet_len, 14);
        }

        if (ipv4_1.valid == TRUE) {
            modify_field(ipv4_1.totalLen, scratch_metadata.packet_len);
            subtract_from_field(scratch_metadata.packet_len, ipv4_1.ihl << 2);
            modify_field(scratch_metadata.update_ip_chksum, TRUE);
        } else {
            if (ipv6_1.valid == TRUE) {
                subtract_from_field(scratch_metadata.packet_len, 40);
                modify_field(ipv6_1.payloadLen, scratch_metadata.packet_len);
            }
        }
    } else {
        if (ipv4_1.valid == TRUE) {
            subtract(scratch_metadata.packet_len, ipv4_1.totalLen,
                     ipv4_1.ihl << 2);
        } else {
            if (ipv6_1.valid == TRUE) {
                modify_field(scratch_metadata.packet_len, ipv6_1.payloadLen);
            }
        }
    }

    // update L4 length
    if (udp_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.packet_len,
                            p4plus_to_p4.udp_opt_bytes);
    }

    // update IP id
    if (p4plus_to_p4.update_ip_id == TRUE) {
        add(ipv4_1.identification, ipv4_1.identification,
            p4plus_to_p4.ip_id_delta);
        modify_field(scratch_metadata.update_ip_chksum, TRUE);
    }

    // update UDP length
    if (p4plus_to_p4.update_udp_len == 1) {
        modify_field(udp_1.len, scratch_metadata.packet_len);
        modify_field(scratch_metadata.update_l4_chksum, TRUE);
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

    // update checksum compute flags
    if (p4plus_to_p4.p4plus_app_id == P4PLUS_APPTYPE_CLASSIC_NIC) {
        if (p4plus_to_p4.compute_ip_csum == 1) {
            if (ipv4_1.valid == TRUE) {
                modify_field(scratch_metadata.update_ip_chksum, TRUE);
            }
        }
        if (p4plus_to_p4.compute_l4_csum == 1) {
            modify_field(scratch_metadata.update_l4_chksum, TRUE);
        }
    } else {
        modify_field(scratch_metadata.update_ip_chksum, TRUE);
        modify_field(scratch_metadata.update_l4_chksum, TRUE);
    }

    if ((scratch_metadata.update_ip_chksum == TRUE) or
        (scratch_metadata.update_l4_chksum == TRUE)) {
        modify_field(p4i_i2e.update_checksum, TRUE);
    }
}

@pragma stage 4
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
