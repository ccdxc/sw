#include "../../../p4/include/intrinsic.p4"

#include "../include/headers.p4"
#include "../include/artemis_defines.h"
#include "../include/artemis_headers.p4"
#include "../include/artemis_table_sizes.h"

#include "metadata.p4"
#include "sacl.p4"
#include "packet_queue.p4"
#include "common_rxdma.p4"

parser start {
    return ingress;
}

control ingress {
    if (app_header.table3_valid == 1) {
        sacl();
        pkt_enqueue();
        //TODO-KSM Launch this table/lookup only in First Pass
        vnic_info();
    } else {
        common_p4plus_stage0();
        if (app_header.table0_valid == 1) {
            apply(rx_table_s1_t0);
            apply(rx_table_s2_t0);
            apply(rx_table_s3_t0);
            apply(rx_table_s4_t0);
            apply(rx_table_s5_t0);
            apply(rx_table_s6_t0);
            apply(rx_table_s7_t0);
        }
        if (app_header.table1_valid == 1) {
            apply(rx_table_s1_t1);
            apply(rx_table_s2_t1);
            apply(rx_table_s3_t1);
            apply(rx_table_s4_t1);
            apply(rx_table_s5_t1);
            apply(rx_table_s6_t1);
            apply(rx_table_s7_t1);
        }
        if (app_header.table2_valid == 1) {
            apply(rx_table_s1_t2);
            apply(rx_table_s2_t2);
            apply(rx_table_s3_t2);
            apply(rx_table_s4_t2);
            apply(rx_table_s5_t2);
            apply(rx_table_s6_t2);
            apply(rx_table_s7_t2);
        }
    }
}

