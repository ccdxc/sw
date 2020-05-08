#include "../include/intrinsic.p4"
#include "../common/defines.h"

#include "parser.p4"
#include "headers.p4"

header_type flow_metadata_t {
    fields {
        snat_valid : 1;
        tunnel_valid : 1;
        snat_index : 16;
        tunnel_index : 16;
        entropy_hash : 16;
    }
}
metadata flow_metadata_t flow_metadata;

header_type scratch_metadata_t {
    fields {
        flag : 1;
        in_pkts : 64;
        in_bytes : 64;
        ip_totallen : 16;
    }
}
@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata;

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

/*****************************************************************************/
/* dst_mac_lkp                                                               */
/*****************************************************************************/
action update_port() {
    add_header(capri_p4_intrinsic);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_UPLINK_0);
}

action update_port_and_modify_vlan(vid) {
    add_header(capri_p4_intrinsic);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_UPLINK_0);
    //modify_field(ctag_1.vid, vid);
    if (ctag_1.valid != TRUE) {
        add_header(ctag_1);
        modify_field(ctag_1.vid, vid);
        modify_field(ctag_1.etherType, ethernet_1.etherType);
        modify_field(ethernet_1.etherType, ETHERTYPE_CTAG);
    } else {
        modify_field(ctag_1.vid, vid);
    }
}

action dst_mac_lkp_miss() {
    drop_packet();
}

@pragma stage 0
table dst_mac_lkp {
    reads {
        ethernet_1.dstAddr : exact;
    }
    actions {
        dst_mac_lkp_miss;
        update_port;
        update_port_and_modify_vlan;
    }
    size : 1024;
}

/*****************************************************************************/
/* vlan_stats                                                                */
/*****************************************************************************/
action update_stats(in_pkts, in_bytes) {
    modify_field(scratch_metadata.in_pkts, in_pkts);
    modify_field(scratch_metadata.in_bytes, in_bytes);

    add_to_field(scratch_metadata.in_pkts, 1);
    add_to_field(scratch_metadata.in_bytes,
                 capri_p4_intrinsic.frame_size - ASICPD_GLOBAL_INTRINSIC_HDR_SZ);
}

@pragma stage 1
@pragma table_write
table vlan_stats {
    reads {
        ctag_1.vid : exact;
    }
    actions {
        update_stats;
    }
    size : 4096;
}

/*****************************************************************************/
/* flow_lkp                                                                  */
/*****************************************************************************/
action flow_info(entry_valid, snat_valid, tunnel_valid,
                 snat_index, tunnel_index) {
    modify_field(scratch_metadata.flag, entry_valid);
    if (scratch_metadata.flag == TRUE) {
        modify_field(flow_metadata.snat_index, snat_index);
        modify_field(flow_metadata.tunnel_index, tunnel_index);
        modify_field(flow_metadata.snat_valid, snat_valid);
        modify_field(flow_metadata.tunnel_valid, tunnel_valid);
    }
}

@pragma stage 2
@pragma hbm_table
table flow_lkp {
    reads {
        ipv4_1.srcAddr : exact;
        ipv4_1.dstAddr : exact;
        ipv4_1.protocol : exact;
        tcp.srcPort : exact;
        tcp.dstPort : exact;
    }
    actions {
        flow_info;
    }
    size : 65536;
}

/*****************************************************************************/
/* snat                                                                      */
/*****************************************************************************/
action snat_rewrite(sip, sport) {
    modify_field(ipv4_1.srcAddr, sip);
    modify_field(tcp.srcPort, sport);
}

@pragma stage 3
@pragma hbm_table
table snat {
    reads {
        flow_metadata.snat_index : exact;
    }
    actions {
        snat_rewrite;
    }
    size : 65536;
}

/*****************************************************************************/
/* tunnel                                                                    */
/*****************************************************************************/
action tunnel_rewrite(dmac, smac, sip, dip, slot_id) {
    remove_header(ethernet_1);
    remove_header(ctag_1);

    add_header(ethernet_0);
    add_header(ipv4_0);
    add_header(udp_0);
    add_header(mpls_dst_0);

    // subtract ethernet + vlan (optional) header size
    subtract(scratch_metadata.ip_totallen, capri_p4_intrinsic.frame_size,
             (ASICPD_GLOBAL_INTRINSIC_HDR_SZ + 14));
    if (ctag_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.ip_totallen, 4);
    }

    // account for new headers that are added
    // 8 bytes of UDP header, 4 bytes of mpls header, 20 bytes of IP header
    add_to_field(scratch_metadata.ip_totallen, 20 + 8 + 4);

    modify_field(ethernet_0.dstAddr, dmac);
    modify_field(ethernet_0.srcAddr, smac);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_UDP);
    modify_field(ipv4_0.dstAddr, dip);
    modify_field(ipv4_0.srcAddr, sip);

    modify_field(udp_0.srcPort, flow_metadata.entropy_hash);
    modify_field(udp_0.dstPort, UDP_PORT_MPLS);
    subtract(udp_0.len, scratch_metadata.ip_totallen, 20);

    modify_field(mpls_dst_0.label, slot_id);
    modify_field(mpls_dst_0.bos, 1);
}

@pragma stage 3
@pragma hbm_table
table tunnel {
    reads {
        flow_metadata.tunnel_index : exact;
    }
    actions {
        tunnel_rewrite;
    }
    size : 65536;
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    apply(dst_mac_lkp);
    if (ctag_1.valid == 1) {
        apply(vlan_stats);
    }
    apply(flow_lkp);
    if (flow_metadata.snat_valid == 1) {
        apply(snat);
    }
    if (flow_metadata.tunnel_valid == 1) {
        apply(tunnel);
    }
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
}
