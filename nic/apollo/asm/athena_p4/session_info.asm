#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_session_info_k.h"

struct session_info_k_          k;
struct session_info_d           d;
struct phv_                     p;

%%

session_info:
    seq             c1, d.session_info_d.valid_flag, r0
    b.c1            session_info_slow_path

    sra             r1, r4, 23
    tblwr.f         d.session_info_d.timestamp, r1
    
    sne             c1, d.session_info_d.conntrack_id, r0
    phvwr.c1        p.control_metadata_conntrack_index, d.session_info_d.conntrack_id
    phvwr.c1        p.control_metadata_conntrack_index_valid, TRUE


    seq             c1, k.control_metadata_direction, TX_FROM_HOST
    b.!c1            session_info_s2h
    phvwr           p.control_metadata_skip_flow_log, d.session_info_d.skip_flow_log

session_info_h2s:
    seq             c1, k.control_metadata_l2_vnic, TRUE
    b.c1            session_info_h2s_l2_vnic
    seq             c1, d.session_info_d.h2s_session_rewrite_id, r0
    b.c1            session_info_slow_path
    phvwr.!c1       p.control_metadata_session_rewrite_id, d.session_info_d.h2s_session_rewrite_id
    phvwr           p.control_metadata_session_rewrite_id_valid, TRUE

session_info_h2s_l2_vnic:
    
    and             r1, k.tcp_flags, d.session_info_d.h2s_slow_path_tcp_flags_match
    bne             r1, r0, session_info_slow_path

    sne             c1, d.session_info_d.smac, r0
    sne.c1          c1, k.ethernet_1_srcAddr, d.session_info_d.smac
    b.c1            session_info_slow_path
    
    sne             c1, d.session_info_d.h2s_epoch_vnic_id, r0
    phvwr.c1        p.control_metadata_epoch1_id, d.session_info_d.h2s_epoch_vnic_id
    phvwr.c1        p.control_metadata_epoch1_value, d.session_info_d.h2s_epoch_vnic_value
    phvwr.c1        p.control_metadata_epoch1_id_valid, TRUE

    sne             c1, d.session_info_d.h2s_epoch_mapping_id, r0
    phvwr.c1        p.control_metadata_epoch2_id, d.session_info_d.h2s_epoch_mapping_id
    phvwr.c1        p.control_metadata_epoch2_value, d.session_info_d.h2s_epoch_mapping_value
    phvwr.c1        p.control_metadata_epoch2_id_valid, TRUE

    sne             c1, d.session_info_d.h2s_throttle_bw1_id, r0
    phvwr.c1        p.control_metadata_throttle_bw1_id, d.session_info_d.h2s_throttle_bw1_id
    phvwr.c1        p.control_metadata_throttle_bw1_id_valid, TRUE
    
    sne             c1, d.session_info_d.h2s_throttle_bw2_id, r0
    phvwr.c1        p.control_metadata_throttle_bw2_id, d.session_info_d.h2s_throttle_bw2_id
    phvwr.c1        p.control_metadata_throttle_bw2_id_valid, TRUE

    sne             c1, d.session_info_d.h2s_vnic_statistics_id, r0
    phvwr.c1        p.control_metadata_vnic_statistics_id, d.session_info_d.h2s_vnic_statistics_id
    phvwr.c1        p.control_metadata_statistics_id_valid, TRUE
    phvwr.c1        p.control_metadata_vnic_statistics_mask, d.session_info_d.h2s_vnic_statistics_mask

    sne             c1, d.session_info_d.h2s_vnic_histogram_packet_len_id, r0
    phvwr.c1        p.control_metadata_histogram_packet_len_id, d.session_info_d.h2s_vnic_histogram_packet_len_id
    phvwr.c1        p.control_metadata_histogram_packet_len_id_valid, TRUE

    sne             c1, d.session_info_d.h2s_vnic_histogram_latency_id, r0
    phvwr.c1        p.control_metadata_histogram_latency_id, d.session_info_d.h2s_vnic_histogram_latency_id
    phvwr.c1        p.control_metadata_histogram_latency_id_valid, TRUE

    phvwr           p.control_metadata_allowed_flow_state_bitmap, d.session_info_d.h2s_allowed_flow_state_bitmap

    phvwr.e         p.control_metadata_redir_type, PACKET_ACTION_REDIR_UPLINK
    phvwr           p.control_metadata_redir_oport, TM_PORT_UPLINK_1

session_info_s2h:
    seq             c1, k.control_metadata_l2_vnic, TRUE
    b.c1            session_info_s2h_l2_vnic
    seq             c1, d.session_info_d.s2h_session_rewrite_id, r0
    b.c1            session_info_slow_path
    phvwr.!c1       p.control_metadata_session_rewrite_id, d.session_info_d.s2h_session_rewrite_id
    phvwr           p.control_metadata_session_rewrite_id_valid, TRUE
session_info_s2h_l2_vnic:
    
    and             r1, k.tcp_flags, d.session_info_d.s2h_slow_path_tcp_flags_match
    bne             r1, r0, session_info_slow_path

    sne             c1, d.session_info_d.s2h_epoch_vnic_id, r0
    phvwr.c1        p.control_metadata_epoch1_id, d.session_info_d.s2h_epoch_vnic_id
    phvwr.c1        p.control_metadata_epoch1_value, d.session_info_d.s2h_epoch_vnic_value
    phvwr.c1        p.control_metadata_epoch1_id_valid, TRUE

    sne             c1, d.session_info_d.s2h_epoch_mapping_id, r0
    phvwr.c1        p.control_metadata_epoch2_id, d.session_info_d.s2h_epoch_mapping_id
    phvwr.c1        p.control_metadata_epoch2_value, d.session_info_d.s2h_epoch_mapping_value
    phvwr.c1        p.control_metadata_epoch2_id_valid, TRUE

    sne             c1, d.session_info_d.s2h_throttle_bw1_id, r0
    phvwr.c1        p.control_metadata_throttle_bw1_id, d.session_info_d.s2h_throttle_bw1_id
    phvwr.c1        p.control_metadata_throttle_bw1_id_valid, TRUE
    
    sne             c1, d.session_info_d.s2h_throttle_bw2_id, r0
    phvwr.c1        p.control_metadata_throttle_bw2_id, d.session_info_d.s2h_throttle_bw2_id
    phvwr.c1        p.control_metadata_throttle_bw2_id_valid, TRUE

    sne             c1, d.session_info_d.s2h_vnic_statistics_id, r0
    phvwr.c1        p.control_metadata_vnic_statistics_id, d.session_info_d.s2h_vnic_statistics_id
    phvwr.c1        p.control_metadata_statistics_id_valid, TRUE
    phvwr.c1        p.control_metadata_vnic_statistics_mask, d.session_info_d.s2h_vnic_statistics_mask

    sne             c1, d.session_info_d.s2h_vnic_histogram_packet_len_id, r0
    phvwr.c1        p.control_metadata_histogram_packet_len_id, d.session_info_d.s2h_vnic_histogram_packet_len_id
    phvwr.c1        p.control_metadata_histogram_packet_len_id_valid, TRUE

    sne             c1, d.session_info_d.s2h_vnic_histogram_latency_id, r0
    phvwr.c1        p.control_metadata_histogram_latency_id, d.session_info_d.s2h_vnic_histogram_latency_id
    phvwr.c1        p.control_metadata_histogram_latency_id_valid, TRUE

    phvwr           p.control_metadata_allowed_flow_state_bitmap, d.session_info_d.s2h_allowed_flow_state_bitmap

    phvwr.e         p.control_metadata_redir_type, PACKET_ACTION_REDIR_UPLINK
    phvwr           p.control_metadata_redir_oport, TM_PORT_UPLINK_0

session_info_slow_path:
    phvwr.e         p.control_metadata_flow_miss, TRUE
    phvwr           p.control_metadata_redir_type, PACKET_ACTION_REDIR_RXDMA
    


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
