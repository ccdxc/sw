#include "../../../p4/include/intrinsic.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/headers.p4"

#include "qstate.p4"
#include "route.p4"
#include "dma.p4"
#include "metadata.p4"
#include "common_txdma.p4"

parser start {
    return ingress;
}


control ingress {
    common_tx_p4plus_stage0();
    if (app_header.table3_valid == 1) {
        read_qstate();
        if (app_header.table0_valid == 1) {
            route_lookup();
        }
        dma();
    } else {
        if (app_header.table0_valid == 1) {
            apply(tx_table_s1_t0);
            apply(tx_table_s2_t0);
            apply(tx_table_s3_t0);
            apply(tx_table_s4_t0);
            apply(tx_table_s5_t0);
            apply(tx_table_s6_t0);
            apply(tx_table_s7_t0);
        }
        if (app_header.table1_valid == 1) {
            apply(tx_table_s1_t1);
            apply(tx_table_s2_t1);
            apply(tx_table_s3_t1);
            apply(tx_table_s4_t1);
            apply(tx_table_s5_t1);
            apply(tx_table_s6_t1);
            apply(tx_table_s7_t1);
        }
        if (app_header.table2_valid == 1) {
            apply(tx_table_s1_t2);
            apply(tx_table_s2_t2);
            apply(tx_table_s3_t2);
            apply(tx_table_s4_t2);
            apply(tx_table_s5_t2);
            apply(tx_table_s6_t2);
            apply(tx_table_s7_t2);
        }
        apply(tx_table_s5_t4_lif_rate_limiter_table);
    }
}
