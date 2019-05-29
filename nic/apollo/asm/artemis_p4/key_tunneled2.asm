#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled2_k.h"

struct key_tunneled2_k_ k;
struct key_tunneled2_d  d;
struct phv_ p;

%%

nop:
    nop.e
    nop

.align
tunneled2_ipv4_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr           p.key_metadata_src, k.ipv4_3_srcAddr
    phvwr           p.key_metadata_dst, k.ipv4_3_dstAddr
    phvwr           p.key_metadata_ipv4_src, k.ipv4_3_srcAddr
    phvwr           p.key_metadata_ipv4_dst, k.ipv4_3_dstAddr
    phvwr           p.key_metadata_proto, k.ipv4_3_protocol
    phvwr.e         p.key_metadata_mapping_ip2, k.ipv4_3_dstAddr
    phvwr.f         p.key_metadata_mapping_port, k.key_metadata_dport

.align
tunneled2_ipv6_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV6
    phvwr           p.key_metadata_src, k.ipv6_3_srcAddr
    phvwr           p.key_metadata_dst, k.ipv6_3_dstAddr
    phvwr           p.key_metadata_proto, k.ipv6_3_nextHdr
    phvwr.e         p.key_metadata_mapping_ip2, k.ipv6_3_dstAddr
    phvwr.f         p.key_metadata_mapping_port, k.key_metadata_dport

.align
tunneled2_nonip_packet:
    phvwr           p.key_metadata_ktype, KEY_TYPE_MAC
    phvwr           p.key_metadata_src, k.ethernet_3_srcAddr
    phvwr.e         p.key_metadata_dst, k.ethernet_3_dstAddr
    phvwr.f         p.key_metadata_dport, k.ethernet_3_etherType

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled2_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
