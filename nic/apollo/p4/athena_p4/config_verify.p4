
action config1_epoch_verify(epoch) {
    if (control_metadata.config1_epoch != epoch) {
        modify_field(control_metadata.flow_miss, TRUE);
    }
    modify_field(scratch_metadata.config_epoch, epoch);
}

action config2_epoch_verify(epoch) {
    if (control_metadata.config2_epoch != epoch) {
        modify_field(control_metadata.flow_miss, TRUE);
    }
    modify_field(scratch_metadata.config_epoch, epoch);
}

@pragma stage 4
@pragma hbm_table
@pragma index_table
table config1 {
    reads {
        control_metadata.config1_idx    : exact;
    }
    actions {
        config1_epoch_verify;
    }
    size : SESSION_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
@pragma index_table
table config2 {
    reads {
        control_metadata.config2_idx    : exact;
    }
    actions {
        config2_epoch_verify;
    }
    size : SESSION_TABLE_SIZE;
}

control config_verify {
    if (control_metadata.flow_miss == FALSE) {
        if (control_metadata.config1_idx_valid == TRUE) {
            apply(config1);
        }
        if (control_metadata.config2_idx_valid == TRUE) {
            apply(config2);
        }
    }
}
