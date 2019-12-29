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
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.key_metadata_vlan, k.ctag_1_vid
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_1_srcAddr
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.tcp_srcPort
    phvwr.c1        p.key_metadata_dport, k.tcp_dstPort
    phvwr.e         p.key_metadata_dst, k.ipv4_1_dstAddr
    phvwr.f         p.key_metadata_proto, k.ipv4_1_protocol

.align
native_ipv6_packet:
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.key_metadata_vlan, k.ctag_1_vid
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_1_srcAddr
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_1_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_1_dstPort
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.tcp_srcPort
    phvwr.c1        p.key_metadata_dport, k.tcp_dstPort
    phvwr.e         p.key_metadata_dst, k.ipv6_1_dstAddr
    phvwr.f         p.key_metadata_proto, k.ipv6_1_nextHdr

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
