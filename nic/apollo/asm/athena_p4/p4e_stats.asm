#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_p4e_stats_k.h"


struct p4e_stats_k_     k;
struct p4e_stats_d      d;
struct phv_             p;



%%


p4e_stats:
    smeqb           c1, k.control_metadata_p4e_stats_flag, \
                    P4E_STATS_FLAG_TX_TO_HOST,\
                    P4E_STATS_FLAG_TX_TO_HOST
    tbladd.c1       d.p4e_stats_d.tx_to_host, 1

    smeqb           c1, k.control_metadata_p4e_stats_flag, \
                    P4E_STATS_FLAG_TX_TO_SWITCH,\
                    P4E_STATS_FLAG_TX_TO_SWITCH
    tbladd.c1       d.p4e_stats_d.tx_to_switch, 1

    smeqb           c1, k.control_metadata_p4e_stats_flag, \
                    P4E_STATS_FLAG_TX_TO_ARM,\
                    P4E_STATS_FLAG_TX_TO_ARM
    tbladd.c1       d.p4e_stats_d.tx_to_arm, 1

    nop.e
    nop
