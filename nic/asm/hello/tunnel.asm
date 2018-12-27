#include "ingress.h"
#include "INGRESS_p.h"
#include "sdk/include/sdk/platform/capri/capri_p4.h"
#include "p4/common/defines.h"

struct tunnel_k k;
struct tunnel_d d;
struct phv_     p;

%%

tunnel_rewrite:
    phvwr           p.{ctag_1_valid,ethernet_1_valid}, 0
    phvwr           p.{ipv4_0_valid,ctag_0_valid,ethernet_0_valid}, 0x5
    phvwrpair       p.mpls_dst_0_valid, 1, p.udp_0_valid, 1
    phvwr           p.{ethernet_0_dstAddr,ethernet_0_srcAddr}, \
                        d.{tunnel_rewrite_d.dmac,tunnel_rewrite_d.smac}
    phvwr           p.ethernet_0_etherType, ETHERTYPE_IPV4
    add             r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5, \
                       capri_p4_intrinsic_frame_size_sbit6_ebit13}, 1
    seq             c1, k.ctag_1_valid, TRUE
    sub.c1          r1, r1, 4
    phvwr           p.{ipv4_0_version,ipv4_0_ihl}, 0x45
    phvwrpair       p.ipv4_0_srcAddr, d.tunnel_rewrite_d.sip, \
                        p.ipv4_0_dstAddr, d.tunnel_rewrite_d.dip
    phvwr           p.{ipv4_0_ttl,ipv4_0_protocol}, (64 << 8) | IP_PROTO_UDP
    phvwr           p.mpls_dst_0_label, d.tunnel_rewrite_d.slot_id
    phvwr           p.mpls_dst_0_bos, 1
    phvwr           p.ipv4_0_totalLen, r1
    sub             r1, r1, 20
    add             r7, UDP_PORT_MPLS, k.flow_metadata_entropy_hash, 16
    phvwr.e         p.{udp_0_srcPort,udp_0_dstPort}, r7
    phvwr.f         p.udp_0_len, r1
