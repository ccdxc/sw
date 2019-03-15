#define LPM_STAGE_FANOUT 16

action action_name(key0, key1, key2, key3, key4, key5, key6, key7,
                   key8, key9, key10, key11, key12, key13, key14) {

    modify_field(key_field, key0);
    modify_field(key_field, key1);
    modify_field(key_field, key2);
    modify_field(key_field, key3);
    modify_field(key_field, key4);
    modify_field(key_field, key5);
    modify_field(key_field, key6);
    modify_field(key_field, key7);
    modify_field(key_field, key8);
    modify_field(key_field, key9);
    modify_field(key_field, key10);
    modify_field(key_field, key11);
    modify_field(key_field, key12);
    modify_field(key_field, key13);
    modify_field(key_field, key14);

    if (key >= key7) {
        if (key >= key11) {
            if (key >= key13) {
                if (key >= key14) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B16_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B15_OFFSET));
                }
            }
            else {
                if (key >= key12) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B14_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B13_OFFSET));
                }
            }
        }
        else {
            if (key >= key9) {
                if (key >= key10) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B12_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B11_OFFSET));
                }
            }
            else {
                if (key >= key8) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B10_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B09_OFFSET));
                }
            }
        }
    }
    else {
        if (key >= key3) {
            if (key >= key5) {
                if (key >= key6) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B08_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B07_OFFSET));
                }
            }
            else {
                if (key >= key4) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B06_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B05_OFFSET));
                }
            }
        }
        else {
            if (key >= key1) {
                if (key >= key2) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B04_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B03_OFFSET));
                }
            }
            else {
                if (key >= key0) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B02_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT) + \
                             (LPM_B01_OFFSET));
                }
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

