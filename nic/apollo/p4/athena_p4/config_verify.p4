
action config1_epoch_verify(epoch) {
    if (control_metadata.epoch1_value != epoch) {
        modify_field(control_metadata.flow_miss, TRUE);
    }
    modify_field(scratch_metadata.config_epoch, epoch);
}

action config2_epoch_verify(epoch) {
    if (control_metadata.epoch2_value != epoch) {
        modify_field(control_metadata.flow_miss, TRUE);
    }
    modify_field(scratch_metadata.config_epoch, epoch);
}

@pragma stage 4
@pragma hbm_table
@pragma index_table
table config1 {
    reads {
        control_metadata.epoch1_id  : exact;
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
        control_metadata.epoch2_id  : exact;
    }
    actions {
        config2_epoch_verify;
    }
    size : SESSION_TABLE_SIZE;
}

control config_verify {
    if (control_metadata.flow_miss == FALSE) {
        if (control_metadata.epoch1_id_valid == TRUE) {
            apply(config1);
        }
        if (control_metadata.epoch2_id_valid == TRUE) {
            apply(config2);
        }
    }
}
