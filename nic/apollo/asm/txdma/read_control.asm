#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct read_control_k       k;
struct read_control_d       d;

%%

read_control_info:
    // table0_valid = (lpm_bypass == 0)
    seq         c1, d.read_control_info_d.data[505], 0
    phvwr.c1    p.app_header_table0_valid, TRUE
    phvwr.e     p.{predicate_header_redirect_to_arm...predicate_header_direction}, \
                    d.read_control_info_d.data[507:504]
    phvwr       p.{p4_to_txdma_header_p4plus_app_id...p4_to_txdma_header_vpc_id}, \
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
