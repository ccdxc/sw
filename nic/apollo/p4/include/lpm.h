#include "lpm_defines.h"

action s0_name(key0, key1, key2, key3, key4, key5, key6, key7,
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
                    modify_field(next_addr, base_addr + LPM_S0_B15_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK15_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B14_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK14_OFFSET);
                }
            }
            else {
                if (key >= key12) {
                    modify_field(next_addr, base_addr + LPM_S0_B13_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK13_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B12_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK12_OFFSET);
                }
            }
        }
        else {
            if (key >= key9) {
                if (key >= key10) {
                    modify_field(next_addr, base_addr + LPM_S0_B11_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK11_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B10_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK10_OFFSET);
                }
            }
            else {
                if (key >= key8) {
                    modify_field(next_addr, base_addr + LPM_S0_B9_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK9_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B8_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK8_OFFSET);
                }
            }
        }
    }
    else {
        if (key >= key3) {
            if (key >= key5) {
                if (key >= key6) {
                    modify_field(next_addr, base_addr + LPM_S0_B7_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK7_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B6_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK6_OFFSET);
                }
            }
            else {
                if (key >= key4) {
                    modify_field(next_addr, base_addr + LPM_S0_B5_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK5_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B4_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK4_OFFSET);
                }
            }
        }
        else {
            if (key >= key1) {
                if (key >= key2) {
                    modify_field(next_addr, base_addr + LPM_S0_B3_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK3_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B2_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK2_OFFSET);
                }
            }
            else {
                if (key >= key0) {
                    modify_field(next_addr, base_addr + LPM_S0_B1_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK1_OFFSET);
                }
                else {
                    modify_field(next_addr, base_addr + LPM_S0_B0_OFFSET);
                    modify_field(s2_offset, LPM_S2_BLOCK0_OFFSET);
                }
            }
        }
    }
}

action s1_name(key0, key1, key2, key3, key4, key5, key6, key7,
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
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B15_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B14_OFFSET);
                }
            }
            else {
                if (key >= key12) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B13_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B12_OFFSET);
                }
            }
        }
        else {
            if (key >= key9) {
                if (key >= key10) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B11_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B10_OFFSET);
                }
            }
            else {
                if (key >= key8) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B9_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B8_OFFSET);
                }
            }
        }
    }
    else {
        if (key >= key3) {
            if (key >= key5) {
                if (key >= key6) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B7_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B6_OFFSET);
                }
            }
            else {
                if (key >= key4) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B5_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B4_OFFSET);
                }
            }
        }
        else {
            if (key >= key1) {
                if (key >= key2) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B3_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B2_OFFSET);
                }
            }
            else {
                if (key >= key0) {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B1_OFFSET);
                }
                else {
                    modify_field(next_addr,
                                 base_addr + s2_offset + LPM_S1_B0_OFFSET);
                }
            }
        }
    }
}

action s2_name(data_, key0, data0, key1, data1, key2, data2, key3,
               data3, key4, data4, key5, data5, key6, data6) {

    modify_field(res_field, data_);
    modify_field(key_field, key0);
    modify_field(res_field, data0);
    modify_field(key_field, key1);
    modify_field(res_field, data1);
    modify_field(key_field, key2);
    modify_field(res_field, data2);
    modify_field(key_field, key3);
    modify_field(res_field, data3);
    modify_field(key_field, key4);
    modify_field(res_field, data4);
    modify_field(key_field, key5);
    modify_field(res_field, data5);
    modify_field(key_field, key6);
    modify_field(res_field, data6);

    if (key >= key3) {
        if (key >= key5) {
            if (key >= key6) {
                modify_field(result, data6);
            }
            else {
                modify_field(result, data5);
            }
        }
        else {
            if (key >= key4) {
                modify_field(result, data4);
            }
            else {
                modify_field(result, data3);
            }
        }
    }
    else {
        if (key >= key1) {
            if (key >= key2) {
                modify_field(result, data2);
            }
            else {
                modify_field(result, data1);
            }
        }
        else {
            if (key >= key0) {
                modify_field(result, data0);
            }
            else {
                modify_field(result, data_);
            }
        }
    }
    s2_name_ext();
}

@pragma stage s0_stage
@pragma hbm_table
@pragma raw_index_table
table s0_name {
    reads {
        next_addr : exact;
    }
    actions {
        s0_name;
    }
}

@pragma stage s1_stage
@pragma hbm_table
@pragma raw_index_table
table s1_name {
    reads {
        next_addr : exact;
    }
    actions {
        s1_name;
    }
}

@pragma stage s2_stage
@pragma hbm_table
@pragma raw_index_table
table s2_name {
    reads {
        next_addr : exact;
    }
    actions {
        s2_name;
    }
}
