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
#include "nacl.p4"
#include "resource_pool.p4"
#include "ingress_to_rxdma.p4"

action nop() {
}

action drop_packet() {
    modify_field(capri_intrinsic.drop, TRUE);
}

action slacl_init(base_addr) {
    modify_field(p4_to_rxdma_header.slacl_base_addr, base_addr);
    modify_field(p4_to_rxdma_header.slacl_addr1, base_addr);
    modify_field(p4_to_rxdma_header.slacl_addr2, base_addr + SLACL_IP_TABLE_OFFSET);
}

action lpm_init(base_addr) {
    modify_field(p4_to_txdma_header.lpm_addr, base_addr);
}

action remove_egress_headers() {
    remove_header(capri_txdma_intrinsic);
    remove_header(txdma_to_p4e_header);
    remove_header(apollo_i2e_metadata);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
control ingress {
    ingress_vnic_info();
    key_init();
    local_ip_mapping();
    rvpath_check();
    nacl();
    flow_lookup();
    ingress_resource_pool();
    ingress_stats();
    ingress_to_rxdma();
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
control egress {
    remote_vnic_mapping();
    egress_vnic_info();
    mirror();
    rewrite();
    egress_resource_pool();
    egress_stats();
}
