#define LPM_STAGE_FANOUT_16B     32
#define LPM_STAGE_FANOUT_32B     16
#define LPM_STAGE_FANOUT_64B     8
#define LPM_STAGE_FANOUT_128B    4

#ifdef action_keys16b
action action_keys16b(key0, key1, key2, key3, key4, key5, key6, key7, key8,
                      key9, key10, key11, key12, key13, key14, key15, key16,
                      key17, key18, key19, key20, key21, key22, key23,
                      key24, key25, key26, key27, key28, key29, key30) {

    modify_field(key_field16b, key0);
    modify_field(key_field16b, key1);
    modify_field(key_field16b, key2);
    modify_field(key_field16b, key3);
    modify_field(key_field16b, key4);
    modify_field(key_field16b, key5);
    modify_field(key_field16b, key6);
    modify_field(key_field16b, key7);
    modify_field(key_field16b, key8);
    modify_field(key_field16b, key9);
    modify_field(key_field16b, key10);
    modify_field(key_field16b, key11);
    modify_field(key_field16b, key12);
    modify_field(key_field16b, key13);
    modify_field(key_field16b, key14);
    modify_field(key_field16b, key15);
    modify_field(key_field16b, key16);
    modify_field(key_field16b, key17);
    modify_field(key_field16b, key18);
    modify_field(key_field16b, key19);
    modify_field(key_field16b, key20);
    modify_field(key_field16b, key21);
    modify_field(key_field16b, key22);
    modify_field(key_field16b, key23);
    modify_field(key_field16b, key24);
    modify_field(key_field16b, key25);
    modify_field(key_field16b, key26);
    modify_field(key_field16b, key27);
    modify_field(key_field16b, key28);
    modify_field(key_field16b, key29);
    modify_field(key_field16b, key30);

    if (key >= key15) {
        if (key >= key23) {
            if (key >= key27) {
                if (key >= key29) {
                    if (key >= key30) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B32_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B31_OFFSET));
                    }
                } else {
                    if (key >= key28) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B30_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B29_OFFSET));
                    }
                }
            } else {
                if (key >= key25) {
                    if (key >= key26) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B28_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B27_OFFSET));
                    }
                } else {
                    if (key >= key24) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B26_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B25_OFFSET));
                    }
                }
            }
        } else {
            if (key >= 19) {
                if (key >= key21) {
                    if (key >= key22) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B24_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B23_OFFSET));
                    }
                } else {
                    if (key >= key20) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B22_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B21_OFFSET));
                    }
                }
            } else {
                if (key >= key17) {
                    if (key >= key18) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B20_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B19_OFFSET));
                    }
                } else {
                    if (key >= key16) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B18_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B17_OFFSET));
                    }
                }
            }
        }
    } else {
        if (key >= key7) {
            if (key >= key11) {
                if (key >= key13) {
                    if (key >= key14) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B16_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B15_OFFSET));
                    }
                } else {
                    if (key >= key12) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B14_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B13_OFFSET));
                    }
                }
            } else {
                if (key >= key9) {
                    if (key >= key10) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B12_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B11_OFFSET));
                    }
                } else {
                    if (key >= key8) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B10_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B09_OFFSET));
                    }
                }
            }
        } else {
            if (key >= key3) {
                if (key >= key5) {
                    if (key >= key6) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B08_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B07_OFFSET));
                    }
                } else {
                    if (key >= key4) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B06_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B05_OFFSET));
                    }
                }
            } else {
                if (key >= key1) {
                    if (key >= key2) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B04_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B03_OFFSET));
                    }
                } else {
                    if (key >= key0) {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B02_OFFSET));
                    } else {
                        modify_field(next_addr, curr_addr + \
                                 ((curr_addr - base_addr) * LPM_STAGE_FANOUT_16B) + \
                                 (LPM_B01_OFFSET));
                    }
                }
            }
        }
    }
}
#endif

#ifdef action_keys32b
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
#endif

#ifdef action_keys64b
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
#endif

#ifdef action_keys128b
action action_keys128b(key0, key1, key2) {

    modify_field(key_field128b, key0);
    modify_field(key_field128b, key1);
    modify_field(key_field128b, key2);

    if (key >= key1) {
        if (key >= key2) {
            modify_field(next_addr, curr_addr + \
                         ((curr_addr - base_addr) * LPM_STAGE_FANOUT_128B) + \
                         (LPM_B04_OFFSET));
        }
        else {
            modify_field(next_addr, curr_addr + \
                         ((curr_addr - base_addr) * LPM_STAGE_FANOUT_128B) + \
                         (LPM_B03_OFFSET));
        }
    }
    else {
        if (key >= key0) {
            modify_field(next_addr, curr_addr + \
                         ((curr_addr - base_addr) * LPM_STAGE_FANOUT_128B) + \
                         (LPM_B02_OFFSET));
        }
        else {
            modify_field(next_addr, curr_addr + \
                         ((curr_addr - base_addr) * LPM_STAGE_FANOUT_128B) + \
                         (LPM_B01_OFFSET));
        }
    }
}
#endif

#ifdef action_data16b
action action_data16b(data_, key0, data0, key1, data1, key2, data2, key3, data3,
                             key4, data4, key5, data5, key6, data6, key7, data7,
                             key8, data8, key9, data9, key10, data10, key11, data11,
                             key12, data12, key13, data13, key14, data14) {

    modify_field(dat_field16b, data_);
    modify_field(key_field16b, key0);
    modify_field(dat_field16b, data0);
    modify_field(key_field16b, key1);
    modify_field(dat_field16b, data1);
    modify_field(key_field16b, key2);
    modify_field(dat_field16b, data2);
    modify_field(key_field16b, key3);
    modify_field(dat_field16b, data3);
    modify_field(key_field16b, key4);
    modify_field(dat_field16b, data4);
    modify_field(key_field16b, key5);
    modify_field(dat_field16b, data5);
    modify_field(key_field16b, key6);
    modify_field(dat_field16b, data6);
    modify_field(key_field16b, key7);
    modify_field(dat_field16b, data7);
    modify_field(key_field16b, key8);
    modify_field(dat_field16b, data8);
    modify_field(key_field16b, key9);
    modify_field(dat_field16b, data9);
    modify_field(key_field16b, key10);
    modify_field(dat_field16b, data10);
    modify_field(key_field16b, key11);
    modify_field(dat_field16b, data11);
    modify_field(key_field16b, key12);
    modify_field(dat_field16b, data12);
    modify_field(key_field16b, key13);
    modify_field(dat_field16b, data13);
    modify_field(key_field16b, key14);
    modify_field(dat_field16b, data14);

    if (key >= key7) {
        if (key >= key11) {
            if (key >= key13) {
                if (key >= key14) {
                    modify_field(lpm_result16b, data14);
                }
                else {
                    modify_field(lpm_result16b, data13);
                }
            }
            else {
                if (key >= key12) {
                    modify_field(lpm_result16b, data12);
                }
                else {
                    modify_field(lpm_result16b, data11);
                }
            }
        }
        else {
            if (key >= key9) {
                if (key >= key10) {
                    modify_field(lpm_result16b, data10);
                }
                else {
                    modify_field(lpm_result16b, data9);
                }
            }
            else {
                if (key >= key8) {
                    modify_field(lpm_result16b, data8);
                }
                else {
                    modify_field(lpm_result16b, data7);
                }
            }
        }
    }
    else {
        if (key >= key3) {
            if (key >= key5) {
                if (key >= key6) {
                    modify_field(lpm_result16b, data6);
                }
                else {
                    modify_field(lpm_result16b, data5);
                }
            }
            else {
                if (key >= key4) {
                    modify_field(lpm_result16b, data4);
                }
                else {
                    modify_field(lpm_result16b, data3);
                }
            }
        }
        else {
            if (key >= key1) {
                if (key >= key2) {
                    modify_field(lpm_result16b, data2);
                }
                else {
                    modify_field(lpm_result16b, data1);
                }
            }
            else {
                if (key >= key0) {
                    modify_field(lpm_result16b, data0);
                }
                else {
                    modify_field(lpm_result16b, data_);
                }
            }
        }
    }

    // Handle Result.
    result_handler16b();
}
#endif

#ifdef action_data32b
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
                modify_field(lpm_result32b, data6);
            }
            else {
                modify_field(lpm_result32b, data5);
            }
        }
        else {
            if (key >= key4) {
                modify_field(lpm_result32b, data4);
            }
            else {
                modify_field(lpm_result32b, data3);
            }
        }
    }
    else {
        if (key >= key1) {
            if (key >= key2) {
                modify_field(lpm_result32b, data2);
            }
            else {
                modify_field(lpm_result32b, data1);
            }
        }
        else {
            if (key >= key0) {
                modify_field(lpm_result32b, data0);
            }
            else {
                modify_field(lpm_result32b, data_);
            }
        }
    }

    // Handle Result.
    result_handler32b();
}
#endif

#ifdef action_data64b
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
            modify_field(lpm_result64b, data2);
        }
        else {
            modify_field(lpm_result64b, data1);
        }
    }
    else {
        if (key >= key0) {
            modify_field(lpm_result64b, data0);
        }
        else {
            modify_field(lpm_result64b, data_);
        }
    }

    // Handle Result.
    result_handler64b();
}
#endif

#ifdef action_data128b
action action_data128b(data_, key0, data0) {

    modify_field(dat_field128b, data_);
    modify_field(key_field128b, key0);
    modify_field(dat_field128b, data0);

    if (key >= key0) {
        modify_field(lpm_result128b, data0);
    }
    else {
        modify_field(lpm_result128b, data_);
    }

    // Handle Result.
    result_handler128b();
}
#endif

@pragma stage stage_num
@pragma hbm_table
@pragma raw_index_table
table table_name {
    reads {
            curr_addr : exact;
    }
    actions {
#ifdef action_keys16b
            action_keys16b;
#endif
#ifdef action_keys32b
            action_keys32b;
#endif
#ifdef action_keys64b
            action_keys64b;
#endif
#ifdef action_keys128b
            action_keys128b;
#endif
#ifdef action_data16b
            action_data16b;
#endif
#ifdef action_data32b
            action_data32b;
#endif
#ifdef action_data64b
            action_data64b;
#endif
#ifdef action_data128b
            action_data128b;
#endif
    }
}
