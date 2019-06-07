#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nat_k.h"

struct nat_k_ k;
struct nat_d  d;
struct phv_ p;

%%

nat_rewrite:
    seq             c1, k.nat_metadata_xlate_idx, r0
    nop.c1.e
    seq             c1, k.ipv4_1_valid, TRUE
    seq             c2, k.ipv6_1_valid, TRUE
    bcf             [!c1 & !c2], nat_rewrite_done
    seq             c2, k.udp_1_valid, TRUE
    seq             c3, k.tcp_valid, TRUE
    bbne            k.control_metadata_direction, TX_FROM_HOST, \
                        nat_rewrite_rx
nat_rewrite_tx:
    seq             c4, k.rewrite_metadata_flags[TX_REWRITE_SRC_IP_BITS], \
                        TX_REWRITE_SRC_IP_FROM_PUBLIC
    seq             c5, k.rewrite_metadata_flags[TX_REWRITE_SRC_IP_BITS], \
                        TX_REWRITE_SRC_IP_FROM_SERVICE
    setcf           c6, [c4|c5]
    b.!c6           nat_rewrite_tx_dst_ip
    setcf           c4, [c6&c1]
    phvwr.c4        p.ipv4_1_srcAddr, d.nat_rewrite_d.nat_ip
    setcf           c4, [c6&!c1]
    phvwr.c4        p.ipv6_1_srcAddr, d.nat_rewrite_d.nat_ip
    b.!c5           nat_rewrite_tx_dst_ip
    nop
    phvwr.c2        p.udp_1_srcPort, k.p4e_i2e_service_xlate_port
    phvwr.c3        p.tcp_srcPort, k.p4e_i2e_service_xlate_port
nat_rewrite_tx_dst_ip:
    seq             c4, k.rewrite_metadata_flags[TX_REWRITE_DST_IP_BITS], \
                        TX_REWRITE_DST_IP_FROM_SESSION
    b.!c4           nat_rewrite_tx_dport
    setcf           c5, [c4&c1]
    phvwr.c5        p.ipv4_1_dstAddr, k.rewrite_metadata_ip
    setcf           c5, [c4&!c1]
    phvwr.c5        p.ipv6_1_dstAddr, k.rewrite_metadata_ip
nat_rewrite_tx_dport:
    seq             c4, k.rewrite_metadata_flags[TX_REWRITE_DPORT_BITS], \
                        TX_REWRITE_DPORT_FROM_SESSION
    b.!c4           nat_rewrite_encap
    nop
    phvwr.c2        p.udp_1_dstPort, k.rewrite_metadata_l4port
    phvwr.c3        p.tcp_dstPort, k.rewrite_metadata_l4port
nat_rewrite_encap:
    seq.e           c4, k.rewrite_metadata_flags[TX_REWRITE_ENCAP_BITS], \
                        TX_REWRITE_ENCAP_VXLAN
    phvwr.c4        p.rewrite_metadata_encap_src_ip, d.nat_rewrite_d.nat_ip

nat_rewrite_rx:
    seq             c4, k.rewrite_metadata_flags[RX_REWRITE_SRC_IP_BITS], \
                        RX_REWRITE_SRC_IP_FROM_SESSION
    b.!c4           nat_rewrite_rx_sport
    setcf           c5, [c4&c1]
    phvwr.c5        p.ipv4_1_srcAddr, k.rewrite_metadata_ip
    setcf           c5, [c4&!c1]
    phvwr.c5        p.ipv6_1_srcAddr, k.rewrite_metadata_ip
nat_rewrite_rx_sport:
    seq             c4, k.rewrite_metadata_flags[RX_REWRITE_SPORT_BITS], \
                        RX_REWRITE_SPORT_FROM_SESSION
    b.!c4           nat_rewrite_rx_dst_ip
    nop
    phvwr.c2        p.udp_1_srcPort, k.rewrite_metadata_l4port
    phvwr.c3        p.tcp_srcPort, k.rewrite_metadata_l4port
nat_rewrite_rx_dst_ip:
    seq             c4, k.rewrite_metadata_flags[RX_REWRITE_DST_IP_BITS], \
                        RX_REWRITE_DST_IP_FROM_CA
    seq             c5, k.rewrite_metadata_flags[RX_REWRITE_DST_IP_BITS], \
                        RX_REWRITE_DST_IP_FROM_SERVICE
    setcf           c6, [c4|c5]
    nop.!c6.e
    setcf           c4, [c6&c1]
    phvwr.c4        p.ipv4_1_dstAddr, d.nat_rewrite_d.nat_ip
    setcf           c4, [c6&!c1]
    phvwr.c4        p.ipv6_1_dstAddr, d.nat_rewrite_d.nat_ip
    nop.!c5.e
    nop
    phvwr.c2        p.udp_1_dstPort, k.p4e_i2e_service_xlate_port
    nop.e
    phvwr.c3        p.tcp_dstPort, k.p4e_i2e_service_xlate_port

nat_rewrite_done:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nat_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
