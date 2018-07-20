#include "../../include/intrinsic.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/slacl_defines.h"
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

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

action slacl_init(base_addr) {
    modify_field(slacl_metadata.base_addr, base_addr);
    modify_field(slacl_metadata.addr1, base_addr);
    modify_field(slacl_metadata.addr2, base_addr + SLACL_IP_TABLE_OFFSET);
}

action lpm_init(base_addr) {
    modify_field(lpm_metadata.addr, base_addr);
    modify_field(lpm_metadata.base_addr, base_addr);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    ingress_vnic_info();
    key_init();
    local_ip_mapping();
    rvpath_check();
    flow_lookup();
    ingress_stats();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    egress_vnic_info();
    mirror();
    rewrite();
    egress_stats();
}
