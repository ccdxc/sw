action gettimeofday(time_in_ns, ticks, multiplier, shift, pad) {
    if (multiplier == 0) {
        // return;
    }

    modify_field(scratch_metadata.ticks, 0 /* current ticks from R4 */);
    if (scratch_metadata.ticks > ticks) {
        modify_field(scratch_metadata.ticks, scratch_metadata.ticks - ticks);
    } else {
        modify_field(scratch_metadata.ticks,
                     0xFFFFFFFFFFFF + (scratch_metadata.ticks - ticks));
    }
    modify_field(scratch_metadata.delta,
                 (scratch_metadata.ticks * multiplier) >> scratch_metadata.size8);
    modify_field(control_metadata.current_time_in_ns,
                 time_in_ns + scratch_metadata.delta);

    modify_field(scratch_metadata.size64, time_in_ns);
    modify_field(scratch_metadata.size64, ticks);
    modify_field(scratch_metadata.size64, multiplier);
    modify_field(scratch_metadata.size8, shift);
    modify_field(scratch_metadata.pad312, pad);
}

@pragma stage 2
@pragma hbm_table
@pragma index_table
table clock {
    reads {
        control_metadata.clock_idx  : exact;
    }
    actions {
        gettimeofday;
    }
    size : 1;
}
