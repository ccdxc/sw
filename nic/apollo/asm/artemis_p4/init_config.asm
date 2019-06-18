#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_init_config_k.h"

struct init_config_k_ k;
struct phv_ p;

%%

init_config:
    bbne            k.ingress_recirc_valid, TRUE, ingress_recirc_done
    xor             r1, k.ingress_recirc_flow_done, 0x1
    phvwr           p.control_metadata_flow_ohash_lkp, r1
    phvwr           p.capri_p4_intrinsic_recirc, FALSE

ingress_recirc_done:
    sub             r1, k.capri_p4_intrinsic_frame_size, \
                        k.offset_metadata_l2_1
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
init_config_error:
    phvwr           p.capri_intrinsic_drop, 1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
