#include "include/defines.h"
#include "include/table_sizes.h"

#include "parser.p4"
#include "headers.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"
#include "../include/intrinsic.p4"
#include "../include/common_defines.h"

#include "key.p4"
#include "flow.p4"
#include "apps.p4"
#include "roce.p4"
#include "stats.p4"
#include "vport.p4"
#include "policer.p4"
#include "checksum.p4"
#include "transpositions.p4"

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    rx_vport();
    rx_key();
    rx_flow_lookup();
    rx_policer();
    rx_transpositions();
    rx_vport_stats();
    rx_apps();
    rx_checksum();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    tx_apps();
    tx_key();
    tx_flow_lookup();
    tx_transpositions();
    tx_policer();
    tx_vport_stats();
    tx_vport();
}
