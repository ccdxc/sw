#include "egress.h"
#include "EGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/capri_common.h"

struct tx_stats_k k;
struct phv_       p;

%%

tx_stats:
    add         r7, r0, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                    capri_p4_intrinsic_packet_len_sbit6_ebit13}
    addi        r6, r0, 0x1000001
    or          r7, r7, r6, 32

    or          r1, k.flow_lkp_metadata_pkt_type, k.capri_intrinsic_drop, 2
    addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    .brbegin
    br          r1[2:0]
    add         r5, r5, k.control_metadata_src_lif, 9
    .brcase 0
    add         r5, r5, LIF_STATS_TX_UCAST_OFFSET
    add         r6, r6, r5[26:0]
    or.e        r7, r7, r5[31:27], 58
    memwr.dx    r6, r7
    .brcase 1
    add         r5, r5, LIF_STATS_TX_MCAST_OFFSET
    add         r6, r6, r5[26:0]
    or.e        r7, r7, r5[31:27], 58
    memwr.dx    r6, r7
    .brcase 2
    add         r5, r5, LIF_STATS_TX_BCAST_OFFSET
    add         r6, r6, r5[26:0]
    or.e        r7, r7, r5[31:27], 58
    memwr.dx    r6, r7
    .brcase 3
    nop.e
    nop
    .brcase 4
    add         r5, r5, LIF_STATS_TX_UCAST_DROP_OFFSET
    add         r6, r6, r5[26:0]
    or.e        r7, r7, r5[31:27], 58
    memwr.dx    r6, r7
    .brcase 5
    add         r5, r5, LIF_STATS_TX_MCAST_DROP_OFFSET
    add         r6, r6, r5[26:0]
    or.e        r7, r7, r5[31:27], 58
    memwr.dx    r6, r7
    .brcase 6
    add         r5, r5, LIF_STATS_TX_BCAST_DROP_OFFSET
    add         r6, r6, r5[26:0]
    or.e        r7, r7, r5[31:27], 58
    memwr.dx    r6, r7
    .brcase 7
    nop.e
    nop
    .brend


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_stats_error:
  phvwr.e     p.capri_intrinsic_drop, TRUE
  nop
