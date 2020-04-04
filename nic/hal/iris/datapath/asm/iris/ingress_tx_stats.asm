#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_ingress_tx_stats_k.h"
#include "platform/capri/capri_common.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct ingress_tx_stats_k_  k;
struct ingress_tx_stats_d   d;
struct phv_                 p;

#define DROP_(pkt_type)     ((1 << 2) | pkt_type)

%%

ingress_tx_stats:
    seq         c1, k.recirc_header_valid, TRUE
    phvwr.c1.e  p.capri_intrinsic_tm_oport, TM_PORT_INGRESS

    or          r1, k.flow_lkp_metadata_pkt_type, k.capri_intrinsic_drop, 2
    phvwr       p.capri_p4_intrinsic_valid, TRUE
    .brbegin
    br          r1[2:0]
    add         r5, r5, k.control_metadata_src_lif, LIF_STATS_SIZE_SHIFT
    .brcase PACKET_TYPE_UNICAST
    tbladd      d.{ingress_tx_stats_d.ucast_bytes}.dx, \
                    k.capri_p4_intrinsic_packet_len
    b           ingress_tx_stats2
    tbladd.f    d.{ingress_tx_stats_d.ucast_pkts}.dx, 1
    .brcase PACKET_TYPE_MULTICAST
    tbladd      d.{ingress_tx_stats_d.mcast_bytes}.dx, \
                    k.capri_p4_intrinsic_packet_len
    b           ingress_tx_stats2
    tbladd.f    d.{ingress_tx_stats_d.mcast_pkts}.dx, 1
    .brcase PACKET_TYPE_BROADCAST
    tbladd      d.{ingress_tx_stats_d.bcast_bytes}.dx, \
                    k.capri_p4_intrinsic_packet_len
    b           ingress_tx_stats2
    tbladd.f    d.{ingress_tx_stats_d.bcast_pkts}.dx, 1
    .brcase 3
    b           ingress_tx_stats2
    tbladd.f    d.{ingress_tx_stats_d.pad1}.dx, 0
    .brcase DROP_(PACKET_TYPE_UNICAST)
    tbladd.f    d.{ingress_tx_stats_d.pad1}.dx, 0
    addi        r6, r0, 0x1000001
    or          r7, k.capri_p4_intrinsic_packet_len, r6, 32
    addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    add         r5, r5, LIF_STATS_TX_UCAST_DROP_BYTES_OFFSET
    add         r6, r6, r5[26:0]
    or          r7, r7, r5[31:27], 58
    b           ingress_tx_stats2
    memwr.dx    r6, r7
    .brcase DROP_(PACKET_TYPE_MULTICAST)
    tbladd.f    d.{ingress_tx_stats_d.pad1}.dx, 0
    addi        r6, r0, 0x1000001
    or          r7, k.capri_p4_intrinsic_packet_len, r6, 32
    addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    add         r5, r5, LIF_STATS_TX_MCAST_DROP_BYTES_OFFSET
    add         r6, r6, r5[26:0]
    or          r7, r7, r5[31:27], 58
    b           ingress_tx_stats2
    memwr.dx    r6, r7
    .brcase DROP_(PACKET_TYPE_BROADCAST)
    tbladd.f    d.{ingress_tx_stats_d.pad1}.dx, 0
    addi        r6, r0, 0x1000001
    or          r7, k.capri_p4_intrinsic_packet_len, r6, 32
    addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    add         r5, r5, LIF_STATS_TX_BCAST_DROP_BYTES_OFFSET
    add         r6, r6, r5[26:0]
    or          r7, r7, r5[31:27], 58
    b           ingress_tx_stats2
    memwr.dx    r6, r7
    .brcase 7
    b           ingress_tx_stats2
    tbladd.f    d.{ingress_tx_stats_d.pad1}.dx, 0
    .brend

ingress_tx_stats2:
    seq         c1, k.l4_metadata_policy_enf_cfg_en, TRUE
    bcf         [!c1], ingress_tx_stats_no_repl
    seq         c2, k.control_metadata_flow_miss_ingress, TRUE
    bcf         [c2], ingress_tx_stats_frag

ingress_tx_stats_no_repl:
    seq         c2, k.control_metadata_clear_promiscuous_repl, TRUE
    seq         c3, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
    setcf       c2, [c2 & c3]
    phvwr.c2    p.capri_intrinsic_tm_replicate_en, 0

ingress_tx_stats_frag:
    seq         c1, k.control_metadata_clear_ingresss_mirror, TRUE
    phvwr.c1    p.capri_intrinsic_tm_span_session, 0
    or          r1, r0, k.control_metadata_uplink, P4_I2E_FLAGS_UPLINK
    or          r1, r1, k.l3_metadata_ip_frag, P4_I2E_FLAGS_IP_FRAGMENT
    phvwrm      p.control_metadata_i2e_flags, r1, \
                    ((1 << P4_I2E_FLAGS_UPLINK) | \
                     (1 << P4_I2E_FLAGS_IP_FRAGMENT))

#include "tcp_options_fixup.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_tx_stats_error:
    b           tcp_options_fixup
    phvwr       p.capri_p4_intrinsic_valid, TRUE
