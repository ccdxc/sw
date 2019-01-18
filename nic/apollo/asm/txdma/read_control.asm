#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct read_control_k       k;
struct read_control_d       d;

%%

read_control_info:
    phvwr.e     p.{predicate_header_pad0...predicate_header_direction}, \
                    d.read_control_info_d.data[510: 510+1 - \
                        ((APOLLO_PREDICATE_HDR_SZ << 3) - 1)]
    phvwr       p.{p4_to_txdma_header_p4plus_app_id...p4_to_txdma_header_vcn_id}, \
                    d.read_control_info_d.data[511-(APOLLO_PREDICATE_HDR_SZ<<3): \
                                               511-(APOLLO_PREDICATE_HDR_SZ<<3)+ \
                                               1-(APOLLO_P4_TO_TXDMA_HDR_SZ<<3)]

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_control_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
