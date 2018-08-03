#include "../../include/intrinsic.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/slacl_defines.h"
#include "../include/headers.p4"

#include "slacl.p4"
#include "metadata.p4"
#include "udp_flow.p4"

@pragma dont_trim
metadata cap_phv_intr_global_t capri_intrinsic;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intrinsic;
@pragma dont_trim
metadata cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
@pragma dont_trim
metadata p4_to_rxdma_header_t p4_to_rxdma_header;
@pragma dont_trim
metadata p4_to_arm_header_t p4_to_arm_header;

parser start {
    return ingress;
}

control ingress {
    slacl();
    udp_flow_queuing();
}

