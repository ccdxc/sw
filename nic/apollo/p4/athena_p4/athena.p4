#include "../../../p4/include/intrinsic.p4"
#include "../../../p4/include/app_headers.p4"
#include "../../../p4/common/defines.h"

#include "../include/athena_defines.h"
#include "../include/athena_table_sizes.h"
#include "headers.p4"

#include "parser.p4"
#include "metadata.p4"

#include "vnic.p4"
#include "key.p4"
#include "dnat.p4"
#include "offloads.p4"
#include "flow.p4"
#include "nacl.p4"
#include "session.p4"
#include "statistics.p4"
#include "config_verify.p4"
#include "inter_pipe.p4"
#include "policers.p4"
#include "checksum.p4"
#include "conntrack.p4"

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    vnic();
    key_init();
    dnat_lookup();
    offloads();
    flow_lookup();
    ingress_inter_pipe();
    p4i_statistics();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    //session_info_encap_lookup();
    //statistics();
    session_info_lookup();
    config_verify();
    nacl();
    //policers();
    update_checksums();
    egress_inter_pipe();
    conntrack_state_update();
    p4e_statistics();
}
