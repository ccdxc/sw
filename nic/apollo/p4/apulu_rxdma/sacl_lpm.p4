#include "rxlpm1.p4"
#include "rxlpm2.p4"

action rxlpm1_res_handler()
{
    if (lpm_metadata.recirc_count == 0) {
        // SPORT class id for sacl0
        modify_field(rx_to_tx_hdr.sport_classid0,
                     scratch_metadata.field8);
    } else {
        if (lpm_metadata.recirc_count == 1) {
            // SIP class id for sacl0
            modify_field(rx_to_tx_hdr.sip_classid0,
                         scratch_metadata.field10);
        } else {
            if (lpm_metadata.recirc_count == 2) {
                // SPORT class id for sacl1
                modify_field(rx_to_tx_hdr.sport_classid1,
                             scratch_metadata.field8);

            } else {
                if (lpm_metadata.recirc_count == 3) {
                    // SIP class id for sacl1
                    modify_field(rx_to_tx_hdr.sip_classid1,
                                 scratch_metadata.field10);
                } else {
                    if (lpm_metadata.recirc_count == 4) {
                        // SPORT class id for sacl2
                        modify_field(rx_to_tx_hdr.sport_classid2,
                                     scratch_metadata.field8);
                    } else {
                        if (lpm_metadata.recirc_count == 5) {
                            // SIP class id for sacl2
                            modify_field(rx_to_tx_hdr.sip_classid2,
                                         scratch_metadata.field10);
                        } else {
                            if (lpm_metadata.recirc_count == 6) {
                                // SPORT class id for sacl3
                                modify_field(rx_to_tx_hdr.sport_classid3,
                                             scratch_metadata.field8);
                            } else {
                                if (lpm_metadata.recirc_count == 7) {
                                    // SIP class id for sacl3
                                    modify_field(rx_to_tx_hdr.sip_classid3,
                                                 scratch_metadata.field10);
                                } else {
                                    if (lpm_metadata.recirc_count == 8) {
                                        // SPORT class id for sacl4
                                        modify_field(rx_to_tx_hdr.sport_classid4,
                                                     scratch_metadata.field8);
                                    } else {
                                        if (lpm_metadata.recirc_count == 9) {
                                            // SIP class id for sacl4
                                            modify_field(rx_to_tx_hdr.sip_classid4,
                                                         scratch_metadata.field10);
                                        } else {
                                            if (lpm_metadata.recirc_count == 10) {
                                                // SPORT class id for sacl5
                                                modify_field(rx_to_tx_hdr.sport_classid5,
                                                             scratch_metadata.field8);
                                            } else {
                                                // SIP class id for sacl5
                                                modify_field(rx_to_tx_hdr.sip_classid5,
                                                             scratch_metadata.field10);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Disable further lookups for this LPM
    modify_field(p4_to_rxdma.lpm1_enable, FALSE);
}

action rxlpm2_res_handler()
{
    if (lpm_metadata.recirc_count == 0) {
        // DPORT class id for sacl0
        modify_field(rx_to_tx_hdr.dport_classid0,
                     scratch_metadata.field10);
    } else {
        if (lpm_metadata.recirc_count == 1) {
            // DIP class id for sacl0
            modify_field(rx_to_tx_hdr.dip_classid0,
                         scratch_metadata.field10);
        } else {
            if (lpm_metadata.recirc_count == 2) {
                // DPORT class id for sacl1
                modify_field(rx_to_tx_hdr.dport_classid1,
                             scratch_metadata.field10);

            } else {
                if (lpm_metadata.recirc_count == 3) {
                    // DIP class id for sacl1
                    modify_field(rx_to_tx_hdr.dip_classid1,
                                 scratch_metadata.field10);
                } else {
                    if (lpm_metadata.recirc_count == 4) {
                        // DPORT class id for sacl2
                        modify_field(rx_to_tx_hdr.dport_classid2,
                                     scratch_metadata.field10);
                    } else {
                        if (lpm_metadata.recirc_count == 5) {
                            // DIP class id for sacl2
                            modify_field(rx_to_tx_hdr.dip_classid2,
                                         scratch_metadata.field10);
                        } else {
                            if (lpm_metadata.recirc_count == 6) {
                                // DPORT class id for sacl3
                                modify_field(rx_to_tx_hdr.dport_classid3,
                                             scratch_metadata.field10);
                            } else {
                                if (lpm_metadata.recirc_count == 7) {
                                    // DIP class id for sacl3
                                    modify_field(rx_to_tx_hdr.dip_classid3,
                                                 scratch_metadata.field10);
                                } else {
                                    if (lpm_metadata.recirc_count == 8) {
                                        // DPORT class id for sacl4
                                        modify_field(rx_to_tx_hdr.dport_classid4,
                                                     scratch_metadata.field10);
                                    } else {
                                        if (lpm_metadata.recirc_count == 9) {
                                            // DIP class id for sacl4
                                            modify_field(rx_to_tx_hdr.dip_classid4,
                                                         scratch_metadata.field10);
                                        } else {
                                            if (lpm_metadata.recirc_count == 10) {
                                                // DPORT class id for sacl5
                                                modify_field(rx_to_tx_hdr.dport_classid5,
                                                             scratch_metadata.field10);
                                            } else {
                                                // DIP class id for sacl5
                                                modify_field(rx_to_tx_hdr.dip_classid5,
                                                             scratch_metadata.field10);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Disable further lookups for this LPM
    modify_field(p4_to_rxdma.lpm2_enable, FALSE);
}

action setup_lpm_dport_sport()
{
    if (lpm_metadata.sacl_base_addr != 0) {
        // Setup LPM1 for SPORT lookup
        modify_field(lpm_metadata.lpm1_base_addr, lpm_metadata.sacl_base_addr +
                     SACL_SPORT_TABLE_OFFSET);
        modify_field(lpm_metadata.lpm1_key, p4_to_rxdma.flow_sport);

        // Setup LPM2 for DPORT lookup
        modify_field(lpm_metadata.lpm2_base_addr, lpm_metadata.sacl_base_addr +
                     SACL_PROTO_DPORT_TABLE_OFFSET);
        modify_field(lpm_metadata.lpm2_key, (p4_to_rxdma.flow_dport |
                                            (p4_to_rxdma.flow_proto << 24)));

        // Enable LPMs
        modify_field(p4_to_rxdma.lpm1_enable, TRUE);
        modify_field(p4_to_rxdma.lpm2_enable, TRUE);

        // Setup the packet for recirc
        modify_field(capri_p4_intr.recirc, TRUE);
    } else {
        // No more recirc
        modify_field(capri_p4_intr.recirc, FALSE);
    }
}

action setup_lpm_dip_sip()
{
    if (lpm_metadata.sacl_base_addr != 0) {
        // Setup LPM1 for SIP lookup
        if (p4_to_rxdma.iptype == IPTYPE_IPV4) {
            modify_field(lpm_metadata.lpm1_base_addr, lpm_metadata.sacl_base_addr +
                         SACL_IPV4_SIP_TABLE_OFFSET);
        } else {
            modify_field(lpm_metadata.lpm1_base_addr, lpm_metadata.sacl_base_addr +
                         SACL_IPV6_SIP_TABLE_OFFSET);
        }

        modify_field(lpm_metadata.lpm1_key, p4_to_rxdma.flow_src);

        // Setup LPM2 for DIP lookup
        modify_field(lpm_metadata.lpm2_base_addr, lpm_metadata.sacl_base_addr +
                     SACL_DIP_TABLE_OFFSET);
        modify_field(lpm_metadata.lpm2_key, p4_to_rxdma.flow_dst);

        // Enable LPMs
        modify_field(p4_to_rxdma.lpm1_enable, TRUE);
        modify_field(p4_to_rxdma.lpm2_enable, TRUE);

        // Setup the packet for recirc
        modify_field(capri_p4_intr.recirc, TRUE);
    } else {
        // No more recirc
        modify_field(capri_p4_intr.recirc, FALSE);
    }
}

action setup_lpm1()
{
    if (lpm_metadata.recirc_count == 1) {
        setup_lpm_dport_sport();
    } else {
        if (lpm_metadata.recirc_count == 3) {
            setup_lpm_dport_sport();
        } else {
            if (lpm_metadata.recirc_count == 5) {
                setup_lpm_dport_sport();
            } else {
                if (lpm_metadata.recirc_count == 7) {
                    setup_lpm_dport_sport();
                } else {
                    if (lpm_metadata.recirc_count == 9) {
                        setup_lpm_dport_sport();
                    } else {
                        if (lpm_metadata.recirc_count == 11) {
                            // No more recirc
                            modify_field(capri_p4_intr.recirc, FALSE);
                        }
                    }
                }
            }
        }
    }
}

@pragma stage 7
table setup_lpm1 {
    actions {
        setup_lpm1;
    }
}

action setup_lpm2()
{
    if (lpm_metadata.recirc_count == 0) {
        setup_lpm_dip_sip();
        modify_field(lpm_metadata.sacl_base_addr, rx_to_tx_hdr.sacl_base_addr1);
    } else {
        if (lpm_metadata.recirc_count == 2) {
            setup_lpm_dip_sip();
            modify_field(lpm_metadata.sacl_base_addr, rx_to_tx_hdr.sacl_base_addr2);
        } else {
            if (lpm_metadata.recirc_count == 4) {
                setup_lpm_dip_sip();
                modify_field(lpm_metadata.sacl_base_addr, rx_to_tx_hdr.sacl_base_addr3);
            } else {
                if (lpm_metadata.recirc_count == 6) {
                    setup_lpm_dip_sip();
                    modify_field(lpm_metadata.sacl_base_addr, rx_to_tx_hdr.sacl_base_addr4);
                } else {
                    if (lpm_metadata.recirc_count == 8) {
                        setup_lpm_dip_sip();
                        modify_field(lpm_metadata.sacl_base_addr, rx_to_tx_hdr.sacl_base_addr5);
                    } else {
                        if (lpm_metadata.recirc_count == 10) {
                            setup_lpm_dip_sip();
                            modify_field(lpm_metadata.sacl_base_addr, 0);
                        }
                    }
                }
            }
        }
    }
}

@pragma stage 7
table setup_lpm2 {
    actions {
        setup_lpm2;
    }
}

control sacl_lpm {

    if (p4_to_rxdma.lpm1_enable == TRUE) {
        rxlpm1();
    }

    if (p4_to_rxdma.lpm2_enable == TRUE) {
        rxlpm2();
    }

    apply(setup_lpm1);
    apply(setup_lpm2);
}
