#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"

struct flow_info_k   k;
struct flow_info_d   d;
struct phv_     p;

%%
//TODO: Vikasd: Add conn_state processing.

flow_info:
    seq             c1, d.flow_info_d.drop, 1
    b.c1            label_flow_info_drop
    tbladd.!c1.e    d.flow_info_d.permit_packets, 1
    tbladd          d.flow_info_d.permit_bytes, \
                    k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}

label_flow_info_drop:
    tbladd          d.flow_info_d.deny_packets, 1
    tbladd          d.flow_info_d.deny_bytes, \
                    k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}
    phvwr.e         p.control_metadata_p4i_drop_reason[DROP_FLOW_HIT], 1
    phvwr           p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_info_error:
    nop.e
    nop
