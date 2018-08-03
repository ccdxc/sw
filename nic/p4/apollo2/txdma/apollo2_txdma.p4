#include "../../include/intrinsic.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/headers.p4"

#include "route.p4"
#include "metadata.p4"

@pragma dont_trim
metadata cap_phv_intr_global_t capri_intrinsic;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intrinsic;
@pragma dont_trim
metadata cap_phv_intr_txdma_t capri_txdma_intrinsic;
@pragma dont_trim
metadata arm_to_txdma_header_t arm_to_txdma_header;
@pragma dont_trim
metadata p4_to_txdma_header_t p4_to_txdma_header;
@pragma dont_trim
metadata txdma_to_p4i_header_t txdma_to_p4i_header;
@pragma dont_trim
metadata txdma_to_p4e_header_t txdma_to_p4e_header;

parser start {
    return ingress;
}


control ingress {
    route();
}
