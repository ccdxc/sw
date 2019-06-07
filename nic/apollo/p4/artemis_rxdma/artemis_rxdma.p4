#include "../../../p4/include/intrinsic.p4"

#include "../include/headers.p4"
#include "../include/artemis_defines.h"
#include "../include/artemis_headers.p4"
#include "../include/artemis_table_sizes.h"
#include "../include/lpm_defines.h"

#include "metadata.p4"
#include "rxlpm1.p4"
#include "rxlpm2.p4"
#include "recirc.p4"
#include "vnic_info_rxdma.p4"
#include "packet_queue.p4"
#include "common_rxdma.p4"
#include "flow_aging.p4"
#include "nat_rxdma.p4"

parser start {
    return ingress;
}

control ingress {
    if (p4_to_rxdma.cps_path_en == 1) {
        if (p4_to_rxdma.lpm1_enable == TRUE) {
            rxlpm1();
        }
        // This bit is indication of first pass - change name
        if (p4_to_rxdma.vnic_info_en == TRUE) {
            vnic_info_rxdma();
            nat_rxdma();
        }
        if (p4_to_rxdma.lpm2_enable == TRUE) {
            rxlpm2();
        }
        recirc();
        pkt_enqueue();
        if (p4_to_rxdma.aging_enable == TRUE) {
            flow_aging();
        }
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

