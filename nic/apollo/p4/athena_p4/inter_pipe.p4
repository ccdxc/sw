/*****************************************************************************/
/* Inter pipe : ingress pipeline                                             */
/*****************************************************************************/



action p4i_to_p4e() {
    if (ingress_recirc_header.flow_done != TRUE) {
        /* Recirc back to P4I */
        add_header(ingress_recirc_header);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
    }
    else {
        /* To P4E */
        remove_header(ingress_recirc_header);
        add_header(p4i_to_p4e_header);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    }
}



@pragma stage 5
table p4i_to_p4e {
    actions {
        p4i_to_p4e;
    }
}

control ingress_inter_pipe {
    apply(p4i_to_p4e);
}


/*****************************************************************************/
/* Inter pipe : egress pipeline                                              */
/*****************************************************************************/

action p4e_to_uplink() {
    modify_field(capri_intrinsic.tm_oport, p4i_to_p4e_header.nacl_redir_oport);
    modify_field(capri_intrinsic.lif, p4i_to_p4e_header.nacl_redir_lif); /* TODO: Is LIF required for uplinks? */
}

@pragma stage 5
table p4e_to_uplink {
    actions {
        p4e_to_uplink;
    }
}


action p4e_to_rxdma() {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.lif, p4i_to_p4e_header.nacl_redir_lif);
    add_header(capri_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.qid, p4i_to_p4e_header.nacl_redir_qid);
    modify_field(capri_rxdma_intrinsic.qtype, p4i_to_p4e_header.nacl_redir_qtype);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
            CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ +
            CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_CLASSIC_NIC_HDR_SZ);
    add_header(p4e_to_p4plus_classic_nic);
    modify_field(p4e_to_p4plus_classic_nic.p4plus_app_id, p4i_to_p4e_header.nacl_redir_app_id);
    add_header(p4e_to_p4plus_classic_nic_ip);

    modify_field(p4e_to_p4plus_classic_nic.packet_len, capri_p4_intrinsic.packet_len);

    if (ipv4_1.valid == TRUE) {
        modify_field(p4e_to_p4plus_classic_nic_ip.ip_sa, ipv4_1.srcAddr);
        modify_field(p4e_to_p4plus_classic_nic_ip.ip_da, ipv4_1.dstAddr);
        if (tcp.valid == TRUE) {
            modify_field(p4e_to_p4plus_classic_nic.pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4_TCP);
        }
        else {
            if (udp_1.valid == TRUE) {
                modify_field(p4e_to_p4plus_classic_nic.pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4_UDP);
            }
            else {
                modify_field(p4e_to_p4plus_classic_nic.pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4);
            }
        }
    }
    if (ipv6_1.valid == TRUE) {
        modify_field(p4e_to_p4plus_classic_nic_ip.ip_sa, ipv6_1.srcAddr);
        modify_field(p4e_to_p4plus_classic_nic_ip.ip_da, ipv6_1.dstAddr);
        if (tcp.valid == TRUE) {
            modify_field(p4e_to_p4plus_classic_nic.pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6_TCP);
        }
        else {
            if (udp_1.valid == TRUE) {
                modify_field(p4e_to_p4plus_classic_nic.pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6_UDP);
            }
            else {
                modify_field(p4e_to_p4plus_classic_nic.pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6);
            }
        }
    }
}

@pragma stage 5
table p4e_to_rxdma {

    actions {
        p4e_to_rxdma;
    }
}

control egress_inter_pipe {
    /* TODO: Predicate the tables */
    apply(p4e_to_uplink);
    apply(p4e_to_rxdma);
}
