/*****************************************************************************/
/* Output mapping                                                            */
/*****************************************************************************/
action redirect_to_remote(tunnel_index, tm_oport, egress_mirror_en, tm_oqueue) {
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_index);
    modify_field(capri_intrinsic.tm_oport, tm_oport);
    modify_field(capri_intrinsic.tm_oq, tm_oqueue);

    if (egress_mirror_en == TRUE) {
        modify_field(capri_intrinsic.tm_span_session,
                     control_metadata.egress_mirror_session_id);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.flag, egress_mirror_en);
}

action redirect_to_cpu(dst_lif, egress_mirror_en, tm_oqueue) {
    modify_field(capri_intrinsic.lif, dst_lif);
    modify_field(control_metadata.cpu_copy, TRUE);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.tm_oq, tm_oqueue);
    modify_field(control_metadata.p4plus_app_id, P4PLUS_APPTYPE_CPU);

    if (egress_mirror_en == TRUE) {
        modify_field(capri_intrinsic.tm_span_session,
                     control_metadata.egress_mirror_session_id);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.flag, egress_mirror_en);
}

action set_tm_oport(vlan_strip, nports, egress_mirror_en,
                    p4plus_app_id, rdma_enabled, dst_lif,
                    encap_vlan_id, encap_vlan_id_valid,
                    egress_port1, egress_port2, egress_port3, egress_port4,
                    egress_port5, egress_port6, egress_port7, egress_port8) {
    if (nports == 1) {
        modify_field(capri_intrinsic.tm_oport, egress_port1);
        // Set the Output queue to use
        modify_field(capri_intrinsic.tm_oq, control_metadata.egress_tm_oqueue);
    } else {
        // use entropy hash to choose the destination port
        modify_field(scratch_metadata.entropy_hash,
                     rewrite_metadata.entropy_hash);
    }

    if (egress_mirror_en == TRUE) {
        modify_field(capri_intrinsic.tm_span_session,
                     control_metadata.egress_mirror_session_id);
    }

    modify_field(capri_intrinsic.lif, dst_lif);
    if (encap_vlan_id_valid == TRUE) {
        modify_field(rewrite_metadata.tunnel_vnid, encap_vlan_id);
    }
    modify_field(control_metadata.rdma_enabled, rdma_enabled);
    modify_field(control_metadata.p4plus_app_id, p4plus_app_id);
    modify_field(control_metadata.vlan_strip, vlan_strip);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.egress_port, egress_port1);
    modify_field(scratch_metadata.egress_port, egress_port2);
    modify_field(scratch_metadata.egress_port, egress_port3);
    modify_field(scratch_metadata.egress_port, egress_port4);
    modify_field(scratch_metadata.egress_port, egress_port5);
    modify_field(scratch_metadata.egress_port, egress_port6);
    modify_field(scratch_metadata.egress_port, egress_port7);
    modify_field(scratch_metadata.egress_port, egress_port8);
    modify_field(scratch_metadata.egress_port, nports);
    modify_field(scratch_metadata.flag, egress_mirror_en);
    modify_field(scratch_metadata.flag, encap_vlan_id_valid);
}

@pragma stage 1
table output_mapping {
    reads {
        control_metadata.dst_lport : exact;
    }
    actions {
        nop;
        set_tm_oport;
        redirect_to_cpu;
        redirect_to_remote;
    }
    size : OUTPUT_MAPPING_TABLE_SIZE;
}

control process_output_mapping {
    apply(output_mapping);
}
