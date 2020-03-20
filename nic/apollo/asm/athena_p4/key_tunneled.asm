#include "athena.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled_k.h"

struct key_tunneled_k_  k;
struct key_tunneled_d   d;
struct phv_             p;

%%

nop:
    nop.e
    nop

.align
tunneled_ipv4_packet:
    phvwr           p.key_metadata_proto, k.ipv4_2_protocol

    seq             c2, k.control_metadata_direction, RX_FROM_SWITCH
    b.c2            tunneled_ipv4_packet_from_switch
    phvwr           p.key_metadata_ktype, P4_KEY_TYPE_IPV4

    seq             c1, k.udp_2_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_2_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_2_dstPort
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
    phvwr.e         p.key_metadata_src, k.ipv4_2_srcAddr
    phvwr.f         p.key_metadata_dst, k.ipv4_2_dstAddr

tunneled_ipv4_packet_from_switch:
    seq             c1, k.udp_2_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.udp_2_srcPort
    phvwr.c1        p.key_metadata_sport, k.udp_2_dstPort
    b.c1            tunneled_ipv4_packet_done
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.key_metadata_parsed_sport
    phvwr.c1        p.key_metadata_sport, k.key_metadata_parsed_dport
    /* ICMP */
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
tunneled_ipv4_packet_done:
    phvwr.e         p.key_metadata_dst, k.ipv4_2_srcAddr
    phvwr.f         p.key_metadata_src, k.ipv4_2_dstAddr

.align
tunneled_ipv6_packet:
    phvwr           p.key_metadata_proto, k.ipv6_2_nextHdr

    seq             c2, k.control_metadata_direction, RX_FROM_SWITCH
    b.c2            tunneled_ipv6_packet_from_switch
    phvwr           p.key_metadata_ktype, P4_KEY_TYPE_IPV6

    seq             c1, k.udp_2_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_2_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_2_dstPort
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
    phvwr.e         p.key_metadata_src, k.ipv6_2_srcAddr
    phvwr.f         p.key_metadata_dst, k.ipv6_2_dstAddr

tunneled_ipv6_packet_from_switch:
    seq             c1, k.udp_2_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.udp_2_srcPort
    phvwr.c1        p.key_metadata_sport, k.udp_2_dstPort
    b.c1            tunneled_ipv6_packet_done
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_dport, k.key_metadata_parsed_sport
    phvwr.c1        p.key_metadata_sport, k.key_metadata_parsed_dport
    /* ICMP */
    phvwr.!c1       p.key_metadata_sport, k.key_metadata_parsed_sport
    phvwr.!c1       p.key_metadata_dport, k.key_metadata_parsed_dport
tunneled_ipv6_packet_done:
    phvwr.e         p.key_metadata_dst, k.ipv6_2_srcAddr
    phvwr.f         p.key_metadata_src, k.ipv6_2_dstAddr

.align
tunneled_nonip_packet:
    phvwr.e.f       p.control_metadata_skip_flow_lkp, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
