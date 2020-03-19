#include "athena.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_native_k.h"

struct key_native_k_ k;
struct key_native_d d;
struct phv_ p;

%%

nop:
    nop.e
    nop

.align
native_ipv4_packet:
    phvwr           p.key_metadata_proto, k.ipv4_1_protocol

    seq             c2, k.control_metadata_direction, RX_FROM_SWITCH
    b.c2            native_ipv4_packet_from_switch
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4

    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
    phvwr.e         p.key_metadata_src, k.ipv4_1_srcAddr
    phvwr.f         p.key_metadata_dst, k.ipv4_1_dstAddr

native_ipv4_packet_from_switch:
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_sport, k.udp_1_dstPort
    b.c1            native_ipv4_packet_done
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.key_metadata_parsed_sport
    phvwr.c1        p.key_metadata_sport, k.key_metadata_parsed_dport
    /* ICMP */
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
native_ipv4_packet_done:
    phvwr.e         p.key_metadata_dst, k.ipv4_1_srcAddr
    phvwr.f         p.key_metadata_src, k.ipv4_1_dstAddr
    

.align
native_ipv6_packet:
    phvwr           p.key_metadata_proto, k.ipv6_1_nextHdr

    seq             c2, k.control_metadata_direction, RX_FROM_SWITCH
    b.c2            native_ipv6_packet_from_switch
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6

    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
    phvwr.e         p.key_metadata_src, k.ipv6_1_srcAddr
    phvwr.f         p.key_metadata_dst, k.ipv6_1_dstAddr

native_ipv6_packet_from_switch:
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_sport, k.udp_1_dstPort
    b.c1            native_ipv6_packet_done
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.key_metadata_parsed_sport
    phvwr.c1        p.key_metadata_sport, k.key_metadata_parsed_dport
    /* ICMP */
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
native_ipv6_packet_done:
    phvwr.e         p.key_metadata_dst, k.ipv6_1_srcAddr
    phvwr.f         p.key_metadata_src, k.ipv6_1_dstAddr


.align
native_nonip_packet:
    phvwr.e.f       p.control_metadata_skip_flow_lkp, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
