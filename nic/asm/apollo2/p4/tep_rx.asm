#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tep_rx_k     k;
struct tep_rx_d     d;
struct phv_         p;

%%

tep_rx:
    sne         c1, d.tep_rx_d.tep_ip, k.ipv4_1_srcAddr
    nop.!c1.e
    phvwr.c1.e  p.control_metadata_p4i_drop_reason[DROP_TEP_RX_SRC_IP_MISMATCH], \
                    TRUE
    phvwr.c1    p.capri_intrinsic_drop, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep_rx_error:
    nop.e
    nop
