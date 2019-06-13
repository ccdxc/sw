#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_local_46_mapping_k.h"

struct local_46_mapping_k_ k;
struct local_46_mapping_d  d;
struct phv_ p;

%%

local_46_info:
    bbne            k.control_metadata_direction, TX_FROM_HOST, rx_from_switch_6to4
tx_from_host_4to6:
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_SRC_IP_BITS], \
                        TX_REWRITE_SRC_IP_FROM_46
    nop.!c1.e
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.ctag_1_etherType, ETHERTYPE_IPV6
    phvwr.!c1       p.ethernet_1_etherType, ETHERTYPE_IPV6
    phvwr           p.ipv4_1_valid, FALSE
    phvwr           p.ipv6_1_valid, TRUE
    add             r1, 6, k.ipv4_1_diffserv, 4
    phvwr           p.{ipv6_1_version,ipv6_1_trafficClass}, r1
    add             r1, k.ipv4_1_ttl, k.ipv4_1_protocol, 8
    sub             r2, k.ipv4_1_totalLen, 20
    add             r1, r1, r2, 16
    phvwr           p.{ipv6_1_flowLabel,ipv6_1_payloadLen,ipv6_1_nextHdr, \
                        ipv6_1_hopLimit}, r1
    or              r1, d.local_46_info_d.prefix[63:0], k.ipv4_1_srcAddr
    phvwrpair       p.ipv6_1_srcAddr[127:64], d.local_46_info_d.prefix[127:64],\
                        p.ipv6_1_srcAddr[63:0], r1
    or.e            r1, k.rewrite_metadata_ip[63:0], k.ipv4_1_dstAddr
    phvwrpair       p.ipv6_1_dstAddr[127:64], k.rewrite_metadata_ip[127:64],\
                        p.ipv6_1_dstAddr[63:0], r1

rx_from_switch_6to4:
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_SRC_IP_BITS], \
                        RX_REWRITE_SRC_IP_FROM_64
    nop.!c1.e
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.c1        p.ctag_1_etherType, ETHERTYPE_IPV4
    phvwr.!c1       p.ethernet_1_etherType, ETHERTYPE_IPV4
    phvwr           p.ipv6_1_valid, FALSE
    phvwr           p.ipv4_1_valid, TRUE
    phvwr           p.{ipv4_1_version,ipv4_1_ihl}, 0x45
    add             r1, k.ipv6_1_payloadLen, 20
    add             r1, r1, k.ipv6_1_trafficClass, 16
    phvwr           p.{ipv4_1_diffserv,ipv4_1_totalLen}, r1
    add             r1, k.ipv6_1_dstAddr_s120_e127, k.ipv6_1_dstAddr_s112_e119, 8
    add             r1, r1, k.ipv6_1_dstAddr_s0_e111[15:0], 16
    phvwrpair       p.ipv4_1_srcAddr, k.ipv6_1_srcAddr[31:0], \
                        p.ipv4_1_dstAddr, r1
    add.e           r1, k.ipv6_1_nextHdr, k.ipv6_1_hopLimit, 8
    phvwr.f         p.{ipv4_1_identification,ipv4_1_flags,ipv4_1_fragOffset, \
                        ipv4_1_ttl,ipv4_1_protocol}, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_46_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
