#include "../../../p4/include/intrinsic.p4"
#include "../../../p4/include/app_headers.p4"
#include "../../../p4/common/defines.h"

#include "../include/defines.h"
#include "../include/athena_defines.h"
#include "../include/athena_table_sizes.h"
#include "headers.p4"

#include "parser.p4"
#include "metadata.p4"

#include "key.p4"
#include "flow.p4"
#include "nacl.p4"
#include "session.p4"
#include "statistics.p4"
#include "config_verify.p4"
#include "inter_pipe.p4"

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
    flow_lookup();
    nacl();
    session_info_lookup();
    config_verify();
    ingress_inter_pipe();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    session_info_encap_lookup();
    statistics();
    egress_inter_pipe();
}
