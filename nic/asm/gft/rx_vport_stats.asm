#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_vport_stats_k k;
struct rx_vport_stats_d d;
struct phv_ p;

%%

rx_vport_stats:
#ifdef SUPPORT_GFT_GTEST
    seq             c1, k.{capri_intrinsic_lif_sbit0_ebit2, \
                           capri_intrinsic_lif_sbit3_ebit10}, EXCEPTION_VPORT
    bcf             [!c1], rx_vport_stats_2
    phvwr.c1        p.{p4plus_to_p4_valid,capri_txdma_intrinsic_valid}, 0x3
    phvwrpair       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS, \
                        p.capri_intrinsic_tm_oq, 0
    seq             c7, k.roce_bth_1_valid, TRUE
    seq.!c7         c7, k.roce_bth_2_valid, TRUE
    phvwr.c7        p.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
#endif

rx_vport_stats_2:
    bbeq            k.capri_intrinsic_drop, TRUE, rx_vport_stats_drop
    phvwr.!c1.f     p.capri_p4_intrinsic_valid, TRUE
    tbladd          d.rx_vport_stats_d.permit_packets, 1
    tbladd.e        d.rx_vport_stats_d.permit_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}
    nop

rx_vport_stats_drop:
    tbladd.e        d.rx_vport_stats_d.drop_packets, 1
    tbladd          d.rx_vport_stats_d.drop_bytes, \
                        k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_vport_stats_error:
    nop.e
    nop
