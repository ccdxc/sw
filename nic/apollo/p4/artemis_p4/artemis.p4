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

action nop() {
}

action drop_packet() {
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
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
}
