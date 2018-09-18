#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct init_config_k k;
struct phv_ p;

%%

init_config:
    add             r1, r0, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5,\
                               capri_p4_intrinsic_frame_size_sbit6_ebit13}
    seq             c1, k.capri_intrinsic_tm_iport, TM_PORT_DMA
    sub.!c1         r1, r1, CAPRI_GLOBAL_INTRINSIC_HDR_SZ
    phvwr.e         p.capri_p4_intrinsic_packet_len, r1
    phvwr           p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
init_config_error:
    nop.e
    nop
