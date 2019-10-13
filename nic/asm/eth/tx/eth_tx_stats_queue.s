
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s7_t1_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s7_t1_k_ k;


union eth_tx_stats_queue_d {
    struct {
        oper_csum_hw : 64;
        oper_csum_hw_inner : 64;
        oper_vlan_insert : 64;
        oper_sg : 64;
        oper_tso_sg : 64;
        oper_tso_sot : 64;
        oper_tso_eot : 64;
        rsvd0 : 64;
    };
    struct {
        queue_disabled : 64;
        queue_error : 64;
        desc_fetch_error : 64;
        desc_data_error : 64;
        queue_empty : 64;
        event_disabled : 64;
        event_error : 64;
        rsvd1 : 64;
    };
};

union eth_tx_stats_queue_d d;

%%

.align
eth_tx_stats_queue_accept:
    phvwri.f        p.app_header_table1_valid, 0

    crestore        [c7-c1], k.eth_tx_global_stats[STAT_oper_tso_eot:STAT_oper_csum_hw], 0x3f
    tbladd.c1.e.f   d.{oper_csum_hw}.dx, 1
    tbladd.c2.e.f   d.{oper_csum_hw_inner}.dx, 1
    tbladd.c3.e.f   d.{oper_vlan_insert}.dx, 1
    tbladd.c4.e.f   d.{oper_sg}.dx, 1
    tbladd.c5.e.f   d.{oper_tso_sg}.dx, 1
    tbladd.c6.e.f   d.{oper_tso_sot}.dx, 1
    tbladd.c7.e.f   d.{oper_tso_eot}.dx, 1

.align
eth_tx_stats_queue_drop:
    phvwri.f        p.app_header_table1_valid, 0

    crestore        [c7-c1], k.eth_tx_global_stats[STAT_event_error:STAT_queue_disabled], 0x7f
    tbladd.c1.e.f   d.{queue_disabled}.dx, 1
    tbladd.c2.e.f   d.{queue_error}.dx, 1
    tbladd.c3.e.f   d.{desc_fetch_error}.dx, 1
    tbladd.c4.e.f   d.{desc_data_error}.dx, 1
    tbladd.c5.e.f   d.{queue_empty}.dx, 1
    tbladd.c6.e.f   d.{event_disabled}.dx, 1
    tbladd.c7.e.f   d.{event_error}.dx, 1
