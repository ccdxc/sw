#define LPM_STAGE_FANOUT_64B 8
#define LPM_STAGE_FANOUT_32B 16

action action_keys32b(key0, key1, key2, key3, key4, key5, key6, key7,
                      key8, key9, key10, key11, key12, key13, key14) {

    modify_field(key_field32b, key0);
    modify_field(key_field32b, key1);
    modify_field(key_field32b, key2);
    modify_field(key_field32b, key3);
    modify_field(key_field32b, key4);
    modify_field(key_field32b, key5);
    modify_field(key_field32b, key6);
    modify_field(key_field32b, key7);
    modify_field(key_field32b, key8);
    modify_field(key_field32b, key9);
    modify_field(key_field32b, key10);
    modify_field(key_field32b, key11);
    modify_field(key_field32b, key12);
    modify_field(key_field32b, key13);
    modify_field(key_field32b, key14);

    if (key >= key7) {
        if (key >= key11) {
            if (key >= key13) {
                if (key >= key14) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B16_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B15_OFFSET));
                }
            }
            else {
                if (key >= key12) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B14_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B13_OFFSET));
                }
            }
        }
        else {
            if (key >= key9) {
                if (key >= key10) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B12_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B11_OFFSET));
                }
            }
            else {
                if (key >= key8) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B10_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
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
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B08_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B07_OFFSET));
                }
            }
            else {
                if (key >= key4) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B06_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B05_OFFSET));
                }
            }
        }
        else {
            if (key >= key1) {
                if (key >= key2) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B04_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B03_OFFSET));
                }
            }
            else {
                if (key >= key0) {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B02_OFFSET));
                }
                else {
                    modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_32B) + \
                             (LPM_B01_OFFSET));
                }
            }
        }
    }
}

action action_data32b(data_, key0, data0, key1, data1, key2, data2, key3,
                      data3, key4, data4, key5, data5, key6, data6) {

    modify_field(dat_field32b, data_);
    modify_field(key_field32b, key0);
    modify_field(dat_field32b, data0);
    modify_field(key_field32b, key1);
    modify_field(dat_field32b, data1);
    modify_field(key_field32b, key2);
    modify_field(dat_field32b, data2);
    modify_field(key_field32b, key3);
    modify_field(dat_field32b, data3);
    modify_field(key_field32b, key4);
    modify_field(dat_field32b, data4);
    modify_field(key_field32b, key5);
    modify_field(dat_field32b, data5);
    modify_field(key_field32b, key6);
    modify_field(dat_field32b, data6);

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

    // Disable further LPM stages.
    modify_field(ctrl_field, FALSE);
    // Handle any post processing.
    res_handler();
}

action action_keys64b(key0, key1, key2, key3, key4, key5, key6) {

    modify_field(key_field64b, key0);
    modify_field(key_field64b, key1);
    modify_field(key_field64b, key2);
    modify_field(key_field64b, key3);
    modify_field(key_field64b, key4);
    modify_field(key_field64b, key5);
    modify_field(key_field64b, key6);

    if (key >= key3) {
        if (key >= key5) {
            if (key >= key6) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B08_OFFSET));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B07_OFFSET));
            }
        }
        else {
            if (key >= key4) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B06_OFFSET));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B05_OFFSET));
            }
        }
    }
    else {
        if (key >= key1) {
            if (key >= key2) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B04_OFFSET));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B03_OFFSET));
            }
        }
        else {
            if (key >= key0) {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B02_OFFSET));
            }
            else {
                modify_field(next_addr, curr_addr + \
                             ((curr_addr - base_addr) * LPM_STAGE_FANOUT_64B) + \
                             (LPM_B01_OFFSET));
            }
        }
    }
}

action action_data64b(data_, key0, data0, key1, data1, key2, data2) {

    modify_field(dat_field64b, data_);
    modify_field(key_field64b, key0);
    modify_field(dat_field64b, data0);
    modify_field(key_field64b, key1);
    modify_field(dat_field64b, data1);
    modify_field(key_field64b, key2);
    modify_field(dat_field64b, data2);

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

    // Disable further LPM stages.
    modify_field(ctrl_field, FALSE);
    // Handle any post processing.
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
        action_keys32b;
        action_data32b;
        action_keys64b;
        action_data64b;
    }
}
