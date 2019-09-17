#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled_k.h"

struct key_tunneled_k_ k;
struct phv_ p;

%%

tunneled_packet:
    bbeq            k.ipv4_2_valid, 1, tunneled_ipv4_packet
    bbeq            k.ipv6_2_valid, 1, tunneled_ipv6_packet
    nop
tunneled_nonip_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr           p.key_metadata_src, k.ethernet_2_srcAddr
    phvwr.e         p.key_metadata_dst, k.ethernet_2_dstAddr
    phvwr.f         p.key_metadata_dport, k.ethernet_2_etherType

tunneled_ipv4_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_2_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_2_dstAddr
    phvwr           p.key_metadata_ipv4_src, k.ipv4_2_srcAddr
    phvwr.e         p.key_metadata_ipv4_dst, k.ipv4_2_dstAddr
    phvwr.f         p.key_metadata_proto, k.ipv4_2_protocol

tunneled_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_2_srcAddr
    phvwr.e         p.key_metadata_dst, k.ipv6_2_dstAddr
    phvwr.f         p.key_metadata_proto, k.ipv6_2_nextHdr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
