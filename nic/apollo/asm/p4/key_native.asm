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
    add             r6, k.ipv4_1_ihl, k.tcp_dataOffset
    sub             r7, k.ipv4_1_totalLen, r6, 2
    phvwr           p.p4_to_p4plus_tcp_proxy_payload_len, r7
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_1_dstAddr
    phvwr           p.key_metadata_ipv4_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_ipv4_dst, k.ipv4_1_dstAddr
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr           p.key_metadata_proto, k.ipv4_1_protocol
    phvwr           p.key_metadata_l2_dst, k.ethernet_1_dstAddr
    phvwr.e         p.control_metadata_mapping_lkp_addr, k.ipv4_1_srcAddr
    phvwr.f         p.control_metadata_local_l2_mapping_lkp_addr, \
                        k.ethernet_1_srcAddr

.align
native_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, \
                        k.{ipv6_1_srcAddr_sbit0_ebit31...ipv6_1_srcAddr_sbit64_ebit127}
    phvwr           p.key_metadata_dst[127:8], k.ipv6_1_dstAddr_sbit0_ebit119
    phvwr           p.key_metadata_dst[7:0], k.ipv6_1_dstAddr_sbit120_ebit127
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr           p.key_metadata_proto, k.ipv6_1_nextHdr
    phvwr           p.key_metadata_l2_dst, k.ethernet_1_dstAddr
    phvwr.e         p.control_metadata_mapping_lkp_addr, \
                        k.{ipv6_1_srcAddr_sbit0_ebit31...ipv6_1_srcAddr_sbit64_ebit127}
    phvwr.f         p.control_metadata_local_l2_mapping_lkp_addr, \
                        k.ethernet_1_srcAddr

.align
native_nonip_packet:
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.ctag_1_etherType
    phvwr.!c1       p.key_metadata_dport, k.ethernet_1_etherType
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr           p.key_metadata_src, k.ethernet_1_srcAddr
    phvwr           p.key_metadata_l2_dst, k.ethernet_1_dstAddr
    phvwr.e         p.key_metadata_dst, k.ethernet_1_dstAddr
    phvwr.f         p.control_metadata_local_l2_mapping_lkp_addr, \
                        k.ethernet_1_srcAddr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
