#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct ingress_to_rxdma_k k;
struct phv_     p;

%%

ingress_to_rxdma:
    seq             c1, k.service_header_local_ip_mapping_done, FALSE
    seq.!c1         c1, k.service_header_flow_done, FALSE
    bcf             [c1], recirc_packet
    phvwr.!c1       p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                         APOLLO_P4_TO_RXDMA_HDR_SZ)
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    phvwr           p.p4_to_arm_header_valid, TRUE
    phvwr           p.p4_to_rxdma_header_valid, TRUE
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.p4_to_txdma_header_valid, TRUE
    phvwr           p.apollo_i2e_metadata_valid, TRUE
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1        p.predicate_header_lpm_bypass, TRUE
    phvwr.e         p.service_header_valid, FALSE
    phvwr           p.predicate_header_direction, k.control_metadata_direction

recirc_packet:
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
    phvwr           p.service_header_valid, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_to_rxdma_error:
    nop.e
    nop
