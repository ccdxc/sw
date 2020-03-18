#include "ingress.h"
#include "INGRESS_p.h"
#include "athena.h"
#include "INGRESS_p4i_stats_k.h"


struct p4i_stats_k_     k;
struct p4i_stats_d      d;
struct phv_             p;



%%


p4i_stats:

    seq             c1, k.control_metadata_from_arm, TRUE
    b.c1            p4i_stats_from_arm
    tbladd.c1       d.p4i_stats_d.rx_from_arm, 1

    seq             c1, k.control_metadata_direction, TX_FROM_HOST
    tbladd.c1       d.p4i_stats_d.rx_from_host, 1
    tbladd.!c1      d.p4i_stats_d.rx_from_switch, 1

p4i_stats_from_arm:
    nop.e
    nop
