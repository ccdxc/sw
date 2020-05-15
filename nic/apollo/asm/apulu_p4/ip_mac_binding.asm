#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_ip_mac_binding_k.h"

struct ip_mac_binding_k_    k;
struct ip_mac_binding_d     d;
struct phv_                 p;

%%

binding_info:
    seq             c1, k.key_metadata_local_mapping_lkp_type, KEY_TYPE_IPV4
    bcf             [!c1], binding_info_non_ipv4
    sne             c1, k.ethernet_1_srcAddr, d.binding_info_d.addr
    nop.e
    phvwr.c1.f      p.p4i_i2e_binding_check_drop, TRUE

binding_info_non_ipv4:
    seq             c1, k.ipv6_1_valid, TRUE
    nop.!c1.e
    seq             c1, k.ipv6_1_srcAddr[127:64], d.binding_info_d.addr[127:64]
    seq.c1          c1, k.ipv6_1_srcAddr[63:0], d.binding_info_d.addr[63:0]
    nop.e
    phvwr.!c1.f     p.p4i_i2e_binding_check_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ip_mac_binding_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
