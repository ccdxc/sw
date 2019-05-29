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
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_1_dstAddr
    phvwr           p.key_metadata_ipv4_src, k.ipv4_1_srcAddr
    phvwr           p.key_metadata_ipv4_dst, k.ipv4_1_dstAddr
    phvwr           p.key_metadata_proto, k.ipv4_1_protocol
    seq             c1, k.udp_1_valid, TRUE
    or.c1           r1, k.udp_1_srcPort, k.udp_1_dstPort, 16
    or.!c1          r1, k.key_metadata_sport, k.key_metadata_dport, 16
    phvwr.c1        p.{key_metadata_dport,key_metadata_sport}, r1
    bbeq            k.control_metadata_direction, TX_FROM_HOST, \
                        native_ipv4_packet_from_host
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
native_ipv4_packet_from_network:
    phvwr.e         p.key_metadata_mapping_ip, k.ipv4_1_dstAddr
    phvwr.f         p.key_metadata_mapping_port, r1[31:16]
native_ipv4_packet_from_host:
    phvwr.e         p.key_metadata_mapping_ip, k.ipv4_1_srcAddr
    phvwr.f         p.key_metadata_mapping_port, r1[15:0]

.align
native_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_1_srcAddr
    phvwr           p.key_metadata_dst[127:8], k.ipv6_1_dstAddr_s0_e119
    phvwr           p.key_metadata_dst[7:0], k.ipv6_1_dstAddr_s120_e127
    phvwr           p.key_metadata_proto, k.ipv6_1_nextHdr
    seq             c1, k.udp_1_valid, TRUE
    or.c1           r1, k.udp_1_srcPort, k.udp_1_dstPort, 16
    or.!c1          r1, k.key_metadata_sport, k.key_metadata_dport, 16
    phvwr.c1        p.{key_metadata_dport,key_metadata_sport}, r1
    bbeq            k.control_metadata_direction, TX_FROM_HOST, \
                        native_ipv6_packet_from_host
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
native_ipv6_packet_from_network:
    phvwr           p.key_metadata_mapping_ip[127:8], k.ipv6_1_dstAddr_s0_e119
    phvwr.e         p.key_metadata_mapping_ip[7:0], k.ipv6_1_dstAddr_s120_e127
    phvwr.f         p.key_metadata_mapping_port, r1[31:16]
native_ipv6_packet_from_host:
    phvwr.e         p.key_metadata_mapping_ip, k.ipv6_1_srcAddr
    phvwr.f         p.key_metadata_mapping_port, r1[15:0]

.align
native_nonip_packet:
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.ctag_1_etherType
    phvwr.!c1       p.key_metadata_dport, k.ethernet_1_etherType
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr.e         p.key_metadata_src, k.ethernet_1_srcAddr
    phvwr.f         p.key_metadata_dst, k.ethernet_1_dstAddr

.align
set_tep1_dst:
    seq             c1, k.ipv4_1_valid, TRUE
    phvwr.c1.e      p.key_metadata_mapping_ip, k.ipv4_1_dstAddr
    seq             c1, k.ipv6_1_valid, TRUE
    phvwr.c1        p.key_metadata_mapping_ip[127:8], k.ipv6_1_dstAddr_s0_e119
    nop.e
    phvwr.c1        p.key_metadata_mapping_ip[7:0], k.ipv6_1_dstAddr_s120_e127

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
