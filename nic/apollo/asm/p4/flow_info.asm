#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"
#include "platform/capri/capri_common.hpp"

struct flow_info_k  k;
struct flow_info_d  d;
struct phv_         p;

%%

flow_info:
    add             r5, r0, d.flow_info_d.flow_stats_addr
    seq             c1, k.control_metadata_flow_role, TCP_FLOW_RESPONDER
    add.c1          r5, r5, 32
    add             r7, r0, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                               capri_p4_intrinsic_packet_len_sbit6_ebit13}
    addi            r1, r0, 0x1000001
    or              r7, r7, r1, 32
    bbeq            d.flow_info_d.drop, TRUE, flow_info_drop
    addi            r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    or              r7, r7, r5[32:27], 58
    add.e           r6, r6, r5[26:0]
    memwr.dx        r6, r7

flow_info_drop:
    add             r5, r5, 16
    or              r7, r7, r5[32:27], 58
    add             r6, r6, r5[26:0]
    memwr.dx        r6, r7
    phvwr.e         p.control_metadata_p4i_drop_reason[DROP_FLOW_HIT], 1
    phvwr           p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
