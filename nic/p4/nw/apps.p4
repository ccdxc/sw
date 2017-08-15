/*****************************************************************************/
/* P4+ APP related processing                                                */
/*****************************************************************************/
action p4plus_app_tcp_proxy() {
    remove_header(ethernet);
    remove_header(vlan_tag);
    remove_header(ipv4);
    remove_header(ipv6);
    remove_header(tcp);

    add_header(capri_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    add_header(p4_to_p4plus_tcp_proxy);
    add_header(p4_to_p4plus_tcp_proxy_sack);
    modify_field(p4_to_p4plus_tcp_proxy.payload_len, l4_metadata.tcp_data_len);
    modify_field(p4_to_p4plus_tcp_proxy.p4plus_app_id, control_metadata.p4plus_app_id);
    if (tcp_option_one_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 1);
    }
    if (tcp_option_two_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 2);
    }
    if (tcp_option_three_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 3);
    }
    if (tcp_option_four_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 4);
    }

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_TCP_PROXY_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_classic_nic() {
    add_header(p4_to_p4plus_header);
    modify_field(p4_to_p4plus_header.p4plus_app_id, control_metadata.p4plus_app_id);

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CLASSIC_NIC_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_ipsec() {
}

@pragma stage 4
table p4plus_app {
    reads {
        control_metadata.p4plus_app_id : exact;
    }
    actions {
        p4plus_app_classic_nic;
        p4plus_app_tcp_proxy;
        p4plus_app_ipsec;
        nop;
    }
    size : P4PLUS_APP_TABLE_SIZE;
}
