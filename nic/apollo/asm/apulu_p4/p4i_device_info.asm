#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_p4i_device_info_k.h"

struct p4i_device_info_k_   k;
struct p4i_device_info_d    d;
struct phv_                 p;

%%

p4i_device_info:
    sub             r1, k.capri_p4_intrinsic_frame_size, \
                        k.offset_metadata_l2_1
    or              r7, k.capri_intrinsic_tm_oq_s3_e4, \
                        k.capri_intrinsic_tm_oq_s0_e2, 2
    sne             c1, r7, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, r7
    phvwr.!c1       p.capri_intrinsic_tm_oq, k.capri_intrinsic_tm_iq
    bbeq            k.ingress_recirc_valid, FALSE, p4i_recirc_done
    phvwr           p.capri_p4_intrinsic_packet_len, r1

p4i_recirc:
    xor             r1, k.ingress_recirc_local_mapping_done, 0x1
    phvwr           p.control_metadata_local_mapping_ohash_lkp, r1
    xor             r1, k.ingress_recirc_flow_done, 0x1
    phvwr           p.control_metadata_flow_ohash_lkp, r1

p4i_recirc_done:
    seq             c1, k.ethernet_1_dstAddr, \
                        d.p4i_device_info_d.device_mac_addr1
    seq.!c1         c1, k.ethernet_1_dstAddr, \
                        d.p4i_device_info_d.device_mac_addr2
    seq             c2, k.ipv4_1_valid, TRUE
    seq.c2          c2, k.ipv4_1_dstAddr, d.p4i_device_info_d.device_ipv4_addr
    seq             c3, k.ipv6_1_valid, TRUE
    seq.c3          c3, k.ipv6_1_dstAddr[127:64], \
                        d.p4i_device_info_d.device_ipv6_addr[127:64]
    seq.c3          c3, k.ipv6_1_dstAddr[63:0], \
                        d.p4i_device_info_d.device_ipv6_addr[63:0]
    andcf           c1, [c2 | c3]
    phvwr.e         p.control_metadata_l2_enabled, \
                        d.p4i_device_info_d.l2_enabled
    phvwr.c1        p.control_metadata_to_device_ip, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_device_info_error:
    phvwr           p.capri_intrinsic_drop, 1
    or              r7, k.capri_intrinsic_tm_oq_s3_e4, \
                        k.capri_intrinsic_tm_oq_s0_e2, 2
    sne.e           c1, r7, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, r7
