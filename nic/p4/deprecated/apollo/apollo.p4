#include "../include/intrinsic.p4"

#include "include/defines.h"
#include "include/table_sizes.h"

#include "parser.p4"
#include "headers.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"

#include "source_guard.p4"
#include "mappings.p4"
#include "rewrite.p4"
#include "mirror.p4"
#include "policy.p4"
#include "slacl.p4"
#include "stats.p4"
#include "lpm.p4"
#include "ep.p4"

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    mappings();
    ep_lookup();
    lpm_lookup();
    source_guard();
    policy_lookup();
    ingress_stats();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    mirror();
    slacl();
    rewrite();
    egress_stats();
}
