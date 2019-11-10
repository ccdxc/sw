
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s7_t2_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s7_t2_k_ k;


union eth_rx_stats_queue_d {
    struct {
        oper_rss : 64;
        oper_csum_complete : 64;
        oper_csum_ip_bad : 64;
        oper_csum_tcp_bad : 64;
        oper_csum_udp_bad : 64;
        oper_vlan_strip : 64;
        rsvd0 : 64;
        rsvd1 : 64;
    };
    struct {
        queue_disabled : 64;
        queue_empty : 64;
        queue_error : 64;
        desc_fetch_error : 64;
        desc_data_error : 64;
        event_disabled : 64;
        event_error : 64;
        rsvd2 : 64;
    };
};

union eth_rx_stats_queue_d d;

%%

.align
eth_rx_stats_queue_accept:
    phvwri.f        p.app_header_table2_valid, 0

    crestore        [c6-c1], k.eth_rx_global_stats[STAT_oper_vlan_strip:STAT_oper_rss], 0x3f
    tbladd.c1.e.f   d.{oper_rss}.dx, 1
    tbladd.c2.e.f   d.{oper_csum_complete}.dx, 1
    tbladd.c3.e.f   d.{oper_csum_ip_bad}.dx, 1
    tbladd.c4.e.f   d.{oper_csum_tcp_bad}.dx, 1
    tbladd.c5.e.f   d.{oper_csum_udp_bad}.dx, 1
    tbladd.c6.e.f   d.{oper_vlan_strip}.dx, 1
    nop

.align
eth_rx_stats_queue_drop:
    phvwri.f        p.app_header_table2_valid, 0

    crestore        [c7-c1], k.eth_rx_global_stats[STAT_event_error:STAT_queue_disabled], 0x7f
    tbladd.c1.e.f   d.{queue_disabled}.dx, 1
    tbladd.c2.e.f   d.{queue_empty}.dx, 1
    tbladd.c3.e.f   d.{queue_error}.dx, 1
    tbladd.c4.e.f   d.{desc_fetch_error}.dx, 1
    tbladd.c5.e.f   d.{desc_data_error}.dx, 1
    tbladd.c6.e.f   d.{event_disabled}.dx, 1
    tbladd.c7.e.f   d.{event_error}.dx, 1
    nop
