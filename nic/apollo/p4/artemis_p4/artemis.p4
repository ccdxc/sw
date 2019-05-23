#include "../../../p4/include/intrinsic.p4"
#include "../../../p4/include/app_headers.p4"

#include "../include/artemis_defines.h"
#include "../include/artemis_table_sizes.h"
#include "../include/artemis_headers.p4"
#include "../include/headers.p4"

#include "parser.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"

#include "key.p4"
#include "vnic.p4"
#include "tunnel.p4"
#include "mappings.p4"
#include "flow.p4"
#include "session.p4"
#include "nacl.p4"
#include "inter_pipe.p4"
#include "nat.p4"
#include "nexthop.p4"
#include "stats.p4"

action nop() {
}

action ingress_drop(drop_bit) {
    modify_field(control_metadata.p4i_drop_reason, 1 << drop_bit);
    modify_field(capri_intrinsic.drop, TRUE);
}

action egress_drop(drop_bit) {
    modify_field(control_metadata.p4e_drop_reason, 1 << drop_bit);
    modify_field(capri_intrinsic.drop, TRUE);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    key_init();
    ingress_vnic_info();
    tunnel_rx();
    ingress_mappings();
    flow_lookup();
    nacl();
    ingress_stats();
    inter_pipe_ingress();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    session_lookup();
    egress_vnic_info();
    nat();
    nexthop();
    egress_stats();
}
