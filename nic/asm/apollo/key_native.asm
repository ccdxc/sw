#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct key_native_k k;
struct key_native_d d;
struct phv_ p;

%%

nop:
    nop.e
    nop

.align
native_ipv4_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_1_dstAddr
    phvwr           p.key_metadata_proto, k.ipv4_1_protocol
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr.e         p.slacl_metadata_ip_15_00, k.ipv4_1_dstAddr[15:0]
    phvwr           p.slacl_metadata_ip_31_16, k.ipv4_1_dstAddr[31:16]

.align
native_ipv6_packet:
    nop.e
    nop

.align
native_nonip_packet:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    nop.e
    nop
