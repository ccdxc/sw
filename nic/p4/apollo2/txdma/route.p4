/*****************************************************************************/
/* Route lookup                                                              */
/*****************************************************************************/

action route_lpm_s0(ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7,
                    ip8, ip9, ip10, ip11, ip12, ip13, ip14) {

    modify_field(scratch_metadata.v4_addr, ip0);
    modify_field(scratch_metadata.v4_addr, ip1);
    modify_field(scratch_metadata.v4_addr, ip2);
    modify_field(scratch_metadata.v4_addr, ip3);
    modify_field(scratch_metadata.v4_addr, ip4);
    modify_field(scratch_metadata.v4_addr, ip5);
    modify_field(scratch_metadata.v4_addr, ip6);
    modify_field(scratch_metadata.v4_addr, ip7);
    modify_field(scratch_metadata.v4_addr, ip8);
    modify_field(scratch_metadata.v4_addr, ip9);
    modify_field(scratch_metadata.v4_addr, ip10);
    modify_field(scratch_metadata.v4_addr, ip11);
    modify_field(scratch_metadata.v4_addr, ip12);
    modify_field(scratch_metadata.v4_addr, ip13);
    modify_field(scratch_metadata.v4_addr, ip14);

    if (p4_to_txdma_header.lpm_dst >= ip7) {
        if (p4_to_txdma_header.lpm_dst >= ip11) {
            if (p4_to_txdma_header.lpm_dst >= ip13) {
                if (p4_to_txdma_header.lpm_dst >= ip14) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 960); // 60 + ((15 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 10620); // 60 + 960 + (15 * 704) - 960
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 900); // 60 + ((14 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 9976); // 60 + 960 + (14 * 704) - 900
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip12) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 840); // 60 + ((13 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 9332); // 60 + 960 + (13 * 704) - 840
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 780); // 60 + ((12 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 8688); // 60 + 960 + (12 * 704) - 780
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip9) {
                if (p4_to_txdma_header.lpm_dst >= ip10) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 720); // 60 + ((11 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 8044); // 60 + 960 + (11 * 704) - 720
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 660); // 60 + ((10 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 7400); // 60 + 960 + (10 * 704) - 660
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip8) {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     600); // 60 + ((9 * 15) * 4)
                        modify_field(txdma_control.lpm_s3_offset,
                                     6756); // 60 + 960 + (9 * 704) - 600
                }
                else {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     540); // 60 + ((8 * 15) * 4)
                        modify_field(txdma_control.lpm_s3_offset,
                                     6112); // 60 + 960 + (8 * 704) - 540
                }
            }
        }
    }
    else {
        if (p4_to_txdma_header.lpm_dst >= ip3) {
            if (p4_to_txdma_header.lpm_dst >= ip5) {
                if (p4_to_txdma_header.lpm_dst >= ip6) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 480); // 60 + ((7 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 5468); // 60 + 960 + (7 * 704) - 480
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 420); // 60 + ((6 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 4824); // 60 + 960 + (6 * 704) - 420
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip4) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 360); // 60 + ((5 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 4180); // 60 + 960 + (5 * 704) - 360
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 300); // 60 + ((4 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 3536); // 60 + 960 + (4 * 704) - 300
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip1) {
                if (p4_to_txdma_header.lpm_dst >= ip2) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 240); // 60 + ((3 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 2892); // 60 + 960 + (3 * 704) - 240
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 180); // 60 + ((2 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 2248); // 60 + 960 + (2 * 704) - 180
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip0) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 120); // 60 + ((1 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 1604); // 60 + 960 + (1 * 704) - 120
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 60); // 60 + ((0 * 15) * 4)
                    modify_field(txdma_control.lpm_s3_offset,
                                 960); // 60 + 960 + (0 * 704) - 60
                }
            }
        }
    }
}

action route_lpm_s1(ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7,
                    ip8, ip9, ip10, ip11, ip12, ip13, ip14) {

    modify_field(scratch_metadata.v4_addr, ip0);
    modify_field(scratch_metadata.v4_addr, ip1);
    modify_field(scratch_metadata.v4_addr, ip2);
    modify_field(scratch_metadata.v4_addr, ip3);
    modify_field(scratch_metadata.v4_addr, ip4);
    modify_field(scratch_metadata.v4_addr, ip5);
    modify_field(scratch_metadata.v4_addr, ip6);
    modify_field(scratch_metadata.v4_addr, ip7);
    modify_field(scratch_metadata.v4_addr, ip8);
    modify_field(scratch_metadata.v4_addr, ip9);
    modify_field(scratch_metadata.v4_addr, ip10);
    modify_field(scratch_metadata.v4_addr, ip11);
    modify_field(scratch_metadata.v4_addr, ip12);
    modify_field(scratch_metadata.v4_addr, ip13);
    modify_field(scratch_metadata.v4_addr, ip14);

    if (p4_to_txdma_header.lpm_dst >= ip7) {
        if (p4_to_txdma_header.lpm_dst >= ip11) {
            if (p4_to_txdma_header.lpm_dst >= ip13) {
                if (p4_to_txdma_header.lpm_dst >= ip14) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 660); // (15 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 616); // (14 * 44)
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip12) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 572); // (13 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 528); // (12 * 44)
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip9) {
                if (p4_to_txdma_header.lpm_dst >= ip10) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 484); // (11 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 440); // (10 * 44)
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip8) {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     txdma_control.lpm_s3_offset +
                                     396); // (9 * 44)
                }
                else {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     txdma_control.lpm_s3_offset +
                                     352); // (8 * 44)
                }
            }
        }
    }
    else {
        if (p4_to_txdma_header.lpm_dst >= ip3) {
            if (p4_to_txdma_header.lpm_dst >= ip5) {
                if (p4_to_txdma_header.lpm_dst >= ip6) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 308); // (7 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 264); // (6 * 44)
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip4) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 220); // (5 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 176); // (4 * 44)
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip1) {
                if (p4_to_txdma_header.lpm_dst >= ip2) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 132); // (3 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 88); // (2 * 44)
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip0) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 44); // (1 * 44)
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 0); // (0 * 44)
                }
            }
        }
    }
}

action route_lpm_s2(nh_, ip0, nh0, ip1, nh1, ip2, nh2, ip3,
                    nh3, ip4, nh4, ip5, nh5, ip6, nh6) {

    modify_field(scratch_metadata.nh_idx,  nh_);
    modify_field(scratch_metadata.v4_addr, ip0);
    modify_field(scratch_metadata.nh_idx,  nh0);
    modify_field(scratch_metadata.v4_addr, ip1);
    modify_field(scratch_metadata.nh_idx,  nh1);
    modify_field(scratch_metadata.v4_addr, ip2);
    modify_field(scratch_metadata.nh_idx,  nh2);
    modify_field(scratch_metadata.v4_addr, ip3);
    modify_field(scratch_metadata.nh_idx,  nh3);
    modify_field(scratch_metadata.v4_addr, ip4);
    modify_field(scratch_metadata.nh_idx,  nh4);
    modify_field(scratch_metadata.v4_addr, ip5);
    modify_field(scratch_metadata.nh_idx,  nh5);
    modify_field(scratch_metadata.v4_addr, ip6);
    modify_field(scratch_metadata.nh_idx,  nh6);

    if (p4_to_txdma_header.lpm_dst >= ip3) {
        if (p4_to_txdma_header.lpm_dst >= ip5) {
            if (p4_to_txdma_header.lpm_dst >= ip6) {
                modify_field(txdma_to_p4e_header.nexthop_index, nh6);
            }
            else {
                modify_field(txdma_to_p4e_header.nexthop_index, nh5);
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip4) {
                modify_field(txdma_to_p4e_header.nexthop_index, nh4);
            }
            else {
                modify_field(txdma_to_p4e_header.nexthop_index, nh3);
            }
        }
    }
    else {
        if (p4_to_txdma_header.lpm_dst >= ip1) {
            if (p4_to_txdma_header.lpm_dst >= ip2) {
                modify_field(txdma_to_p4e_header.nexthop_index, nh2);
            }
            else {
                modify_field(txdma_to_p4e_header.nexthop_index, nh1);
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip0) {
                modify_field(txdma_to_p4e_header.nexthop_index, nh0);
            }
            else {
                modify_field(txdma_to_p4e_header.nexthop_index, nh_);
            }
        }
    }
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table route_lpm_s0 {
    reads {
        p4_to_txdma_header.lpm_addr : exact;
    }
    actions {
        route_lpm_s0;
    }
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table route_lpm_s1 {
    reads {
        p4_to_txdma_header.lpm_addr : exact;
    }
    actions {
        route_lpm_s1;
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table route_lpm_s2 {
    reads {
        p4_to_txdma_header.lpm_addr : exact;
    }
    actions {
        route_lpm_s2;
    }
}

control route {
    if (predicate_header.lpm_bypass == FALSE) {
        apply(route_lpm_s0);
        apply(route_lpm_s1);
        apply(route_lpm_s2);
    }
}
