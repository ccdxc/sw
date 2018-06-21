/******************************************************************************/
/* VNIC mapping table                                                         */
/******************************************************************************/
action vnic_mapping(vnic, subnet_id, resource_group1_1, resource_group2_1,
                    resource_group1_2, resource_group2_2,
                    lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                    epoch1, epoch2) {
    modify_field(control_metadata.direction, TX_PACKET);
    modify_field(control_metadata.ingress_vnic, vnic);
    modify_field(control_metadata.subnet_id, subnet_id);
    if (service_header.valid == TRUE) {
        if (service_header.epoch == epoch1) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
        }
    } else {
        if (epoch1 > epoch2) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
        }
    }
    if (scratch_metadata.use_epoch1 == TRUE) {
        modify_field(policer_metadata.resource_group1, resource_group1_1);
        modify_field(policer_metadata.resource_group2, resource_group2_1);
        lpm_init(lpm_addr_1);
        slacl_init(slacl_addr_1);
        modify_field(service_header.epoch, epoch1);
    } else {
        modify_field(policer_metadata.resource_group1, resource_group1_2);
        modify_field(policer_metadata.resource_group2, resource_group2_2);
        lpm_init(lpm_addr_2);
        slacl_init(slacl_addr_2);
        modify_field(service_header.epoch, epoch2);
    }
}

@pragma stage 0
table vnic {
    reads {
        ctag_1.vid : exact;
    }
    actions {
        vnic_mapping;
    }
    size : VNIC_MAPPING_TABLE_SIZE;
}

/******************************************************************************/
/* Encap mapping table                                                        */
/******************************************************************************/
action encap_mapping(vnic, subnet_id, resource_group1_1, resource_group2_1,
                     resource_group1_2, resource_group2_2,
                     lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                     epoch1, epoch2) {
    modify_field(control_metadata.direction, RX_PACKET);
    modify_field(control_metadata.ingress_vnic, vnic);
    modify_field(control_metadata.subnet_id, subnet_id);
    modify_field(rewrite_metadata.tunnel_terminate, TRUE);
    if (service_header.valid == TRUE) {
        if (service_header.epoch == epoch1) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
        }
    } else {
        if (epoch1 > epoch2) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
        }
    }
    if (scratch_metadata.use_epoch1 == TRUE) {
        modify_field(policer_metadata.resource_group1, resource_group1_1);
        modify_field(policer_metadata.resource_group2, resource_group2_1);
        lpm_init(lpm_addr_1);
        slacl_init(slacl_addr_1);
        modify_field(service_header.epoch, epoch1);
    } else {
        modify_field(policer_metadata.resource_group1, resource_group1_2);
        modify_field(policer_metadata.resource_group2, resource_group2_2);
        lpm_init(lpm_addr_2);
        slacl_init(slacl_addr_2);
        modify_field(service_header.epoch, epoch2);
    }
}

@pragma stage 0
table encap {
    reads {
        tunnel_metadata.tunnel_vni : exact;
    }
    actions {
        encap_mapping;
    }
    size : ENCAP_MAPPING_TABLE_SIZE;
}

@pragma stage 0
@pragma overflow_table encap
table encap_otcam {
    reads {
        tunnel_metadata.tunnel_vni : ternary;
    }
    actions {
        encap_mapping;
    }
    size : ENCAP_MAPPING_OTCAM_TABLE_SIZE;
}

/******************************************************************************/
/* Key derivation tables                                                      */
/******************************************************************************/
action native_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_1.srcAddr);
    modify_field(key_metadata.dst, ipv4_1.dstAddr);
    modify_field(key_metadata.proto, ipv4_1.protocol);
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    }
    modify_field(slacl_metadata.ip_15_00, ipv4_1.dstAddr);
    modify_field(slacl_metadata.ip_31_16, (ipv4_1.dstAddr >> 16) & 0xFFFF);
}

action native_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_1.srcAddr);
    modify_field(key_metadata.dst, ipv6_1.dstAddr);
    modify_field(key_metadata.proto, ipv6_1.nextHdr);
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    }
}

action native_nonip_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_1.srcAddr);
    modify_field(key_metadata.dst, ethernet_1.dstAddr);
    if (ctag_1.valid == TRUE) {
        modify_field(key_metadata.dport, ctag_1.etherType);
    } else {
        modify_field(key_metadata.dport, ethernet_1.etherType);
    }
}

action tunneled_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_2.srcAddr);
    modify_field(key_metadata.dst, ipv4_2.dstAddr);
    modify_field(key_metadata.proto, ipv4_2.protocol);
    modify_field(slacl_metadata.ip_15_00, ipv4_2.srcAddr);
    modify_field(slacl_metadata.ip_31_16, (ipv4_2.srcAddr >> 16) & 0xFFFF);
}

action tunneled_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_2.srcAddr);
    modify_field(key_metadata.dst, ipv6_2.dstAddr);
    modify_field(key_metadata.proto, ipv6_2.nextHdr);
}

action tunneled_nonip_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_2.srcAddr);
    modify_field(key_metadata.dst, ethernet_2.dstAddr);
    modify_field(key_metadata.dport, ethernet_2.etherType);
}

@pragma stage 0
table key_native {
    reads {
        tunnel_metadata.tunnel_type : ternary;
        ipv4_1.valid                : ternary;
        ipv6_1.valid                : ternary;
        ipv4_2.valid                : ternary;
        ipv6_2.valid                : ternary;
    }
    actions {
        nop;
        native_ipv4_packet;
        native_ipv6_packet;
        native_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

@pragma stage 0
table key_tunneled {
    reads {
        tunnel_metadata.tunnel_type : ternary;
        ipv4_1.valid                : ternary;
        ipv6_1.valid                : ternary;
        ipv4_2.valid                : ternary;
        ipv6_2.valid                : ternary;
    }
    actions {
        nop;
        tunneled_ipv4_packet;
        tunneled_ipv6_packet;
        tunneled_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

action init_config() {
    if (service_header.valid == TRUE) {
        modify_field(control_metadata.ep_mapping_ohash_lkp,
                     ~service_header.ep_mapping_done);
        modify_field(control_metadata.source_guard_ohash_lkp,
                     ~service_header.source_guard_done);
        modify_field(control_metadata.policy_ohash_lkp,
                     ~service_header.policy_done);
        modify_field(rewrite_metadata.nexthop_index,
                     service_header.nexthop_index);
    }
    modify_field(lpm_metadata.addr, lpm_metadata.addr + key_metadata.dst);
}

@pragma stage 1
table init_config {
    actions {
        init_config;
    }
}

control mappings {
    apply(vnic);
    apply(encap);
    apply(encap_otcam);
    apply(key_native);
    apply(key_tunneled);
    apply(init_config);
}
