#include "apulu.h"
#include "INGRESS_p.h"
#include "INGRESS_p4i_drop_stats_k.h"

struct p4i_drop_stats_k_    k;
struct phv_                 p;

%%

p4i_drop_stats:
    seq             c1, r5, r0
    seq.!c1         c1, k.control_metadata_rx_packet, TRUE
    nop.c1.e
    phvwr.f         p.{capri_p4_intrinsic_valid, \
                        arm_to_p4i_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x10
    add             r7, r0, k.capri_p4_intrinsic_packet_len
    addi            r6, r0, 0x1000001
    or              r7, r7, r6, 32
    seq             c1, k.ethernet_1_dstAddr[40], 0
    seq             c2, k.ethernet_1_dstAddr, -1
    cmov.c1         r4, c1, LIF_STATS_TX_UCAST_DROP_BYTES_OFFSET, 0
    cmov.!c1        r4, c2, LIF_STATS_TX_BCAST_DROP_BYTES_OFFSET, \
                        LIF_STATS_TX_MCAST_DROP_BYTES_OFFSET
    addi            r6, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    add             r5, r5, k.p4i_i2e_src_lif, LIF_STATS_SIZE_SHIFT
    add             r5, r5, r4
    add             r6, r6, r5[26:0]
    or.e            r7, r7, r5[31:27], 58
    memwr.dx        r6, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_drop_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr.f         p.capri_p4_intrinsic_valid, 1
