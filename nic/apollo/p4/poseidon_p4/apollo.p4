#include "../../../p4/include/intrinsic.p4"
#include "../../../p4/include/app_headers.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/sacl_defines.h"
#include "../include/headers.p4"

#include "parser.p4"
#include "metadata.p4"
#include "i2e_metadata.p4"

#include "rewrite.p4"
#include "mirror.p4"
#include "rvpath.p4"
#include "stats.p4"
#include "flow.p4"
#include "vnic.p4"
#include "key.p4"
#include "mapping.p4"
#include "nacl.p4"
#include "inter_pipe.p4"
#include "offload.p4"

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

action sacl_init(base_v4addr, base_v6addr) {
    modify_field(p4_to_rxdma_header.sacl_base_addr, base_v4addr);
    modify_field(control_metadata.sacl_v6addr, base_v6addr);
}

action lpm_init(base_v4addr, base_v6addr) {
    modify_field(p4_to_txdma_header.lpm_addr, base_v4addr);
    modify_field(control_metadata.lpm_v6addr, base_v6addr);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    ingress_vnic_info();
    key_init();
    local_mapping();
    rvpath_check();
    nacl();
    flow_lookup();
    ingress_stats();
    offloads();
    ingress_to_rxdma();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    if (control_metadata.span_copy == TRUE) {
        mirror();
    } else {
        remote_vnic_mapping_tx();
        egress_vnic_info();
        rewrite();
        egress_stats();
        egress_to_uplink();
    }
}
