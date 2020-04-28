
action rfc_action_p1(id63, id62, id61, id60, id59, id58, id57, id56,
                     id55, id54, id53, id52, id51, id50, id49, id48,
                     id47, id46, id45, id44, id43, id42, id41, id40,
                     id39, id38, id37, id36, id35, id34, id33, id32,
                     id31, id30, id29, id28, id27, id26, id25, id24,
                     id23, id22, id21, id20, id19, id18, id17, id16,
                     id15, id14, id13, id12, id11, id10, id09, id08,
                     id07, id06, id05, id04, id03, id02, id01, id00
                    )
{
    modify_field(scratch_metadata.field8, id00);
    modify_field(scratch_metadata.field8, id01);
    modify_field(scratch_metadata.field8, id02);
    modify_field(scratch_metadata.field8, id03);
    modify_field(scratch_metadata.field8, id04);
    modify_field(scratch_metadata.field8, id05);
    modify_field(scratch_metadata.field8, id06);
    modify_field(scratch_metadata.field8, id07);
    modify_field(scratch_metadata.field8, id08);
    modify_field(scratch_metadata.field8, id09);
    modify_field(scratch_metadata.field8, id10);
    modify_field(scratch_metadata.field8, id11);
    modify_field(scratch_metadata.field8, id12);
    modify_field(scratch_metadata.field8, id13);
    modify_field(scratch_metadata.field8, id14);
    modify_field(scratch_metadata.field8, id15);
    modify_field(scratch_metadata.field8, id16);
    modify_field(scratch_metadata.field8, id17);
    modify_field(scratch_metadata.field8, id18);
    modify_field(scratch_metadata.field8, id19);
    modify_field(scratch_metadata.field8, id20);
    modify_field(scratch_metadata.field8, id21);
    modify_field(scratch_metadata.field8, id22);
    modify_field(scratch_metadata.field8, id23);
    modify_field(scratch_metadata.field8, id24);
    modify_field(scratch_metadata.field8, id25);
    modify_field(scratch_metadata.field8, id26);
    modify_field(scratch_metadata.field8, id27);
    modify_field(scratch_metadata.field8, id28);
    modify_field(scratch_metadata.field8, id29);
    modify_field(scratch_metadata.field8, id30);
    modify_field(scratch_metadata.field8, id31);
    modify_field(scratch_metadata.field8, id32);
    modify_field(scratch_metadata.field8, id33);
    modify_field(scratch_metadata.field8, id34);
    modify_field(scratch_metadata.field8, id35);
    modify_field(scratch_metadata.field8, id36);
    modify_field(scratch_metadata.field8, id37);
    modify_field(scratch_metadata.field8, id38);
    modify_field(scratch_metadata.field8, id39);
    modify_field(scratch_metadata.field8, id40);
    modify_field(scratch_metadata.field8, id41);
    modify_field(scratch_metadata.field8, id42);
    modify_field(scratch_metadata.field8, id43);
    modify_field(scratch_metadata.field8, id44);
    modify_field(scratch_metadata.field8, id45);
    modify_field(scratch_metadata.field8, id46);
    modify_field(scratch_metadata.field8, id47);
    modify_field(scratch_metadata.field8, id48);
    modify_field(scratch_metadata.field8, id49);
    modify_field(scratch_metadata.field8, id50);
    modify_field(scratch_metadata.field8, id51);
    modify_field(scratch_metadata.field8, id52);
    modify_field(scratch_metadata.field8, id53);
    modify_field(scratch_metadata.field8, id54);
    modify_field(scratch_metadata.field8, id55);
    modify_field(scratch_metadata.field8, id56);
    modify_field(scratch_metadata.field8, id57);
    modify_field(scratch_metadata.field8, id58);
    modify_field(scratch_metadata.field8, id59);
    modify_field(scratch_metadata.field8, id60);
    modify_field(scratch_metadata.field8, id61);
    modify_field(scratch_metadata.field8, id62);
    modify_field(scratch_metadata.field8, id63);

    /* Get the P1 classid by indexing into the classid array */
    modify_field(scratch_metadata.field8, (id00 >> ((txdma_control.rfc_index %
                 SACL_P1_ENTRIES_PER_CACHE_LINE)*SACL_P1_CLASSID_WIDTH)));

    // Write the P1 classid to PHV
    modify_field(txdma_control.rfc_p1_classid, scratch_metadata.field8);

    // Initialize the correct table base and index based on the recirc count
    if ((txdma_control.recirc_count & 0x1) == 0) {
        // P2 table base
        modify_field(scratch_metadata.field40, SACL_P2_1_TABLE_OFFSET);
        // P2 table index
        modify_field(scratch_metadata.field20, (rx_to_tx_hdr.dip_classid0 <<
                                                SACL_PROTO_DPORT_CLASSID_WIDTH)|
                                                rx_to_tx_hdr.dport_classid0);
    } else {
        // P2 table base
        modify_field(scratch_metadata.field40, SACL_P2_3_TABLE_OFFSET);
        // P2 table index
        modify_field(scratch_metadata.field20, (rx_to_tx_hdr.sport_classid0 <<
                                                SACL_PROTO_DPORT_CLASSID_WIDTH)|
                                                rx_to_tx_hdr.dport_classid0);
    }

    // Write P2 table index to PHV
    modify_field(txdma_control.rfc_index, scratch_metadata.field20);

    // Write P2 table lookup address to PHV
    modify_field(txdma_control.rfc_table_addr,              // P2 Lookup Addr =
                 rx_to_tx_hdr.sacl_base_addr0 +             // Region Base +
                 scratch_metadata.field40 +                 // Table Base +
                 (((scratch_metadata.field20) /
                  SACL_P2_ENTRIES_PER_CACHE_LINE) *
                  SACL_CACHE_LINE_SIZE));                   // Index Bytes
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table rfc_p1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        rfc_action_p1;
    }
}

action rfc_action_p2(id63, id62, id61, id60, id59, id58, id57, id56,
                     id55, id54, id53, id52, id51, id50, id49, id48,
                     id47, id46, id45, id44, id43, id42, id41, id40,
                     id39, id38, id37, id36, id35, id34, id33, id32,
                     id31, id30, id29, id28, id27, id26, id25, id24,
                     id23, id22, id21, id20, id19, id18, id17, id16,
                     id15, id14, id13, id12, id11, id10, id09, id08,
                     id07, id06, id05, id04, id03, id02, id01, id00
                    )
{
    modify_field(scratch_metadata.field8, id00);
    modify_field(scratch_metadata.field8, id01);
    modify_field(scratch_metadata.field8, id02);
    modify_field(scratch_metadata.field8, id03);
    modify_field(scratch_metadata.field8, id04);
    modify_field(scratch_metadata.field8, id05);
    modify_field(scratch_metadata.field8, id06);
    modify_field(scratch_metadata.field8, id07);
    modify_field(scratch_metadata.field8, id08);
    modify_field(scratch_metadata.field8, id09);
    modify_field(scratch_metadata.field8, id10);
    modify_field(scratch_metadata.field8, id11);
    modify_field(scratch_metadata.field8, id12);
    modify_field(scratch_metadata.field8, id13);
    modify_field(scratch_metadata.field8, id14);
    modify_field(scratch_metadata.field8, id15);
    modify_field(scratch_metadata.field8, id16);
    modify_field(scratch_metadata.field8, id17);
    modify_field(scratch_metadata.field8, id18);
    modify_field(scratch_metadata.field8, id19);
    modify_field(scratch_metadata.field8, id20);
    modify_field(scratch_metadata.field8, id21);
    modify_field(scratch_metadata.field8, id22);
    modify_field(scratch_metadata.field8, id23);
    modify_field(scratch_metadata.field8, id24);
    modify_field(scratch_metadata.field8, id25);
    modify_field(scratch_metadata.field8, id26);
    modify_field(scratch_metadata.field8, id27);
    modify_field(scratch_metadata.field8, id28);
    modify_field(scratch_metadata.field8, id29);
    modify_field(scratch_metadata.field8, id30);
    modify_field(scratch_metadata.field8, id31);
    modify_field(scratch_metadata.field8, id32);
    modify_field(scratch_metadata.field8, id33);
    modify_field(scratch_metadata.field8, id34);
    modify_field(scratch_metadata.field8, id35);
    modify_field(scratch_metadata.field8, id36);
    modify_field(scratch_metadata.field8, id37);
    modify_field(scratch_metadata.field8, id38);
    modify_field(scratch_metadata.field8, id39);
    modify_field(scratch_metadata.field8, id40);
    modify_field(scratch_metadata.field8, id41);
    modify_field(scratch_metadata.field8, id42);
    modify_field(scratch_metadata.field8, id43);
    modify_field(scratch_metadata.field8, id44);
    modify_field(scratch_metadata.field8, id45);
    modify_field(scratch_metadata.field8, id46);
    modify_field(scratch_metadata.field8, id47);
    modify_field(scratch_metadata.field8, id48);
    modify_field(scratch_metadata.field8, id49);
    modify_field(scratch_metadata.field8, id50);
    modify_field(scratch_metadata.field8, id51);
    modify_field(scratch_metadata.field8, id52);
    modify_field(scratch_metadata.field8, id53);
    modify_field(scratch_metadata.field8, id54);
    modify_field(scratch_metadata.field8, id55);
    modify_field(scratch_metadata.field8, id56);
    modify_field(scratch_metadata.field8, id57);
    modify_field(scratch_metadata.field8, id58);
    modify_field(scratch_metadata.field8, id59);
    modify_field(scratch_metadata.field8, id60);
    modify_field(scratch_metadata.field8, id61);
    modify_field(scratch_metadata.field8, id62);
    modify_field(scratch_metadata.field8, id63);

    /* Get the P2 classid by indexing into the classid array */
    modify_field(scratch_metadata.field8, (id00 >> ((txdma_control.rfc_index %
                 SACL_P2_ENTRIES_PER_CACHE_LINE)*SACL_P2_CLASSID_WIDTH)));

    // Initialize the correct table base based on the recirc count
    if ((txdma_control.recirc_count & 0x1) == 0) {
        // P3 table base
        modify_field(scratch_metadata.field40, SACL_P3_1_TABLE_OFFSET);
    } else {
        // P3 table base
        modify_field(scratch_metadata.field40, SACL_P3_3_TABLE_OFFSET);
    }

    // P3 table index
    modify_field(scratch_metadata.field20, ((txdma_control.rfc_p1_classid <<
                                            SACL_P2_CLASSID_WIDTH) |
                                            scratch_metadata.field8));
    // Write P3 table index to PHV
    modify_field(txdma_control.rfc_index, scratch_metadata.field20);

    // Write P3 table lookup address to PHV
    modify_field(txdma_control.rfc_table_addr,              // P3 Lookup Addr =
                 rx_to_tx_hdr.sacl_base_addr0 +             // Region Base +
                 scratch_metadata.field40 +                 // Table Base +
                 (((scratch_metadata.field20) /
                  SACL_P3_ENTRIES_PER_CACHE_LINE) *
                  SACL_CACHE_LINE_SIZE));                   // Index Bytes
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table rfc_p2 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        rfc_action_p2;
    }
}

action rfc_action_p3(pad,pr45, res45, pr44, res44,
                         pr43, res43, pr42, res42, pr41, res41, pr40, res40,
                         pr39, res39, pr38, res38, pr37, res37, pr36, res36,
                         pr35, res35, pr34, res34, pr33, res33, pr32, res32,
                         pr31, res31, pr30, res30, pr29, res29, pr28, res28,
                         pr27, res27, pr26, res26, pr25, res25, pr24, res24,
                         pr23, res23, pr22, res22, pr21, res21, pr20, res20,
                         pr19, res19, pr18, res18, pr17, res17, pr16, res16,
                         pr15, res15, pr14, res14, pr13, res13, pr12, res12,
                         pr11, res11, pr10, res10, pr09, res09, pr08, res08,
                         pr07, res07, pr06, res06, pr05, res05, pr04, res04,
                         pr03, res03, pr02, res02, pr01, res01, pr00, res00
                     )
{
    modify_field(scratch_metadata.field10, pr00);
    modify_field(scratch_metadata.field1, res00);
    modify_field(scratch_metadata.field10, pr01);
    modify_field(scratch_metadata.field1, res01);
    modify_field(scratch_metadata.field10, pr02);
    modify_field(scratch_metadata.field1, res02);
    modify_field(scratch_metadata.field10, pr03);
    modify_field(scratch_metadata.field1, res03);
    modify_field(scratch_metadata.field10, pr04);
    modify_field(scratch_metadata.field1, res04);
    modify_field(scratch_metadata.field10, pr05);
    modify_field(scratch_metadata.field1, res05);
    modify_field(scratch_metadata.field10, pr06);
    modify_field(scratch_metadata.field1, res06);
    modify_field(scratch_metadata.field10, pr07);
    modify_field(scratch_metadata.field1, res07);
    modify_field(scratch_metadata.field10, pr08);
    modify_field(scratch_metadata.field1, res08);
    modify_field(scratch_metadata.field10, pr09);
    modify_field(scratch_metadata.field1, res09);
    modify_field(scratch_metadata.field10, pr10);
    modify_field(scratch_metadata.field1, res10);
    modify_field(scratch_metadata.field10, pr11);
    modify_field(scratch_metadata.field1, res11);
    modify_field(scratch_metadata.field10, pr12);
    modify_field(scratch_metadata.field1, res12);
    modify_field(scratch_metadata.field10, pr13);
    modify_field(scratch_metadata.field1, res13);
    modify_field(scratch_metadata.field10, pr14);
    modify_field(scratch_metadata.field1, res14);
    modify_field(scratch_metadata.field10, pr15);
    modify_field(scratch_metadata.field1, res15);
    modify_field(scratch_metadata.field10, pr16);
    modify_field(scratch_metadata.field1, res16);
    modify_field(scratch_metadata.field10, pr17);
    modify_field(scratch_metadata.field1, res17);
    modify_field(scratch_metadata.field10, pr18);
    modify_field(scratch_metadata.field1, res18);
    modify_field(scratch_metadata.field10, pr19);
    modify_field(scratch_metadata.field1, res19);
    modify_field(scratch_metadata.field10, pr20);
    modify_field(scratch_metadata.field1, res20);
    modify_field(scratch_metadata.field10, pr21);
    modify_field(scratch_metadata.field1, res21);
    modify_field(scratch_metadata.field10, pr22);
    modify_field(scratch_metadata.field1, res22);
    modify_field(scratch_metadata.field10, pr23);
    modify_field(scratch_metadata.field1, res23);
    modify_field(scratch_metadata.field10, pr24);
    modify_field(scratch_metadata.field1, res24);
    modify_field(scratch_metadata.field10, pr25);
    modify_field(scratch_metadata.field1, res25);
    modify_field(scratch_metadata.field10, pr26);
    modify_field(scratch_metadata.field1, res26);
    modify_field(scratch_metadata.field10, pr27);
    modify_field(scratch_metadata.field1, res27);
    modify_field(scratch_metadata.field10, pr28);
    modify_field(scratch_metadata.field1, res28);
    modify_field(scratch_metadata.field10, pr29);
    modify_field(scratch_metadata.field1, res29);
    modify_field(scratch_metadata.field10, pr30);
    modify_field(scratch_metadata.field1, res30);
    modify_field(scratch_metadata.field10, pr31);
    modify_field(scratch_metadata.field1, res31);
    modify_field(scratch_metadata.field10, pr32);
    modify_field(scratch_metadata.field1, res32);
    modify_field(scratch_metadata.field10, pr33);
    modify_field(scratch_metadata.field1, res33);
    modify_field(scratch_metadata.field10, pr34);
    modify_field(scratch_metadata.field1, res34);
    modify_field(scratch_metadata.field10, pr35);
    modify_field(scratch_metadata.field1, res35);
    modify_field(scratch_metadata.field10, pr36);
    modify_field(scratch_metadata.field1, res36);
    modify_field(scratch_metadata.field10, pr37);
    modify_field(scratch_metadata.field1, res37);
    modify_field(scratch_metadata.field10, pr38);
    modify_field(scratch_metadata.field1, res38);
    modify_field(scratch_metadata.field10, pr39);
    modify_field(scratch_metadata.field1, res39);
    modify_field(scratch_metadata.field10, pr40);
    modify_field(scratch_metadata.field1, res40);
    modify_field(scratch_metadata.field10, pr41);
    modify_field(scratch_metadata.field1, res41);
    modify_field(scratch_metadata.field10, pr42);
    modify_field(scratch_metadata.field1, res42);
    modify_field(scratch_metadata.field10, pr43);
    modify_field(scratch_metadata.field1, res43);
    modify_field(scratch_metadata.field10, pr44);
    modify_field(scratch_metadata.field1, res44);
    modify_field(scratch_metadata.field10, pr45);
    modify_field(scratch_metadata.field1, res45);
    modify_field(scratch_metadata.field6,  pad);

    /* Get the P3 result by indexing into the classid array */
    modify_field(scratch_metadata.field10,pr00>>(txdma_control.rfc_index%
                 SACL_P3_ENTRIES_PER_CACHE_LINE)*SACL_P3_ENTRY_WIDTH);
    modify_field(scratch_metadata.field1,res00>>(txdma_control.rfc_index%
                 SACL_P3_ENTRIES_PER_CACHE_LINE)*SACL_P3_ENTRY_WIDTH);

    // If the priority of the current result is higher than whats in the PHV
    if (txdma_to_p4e.sacl_priority > scratch_metadata.field10) {
        // Then overwrite the result in the PHV with the current one
        modify_field(txdma_to_p4e.sacl_priority, scratch_metadata.field10);
        modify_field(txdma_to_p4e.drop, scratch_metadata.field1);
        modify_field(txdma_to_p4e.sacl_action, scratch_metadata.field1);
        modify_field(txdma_to_p4e.sacl_root_num, txdma_control.root_count);
    }

    // Initialize the correct table base and index based on the recirc count
    if ((txdma_control.recirc_count & 0x1) == 0) {
        // P1 table base
        modify_field(scratch_metadata.field40, SACL_P1_2_TABLE_OFFSET);
        // P1 table index
        modify_field(scratch_metadata.field20, ((txdma_control.stag_classid <<
                                                SACL_DIP_CLASSID_WIDTH) |
                                                rx_to_tx_hdr.dip_classid0));
    } else {
        // P1 table base
        modify_field(scratch_metadata.field40, SACL_P1_4_TABLE_OFFSET);
        // P1 table index
        modify_field(scratch_metadata.field20, ((txdma_control.stag_classid <<
                                                SACL_SPORT_CLASSID_WIDTH) |
                                                rx_to_tx_hdr.sport_classid0));
    }

    // Write P1 table index to PHV
    modify_field(txdma_control.rfc_index, scratch_metadata.field20);

    // Write P1 table lookup address to PHV
    modify_field(txdma_control.rfc_table_addr,              // P1 Lookup Addr =
                 rx_to_tx_hdr.sacl_base_addr0 +             // Region Base +
                 scratch_metadata.field40 +                 // Table Base +
                 (((scratch_metadata.field20) /
                  SACL_P1_ENTRIES_PER_CACHE_LINE) *
                  SACL_CACHE_LINE_SIZE));                   // Index Bytes
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table rfc_p3 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        rfc_action_p3;
    }
}

action rfc_action_p1_1(id63, id62, id61, id60, id59, id58, id57, id56,
                       id55, id54, id53, id52, id51, id50, id49, id48,
                       id47, id46, id45, id44, id43, id42, id41, id40,
                       id39, id38, id37, id36, id35, id34, id33, id32,
                       id31, id30, id29, id28, id27, id26, id25, id24,
                       id23, id22, id21, id20, id19, id18, id17, id16,
                       id15, id14, id13, id12, id11, id10, id09, id08,
                       id07, id06, id05, id04, id03, id02, id01, id00
                      )
{
    modify_field(scratch_metadata.field8, id00);
    modify_field(scratch_metadata.field8, id01);
    modify_field(scratch_metadata.field8, id02);
    modify_field(scratch_metadata.field8, id03);
    modify_field(scratch_metadata.field8, id04);
    modify_field(scratch_metadata.field8, id05);
    modify_field(scratch_metadata.field8, id06);
    modify_field(scratch_metadata.field8, id07);
    modify_field(scratch_metadata.field8, id08);
    modify_field(scratch_metadata.field8, id09);
    modify_field(scratch_metadata.field8, id10);
    modify_field(scratch_metadata.field8, id11);
    modify_field(scratch_metadata.field8, id12);
    modify_field(scratch_metadata.field8, id13);
    modify_field(scratch_metadata.field8, id14);
    modify_field(scratch_metadata.field8, id15);
    modify_field(scratch_metadata.field8, id16);
    modify_field(scratch_metadata.field8, id17);
    modify_field(scratch_metadata.field8, id18);
    modify_field(scratch_metadata.field8, id19);
    modify_field(scratch_metadata.field8, id20);
    modify_field(scratch_metadata.field8, id21);
    modify_field(scratch_metadata.field8, id22);
    modify_field(scratch_metadata.field8, id23);
    modify_field(scratch_metadata.field8, id24);
    modify_field(scratch_metadata.field8, id25);
    modify_field(scratch_metadata.field8, id26);
    modify_field(scratch_metadata.field8, id27);
    modify_field(scratch_metadata.field8, id28);
    modify_field(scratch_metadata.field8, id29);
    modify_field(scratch_metadata.field8, id30);
    modify_field(scratch_metadata.field8, id31);
    modify_field(scratch_metadata.field8, id32);
    modify_field(scratch_metadata.field8, id33);
    modify_field(scratch_metadata.field8, id34);
    modify_field(scratch_metadata.field8, id35);
    modify_field(scratch_metadata.field8, id36);
    modify_field(scratch_metadata.field8, id37);
    modify_field(scratch_metadata.field8, id38);
    modify_field(scratch_metadata.field8, id39);
    modify_field(scratch_metadata.field8, id40);
    modify_field(scratch_metadata.field8, id41);
    modify_field(scratch_metadata.field8, id42);
    modify_field(scratch_metadata.field8, id43);
    modify_field(scratch_metadata.field8, id44);
    modify_field(scratch_metadata.field8, id45);
    modify_field(scratch_metadata.field8, id46);
    modify_field(scratch_metadata.field8, id47);
    modify_field(scratch_metadata.field8, id48);
    modify_field(scratch_metadata.field8, id49);
    modify_field(scratch_metadata.field8, id50);
    modify_field(scratch_metadata.field8, id51);
    modify_field(scratch_metadata.field8, id52);
    modify_field(scratch_metadata.field8, id53);
    modify_field(scratch_metadata.field8, id54);
    modify_field(scratch_metadata.field8, id55);
    modify_field(scratch_metadata.field8, id56);
    modify_field(scratch_metadata.field8, id57);
    modify_field(scratch_metadata.field8, id58);
    modify_field(scratch_metadata.field8, id59);
    modify_field(scratch_metadata.field8, id60);
    modify_field(scratch_metadata.field8, id61);
    modify_field(scratch_metadata.field8, id62);
    modify_field(scratch_metadata.field8, id63);

    /* Get the P1 classid by indexing into the classid array */
    modify_field(scratch_metadata.field8, (id00 >> ((txdma_control.rfc_index %
                 SACL_P1_ENTRIES_PER_CACHE_LINE)*SACL_P1_CLASSID_WIDTH)));

    // Write the P1 classid to PHV
    modify_field(txdma_control.rfc_p1_classid, scratch_metadata.field8);

    // Initialize the correct table base and index based on the recirc count
    if ((txdma_control.recirc_count & 0x1) == 0) {
        // P2 table base
        modify_field(scratch_metadata.field40, SACL_P2_2_TABLE_OFFSET);
        // P2 table index
        modify_field(scratch_metadata.field20, ((rx_to_tx_hdr.sport_classid0 <<
                                                 SACL_PROTO_DPORT_CLASSID_WIDTH) |
                                                 rx_to_tx_hdr.dport_classid0));
    } else {
        // P2 table base
        modify_field(scratch_metadata.field40, SACL_P2_4_TABLE_OFFSET);
        // P2 table index
        modify_field(scratch_metadata.field20, ((txdma_control.dtag_classid <<
                                                 SACL_PROTO_DPORT_CLASSID_WIDTH) |
                                                 rx_to_tx_hdr.dport_classid0));
    }

    // Write P2 table index to PHV
    modify_field(txdma_control.rfc_index, scratch_metadata.field20);

    // Write P2 table lookup address to PHV
    modify_field(txdma_control.rfc_table_addr,              // P2 Lookup Addr =
                 rx_to_tx_hdr.sacl_base_addr0 +             // Region Base +
                 scratch_metadata.field40 +                 // Table Base +
                 (((scratch_metadata.field20) /
                  SACL_P2_ENTRIES_PER_CACHE_LINE) *
                  SACL_CACHE_LINE_SIZE));                   // Index Bytes
}

@pragma stage 5
@pragma hbm_table
@pragma raw_index_table
table rfc_p1_1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        rfc_action_p1_1;
    }
}

action rfc_action_p2_1(id63, id62, id61, id60, id59, id58, id57, id56,
                       id55, id54, id53, id52, id51, id50, id49, id48,
                       id47, id46, id45, id44, id43, id42, id41, id40,
                       id39, id38, id37, id36, id35, id34, id33, id32,
                       id31, id30, id29, id28, id27, id26, id25, id24,
                       id23, id22, id21, id20, id19, id18, id17, id16,
                       id15, id14, id13, id12, id11, id10, id09, id08,
                       id07, id06, id05, id04, id03, id02, id01, id00
                      )
{
    modify_field(scratch_metadata.field8, id00);
    modify_field(scratch_metadata.field8, id01);
    modify_field(scratch_metadata.field8, id02);
    modify_field(scratch_metadata.field8, id03);
    modify_field(scratch_metadata.field8, id04);
    modify_field(scratch_metadata.field8, id05);
    modify_field(scratch_metadata.field8, id06);
    modify_field(scratch_metadata.field8, id07);
    modify_field(scratch_metadata.field8, id08);
    modify_field(scratch_metadata.field8, id09);
    modify_field(scratch_metadata.field8, id10);
    modify_field(scratch_metadata.field8, id11);
    modify_field(scratch_metadata.field8, id12);
    modify_field(scratch_metadata.field8, id13);
    modify_field(scratch_metadata.field8, id14);
    modify_field(scratch_metadata.field8, id15);
    modify_field(scratch_metadata.field8, id16);
    modify_field(scratch_metadata.field8, id17);
    modify_field(scratch_metadata.field8, id18);
    modify_field(scratch_metadata.field8, id19);
    modify_field(scratch_metadata.field8, id20);
    modify_field(scratch_metadata.field8, id21);
    modify_field(scratch_metadata.field8, id22);
    modify_field(scratch_metadata.field8, id23);
    modify_field(scratch_metadata.field8, id24);
    modify_field(scratch_metadata.field8, id25);
    modify_field(scratch_metadata.field8, id26);
    modify_field(scratch_metadata.field8, id27);
    modify_field(scratch_metadata.field8, id28);
    modify_field(scratch_metadata.field8, id29);
    modify_field(scratch_metadata.field8, id30);
    modify_field(scratch_metadata.field8, id31);
    modify_field(scratch_metadata.field8, id32);
    modify_field(scratch_metadata.field8, id33);
    modify_field(scratch_metadata.field8, id34);
    modify_field(scratch_metadata.field8, id35);
    modify_field(scratch_metadata.field8, id36);
    modify_field(scratch_metadata.field8, id37);
    modify_field(scratch_metadata.field8, id38);
    modify_field(scratch_metadata.field8, id39);
    modify_field(scratch_metadata.field8, id40);
    modify_field(scratch_metadata.field8, id41);
    modify_field(scratch_metadata.field8, id42);
    modify_field(scratch_metadata.field8, id43);
    modify_field(scratch_metadata.field8, id44);
    modify_field(scratch_metadata.field8, id45);
    modify_field(scratch_metadata.field8, id46);
    modify_field(scratch_metadata.field8, id47);
    modify_field(scratch_metadata.field8, id48);
    modify_field(scratch_metadata.field8, id49);
    modify_field(scratch_metadata.field8, id50);
    modify_field(scratch_metadata.field8, id51);
    modify_field(scratch_metadata.field8, id52);
    modify_field(scratch_metadata.field8, id53);
    modify_field(scratch_metadata.field8, id54);
    modify_field(scratch_metadata.field8, id55);
    modify_field(scratch_metadata.field8, id56);
    modify_field(scratch_metadata.field8, id57);
    modify_field(scratch_metadata.field8, id58);
    modify_field(scratch_metadata.field8, id59);
    modify_field(scratch_metadata.field8, id60);
    modify_field(scratch_metadata.field8, id61);
    modify_field(scratch_metadata.field8, id62);
    modify_field(scratch_metadata.field8, id63);

    /* Get the P2 classid by indexing into the classid array */
    modify_field(scratch_metadata.field8, (id00 >> ((txdma_control.rfc_index %
                 SACL_P2_ENTRIES_PER_CACHE_LINE)*SACL_P2_CLASSID_WIDTH)));

    // Initialize the correct table base based on the recirc count
    if ((txdma_control.recirc_count & 0x1) == 0) {
        // P3 table base
        modify_field(scratch_metadata.field40, SACL_P3_2_TABLE_OFFSET);
    } else {
        // P3 table base
        modify_field(scratch_metadata.field40, SACL_P3_4_TABLE_OFFSET);
    }

    // P3 table index
    modify_field(scratch_metadata.field20, ((txdma_control.rfc_p1_classid <<
                                            SACL_P2_CLASSID_WIDTH) |
                                            scratch_metadata.field8));

    // Write P3 table index to PHV
    modify_field(txdma_control.rfc_index, scratch_metadata.field20);

    // Write P3 table lookup address to PHV
    modify_field(txdma_control.rfc_table_addr,              // P3 Lookup Addr =
                 rx_to_tx_hdr.sacl_base_addr0 +             // Region Base +
                 scratch_metadata.field40 +                 // Table Base +
                 (((scratch_metadata.field20) /
                  SACL_P3_ENTRIES_PER_CACHE_LINE) *
                  SACL_CACHE_LINE_SIZE));                   // Index Bytes
}

@pragma stage 6
@pragma hbm_table
@pragma raw_index_table
table rfc_p2_1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        rfc_action_p2_1;
    }
}

action rfc_action_p3_1(pad,pr45, res45, pr44, res44,
                           pr43, res43, pr42, res42, pr41, res41, pr40, res40,
                           pr39, res39, pr38, res38, pr37, res37, pr36, res36,
                           pr35, res35, pr34, res34, pr33, res33, pr32, res32,
                           pr31, res31, pr30, res30, pr29, res29, pr28, res28,
                           pr27, res27, pr26, res26, pr25, res25, pr24, res24,
                           pr23, res23, pr22, res22, pr21, res21, pr20, res20,
                           pr19, res19, pr18, res18, pr17, res17, pr16, res16,
                           pr15, res15, pr14, res14, pr13, res13, pr12, res12,
                           pr11, res11, pr10, res10, pr09, res09, pr08, res08,
                           pr07, res07, pr06, res06, pr05, res05, pr04, res04,
                           pr03, res03, pr02, res02, pr01, res01, pr00, res00
                      )
{
    modify_field(scratch_metadata.field10, pr00);
    modify_field(scratch_metadata.field1, res00);
    modify_field(scratch_metadata.field10, pr01);
    modify_field(scratch_metadata.field1, res01);
    modify_field(scratch_metadata.field10, pr02);
    modify_field(scratch_metadata.field1, res02);
    modify_field(scratch_metadata.field10, pr03);
    modify_field(scratch_metadata.field1, res03);
    modify_field(scratch_metadata.field10, pr04);
    modify_field(scratch_metadata.field1, res04);
    modify_field(scratch_metadata.field10, pr05);
    modify_field(scratch_metadata.field1, res05);
    modify_field(scratch_metadata.field10, pr06);
    modify_field(scratch_metadata.field1, res06);
    modify_field(scratch_metadata.field10, pr07);
    modify_field(scratch_metadata.field1, res07);
    modify_field(scratch_metadata.field10, pr08);
    modify_field(scratch_metadata.field1, res08);
    modify_field(scratch_metadata.field10, pr09);
    modify_field(scratch_metadata.field1, res09);
    modify_field(scratch_metadata.field10, pr10);
    modify_field(scratch_metadata.field1, res10);
    modify_field(scratch_metadata.field10, pr11);
    modify_field(scratch_metadata.field1, res11);
    modify_field(scratch_metadata.field10, pr12);
    modify_field(scratch_metadata.field1, res12);
    modify_field(scratch_metadata.field10, pr13);
    modify_field(scratch_metadata.field1, res13);
    modify_field(scratch_metadata.field10, pr14);
    modify_field(scratch_metadata.field1, res14);
    modify_field(scratch_metadata.field10, pr15);
    modify_field(scratch_metadata.field1, res15);
    modify_field(scratch_metadata.field10, pr16);
    modify_field(scratch_metadata.field1, res16);
    modify_field(scratch_metadata.field10, pr17);
    modify_field(scratch_metadata.field1, res17);
    modify_field(scratch_metadata.field10, pr18);
    modify_field(scratch_metadata.field1, res18);
    modify_field(scratch_metadata.field10, pr19);
    modify_field(scratch_metadata.field1, res19);
    modify_field(scratch_metadata.field10, pr20);
    modify_field(scratch_metadata.field1, res20);
    modify_field(scratch_metadata.field10, pr21);
    modify_field(scratch_metadata.field1, res21);
    modify_field(scratch_metadata.field10, pr22);
    modify_field(scratch_metadata.field1, res22);
    modify_field(scratch_metadata.field10, pr23);
    modify_field(scratch_metadata.field1, res23);
    modify_field(scratch_metadata.field10, pr24);
    modify_field(scratch_metadata.field1, res24);
    modify_field(scratch_metadata.field10, pr25);
    modify_field(scratch_metadata.field1, res25);
    modify_field(scratch_metadata.field10, pr26);
    modify_field(scratch_metadata.field1, res26);
    modify_field(scratch_metadata.field10, pr27);
    modify_field(scratch_metadata.field1, res27);
    modify_field(scratch_metadata.field10, pr28);
    modify_field(scratch_metadata.field1, res28);
    modify_field(scratch_metadata.field10, pr29);
    modify_field(scratch_metadata.field1, res29);
    modify_field(scratch_metadata.field10, pr30);
    modify_field(scratch_metadata.field1, res30);
    modify_field(scratch_metadata.field10, pr31);
    modify_field(scratch_metadata.field1, res31);
    modify_field(scratch_metadata.field10, pr32);
    modify_field(scratch_metadata.field1, res32);
    modify_field(scratch_metadata.field10, pr33);
    modify_field(scratch_metadata.field1, res33);
    modify_field(scratch_metadata.field10, pr34);
    modify_field(scratch_metadata.field1, res34);
    modify_field(scratch_metadata.field10, pr35);
    modify_field(scratch_metadata.field1, res35);
    modify_field(scratch_metadata.field10, pr36);
    modify_field(scratch_metadata.field1, res36);
    modify_field(scratch_metadata.field10, pr37);
    modify_field(scratch_metadata.field1, res37);
    modify_field(scratch_metadata.field10, pr38);
    modify_field(scratch_metadata.field1, res38);
    modify_field(scratch_metadata.field10, pr39);
    modify_field(scratch_metadata.field1, res39);
    modify_field(scratch_metadata.field10, pr40);
    modify_field(scratch_metadata.field1, res40);
    modify_field(scratch_metadata.field10, pr41);
    modify_field(scratch_metadata.field1, res41);
    modify_field(scratch_metadata.field10, pr42);
    modify_field(scratch_metadata.field1, res42);
    modify_field(scratch_metadata.field10, pr43);
    modify_field(scratch_metadata.field1, res43);
    modify_field(scratch_metadata.field10, pr44);
    modify_field(scratch_metadata.field1, res44);
    modify_field(scratch_metadata.field10, pr45);
    modify_field(scratch_metadata.field1, res45);
    modify_field(scratch_metadata.field6,  pad);

    /* Get the P3 result by indexing into the classid array */
    /* Get the P3 result by indexing into the classid array */
    modify_field(scratch_metadata.field10,pr00>>(txdma_control.rfc_index%
                 SACL_P3_ENTRIES_PER_CACHE_LINE)*SACL_P3_ENTRY_WIDTH);
    modify_field(scratch_metadata.field1,res00>>(txdma_control.rfc_index%
                 SACL_P3_ENTRIES_PER_CACHE_LINE)*SACL_P3_ENTRY_WIDTH);

    // If the priority of the current result is higher than whats in the PHV
    if (txdma_to_p4e.sacl_priority > scratch_metadata.field10) {
        // Then overwrite the result in the PHV with the current one
        modify_field(txdma_to_p4e.sacl_priority, scratch_metadata.field10);
        modify_field(txdma_to_p4e.drop, scratch_metadata.field1);
        modify_field(txdma_to_p4e.sacl_action, scratch_metadata.field1);
        modify_field(txdma_to_p4e.sacl_root_num, txdma_control.root_count);
    }

    if (rx_to_tx_hdr.sacl_base_addr0 != 0) {
        if ((txdma_control.recirc_count & 0x1) == 0) {
            // P1 table base
            modify_field(scratch_metadata.field40, SACL_P1_3_TABLE_OFFSET);
            // P1 table index
            modify_field(scratch_metadata.field20, ((rx_to_tx_hdr.sip_classid0 <<
                                                     SACL_TAG_CLASSID_WIDTH) |
                                                    txdma_control.dtag_classid));
        } else {
            // P1 table base
            modify_field(scratch_metadata.field40, SACL_P1_1_TABLE_OFFSET);
            // P1 table index
            modify_field(scratch_metadata.field20, ((rx_to_tx_hdr.sip_classid0 <<
                                                     SACL_SPORT_CLASSID_WIDTH) |
                                                    rx_to_tx_hdr.sport_classid0));
        }

        // Write P1 table index to PHV
        modify_field(txdma_control.rfc_index, scratch_metadata.field20);

        // Write P1 table lookup address to PHV
        modify_field(txdma_control.rfc_table_addr,              // P1 Lookup Addr =
                     rx_to_tx_hdr.sacl_base_addr0 +             // Region Base +
                     scratch_metadata.field40 +                 // Table Base +
                     (((scratch_metadata.field20) /
                      SACL_P1_ENTRIES_PER_CACHE_LINE) *
                      SACL_CACHE_LINE_SIZE));                   // Index Bytes
    } else {
        modify_field(txdma_predicate.rfc_enable, FALSE);
    }

}

@pragma stage 7
@pragma hbm_table
@pragma raw_index_table
table rfc_p3_1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        rfc_action_p3_1;
    }
}

action setup_rfc()
{
    if ((txdma_control.recirc_count & 0x1) == 1) {
        // Done for combination. Initialize for the next combination.

        // Find the next STAG
        if (txdma_control.stag_count == 0) {
            modify_field(scratch_metadata.field10, rx_to_tx_hdr.stag1_classid);
        } else {
            if (txdma_control.stag_count == 1) {
                modify_field(scratch_metadata.field10, rx_to_tx_hdr.stag2_classid);
            } else {
                if (txdma_control.stag_count == 2) {
                    modify_field(scratch_metadata.field10, rx_to_tx_hdr.stag3_classid);
                } else {
                    if (txdma_control.stag_count == 3) {
                        modify_field(scratch_metadata.field10, rx_to_tx_hdr.stag4_classid);
                    } else {
                        modify_field(scratch_metadata.field10, 0x3FF);
                    }
                }
            }
        }

        // If the next STAG is invalid
        if (scratch_metadata.field10 == 0x3FF) {
            // Find the next DTAG
            if (txdma_control.dtag_count == 0) {
                modify_field(scratch_metadata.field10, rx_to_tx_hdr.dtag1_classid);
            } else {
                if (txdma_control.dtag_count == 1) {
                    modify_field(scratch_metadata.field10, rx_to_tx_hdr.dtag2_classid);
                } else {
                    if (txdma_control.dtag_count == 2) {
                        modify_field(scratch_metadata.field10, rx_to_tx_hdr.dtag3_classid);
                    } else {
                        if (txdma_control.dtag_count == 3) {
                            modify_field(scratch_metadata.field10, rx_to_tx_hdr.dtag4_classid);
                        } else {
                            modify_field(scratch_metadata.field10, 0x3FF);
                        }
                    }
                }
            }

            // If the next DTAG is invalid
            if (scratch_metadata.field10 == 0x3FF) {
                // Done for policy root. Initialize for the next policy root.

                // Reinitialized TAG classids to the first
                modify_field(txdma_control.stag_count, 0);
                modify_field(txdma_control.stag_classid, rx_to_tx_hdr.stag0_classid);
                modify_field(txdma_control.dtag_count, 0);
                modify_field(txdma_control.dtag_classid, rx_to_tx_hdr.dtag0_classid);

                if (txdma_control.root_count == 0) {
                    modify_field(rx_to_tx_hdr.sacl_base_addr0, rx_to_tx_hdr.sacl_base_addr1);
                    modify_field(rx_to_tx_hdr.sip_classid0, rx_to_tx_hdr.sip_classid1);
                    modify_field(rx_to_tx_hdr.dip_classid0, rx_to_tx_hdr.dip_classid1);
                    modify_field(rx_to_tx_hdr.sport_classid0, rx_to_tx_hdr.sport_classid1);
                    modify_field(rx_to_tx_hdr.dport_classid0, rx_to_tx_hdr.dport_classid1);
                } else {
                    if (txdma_control.root_count == 1) {
                        modify_field(rx_to_tx_hdr.sacl_base_addr0, rx_to_tx_hdr.sacl_base_addr2);
                        modify_field(rx_to_tx_hdr.sip_classid0, rx_to_tx_hdr.sip_classid2);
                        modify_field(rx_to_tx_hdr.dip_classid0, rx_to_tx_hdr.dip_classid2);
                        modify_field(rx_to_tx_hdr.sport_classid0, rx_to_tx_hdr.sport_classid2);
                        modify_field(rx_to_tx_hdr.dport_classid0, rx_to_tx_hdr.dport_classid2);
                    } else {
                        if (txdma_control.root_count == 2) {
                            modify_field(rx_to_tx_hdr.sacl_base_addr0, rx_to_tx_hdr.sacl_base_addr3);
                            modify_field(rx_to_tx_hdr.sip_classid0, rx_to_tx_hdr.sip_classid3);
                            modify_field(rx_to_tx_hdr.dip_classid0, rx_to_tx_hdr.dip_classid3);
                            modify_field(rx_to_tx_hdr.sport_classid0, rx_to_tx_hdr.sport_classid3);
                            modify_field(rx_to_tx_hdr.dport_classid0, rx_to_tx_hdr.dport_classid3);
                        } else {
                            if (txdma_control.root_count == 3) {
                                modify_field(rx_to_tx_hdr.sacl_base_addr0, rx_to_tx_hdr.sacl_base_addr4);
                                modify_field(rx_to_tx_hdr.sip_classid0, rx_to_tx_hdr.sip_classid4);
                                modify_field(rx_to_tx_hdr.dip_classid0, rx_to_tx_hdr.dip_classid4);
                                modify_field(rx_to_tx_hdr.sport_classid0, rx_to_tx_hdr.sport_classid4);
                                modify_field(rx_to_tx_hdr.dport_classid0, rx_to_tx_hdr.dport_classid4);
                            } else {
                                if (txdma_control.root_count == 4) {
                                    modify_field(rx_to_tx_hdr.sacl_base_addr0, rx_to_tx_hdr.sacl_base_addr5);
                                    modify_field(rx_to_tx_hdr.sip_classid0, rx_to_tx_hdr.sip_classid5);
                                    modify_field(rx_to_tx_hdr.dip_classid0, rx_to_tx_hdr.dip_classid5);
                                    modify_field(rx_to_tx_hdr.sport_classid0, rx_to_tx_hdr.sport_classid5);
                                    modify_field(rx_to_tx_hdr.dport_classid0, rx_to_tx_hdr.dport_classid5);
                                } else {
                                    if (txdma_control.root_count == 5) {
                                        // No more RFC Lookups
                                        modify_field(rx_to_tx_hdr.sacl_base_addr0, 0);
                                    }
                                }
                            }
                        }
                    }
                }

                modify_field(txdma_control.root_count, txdma_control.root_count + 1);
            } else {
                // The next DTAG is valid. Start again with the first STAG.
                modify_field(txdma_control.stag_count, 0);
                modify_field(txdma_control.stag_classid, rx_to_tx_hdr.stag0_classid);
                modify_field(txdma_control.dtag_count, txdma_control.dtag_count + 1);
                modify_field(txdma_control.dtag_classid, scratch_metadata.field10);
            }
        } else {
            // The next STAG is valid.
            modify_field(txdma_control.stag_count, txdma_control.stag_count + 1);
            modify_field(txdma_control.stag_classid, scratch_metadata.field10);
        }
    }
}

@pragma stage 6
table setup_rfc {
    actions {
        setup_rfc;
    }
}

control sacl_rfc {
    if (txdma_predicate.rfc_enable == TRUE) {
        apply(rfc_p1);
        apply(rfc_p2);
        apply(rfc_p3);
        apply(rfc_p1_1);
        apply(rfc_p2_1);
        apply(rfc_p3_1);
    }

    apply(setup_rfc);
}
