#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_lif_tx_stats_k.h"

struct lif_tx_stats_k_  k;
struct lif_tx_stats_d   d;
struct phv_             p;

%%

lif_tx_stats:
    seq             c1, k.egress_recirc_mapping_done, FALSE
    seq.!c1         c1, k.control_metadata_rx_packet, TRUE
    seq.!c1         c1, k.control_metadata_span_copy, TRUE
    seq.!c1         c1, k.p4e_to_arm_valid, TRUE
    nop.c1.e

    add             r1, r0, k.p4e_to_p4plus_classic_nic_l2_pkt_type
    .brbegin
    br              r1[1:0]
    nop
    .brcase PACKET_TYPE_UNICAST
    tbladd.e        d.{lif_tx_stats_d.ucast_bytes}.dx, \
                        k.meter_metadata_meter_len
    tbladd.f        d.{lif_tx_stats_d.ucast_pkts}.dx, 1
    .brcase PACKET_TYPE_MULTICAST
    tbladd.e        d.{lif_tx_stats_d.mcast_bytes}.dx, \
                        k.meter_metadata_meter_len
    tbladd.f        d.{lif_tx_stats_d.mcast_pkts}.dx, 1
    .brcase PACKET_TYPE_BROADCAST
    tbladd.e        d.{lif_tx_stats_d.bcast_bytes}.dx, \
                        k.meter_metadata_meter_len
    tbladd.f        d.{lif_tx_stats_d.bcast_pkts}.dx, 1
    .brcase 3
    tbladd.e.f      d.{lif_tx_stats_d.pad1}.dx, 0
    nop
    .brend

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lif_tx_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
