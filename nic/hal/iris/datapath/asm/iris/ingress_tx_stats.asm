#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/include/capri_common.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct ingress_tx_stats_k k;
struct phv_               p;

%%

ingress_tx_stats:
    phvwr       p.capri_p4_intrinsic_valid, TRUE
    seq         c2, k.control_metadata_clear_promiscuous_repl, TRUE
    // c3 is used later too.
    seq         c3, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
    setcf       c2, [c2 & c3]
    phvwr.c2    p.capri_intrinsic_tm_replicate_en, 0
    seq         c2, k.control_metadata_uplink, TRUE
    seq.c2      c2, k.control_metadata_nic_mode, TRUE
    setcf       c2, [c2 & !c3]
    phvwr.c2    p.control_metadata_dst_lport, 0

    seq         c2, k.capri_intrinsic_drop, 0
    b.c2        tcp_options_fixup

    // update drop stats
    add         r7, r0, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                    capri_p4_intrinsic_packet_len_sbit6_ebit13}
    addi        r6, r0, 0x1000001
    or          r7, r7, r6, 32

    add         r1, r0, k.flow_lkp_metadata_pkt_type
    addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    .brbegin
    br          r1[1:0]
    add         r5, r5, k.control_metadata_src_lif, 9
    .brcase 0
    add         r5, r5, LIF_STATS_TX_UCAST_DROP_OFFSET
    add         r6, r6, r5[26:0]
    or          r7, r7, r5[31:27], 58
    b           tcp_options_fixup
    memwr.dx    r6, r7
    .brcase 1
    add         r5, r5, LIF_STATS_TX_MCAST_DROP_OFFSET
    add         r6, r6, r5[26:0]
    or          r7, r7, r5[31:27], 58
    b           tcp_options_fixup
    memwr.dx    r6, r7
    .brcase 2
    add         r5, r5, LIF_STATS_TX_BCAST_DROP_OFFSET
    add         r6, r6, r5[26:0]
    or          r7, r7, r5[31:27], 58
    b           tcp_options_fixup
    memwr.dx    r6, r7
    .brcase 3
    b           tcp_options_fixup
    nop
    .brend

#include "tcp_options_fixup.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_tx_stats_error:
    phvwr       p.capri_p4_intrinsic_valid, TRUE
    b           tcp_options_fixup
    phvwr       p.capri_intrinsic_drop, TRUE
