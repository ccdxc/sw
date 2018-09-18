#include "include/defines.h"
#include "include/table_sizes.h"

#include "parser.p4"
#include "headers.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"
#include "../include/intrinsic.p4"

#include "key.p4"
#include "flow.p4"
#include "policer.p4"
#include "rewrite.p4"

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    rx_key();
    rx_flow_lookup();
    rx_rewrites();
    rx_policers();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    tx_key();
    tx_flow_lookup();
    tx_rewrites();
    tx_policers();
}
