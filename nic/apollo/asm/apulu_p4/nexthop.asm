#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nexthop_k.h"

struct nexthop_k_   k;
struct nexthop_d    d;
struct phv_         p;

%%

nexthop_info:
    phvwrpair       p.p4e_i2e_valid, FALSE, p.txdma_to_p4e_valid, FALSE

    sne             c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr.!c1       p.capri_intrinsic_tm_oq, k.capri_intrinsic_tm_iq

    seq             c1, k.txdma_to_p4e_nexthop_id, r0
    bcf             [c1], nexthop_invalid
    seq             c1, d.nexthop_info_d.port, TM_PORT_DMA
    nop.!c1.e
    phvwr           p.capri_intrinsic_tm_oport, d.nexthop_info_d.port
    phvwr           p.capri_intrinsic_lif, d.nexthop_info_d.lif
    phvwr.e         p.capri_rxdma_intrinsic_qtype, d.nexthop_info_d.qtype
    phvwr.f         p.capri_rxdma_intrinsic_qid, d.nexthop_info_d.qid

nexthop_invalid:
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_NEXTHOP_INVALID], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
