#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct key_tunneled_k k;
struct key_tunneled_d d;
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
    phvwrpair.e     p.key_metadata_l2_dst, k.ethernet_2_dstAddr, \
                        p.control_metadata_local_l2_mapping_lkp_addr, \
                        k.ethernet_2_dstAddr
    phvwr.f         p.control_metadata_mapping_lkp_addr, k.ipv4_2_dstAddr


.align
tunneled_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_dst, k.{ipv6_2_dstAddr_sbit0_ebit31, \
                                           ipv6_2_dstAddr_sbit32_ebit63, \
                                           ipv6_2_dstAddr_sbit64_ebit127}
    phvwr           p.key_metadata_proto, k.ipv6_2_nextHdr
    phvwrpair.e     p.key_metadata_l2_dst, k.ethernet_2_dstAddr, \
                        p.control_metadata_local_l2_mapping_lkp_addr, \
                        k.ethernet_2_dstAddr
    phvwr.f         p.control_metadata_mapping_lkp_addr, \
                        k.{ipv6_2_dstAddr_sbit0_ebit31, \
                           ipv6_2_dstAddr_sbit32_ebit63, \
                           ipv6_2_dstAddr_sbit64_ebit127}

.align
tunneled_nonip_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr           p.key_metadata_src, k.ethernet_2_srcAddr
    phvwr           p.key_metadata_dst, k.ethernet_2_dstAddr
    phvwrpair.e     p.key_metadata_l2_dst, k.ethernet_2_dstAddr, \
                        p.control_metadata_local_l2_mapping_lkp_addr, \
                        k.ethernet_2_dstAddr
    phvwr.f         p.key_metadata_dport, k.ethernet_2_etherType

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
