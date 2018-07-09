#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct key_tunneled_k k;
struct key_tunneled_d d;
struct phv_ p;

%%

nop:
    nop.e
    nop

.align
tunneled_ipv4_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_2_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_2_dstAddr
    phvwr           p.key_metadata_proto, k.ipv4_2_protocol
    phvwr.e         p.slacl_metadata_ip_15_00, k.ipv4_2_dstAddr[15:0]
    phvwr           p.slacl_metadata_ip_31_16, k.ipv4_2_dstAddr[31:16]

.align
tunneled_ipv6_packet:
    nop.e
    nop

.align
tunneled_nonip_packet:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    nop.e
    nop
