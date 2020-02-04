/*****************************************************************************/
/* Device info                                                               */
/*****************************************************************************/
action p4i_recirc() {
    if (ingress_recirc.local_mapping_ohash != 0) {
        modify_field(control_metadata.local_mapping_ohash_lkp, TRUE);
    }
    if (ingress_recirc.flow_ohash != 0) {
        modify_field(control_metadata.flow_ohash_lkp, TRUE);
    }
}

action p4i_device_info(device_mac_addr1, device_mac_addr2,
                       device_ipv4_addr, device_ipv6_addr, l2_enabled) {
    modify_field(scratch_metadata.mac, device_mac_addr1);
    modify_field(scratch_metadata.mac, device_mac_addr2);
    modify_field(scratch_metadata.ipv4_addr, device_ipv4_addr);
    modify_field(scratch_metadata.ipv6_addr, device_ipv6_addr);

    if (((ethernet_1.dstAddr == device_mac_addr1) or
        (ethernet_1.dstAddr == device_mac_addr2)) and
        (((ipv4_1.valid == TRUE) and (ipv4_1.dstAddr == device_ipv4_addr)) or
        ((ipv6_1.valid == TRUE) and (ipv6_1.dstAddr == device_ipv6_addr)))) {
        modify_field(control_metadata.to_device_ip, TRUE);
    }
    modify_field(control_metadata.l2_enabled, l2_enabled);

    if (ingress_recirc.valid == TRUE) {
        p4i_recirc();
    }

    subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_1);
    if (capri_intrinsic.tm_oq != TM_P4_RECIRC_QUEUE) {
        modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    } else {
        modify_field(capri_intrinsic.tm_oq, capri_intrinsic.tm_iq);
    }
}

@pragma stage 0
table p4i_device_info {
    reads {
        control_metadata.device_profile_id  : exact;
    }
    actions {
        p4i_device_info;
    }
    size : DEVICE_INFO_TABLE_SIZE;
}

control ingress_device_info {
    apply(p4i_device_info);
}

action p4e_recirc() {
    if (egress_recirc.valid == TRUE) {
        modify_field(control_metadata.mapping_ohash_lkp,
                     ~egress_recirc.mapping_done);
    }
}

action p4e_device_info(device_ipv4_addr, device_ipv6_addr) {
    modify_field(rewrite_metadata.device_ipv4_addr, device_ipv4_addr);
    modify_field(rewrite_metadata.device_ipv6_addr, device_ipv6_addr);
    p4e_recirc();
}

@pragma stage 0
table p4e_device_info {
    reads {
        control_metadata.device_profile_id  : exact;
    }
    actions {
        p4e_device_info;
    }
    size : DEVICE_INFO_TABLE_SIZE;
}

control egress_device_info {
    apply(p4e_device_info);
}
