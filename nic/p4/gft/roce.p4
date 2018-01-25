/*****************************************************************************/
/* ROCEv2 processing                                                         */
/*****************************************************************************/
action rx_roce(raw_flags, len, qtype, tm_oq_overwrite, tm_oq) {
    add_header(p4_to_p4plus_roce);
    modify_field(p4_to_p4plus_roce.p4plus_app_id, P4PLUS_APPTYPE_RDMA);
    modify_field(p4_to_p4plus_roce.raw_flags, raw_flags);
    modify_field(p4_to_p4plus_roce.rdma_hdr_len, len);
    if (icrc.valid == TRUE) {
        modify_field(p4_to_p4plus_roce.payload_len,
                     (roce_metadata.udp_len - 12 - len));
    } else {
        modify_field(p4_to_p4plus_roce.payload_len,
                     (roce_metadata.udp_len - 8 - len));
    }
    modify_field(p4_to_p4plus_roce.ecn, roce_metadata.ecn);

    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.qtype, qtype);
    add(capri_rxdma_intrinsic.rx_splitter_offset, len,
        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
         P4PLUS_ROCE_HDR_SZ));
    if ((roce_bth.opCode & 0xE0) == 0x60) {
        add_to_field(capri_rxdma_intrinsic.rx_splitter_offset, 14);
        add_to_field(p4_to_p4plus_roce.payload_len, 40);
        if (ethernet_3.valid == TRUE) {
            add_header(p4_to_p4plus_roce_eth_3);
            add_header(p4_to_p4plus_roce_ip_3);
        } else {
            if (ethernet_2.valid == TRUE) {
                add_header(p4_to_p4plus_roce_eth_2);
                add_header(p4_to_p4plus_roce_ip_2);
            } else {
                add_header(p4_to_p4plus_roce_eth_1);
                add_header(p4_to_p4plus_roce_ip_1);
            }
        }
    }

    if (tm_oq_overwrite == TRUE) {
        modify_field(capri_intrinsic.tm_oq, tm_oq);
    }

    if ((udp_opt_ocs.valid == FALSE) or
        ((udp_opt_ocs.valid == TRUE) and ((capri_intrinsic.csum_err &
                                           (1 << CSUM_HDR_UDP_OPT_OCS)) == 0))) {
        if (udp_opt_timestamp.valid == TRUE) {
            modify_field(p4_to_p4plus_roce.roce_opt_ts_valid, TRUE);
        }
        if (udp_opt_mss.valid == TRUE) {
            modify_field(p4_to_p4plus_roce.roce_opt_mss_valid, TRUE);
        }
    }

#if 0
    remove_header(ethernet_1);
    remove_header(ctag_1);
    remove_header(ipv4_1);
    remove_header(ipv6_1);
    remove_header(vxlan_1);
    remove_header(gre_1);
    remove_header(udp_1);

    remove_header(ethernet_2);
    remove_header(ctag_2);
    remove_header(ipv4_2);
    remove_header(ipv6_2);
    remove_header(vxlan_2);
    remove_header(gre_2);
    remove_header(udp_2);

    remove_header(ethernet_3);
    remove_header(ctag_3);
    remove_header(ipv4_3);
    remove_header(ipv6_3);
    remove_header(vxlan_3);
    remove_header(gre_3);
    remove_header(udp_3);
#endif

    remove_header(icrc);
    remove_header(udp_opt_eol);
    remove_header(udp_opt_nop);
    remove_header(udp_opt_ocs);
    remove_header(udp_opt_mss);

    modify_field(scratch_metadata.flag, tm_oq_overwrite);
}

@pragma stage 5
table rx_roce {
    reads {
        roce_bth.opCode : exact;
    }
    actions {
        nop;
        rx_roce;
    }
    default_action : nop;
    size : ROCE_TABLE_SIZE;
}

control rx_roce {
    if ((roce_metadata.roce_valid == TRUE) and
        (roce_metadata.rdma_enabled == TRUE)) {
        apply(rx_roce);
    }
}
