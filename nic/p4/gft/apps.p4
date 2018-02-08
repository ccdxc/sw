/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
control rx_apps {
    rx_roce();
}

/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
action tx_fixup() {
    // update IP id
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID) != 0) {
        add(ipv4_1.identification, ipv4_1.identification,
            p4plus_to_p4.ip_id_delta);
    }

    // update IP length
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN) != 0) {
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
        } else {
            if (ipv6_1.valid == TRUE) {
                subtract_from_field(scratch_metadata.packet_len, 40);
                modify_field(ipv6_1.payloadLen, scratch_metadata.packet_len);
            }
        }
    }

    // update TCP sequence number
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO) != 0) {
        add(tcp_1.seqNo, tcp_1.seqNo, p4plus_to_p4.tcp_seq_delta);
    }

    // update UDP length
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN) != 0) {
        subtract(udp_1.len, scratch_metadata.packet_len,
                 p4plus_to_p4.udp_opt_bytes);
    }

    // update checksum/icrc compute flags
    if (ipv4_1.valid == TRUE) {
        // compute ipv4 checksum
    }
    if (p4plus_to_p4.p4plus_app_id == P4PLUS_APPTYPE_RDMA) {
        // compute icrc
    } else {
        if ((udp_1.valid == TRUE) or (tcp_1.valid == TRUE)) {
            // compute l4 checksum
        }
    }

    remove_header(p4plus_to_p4);
    remove_header(capri_txdma_intrinsic);
}

@pragma stage 0
table tx_fixup {
    actions {
         tx_fixup;
    }
    default_action : tx_fixup;
}

control tx_apps {
    apply(tx_fixup);
}
