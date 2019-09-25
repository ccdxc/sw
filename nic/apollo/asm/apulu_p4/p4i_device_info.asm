#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_p4i_device_info_k.h"

struct p4i_device_info_k_   k;
struct p4i_device_info_d    d;
struct phv_                 p;

%%

p4i_device_info:
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
    phvwr.c1        p.control_metadata_to_device_ip, TRUE
    phvwr.e         p.control_metadata_l2_enabled, \
                        d.p4i_device_info_d.l2_enabled
    phvwr.f         p.control_metadata_learn_enabled, \
                        d.p4i_device_info_d.learn_enabled

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_device_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
