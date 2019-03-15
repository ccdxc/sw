action action_name(data_, key0, data0, key1, data1, key2, data2) {

    modify_field(dat_field, data_);
    modify_field(key_field, key0);
    modify_field(dat_field, data0);
    modify_field(key_field, key1);
    modify_field(dat_field, data1);
    modify_field(key_field, key2);
    modify_field(dat_field, data2);

    if (key >= key1) {
        if (key >= key2) {
            modify_field(res_field, data2);
        }
        else {
            modify_field(res_field, data1);
        }
    }
    else {
        if (key >= key0) {
            modify_field(res_field, data0);
        }
        else {
            modify_field(res_field, data_);
        }
    }
}

@pragma stage stage_num
@pragma hbm_table
@pragma raw_index_table
table table_name {
    reads {
        curr_addr : exact;
    }
    actions {
        action_name;
    }
}
