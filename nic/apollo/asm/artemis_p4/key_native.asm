#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_native_k.h"

struct key_native_k_    k;
struct key_native_d     d;
struct phv_             p;

%%

nop:
    nop.e
    nop

.align
native_ipv4_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_1_dstAddr
    phvwr           p.key_metadata_ipv4_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_ipv4_dst, k.ipv4_1_dstAddr
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.e         p.key_metadata_proto, k.ipv4_1_protocol
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort

.align
native_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_1_srcAddr
    phvwr           p.key_metadata_dst[127:8], k.ipv6_1_dstAddr_s0_e119
    phvwr           p.key_metadata_dst[7:0], k.ipv6_1_dstAddr_s120_e127
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.e         p.key_metadata_proto, k.ipv6_1_nextHdr
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort

.align
native_nonip_packet:
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.ctag_1_etherType
    phvwr.!c1       p.key_metadata_dport, k.ethernet_1_etherType
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr.e         p.key_metadata_src, k.ethernet_1_srcAddr
    phvwr           p.key_metadata_dst, k.ethernet_1_dstAddr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
