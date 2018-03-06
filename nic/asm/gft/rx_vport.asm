#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_vport_k k;
struct rx_vport_d d;
struct phv_ p;

%%

rx_vport:
    sub         r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5,\
                       capri_p4_intrinsic_frame_size_sbit6_ebit13}, \
                    CAPRI_GLOBAL_INTRINSIC_HDR_SZ
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr       p.capri_p4_intrinsic_packet_len, r1
    cmov        r1, c1, d.rx_vport_d.vport, EXCEPTION_VPORT
    cmov        r2, c1, d.rx_vport_d.tm_oport, TM_PORT_DMA
    phvwr.c1    p.roce_metadata_rdma_enabled, d.rx_vport_d.rdma_enabled
    seq         c7, k.capri_p4_intrinsic_parser_err, TRUE
    nop.!c7.e
    phvwrpair   p.capri_intrinsic_tm_oport, r2, p.capri_intrinsic_lif, r1

    // handler parser errors
    sne         c1, k.capri_p4_intrinsic_len_err, 0
    sne.!c1     c1, k.capri_intrinsic_csum_err[3:0], 0
    sne.!c1     c1, k.capri_p4_intrinsic_crc_err, 0
    nop.e
    phvwr.c1.f  p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_vport_error:
    nop.e
    nop
