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
    add             r2, r0, k.offset_metadata_l2_1
    seq             c1, k.cps_blob_valid, 1
    add.c1          r2, r2, 0x100
    sub             r1, k.capri_p4_intrinsic_frame_size, r2
    sne             c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr.e         p.capri_p4_intrinsic_packet_len, r1
    phvwr.!c1       p.capri_intrinsic_tm_oq, k.capri_intrinsic_tm_iq

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
init_config_error:
    phvwr           p.capri_intrinsic_drop, 1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
