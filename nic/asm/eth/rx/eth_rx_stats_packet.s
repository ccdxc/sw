
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s7_t1_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s7_t1_k_ k;


union eth_rx_stats_packet_d {
    struct {
        ucast_bytes : 64;
        ucast_packets : 64;
        mcast_bytes : 64;
        mcast_packets : 64;
        bcast_bytes : 64;
        bcast_packets : 64;
        rsvd0 : 64;
        rsvd1 : 64;
    };
    struct {
        ucast_drop_bytes : 64;
        ucast_drop_packets : 64;
        mcast_drop_bytes : 64;
        mcast_drop_packets : 64;
        bcast_drop_bytes : 64;
        bcast_drop_packets : 64;
        dma_error : 64;
        rsvd2 : 64;
    };
};

union eth_rx_stats_packet_d d;

%%

.align
eth_rx_stats_packet_accept:
    phvwri.f        p.app_header_table1_valid, 0

    seq             c1, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_UNICAST
    tbladd.c1.e     d.{ucast_bytes}.dx, k.eth_rx_t1_s2s_pkt_len
    tbladd.c1       d.{ucast_packets}.dx, 1
    seq             c2, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_MULTICAST
    tbladd.c2.e     d.{mcast_bytes}.dx, k.eth_rx_t1_s2s_pkt_len
    tbladd.c2       d.{mcast_packets}.dx, 1
    seq             c3, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_BROADCAST
    tbladd.c3.e     d.{bcast_bytes}.dx, k.eth_rx_t1_s2s_pkt_len
    tbladd.c3       d.{bcast_packets}.dx, 1
    nop.e
    nop

.align
eth_rx_stats_packet_drop:
    phvwri.f        p.app_header_table2_valid, 0

    seq             c1, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_UNICAST
    tbladd.c1.e     d.{ucast_drop_bytes}.dx, k.eth_rx_t1_s2s_pkt_len
    tbladd.c1       d.{ucast_drop_packets}.dx, 1
    seq             c2, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_MULTICAST
    tbladd.c2.e     d.{mcast_drop_bytes}.dx, k.eth_rx_t1_s2s_pkt_len
    tbladd.c2       d.{mcast_drop_packets}.dx, 1
    seq             c3, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_BROADCAST
    tbladd.c3.e     d.{bcast_drop_bytes}.dx, k.eth_rx_t1_s2s_pkt_len
    tbladd.c3       d.{bcast_drop_packets}.dx, 1
    nop.e
    nop
