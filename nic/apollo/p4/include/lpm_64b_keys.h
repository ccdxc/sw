#define LPM_STAGE_FANOUT 8

action action_name(key0, key1, key2, key3, key4, key5, key6) {

    modify_field(key_field, key0);
    modify_field(key_field, key1);
    modify_field(key_field, key2);
    modify_field(key_field, key3);
    modify_field(key_field, key4);
    modify_field(key_field, key5);
    modify_field(key_field, key6);

    if (key >= key3) {
        if (key >= key5) {
            if (key >= key6) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (8 * LPM_TBL_SIZE));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (7 * LPM_TBL_SIZE));
            }
        }
        else {
            if (key >= key4) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (6 * LPM_TBL_SIZE));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (5 * LPM_TBL_SIZE));
            }
        }
    }
    else {
        if (key >= key1) {
            if (key >= key2) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (4 * LPM_TBL_SIZE));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (3 * LPM_TBL_SIZE));
            }
        }
        else {
            if (key >= key0) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (2 * LPM_TBL_SIZE));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (1 * LPM_TBL_SIZE));
            }
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
