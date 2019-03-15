action action_name(data_, key0, data0, key1, data1, key2, data2, key3,
                   data3, key4, data4, key5, data5, key6, data6) {

    modify_field(dat_field, data_);
    modify_field(key_field, key0);
    modify_field(dat_field, data0);
    modify_field(key_field, key1);
    modify_field(dat_field, data1);
    modify_field(key_field, key2);
    modify_field(dat_field, data2);
    modify_field(key_field, key3);
    modify_field(dat_field, data3);
    modify_field(key_field, key4);
    modify_field(dat_field, data4);
    modify_field(key_field, key5);
    modify_field(dat_field, data5);
    modify_field(key_field, key6);
    modify_field(dat_field, data6);

    if (key >= key3) {
        if (key >= key5) {
            if (key >= key6) {
                modify_field(res_field, data6);
            }
            else {
                modify_field(res_field, data5);
            }
        }
        else {
            if (key >= key4) {
                modify_field(res_field, data4);
            }
            else {
                modify_field(res_field, data3);
            }
        }
    }
    else {
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

    res_handler();
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
