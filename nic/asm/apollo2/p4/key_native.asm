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
    phvwr           p.p4_to_rxdma_header_slacl_ip_15_00, k.ipv4_1_dstAddr[15:0]
    phvwr           p.p4_to_rxdma_header_slacl_ip_31_16, k.ipv4_1_dstAddr[31:16]
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_1_dstAddr
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr.e         p.key_metadata_proto, k.ipv4_1_protocol
    phvwr           p.control_metadata_mapping_lkp_addr, k.ipv4_1_srcAddr

.align
native_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.{ipv6_1_srcAddr_sbit0_ebit31...ipv6_1_srcAddr_sbit64_ebit127}
    or              r1, k.ipv6_1_dstAddr_sbit112_ebit127, k.ipv6_1_dstAddr_sbit0_ebit111, 16
    phvwr           p.key_metadata_dst, r1
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr.e         p.key_metadata_proto, k.ipv6_1_nextHdr
    phvwr           p.control_metadata_mapping_lkp_addr, k.{ipv6_1_srcAddr_sbit0_ebit31...ipv6_1_srcAddr_sbit64_ebit127}


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
    nop.e
    nop
