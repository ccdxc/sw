#include "apulu.h"
#include "EGRESS_p.h"
#include "EGRESS_p4e_drop_stats_k.h"

struct p4e_drop_stats_k_    k;
struct phv_                 p;

%%

p4e_drop_stats:
    seq             c1, r5, r0
    seq.!c1         c1, k.control_metadata_rx_packet, TRUE
    seq.!c1         c1, k.control_metadata_span_copy, TRUE
    nop.c1.e
    add             r7, r0, k.meter_metadata_meter_len
    addi            r6, r0, 0x1000001
    or              r7, r7, r6, 32
    add             r1, r0, k.p4e_to_p4plus_classic_nic_l2_pkt_type
    addi            r6, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    .brbegin
    br              r1[1:0]
    add             r5, r5, k.p4e_i2e_src_lif, LIF_STATS_SIZE_SHIFT
    .brcase PACKET_TYPE_UNICAST
    add             r5, r5, LIF_STATS_TX_UCAST_DROP_BYTES_OFFSET
    add             r6, r6, r5[26:0]
    or.e            r7, r7, r5[31:27], 58
    memwr.dx        r6, r7
    .brcase PACKET_TYPE_MULTICAST
    add             r5, r5, LIF_STATS_RX_MCAST_DROP_BYTES_OFFSET
    add             r6, r6, r5[26:0]
    or.e            r7, r7, r5[31:27], 58
    memwr.dx        r6, r7
    .brcase PACKET_TYPE_BROADCAST
    add             r5, r5, LIF_STATS_RX_BCAST_DROP_BYTES_OFFSET
    add             r6, r6, r5[26:0]
    or.e            r7, r7, r5[31:27], 58
    memwr.dx        r6, r7
    .brcase 3
    nop.e
    nop
    .brend

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4e_drop_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
