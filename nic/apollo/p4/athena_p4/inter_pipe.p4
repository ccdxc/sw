/*****************************************************************************/
/* Inter pipe : ingress pipeline                                             */
/*****************************************************************************/



action p4i_to_p4e() {
    if ((ingress_recirc_header.flow_done != TRUE) or 
        (ingress_recirc_header.dnat_done != TRUE)) {
        /* Recirc back to P4I */
        add_header(ingress_recirc_header);
        modify_field(ingress_recirc_header.direction, control_metadata.direction);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
    }
    else {
        /* To P4E */
        remove_header(ingress_recirc_header);
        add_header(p4i_to_p4e_header);
        modify_field(p4i_to_p4e_header.flow_miss, control_metadata.flow_miss);
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
    if (capri_intrinsic.drop == 0) {
        apply(p4i_to_p4e);
    }
}


/*****************************************************************************/
/* Inter pipe : egress pipeline                                              */
/*****************************************************************************/
action p4i_to_p4e_state() {
    if (p4i_to_p4e_header.valid == TRUE) {
        if (p4i_to_p4e_header.flow_miss != TRUE) {
            if (p4i_to_p4e_header.index_type == FLOW_CACHE_INDEX_TYPE_SESSION_INFO) {
                modify_field(control_metadata.session_index, p4i_to_p4e_header.index);
                modify_field(control_metadata.session_index_valid, TRUE);
            }
            if (p4i_to_p4e_header.index_type == FLOW_CACHE_INDEX_TYPE_CONNTRACK_INFO) {
                modify_field(control_metadata.conntrack_index, p4i_to_p4e_header.index);
                modify_field(control_metadata.conntrack_index_valid , TRUE);
            }
        }
        else {
            if (udp_1.valid == TRUE) {
                modify_field(p4e_to_p4plus_classic_nic.l4_sport, udp_1.srcPort);
                modify_field(p4e_to_p4plus_classic_nic.l4_dport, udp_1.dstPort);
            }

            if (tcp.valid == TRUE) {
                modify_field(p4e_to_p4plus_classic_nic.l4_sport, tcp.srcPort);
                modify_field(p4e_to_p4plus_classic_nic.l4_dport, tcp.dstPort);
            }
        }
        modify_field(control_metadata.update_checksum, p4i_to_p4e_header.update_checksum);
    }
}

@pragma stage 0
table p4i_to_p4e_state {
    actions {
        p4i_to_p4e_state;
    }
}

action p4e_to_uplink() {
    modify_field(capri_intrinsic.tm_oport, control_metadata.redir_oport);
}

action p4e_to_rxdma() {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.lif, control_metadata.redir_lif);
    add_header(capri_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.redir_qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.redir_qtype);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
            CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ +
            CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_CLASSIC_NIC_HDR_SZ);
    add_header(p4e_to_p4plus_classic_nic);
    modify_field(p4e_to_p4plus_classic_nic.p4plus_app_id, control_metadata.redir_app_id);
    add_header(p4e_to_p4plus_classic_nic_ip);

    modify_field(p4e_to_p4plus_classic_nic.packet_len, p4i_to_p4e_header.packet_len);

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
@pragma index_table
table p4e_redir {
    reads {
        control_metadata.redir_type : exact;
    }

    actions {
        p4e_to_rxdma;
        p4e_to_uplink;
    }
    size : P4E_REDIR_TABLE_SIZE; 
}

control egress_inter_pipe {
    apply(p4i_to_p4e_state);    
    apply(p4e_redir);
}
