#include "include/defines.h"
#include "include/table_sizes.h"

#include "parser.p4"
#include "headers.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"
#include "../include/intrinsic.p4"

#include "key.p4"
#include "flow.p4"
#include "stats.p4"
#include "policer.p4"
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
    ingress_key();
    ingress_flow_lookup();
    ingress_policer();
    ingress_transpositions();
    ingress_vport_stats();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
}
