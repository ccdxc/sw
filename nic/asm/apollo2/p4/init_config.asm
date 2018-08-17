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
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    div             r1, k.p4_to_rxdma_header_slacl_ip_31_16, 51
    div             r2, k.p4_to_rxdma_header_slacl_ip_15_00, 51
    add             r3, \
                     k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1, \
                        p4_to_txdma_header_lpm_addr_sbit2_ebit33}, \
                     k.key_metadata_dst
    phvwr           p.p4_to_txdma_header_lpm_addr, r3
    add             r4, r0, k.{p4_to_rxdma_header_slacl_addr1_sbit0_ebit1, \
                               p4_to_rxdma_header_slacl_addr1_sbit2_ebit33}
    add             r1, r4, r1, 6
    add             r2, r4, r2, 6
    phvwr.e         p.p4_to_rxdma_header_slacl_addr1, r1
    phvwr           p.p4_to_rxdma_header_slacl_addr2, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
init_config_error:
    nop.e
    nop
