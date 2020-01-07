action counter_set_update(rx_bytes, rx_packets, tx_bytes, tx_packets) {

    /* FIXME */
    add_to_field(scratch_metadata.counter_rx, rx_bytes);
    add_to_field(scratch_metadata.counter_rx, rx_packets);

    add_to_field(scratch_metadata.counter_tx, tx_bytes);
    add_to_field(scratch_metadata.counter_tx, tx_packets);
}



@pragma stage 1
@pragma hbm_table
@pragma table_write
@pragma index_table
table counterset1 {
    reads {
        p4i_to_p4e_header.counterset1    : exact;
    }
    actions {
        counter_set_update;
    }
    size : SESSION_TABLE_SIZE;
}

@pragma stage 1
@pragma hbm_table
@pragma table_write
@pragma index_table
table counterset2 {
    reads {
        p4i_to_p4e_header.counterset2    : exact;
    }
    actions {
        counter_set_update;
    }
    size : SESSION_TABLE_SIZE;
}


action histogram_update(lt_64, eq_64, lt_128, lt_256, lt_512, lt_1024,
                        lt_1518, ge_1518) {
    add_to_field(scratch_metadata.counter_rx, lt_64);
    add_to_field(scratch_metadata.counter_rx, eq_64);
    add_to_field(scratch_metadata.counter_rx, lt_128);
    add_to_field(scratch_metadata.counter_rx, lt_256);
    add_to_field(scratch_metadata.counter_rx, lt_512);
    add_to_field(scratch_metadata.counter_rx, lt_1024);
    add_to_field(scratch_metadata.counter_rx, lt_1518);
    add_to_field(scratch_metadata.counter_rx, ge_1518);
}

@pragma stage 2
@pragma hbm_table
@pragma table_write
@pragma index_table
table histogram {
    reads {
        p4i_to_p4e_header.histogram  : exact;
    }
    actions {
        histogram_update;
    }
    size : VNIC_TABLE_SIZE;
}

control statistics {
    if (control_metadata.flow_miss == FALSE) {
        if (control_metadata.counterset1_valid == TRUE) {
            apply(counterset1);
        }
        if (control_metadata.counterset2_valid == TRUE) {
            apply(counterset2);
        }
        if (control_metadata.histogram_valid == TRUE) {
            apply(histogram);
        }
    }
}
