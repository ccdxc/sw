#include "../../../p4/include/intrinsic.p4"
#include "../../../p4/include/app_headers.p4"

#include "../include/apulu_defines.h"
#include "../include/apulu_table_sizes.h"
#include "../include/apulu_headers.p4"
#include "../include/headers.p4"

#include "parser.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"

#include "device.p4"
#include "key.p4"
#include "properties.p4"
#include "mappings.p4"
#include "flow.p4"
#include "nacl.p4"
#include "mirror.p4"
#include "session.p4"
#include "nat.p4"
#include "nexthops.p4"
#include "checksum.p4"
#include "policers.p4"
#include "stats.p4"
#include "inter_pipe.p4"
#include "offloads.p4"

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
    ingress_device_info();
    key_init();
    input_properties();
    local_mapping();
    flow_lookup();
    nacl();
    offloads();
    tx_policers();
    ingress_stats();
    ingress_inter_pipe();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    egress_device_info();
    if (control_metadata.span_copy == TRUE) {
        mirror();
    } else {
        mapping();
        session_lookup();
        output_properties();
        nat();
        rx_policers();
    }
    nexthops();
    update_checksums();
    egress_stats();
    egress_inter_pipe();
}
