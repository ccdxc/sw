#include "ingress.h"
#include "INGRESS_p.h"
#include "athena.h"
#include "INGRESS_p4i_stats_k.h"


/*****************************************************************************/
/* Checksum unit assignment                                                  */
/*****************************************************************************/
#include "CSUM_INGRESS.h"
#define CSUM_FROM_HOST_USER                     ((1 << csum_hdr_ipv4_1) |\
                                                (1 << csum_hdr_udp_1)   |\
                                                (1 << csum_hdr_tcp))
#define CSUM_FROM_SWITCH_USER                   ((1 << csum_hdr_ipv4_2) |\
                                                (1 << csum_hdr_udp_2)   |\
                                                (1 << csum_hdr_tcp))
#define CSUM_FROM_SWITCH_SUBSTRATE              ((1 << csum_hdr_ipv4_1)) 


struct p4i_stats_k_     k;
struct p4i_stats_d      d;
struct phv_             p;



%%


p4i_stats:

    seq             c1, k.control_metadata_from_arm, TRUE
    b.c1            p4i_stats_from_arm
    tbladd.c1       d.p4i_stats_d.rx_from_arm, 1

    seq             c1, k.control_metadata_direction, TX_FROM_HOST
    b.c1            p4i_stats_from_host
    tbladd.!c1      d.p4i_stats_d.rx_from_switch, 1

    smneb           c1, k.capri_intrinsic_csum_err, CSUM_FROM_SWITCH_SUBSTRATE, 0
    tbladd.c1.e     d.p4i_stats_d.rx_substrate_csum_err, 1
    phvwr.c1        p.capri_intrinsic_drop, 1

    smneb           c1, k.capri_intrinsic_csum_err, CSUM_FROM_SWITCH_USER, 0
    tbladd.c1.e     d.p4i_stats_d.rx_user_csum_err, 1
    phvwr.c1        p.capri_intrinsic_drop, 1

    nop.e
    nop


p4i_stats_from_host:
    tbladd          d.p4i_stats_d.rx_from_host, 1

    smneb           c1, k.capri_intrinsic_csum_err, CSUM_FROM_HOST_USER, 0
    tbladd.c1       d.p4i_stats_d.rx_user_csum_err, 1
    phvwr.c1        p.capri_intrinsic_drop, 1

    nop.e
    nop

p4i_stats_from_arm:
    nop.e
    nop
