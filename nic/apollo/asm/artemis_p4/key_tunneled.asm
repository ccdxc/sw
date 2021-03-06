#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled_k.h"

struct key_tunneled_k_ k;
struct key_tunneled_d  d;
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
    phvwr           p.key_metadata_ipv4_src, k.ipv4_2_srcAddr
    phvwr           p.key_metadata_ipv4_dst, k.ipv4_2_dstAddr
    phvwr           p.key_metadata_proto, k.ipv4_2_protocol
    seq             c1, k.udp_2_valid, TRUE
    or.c1           r1, k.udp_2_srcPort, k.udp_2_dstPort, 16
    or.!c1          r1, r0, k.key_metadata_dport, 16
    phvwr.c1        p.{key_metadata_dport,key_metadata_sport}, r1
    phvwr.e         p.key_metadata_mapping_ip2, k.ipv4_2_dstAddr
    phvwr.f         p.key_metadata_mapping_port, r1[31:16]

.align
tunneled_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_2_srcAddr
    phvwr           p.key_metadata_dst, k.ipv6_2_dstAddr
    phvwr           p.key_metadata_proto, k.ipv6_2_nextHdr
    seq             c1, k.udp_2_valid, TRUE
    or.c1           r1, k.udp_2_srcPort, k.udp_2_dstPort, 16
    or.!c1          r1, r0, k.key_metadata_dport, 16
    phvwr.c1        p.{key_metadata_dport,key_metadata_sport}, r1
    phvwr           p.key_metadata_mapping_ip2, k.ipv6_2_dstAddr
    phvwr.f         p.key_metadata_mapping_port, r1[31:16]

.align
tunneled_nonip_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr           p.key_metadata_src, k.ethernet_2_srcAddr
    phvwr.e         p.key_metadata_dst, k.ethernet_2_dstAddr
    phvwr.f         p.key_metadata_dport, k.ethernet_2_etherType

.align
set_tep2_dst:
    seq             c1, k.ipv4_1_valid, TRUE
    phvwr.c1.e      p.tunnel_metadata_tep2_dst, k.ipv4_2_dstAddr
    seq.e           c1, k.ipv6_1_valid, TRUE
    phvwr.c1        p.tunnel_metadata_tep2_dst, k.ipv6_2_dstAddr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
