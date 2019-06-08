action read_qstate_info(PKTQ_QSTATE) {
    // in txdma
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0
    //          dma to rxdma_qstate cindex
    // d-vector
    PKTQ_QSTATE_DVEC_SCRATCH(scratch_qstate_hdr, scratch_qstate_info);

    if (scratch_qstate_info.sw_cindex0 == scratch_qstate_hdr.p_index0) {
        modify_field(capri_intr.drop, TRUE);
    } else {
        modify_field(app_header.table3_valid, TRUE);
        modify_field(txdma_control.pktdesc_addr,
                     scratch_qstate_info.ring0_base +
                     (scratch_qstate_info.sw_cindex0 * PKTQ_PAGE_SIZE));
        modify_field(txdma_control.payload_addr, txdma_control.pktdesc_addr + (1<<6));
        modify_field(scratch_qstate_info.sw_cindex0,
                     scratch_qstate_info.sw_cindex0 + 1);
        modify_field(txdma_control.cindex, scratch_qstate_info.sw_cindex0);
        modify_field(txdma_control.rxdma_cindex_addr,
                     scratch_qstate_info.rxdma_cindex_addr);
    }

}

@pragma stage 0
@pragma raw_index_table
@pragma table_write
table read_qstate {
    reads {
        capri_txdma_intr.qstate_addr    : exact;
    }
    actions {
        read_qstate_info;
    }
}

action read_pktdesc(remote_ip,
                    sacl_base_addr,
                    route_base_addr,
                    meter_result,
                    sip_classid,
                    dip_classid,
                    stag_classid,
                    dtag_classid,
                    sport_classid,
                    dport_classid,
                    vpc_id,
                    vnic_id,
                    iptype,
                    pad0
                   )
{
    modify_field(scratch_metadata.field128, remote_ip);
    modify_field(scratch_metadata.field40, sacl_base_addr);
    modify_field(scratch_metadata.field40, route_base_addr);
    modify_field(scratch_metadata.field10, meter_result);
    modify_field(scratch_metadata.field10, sip_classid);
    modify_field(scratch_metadata.field10, dip_classid);
    modify_field(scratch_metadata.field10, stag_classid);
    modify_field(scratch_metadata.field10, dtag_classid);
    modify_field(scratch_metadata.field8, sport_classid);
    modify_field(scratch_metadata.field8, dport_classid);
    modify_field(scratch_metadata.field8, vpc_id);
    modify_field(scratch_metadata.field8, vnic_id);
    modify_field(scratch_metadata.field1, iptype);
    modify_field(scratch_metadata.field7, pad0);
}

@pragma stage 1
@pragma raw_index_table
table read_pktdesc {
    reads {
        txdma_control.pktdesc_addr  : exact;
    }
    actions {
        read_pktdesc;
    }
}

control read_qstate {
    apply(read_qstate);
    apply(read_pktdesc);
}
