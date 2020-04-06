/******************************************************************************/
/* derive tags based on local_tag_idx                                         */
/******************************************************************************/
action local_mapping_tag_info(classid0, classid1, classid2, classid3, classid4) {
    if (p4_to_rxdma.rx_packet == FALSE) {
        // Tx packet
        modify_field(rx_to_tx_hdr.stag0_classid, classid0);
        modify_field(rx_to_tx_hdr.stag1_classid, classid1);
        modify_field(rx_to_tx_hdr.stag2_classid, classid2);
        modify_field(rx_to_tx_hdr.stag3_classid, classid3);
        modify_field(rx_to_tx_hdr.stag4_classid, classid4);
    } else {
        // Rx packet
        modify_field(rx_to_tx_hdr.dtag0_classid, classid0);
        modify_field(rx_to_tx_hdr.dtag1_classid, classid1);
        modify_field(rx_to_tx_hdr.dtag2_classid, classid2);
        modify_field(rx_to_tx_hdr.dtag3_classid, classid3);
        modify_field(rx_to_tx_hdr.dtag4_classid, classid4);
    }
}

@pragma stage 7
@pragma hbm_table
@pragma index_table
table local_mapping_tag {
    reads {
        p4_to_rxdma.local_tag_idx   : exact;
    }
    actions {
        local_mapping_tag_info;
    }
    size : LOCAL_MAPPING_TAG_TABLE_SIZE;
}

/******************************************************************************/
/* derive tags based on mapping_tag_idx                                       */
/******************************************************************************/
action mapping_tag_info(classid0, classid1, classid2, classid3, classid4) {
    if (p4_to_rxdma.rx_packet == FALSE) {
        // Tx packet
        modify_field(rx_to_tx_hdr.dtag0_classid, classid0);
        modify_field(rx_to_tx_hdr.dtag1_classid, classid1);
        modify_field(rx_to_tx_hdr.dtag2_classid, classid2);
        modify_field(rx_to_tx_hdr.dtag3_classid, classid3);
        modify_field(rx_to_tx_hdr.dtag4_classid, classid4);
    } else {
        // Rx packet
        modify_field(rx_to_tx_hdr.stag0_classid, classid0);
        modify_field(rx_to_tx_hdr.stag1_classid, classid1);
        modify_field(rx_to_tx_hdr.stag2_classid, classid2);
        modify_field(rx_to_tx_hdr.stag3_classid, classid3);
        modify_field(rx_to_tx_hdr.stag4_classid, classid4);
    }
}

@pragma stage 7
@pragma hbm_table
@pragma index_table
table mapping_tag {
    reads {
        lpm_metadata.mapping_tag_idx    : exact;
    }
    actions {
        mapping_tag_info;
    }
    size : MAPPING_TAG_TABLE_SIZE;
}

control derive_tag_classids {
    if (p4_to_rxdma.mapping_done == TRUE) {
        apply(local_mapping_tag);
        apply(mapping_tag);
    }
}
