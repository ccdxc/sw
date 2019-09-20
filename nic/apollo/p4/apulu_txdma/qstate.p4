#include "../include/apulu_sacl_defines.h"

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
                    payload_len,
                    iptype,
                    rx_packet
                   )
{
    modify_field(scratch_metadata.meter_result, meter_result);
    modify_field(scratch_metadata.field10, sip_classid);
    modify_field(scratch_metadata.field8, sport_classid);

    modify_field(rx_to_tx_hdr.remote_ip, remote_ip);
    modify_field(rx_to_tx_hdr.sacl_base_addr, sacl_base_addr);
    modify_field(rx_to_tx_hdr.route_base_addr, route_base_addr);
    modify_field(rx_to_tx_hdr.meter_result, meter_result);
    modify_field(rx_to_tx_hdr.sip_classid, sip_classid);
    modify_field(rx_to_tx_hdr.dip_classid, dip_classid);
    modify_field(rx_to_tx_hdr.stag_classid, stag_classid);
    modify_field(rx_to_tx_hdr.dtag_classid, dtag_classid);
    modify_field(rx_to_tx_hdr.sport_classid, sport_classid);
    modify_field(rx_to_tx_hdr.dport_classid, dport_classid);
    modify_field(rx_to_tx_hdr.vpc_id, vpc_id);
    modify_field(rx_to_tx_hdr.vnic_id, vnic_id);
    modify_field(rx_to_tx_hdr.payload_len, payload_len);
    modify_field(rx_to_tx_hdr.iptype, iptype);
    modify_field(rx_to_tx_hdr.rx_packet, rx_packet);

    modify_field(txdma_to_p4e.meter_id, scratch_metadata.meter_result);

    // Initialize the first P1 table index = (sip_classid << 7) | sport_classid
    modify_field(scratch_metadata.field20, (scratch_metadata.field10 << 7)|
                                            scratch_metadata.field8);

    // Write P1 table index to PHV
    modify_field(txdma_control.rfc_index, scratch_metadata.field20);

    // Write P1 table lookup address to PHV
    modify_field(txdma_control.rfc_table_addr,              // P1 Lookup Addr =
                 sacl_base_addr +                           // Region Base +
                 SACL_P1_1_TABLE_OFFSET +                   // Table Base +
                 (((scratch_metadata.field20) / 51) * 64)); // Index Bytes

    // Setup for route LPM lookup
    if (route_base_addr != 0) {
        modify_field(txdma_control.lpm1_key, remote_ip);
        modify_field(txdma_control.lpm1_base_addr, route_base_addr);
        modify_field(txdma_predicate.lpm1_enable, TRUE);
    }
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
