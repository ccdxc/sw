#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct egress_to_uplink_k k;
struct phv_ p;

%%

egress_to_uplink:
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr       p.capri_txdma_intrinsic_valid, FALSE
    phvwr       p.txdma_to_p4e_header_valid, FALSE
    bbeq        k.control_metadata_local_switching, TRUE, egress_to_ingress
    phvwr       p.p4e_apollo_i2e_valid, FALSE
egress_to_uplink2:
    phvwr       p.capri_intrinsic_tm_span_session, \
                    k.control_metadata_mirror_session
    seq         c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1    p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_0
    phvwr.e     p.predicate_header_valid, FALSE
    phvwr.!c1   p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1

egress_to_ingress:
    seq         c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1    p.{predicate_header_pad0...predicate_header_direction}, \
                    TX_FROM_HOST
    phvwr.e     p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
    phvwr.!c1   p.{predicate_header_pad0...predicate_header_direction}, \
                    RX_FROM_SWITCH

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_to_uplink_error:
    phvwr.e     p.capri_intrinsic_drop, 1
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
