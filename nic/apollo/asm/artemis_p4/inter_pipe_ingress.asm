#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_inter_pipe_ingress_k.h"

struct inter_pipe_ingress_k_ k;
struct inter_pipe_ingress_d  d;
struct phv_ p;

%%

ingress_to_egress:
    phvwr           p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.predicate_header_direction, k.control_metadata_direction
    phvwr.e         p.txdma_to_p4e_valid, TRUE
    phvwr           p.p4i_i2e_valid, TRUE

.align
ingress_to_artemis:
ingress_to_classic_nic:
ingress_to_arm:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
inter_pipe_ingress_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
