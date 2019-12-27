#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_native_k.h"

struct key_native_k_    k;
struct phv_             p;

%%

native_packet:
    bbeq            k.ipv4_1_valid, 1, native_ipv4_packet
    bbeq            k.ipv6_1_valid, 1, native_ipv6_packet
native_nonip_packet:
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.ctag_1_etherType
    phvwr.!c1       p.key_metadata_dport, k.ethernet_1_etherType
    seq             c1, k.arp_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.arp_opcode
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr.e         p.key_metadata_src, k.ethernet_1_srcAddr
    phvwr.f         p.key_metadata_dst, k.ethernet_1_dstAddr

native_ipv4_packet:
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_1_dstAddr
    phvwr           p.key_metadata_ipv4_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_ipv4_dst, k.ipv4_1_dstAddr
    seq             c1, k.udp_1_valid, TRUE
    or.c1           r1, k.udp_1_srcPort, k.udp_1_dstPort, 16
    or.!c1          r1, k.key_metadata_parsed_sport, \
                        k.key_metadata_parsed_dport, 16
    phvwr           p.key_metadata_dport, r1[31:16]
    phvwr           p.key_metadata_sport, r1[15:0]
    phvwr.e         p.key_metadata_proto, k.ipv4_1_protocol
    phvwr.f         p.key_metadata_ktype, KEY_TYPE_IPV4

native_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv6_1_dstAddr
    seq             c1, k.udp_1_valid, TRUE
    or.c1           r1, k.udp_1_srcPort, k.udp_1_dstPort, 16
    or.!c1          r1, k.key_metadata_parsed_sport, \
                        k.key_metadata_parsed_dport, 16
    phvwr           p.key_metadata_dport, r1[31:16]
    phvwr           p.key_metadata_sport, r1[15:0]
    phvwr.e         p.key_metadata_proto, k.ipv6_1_nextHdr
    phvwr.f         p.key_metadata_ktype, KEY_TYPE_IPV6

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
