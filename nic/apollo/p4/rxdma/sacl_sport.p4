/*****************************************************************************/
/* Security ACL source port LPM lookup                                       */
/*****************************************************************************/

header_type sacl_sport_scratch_metadata_t {
    fields {
        key_d   : 16;
        data_d  : 16;
    }
}

#define sacl_sport_key          p4_to_rxdma_header.flow_sport
#define sacl_sport_base_addr    p4_to_rxdma_header.sacl_base_addr
#define sacl_sport_next_addr    sacl_metadata.sport_table_addr
#define sacl_sport_result       sacl_metadata.sport_class_id

@pragma scratch_metadata
metadata sacl_sport_scratch_metadata_t sacl_sport_scratch_metadata;

action sacl_sport_lpm_s0(key0, key1, key2, key3, key4, key5, key6, key7, key8,
                         key9, key10, key11, key12, key13, key14, key15, key16,
                         key17, key18, key19, key20, key21, key22, key23,
                         key24, key25, key26, key27, key28, key29, key30) {
    // Form the address for ipv4 lookup
    modify_field(sacl_metadata.ip_table_addr,
        p4_to_rxdma_header.sacl_base_addr + SACL_IP_TABLE_OFFSET);
    // Form the address for dport+proto lookup
    modify_field(sacl_metadata.proto_dport_table_addr,
        p4_to_rxdma_header.sacl_base_addr + SACL_PROTO_DPORT_TABLE_OFFSET);
    modify_field(sacl_metadata.proto_dport,
        (p4_to_rxdma_header.flow_dport | (p4_to_rxdma_header.flow_proto << 24)));
    if (p4_to_rxdma_header.direction == TX_FROM_HOST) {
        modify_field(sacl_metadata.ip, p4_to_rxdma_header.flow_dst);
    } else {
        modify_field(sacl_metadata.ip, p4_to_rxdma_header.flow_src);
    }

    if (sacl_sport_key >= key15) {
        if (sacl_sport_key >= key23) {
            if (sacl_sport_key >= key27) {
                if (sacl_sport_key >= key29) {
                    if (sacl_sport_key >= key30) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B31_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B30_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key28) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B29_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B28_OFFSET);
                    }
                }
            } else {
                if (sacl_sport_key >= key25) {
                    if (sacl_sport_key >= key26) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B27_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B26_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key24) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B25_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B24_OFFSET);
                    }
                }
            }
        } else {
            if (sacl_sport_key >= 19) {
                if (sacl_sport_key >= key21) {
                    if (sacl_sport_key >= key22) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B23_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B22_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key20) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B21_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B20_OFFSET);
                    }
                }
            } else {
                if (sacl_sport_key >= key17) {
                    if (sacl_sport_key >= key18) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B19_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B18_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key16) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B17_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B16_OFFSET);
                    }
                }
            }
        }
    } else {
        if (sacl_sport_key >= key7) {
            if (sacl_sport_key >= key11) {
                if (sacl_sport_key >= key13) {
                    if (sacl_sport_key >= key14) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B15_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B14_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key12) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B13_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B12_OFFSET);
                    }
                }
            } else {
                if (sacl_sport_key >= key9) {
                    if (sacl_sport_key >= key10) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B11_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B10_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key8) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B9_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B8_OFFSET);
                    }
                }
            }
        } else {
            if (sacl_sport_key >= key3) {
                if (sacl_sport_key >= key5) {
                    if (sacl_sport_key >= key6) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B7_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B6_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key4) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B5_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B4_OFFSET);
                    }
                }
            } else {
                if (sacl_sport_key >= key1) {
                    if (sacl_sport_key >= key2) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B3_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B2_OFFSET);
                    }
                } else {
                    if (sacl_sport_key >= key0) {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B1_OFFSET);
                    } else {
                        modify_field(sacl_sport_next_addr,
                                     sacl_sport_base_addr + SACL_SPORT_S1_B0_OFFSET);
                    }
                }
            }
        }
    }

    modify_field(sacl_sport_scratch_metadata.key_d, key0);
    modify_field(sacl_sport_scratch_metadata.key_d, key1);
    modify_field(sacl_sport_scratch_metadata.key_d, key2);
    modify_field(sacl_sport_scratch_metadata.key_d, key3);
    modify_field(sacl_sport_scratch_metadata.key_d, key4);
    modify_field(sacl_sport_scratch_metadata.key_d, key5);
    modify_field(sacl_sport_scratch_metadata.key_d, key6);
    modify_field(sacl_sport_scratch_metadata.key_d, key7);
    modify_field(sacl_sport_scratch_metadata.key_d, key8);
    modify_field(sacl_sport_scratch_metadata.key_d, key9);
    modify_field(sacl_sport_scratch_metadata.key_d, key10);
    modify_field(sacl_sport_scratch_metadata.key_d, key11);
    modify_field(sacl_sport_scratch_metadata.key_d, key12);
    modify_field(sacl_sport_scratch_metadata.key_d, key13);
    modify_field(sacl_sport_scratch_metadata.key_d, key14);
    modify_field(sacl_sport_scratch_metadata.key_d, key15);
    modify_field(sacl_sport_scratch_metadata.key_d, key16);
    modify_field(sacl_sport_scratch_metadata.key_d, key17);
    modify_field(sacl_sport_scratch_metadata.key_d, key18);
    modify_field(sacl_sport_scratch_metadata.key_d, key19);
    modify_field(sacl_sport_scratch_metadata.key_d, key20);
    modify_field(sacl_sport_scratch_metadata.key_d, key21);
    modify_field(sacl_sport_scratch_metadata.key_d, key22);
    modify_field(sacl_sport_scratch_metadata.key_d, key23);
    modify_field(sacl_sport_scratch_metadata.key_d, key24);
    modify_field(sacl_sport_scratch_metadata.key_d, key25);
    modify_field(sacl_sport_scratch_metadata.key_d, key26);
    modify_field(sacl_sport_scratch_metadata.key_d, key27);
    modify_field(sacl_sport_scratch_metadata.key_d, key28);
    modify_field(sacl_sport_scratch_metadata.key_d, key29);
    modify_field(sacl_sport_scratch_metadata.key_d, key30);
}

action sacl_sport_lpm_s1(data0, key0, data1, key1, data2, key2, data3,
                          key3, data4, key4, data5, key5, data6, key6, data7,
                          key7, data8, key8, data9, key9, data10, key10,
                          data11, key11, data12, key12, data13, key13, data14,
                          key14, data15) {
    if (sacl_sport_key >= key7) {
        if (sacl_sport_key >= key11) {
            if (sacl_sport_key >= key13) {
                if (sacl_sport_key >= key14) {
                    modify_field(sacl_sport_result, data15);
                }
                else {
                    modify_field(sacl_sport_result, data14);
                }
            }
            else {
                if (sacl_sport_key >= key12) {
                    modify_field(sacl_sport_result, data13);
                }
                else {
                    modify_field(sacl_sport_result, data12);
                }
            }
        }
        else {
            if (sacl_sport_key >= key9) {
                if (sacl_sport_key >= key10) {
                    modify_field(sacl_sport_result, data11);
                }
                else {
                    modify_field(sacl_sport_result, data10);
                }
            }
            else {
                if (sacl_sport_key >= key8) {
                    modify_field(sacl_sport_result, data9);
                }
                else {
                    modify_field(sacl_sport_result, data8);
                }
            }
        }
    }
    else {
        if (sacl_sport_key >= key3) {
            if (sacl_sport_key >= key5) {
                if (sacl_sport_key >= key6) {
                    modify_field(sacl_sport_result, data7);
                }
                else {
                    modify_field(sacl_sport_result, data6);
                }
            }
            else {
                if (sacl_sport_key >= key4) {
                    modify_field(sacl_sport_result, data5);
                }
                else {
                    modify_field(sacl_sport_result, data4);
                }
            }
        }
        else {
            if (sacl_sport_key >= key1) {
                if (sacl_sport_key >= key2) {
                    modify_field(sacl_sport_result, data3);
                }
                else {
                    modify_field(sacl_sport_result, data2);
                }
            }
            else {
                if (sacl_sport_key >= key0) {
                    modify_field(sacl_sport_result, data1);
                }
                else {
                    modify_field(sacl_sport_result, data0);
                }
            }
        }
    }

    modify_field(sacl_sport_scratch_metadata.key_d, key0);
    modify_field(sacl_sport_scratch_metadata.key_d, key1);
    modify_field(sacl_sport_scratch_metadata.key_d, key2);
    modify_field(sacl_sport_scratch_metadata.key_d, key3);
    modify_field(sacl_sport_scratch_metadata.key_d, key4);
    modify_field(sacl_sport_scratch_metadata.key_d, key5);
    modify_field(sacl_sport_scratch_metadata.key_d, key6);
    modify_field(sacl_sport_scratch_metadata.key_d, key7);
    modify_field(sacl_sport_scratch_metadata.key_d, key8);
    modify_field(sacl_sport_scratch_metadata.key_d, key9);
    modify_field(sacl_sport_scratch_metadata.key_d, key10);
    modify_field(sacl_sport_scratch_metadata.key_d, key11);
    modify_field(sacl_sport_scratch_metadata.key_d, key12);
    modify_field(sacl_sport_scratch_metadata.key_d, key13);
    modify_field(sacl_sport_scratch_metadata.key_d, key14);
    modify_field(sacl_sport_scratch_metadata.data_d, data0);
    modify_field(sacl_sport_scratch_metadata.data_d, data1);
    modify_field(sacl_sport_scratch_metadata.data_d, data2);
    modify_field(sacl_sport_scratch_metadata.data_d, data3);
    modify_field(sacl_sport_scratch_metadata.data_d, data4);
    modify_field(sacl_sport_scratch_metadata.data_d, data5);
    modify_field(sacl_sport_scratch_metadata.data_d, data6);
    modify_field(sacl_sport_scratch_metadata.data_d, data7);
    modify_field(sacl_sport_scratch_metadata.data_d, data8);
    modify_field(sacl_sport_scratch_metadata.data_d, data9);
    modify_field(sacl_sport_scratch_metadata.data_d, data10);
    modify_field(sacl_sport_scratch_metadata.data_d, data11);
    modify_field(sacl_sport_scratch_metadata.data_d, data12);
    modify_field(sacl_sport_scratch_metadata.data_d, data13);
    modify_field(sacl_sport_scratch_metadata.data_d, data14);
    modify_field(sacl_sport_scratch_metadata.data_d, data15);
}

@pragma stage 0
@pragma hbm_table
@pragma raw_index_table
table sacl_sport_lpm_s0 {
    reads {
        p4_to_rxdma_header.sacl_base_addr : exact;
    }
    actions {
        sacl_sport_lpm_s0;
    }
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table sacl_sport_lpm_s1 {
    reads {
        sacl_metadata.sport_table_addr : exact;
    }
    actions {
        sacl_sport_lpm_s1;
    }
}
