/*****************************************************************************/
/* Route lookup                                                              */
/*****************************************************************************/

#include "../include/lpm_defines.h"

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
                                 LPM_S0_B15_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK15_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B14_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK14_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip12) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B13_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK13_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B12_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK12_OFFSET);
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip9) {
                if (p4_to_txdma_header.lpm_dst >= ip10) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B11_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK11_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B10_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK10_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip8) {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     LPM_S0_B9_OFFSET);
                        modify_field(txdma_control.lpm_s3_offset,
                                     LPM_S2_BLOCK9_OFFSET);
                }
                else {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     LPM_S0_B8_OFFSET);
                        modify_field(txdma_control.lpm_s3_offset,
                                     LPM_S2_BLOCK8_OFFSET);
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
                                 LPM_S0_B7_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK7_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B6_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK6_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip4) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B5_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK5_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B4_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK4_OFFSET);
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip1) {
                if (p4_to_txdma_header.lpm_dst >= ip2) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B3_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK3_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B2_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK2_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip0) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B1_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK1_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 LPM_S0_B0_OFFSET);
                    modify_field(txdma_control.lpm_s3_offset,
                                 LPM_S2_BLOCK0_OFFSET);
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
                                 LPM_S1_B15_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B14_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip12) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B13_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B12_OFFSET);
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip9) {
                if (p4_to_txdma_header.lpm_dst >= ip10) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B11_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B10_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip8) {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     txdma_control.lpm_s3_offset +
                                     LPM_S1_B9_OFFSET);
                }
                else {
                        modify_field(p4_to_txdma_header.lpm_addr,
                                     p4_to_txdma_header.lpm_addr +
                                     txdma_control.lpm_s3_offset +
                                     LPM_S1_B8_OFFSET);
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
                                 LPM_S1_B7_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B6_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip4) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B5_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B4_OFFSET);
                }
            }
        }
        else {
            if (p4_to_txdma_header.lpm_dst >= ip1) {
                if (p4_to_txdma_header.lpm_dst >= ip2) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B3_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B2_OFFSET);
                }
            }
            else {
                if (p4_to_txdma_header.lpm_dst >= ip0) {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B1_OFFSET);
                }
                else {
                    modify_field(p4_to_txdma_header.lpm_addr,
                                 p4_to_txdma_header.lpm_addr +
                                 txdma_control.lpm_s3_offset +
                                 LPM_S1_B0_OFFSET);
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
