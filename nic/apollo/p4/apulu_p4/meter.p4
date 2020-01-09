/*****************************************************************************/
/* Meter                                                                     */
/*****************************************************************************/
action meter_stats(in_packets, in_bytes) {
    add(scratch_metadata.in_packets, in_packets, 1);
    add(scratch_metadata.in_bytes, in_bytes, meter_metadata.meter_len);
}

@pragma stage 1
@pragma table_write
table meter_stats {
    reads {
        meter_metadata.meter_id : exact;
    }
    actions {
        meter_stats;
    }
    size : METER_TABLE_SIZE;
}

control meter_stats {
    if (meter_metadata.meter_enabled == TRUE) {
        apply(meter_stats);
    }
}
