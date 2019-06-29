/*****************************************************************************/
/* Meter stats                                                               */
/*****************************************************************************/
action meter_stats(pkt_bytes) {
    modify_field(scratch_metadata.in_bytes,
                 pkt_bytes + rewrite_metadata.meter_len);
}

@pragma stage 5
@pragma hbm_table
@pragma table_write
@pragma index_table
table meter_stats {
    reads {
        rewrite_metadata.meter_idx  : exact;
    }
    actions {
        meter_stats;
    }
    size : METER_STATS_TABLE_SIZE;
}
