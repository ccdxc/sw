/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
action rx_checksum() {
    if (control_metadata.update_checksum_1 == TRUE) {
        if (ipv4_1.valid == TRUE) {
        }
        if (udp_1.valid == TRUE) {
            if (ipv4_1.valid == TRUE) {
            }
            if (ipv6_1.valid == TRUE) {
            }
            modify_field(capri_deparser_len.rx_l4_payload_len, udp_1.len);
        }
        if (tcp_1.valid == TRUE) {
            if (ipv4_1.valid == TRUE) {
                modify_field(capri_deparser_len.rx_l4_payload_len,
                             (ipv4_1.totalLen - (ipv4_1.ihl << 2)));
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(capri_deparser_len.rx_l4_payload_len,
                             ipv6_1.payloadLen);
            }
        }
    }

    if (control_metadata.update_checksum_2 == TRUE) {
        if (ipv4_2.valid == TRUE) {
        }
        if (udp_2.valid == TRUE) {
            if (ipv4_2.valid == TRUE) {
            }
            if (ipv6_2.valid == TRUE) {
            }
            modify_field(capri_deparser_len.rx_l4_payload_len, udp_2.len);
        }
        if (tcp_2.valid == TRUE) {
            if (ipv4_2.valid == TRUE) {
                modify_field(capri_deparser_len.rx_l4_payload_len,
                             (ipv4_2.totalLen - (ipv4_2.ihl << 2)));
            }
            if (ipv6_2.valid == TRUE) {
                modify_field(capri_deparser_len.rx_l4_payload_len,
                             ipv6_2.payloadLen);
            }
        }
    }

    if (control_metadata.update_checksum_3 == TRUE) {
        if (ipv4_3.valid == TRUE) {
        }
        if (udp_3.valid == TRUE) {
            if (ipv4_3.valid == TRUE) {
            }
            if (ipv6_3.valid == TRUE) {
            }
            modify_field(capri_deparser_len.rx_l4_payload_len, udp_3.len);
        }
        if (tcp_3.valid == TRUE) {
            if (ipv4_3.valid == TRUE) {
                modify_field(capri_deparser_len.rx_l4_payload_len,
                             (ipv4_3.totalLen - (ipv4_3.ihl << 2)));
            }
            if (ipv6_3.valid == TRUE) {
                modify_field(capri_deparser_len.rx_l4_payload_len,
                             ipv6_3.payloadLen);
            }
        }
    }
}

@pragma stage 5
table rx_checksum {
    actions {
        rx_checksum;
    }
    default_action : rx_checksum;
}

control rx_checksum {
    apply(rx_checksum);
}
