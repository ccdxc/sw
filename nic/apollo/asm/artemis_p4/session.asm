#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_session_k.h"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "asic/cmn/asic_common.hpp"

struct session_k_ k;
struct session_d  d;
struct phv_ p;

%%

session_info:
    seq             c1, k.p4e_i2e_session_index, r0
    // r7 : packet length
    sub             r7, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l2_1
    bcf             [c1],session_info_error
    phvwr           p.capri_p4_intrinsic_packet_len, r7

    // update timestamp (flush/unlock if packet is not TCP or dropped)
    seq             c1, k.tcp_valid, 0
    seq.!c1         c1, d.session_info_d.drop, 1
    tblwr.c1.f      d.session_info_d.timestamp, r4
    tblwr.!c1       d.session_info_d.timestamp, r4

    seq             c1, k.tcp_valid, 1
    bcf             [!c1], session_info_common
    seq             c1, k.p4e_i2e_flow_role, TCP_FLOW_RESPONDER
    bcf             [c1], session_tcp_responder
session_tcp_initiator:
    nop
    b               session_info_common
    nop

session_tcp_responder:

session_info_common:
    phvwr           p.rewrite_metadata_nexthop_idx, d.session_info_d.nexthop_idx
    phvwr           p.rewrite_metadata_meter_len, r7
    phvwr           p.rewrite_metadata_ip, d.session_info_d.tx_dst_ip
    bbeq            k.control_metadata_direction, TX_FROM_HOST, session_tx
    phvwr           p.rewrite_metadata_l4port, d.session_info_d.tx_dst_l4port
session_rx:
    add             r1, d.session_info_d.meter_idx, (METER_STATS_TABLE_SIZE >> 1)
    phvwr           p.rewrite_metadata_meter_idx, r1
    phvwr           p.rewrite_metadata_flags, d.session_info_d.rx_rewrite_flags
    phvwr           p.rewrite_metadata_policer_idx, d.session_info_d.rx_policer_idx
    seq             c1, d.session_info_d.rx_rewrite_flags[RX_REWRITE_DST_IP_BITS], \
                        RX_REWRITE_DST_IP_FROM_SERVICE
    phvwr.c1        p.nat_metadata_xlate_idx, k.p4e_i2e_service_xlate_idx
    seq             c1, d.session_info_d.rx_rewrite_flags[RX_REWRITE_DST_IP_BITS], \
                        RX_REWRITE_DST_IP_FROM_CA
    b               session_stats
    phvwr.c1        p.nat_metadata_xlate_idx, k.p4e_i2e_pa_or_ca_xlate_idx

session_tx:
    phvwr           p.rewrite_metadata_meter_idx, d.session_info_d.meter_idx
    phvwr           p.rewrite_metadata_flags, d.session_info_d.tx_rewrite_flags
    phvwr           p.rewrite_metadata_policer_idx, d.session_info_d.tx_policer_idx
    add             r1, r0, k.p4e_i2e_pa_or_ca_xlate_idx
    seq             c1, d.session_info_d.tx_rewrite_flags[TX_REWRITE_SRC_IP_BITS], \
                        TX_REWRITE_SRC_IP_FROM_SERVICE
    add.c1          r1, r0, k.p4e_i2e_service_xlate_idx
    seq             c1, d.session_info_d.tx_rewrite_flags[TX_REWRITE_SRC_IP_BITS], \
                        TX_REWRITE_SRC_IP_FROM_PUBLIC
    add.c1          r1, r0, k.p4e_i2e_public_xlate_idx
    phvwr           p.nat_metadata_xlate_idx, r1

session_stats:
    seq             c1, r5, r0
    nop.c1.e
    add             r5, r5, k.p4e_i2e_session_index, 5
    seq             c1, k.p4e_i2e_flow_role, TCP_FLOW_RESPONDER
    add.c1          r5, r5, 16
    addi            r1, r0, 0x1000001
    or              r7, r7, r1, 32
    addi            r6, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    bbeq            d.session_info_d.drop, TRUE, session_info_drop
    or              r7, r7, r5[32:27], 58
    add.e           r6, r6, r5[26:0]
    memwr.dx        r6, r7

session_info_drop:
    add             r6, r6, r5[26:0]
    memwr.dx        r6, r7
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_SESSION_HIT], 1
    phvwr.f         p.capri_intrinsic_drop, 1

session_info_error:
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_INVALID_SESSION], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
