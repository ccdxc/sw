#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_session_k.h"

struct session_k_   k;
struct session_d    d;
struct phv_         p;

%%

session_info:
    seq             c1, k.p4e_i2e_copp_policer_id, r0
    phvwr.!c1       p.control_metadata_copp_policer_valid, TRUE
    seq             c1, k.p4e_i2e_session_id, r0
    // r7 : packet length
    sub             r7, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l2_1
    phvwr           p.capri_p4_intrinsic_packet_len, r7
    bcf             [c1],session_info_error
    phvwrpair       p.control_metadata_update_checksum, \
                        k.p4e_i2e_update_checksum, \
                        p.control_metadata_rx_packet, \
                        k.p4e_i2e_rx_packet

    // update timestamp (flush/unlock if packet is not TCP or dropped)
    seq             c1, k.tcp_valid, 0
    seq.!c1         c1, d.session_info_d.drop, 1
    tblwr.c1.f      d.session_info_d.timestamp, r4
    tblwr.!c1       d.session_info_d.timestamp, r4

    // recirc packet, skip TCP session management
    bbeq            k.egress_recirc_valid, TRUE, session_info_common
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
    bbeq            k.p4e_i2e_rx_packet, FALSE, session_tx
    nop
session_rx:
    phvwr           p.rewrite_metadata_flags, d.session_info_d.rx_rewrite_flags
    phvwr           p.rewrite_metadata_policer_id, d.session_info_d.rx_policer_id
    seq             c1, d.session_info_d.rx_xlate_id, r0
    cmov            r1, c1, k.p4e_i2e_xlate_id, d.session_info_d.rx_xlate_id
    sne             c1, r1, r0
    phvwr.c1        p.control_metadata_apply_nat, TRUE
    phvwr.c1        p.rewrite_metadata_xlate_id, r1
    sne             c1, d.session_info_d.rx_xlate_id2, r0
    phvwr.c1        p.control_metadata_apply_nat2, TRUE
    b               session_stats
    phvwr.c1        p.rewrite_metadata_xlate_id2, d.session_info_d.tx_xlate_id2

session_tx:
    phvwr           p.rewrite_metadata_flags, d.session_info_d.tx_rewrite_flags
    phvwr           p.rewrite_metadata_policer_id, d.session_info_d.tx_policer_id
    seq             c1, d.session_info_d.tx_xlate_id, r0
    cmov            r1, c1, k.p4e_i2e_xlate_id, d.session_info_d.tx_xlate_id
    sne             c1, r1, r0
    phvwr.c1        p.control_metadata_apply_nat, TRUE
    phvwr.c1        p.rewrite_metadata_xlate_id, r1
    sne             c1, d.session_info_d.tx_xlate_id2, r0
    phvwr.c1        p.control_metadata_apply_nat2, TRUE
    phvwr.c1        p.rewrite_metadata_xlate_id2, d.session_info_d.tx_xlate_id2

session_stats:
    seq             c1, k.egress_recirc_valid, TRUE
    seq.!c1         c1, r5, r0
    nop.c1.e
    add             r5, r5, k.p4e_i2e_session_id, 5
    seq             c1, k.p4e_i2e_flow_role, TCP_FLOW_RESPONDER
    add.c1          r5, r5, 16
    addi            r1, r0, 0x1000001
    or              r7, r7, r1, 32
    addi            r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
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
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_SESSION_INVALID], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
