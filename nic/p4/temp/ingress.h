

/*
 * ingress.h
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains assembly level data structures for all Ingress processing
 * needed for MPU to read and act on action data and action input. 
 *
 * Every Ingress P4table after match hit can optionally provide
 *   1. Action Data (Parameters provided in P4 action functions)
 *   2. Action Input (Table Action routine using data extracted
 *                    into PHV either from header or result of
 *                    previous table action stored in PHV)
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

/* TBD: In HBM case actiondata need to be packed before and after Key Fields
 * For now all actiondata follows Key 
 */


/* ASM Key Structure for p4-table 'ingress_tx_stats' */
/* P4-table 'ingress_tx_stats' is index table */

/* K + I fields */
struct ingress_tx_stats_k {
    /* FieldType = K */
    control_metadata_src_lif : 16; /* phvbit[3680], Flit[7], FlitOffset[96] */
    /* FieldType = I */
    capri_intrinsic_drop : 1; /* phvbit[120], Flit[0], FlitOffset[120] */
    /* FieldType = P */
    __pad_17 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */
struct ingress_tx_stats_ingress_tx_stats_d {

    tx_ingress_drops : 16;
};

struct ingress_tx_stats_d {
    struct ingress_tx_stats_ingress_tx_stats_d  ingress_tx_stats_d;
    __pad_to_512b : 496;
};



/* ASM Key Structure for p4-table 'nacl' */
/* P4-table 'nacl' ternary table.*/

/* K + I fields */
struct nacl_k {
    /* FieldType = K */
    capri_intrinsic_lif_sbit3_ebit10 : 8; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = K */
    vlan_tag_vid_sbit4_ebit11 : 8; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = K */
    l3_metadata_ipv4_option_seen : 8; /* phvbit[1408], Flit[2], FlitOffset[384] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_24 : 24; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = K */
        vxlan_vni : 24; /* phvbit[1424], Flit[2], FlitOffset[400] */
    };
    /* FieldType = K */
    flow_lkp_metadata_lkp_src : 128; /* phvbit[2368], Flit[4], FlitOffset[320] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dst : 128; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = K */
    tcp_flags : 8; /* phvbit[2768], Flit[5], FlitOffset[208] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_sport : 16; /* phvbit[2792], Flit[5], FlitOffset[232] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dport : 16; /* phvbit[2808], Flit[5], FlitOffset[248] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    /* FieldType = K */
    tunnel_metadata_tunnel_terminate : 8; /* phvbit[2856], Flit[5], FlitOffset[296] */
    /* FieldType = K */
    control_metadata_flow_miss : 8; /* phvbit[2864], Flit[5], FlitOffset[304] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dstMacAddr : 48; /* phvbit[2872], Flit[5], FlitOffset[312] */
    /* FieldType = K */
    control_metadata_drop_reason : 32; /* phvbit[2920], Flit[5], FlitOffset[360] */
    /* FieldType = I */
    capri_intrinsic_lif_sbit0_ebit2 : 3; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[505], Flit[0], FlitOffset[505] */
    /* FieldType = I */
    vlan_tag_vid_sbit0_ebit3 : 4; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_type : 4; /* phvbit[2824], Flit[5], FlitOffset[264] */
    /* FieldType = I */
    l3_metadata_ipv4_frag : 1; /* phvbit[2952], Flit[5], FlitOffset[392] */
    /* FieldType = P */
    __pad_493 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 32;
};

/* K + D fields */
struct nacl_nacl_permit_d {

    force_flow_hit : 1;
    policer_index : 5;
    log_en : 1;
    ingress_mirror_session_id : 8;
    egress_mirror_session_id : 8;
    qid_en : 1;
    qid : 24;
};

struct nacl_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct nacl_nacl_permit_d  nacl_permit_d;
    } u;
    __pad_to_512b : 456;
};



/* ASM Key Structure for p4-table 'flow_hash' */
/* P4-table 'flow_hash' is hash table */

/* K + I fields */
struct flow_hash_k {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_src : 128; /* phvbit[2368], Flit[4], FlitOffset[320] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dst : 128; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_sport : 16; /* phvbit[2792], Flit[5], FlitOffset[232] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dport : 16; /* phvbit[2808], Flit[5], FlitOffset[248] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_type : 4; /* phvbit[2824], Flit[5], FlitOffset[264] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_dir : 1; /* phvbit[3392], Flit[6], FlitOffset[320] */
    /* FieldType = P */
    __pad_325 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    control_metadata_flow_miss : 8; /* phvbit[2864], Flit[5], FlitOffset[304] */
    __pad_to_512b : 176;
};

/* K + D fields */
struct flow_hash_flow_hash_info_d {
    /* FieldType = P */
    __pad_0 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_src : 128; /* phvbit[2368], Flit[4], FlitOffset[320] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dst : 128; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_sport : 16; /* phvbit[2792], Flit[5], FlitOffset[232] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dport : 16; /* phvbit[2808], Flit[5], FlitOffset[248] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_type : 4; /* phvbit[2824], Flit[5], FlitOffset[264] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_dir : 1; /* phvbit[3392], Flit[6], FlitOffset[320] */
    /* FieldType = P */
    __pad_325 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    control_metadata_flow_miss : 8; /* phvbit[2864], Flit[5], FlitOffset[304] */

    entry_valid : 1;
    export_en : 1;
    flow_index : 20;
    hash1 : 11;
    hint1 : 14;
    hash2 : 11;
    hint2 : 14;
    hash3 : 11;
    hint3 : 14;
    hash4 : 11;
    hint4 : 14;
    hash5 : 11;
    hint5 : 14;
    hash6 : 11;
    hint6 : 14;
};

struct flow_hash_d {
    struct flow_hash_flow_hash_info_d  flow_hash_info_d;
    __pad_to_512b : 4;
};



/* ASM Key Structure for p4-table 'l4_profile' */
/* P4-table 'l4_profile' is index table */

/* K + I fields */
struct l4_profile_k {
    /* FieldType = I */
    tcp_flags : 8; /* phvbit[2768], Flit[5], FlitOffset[208] */
    /* FieldType = I */
    tunnel_metadata_tunnel_terminate : 8; /* phvbit[2856], Flit[5], FlitOffset[296] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            normalization_metadata_ip_res_action : 2; /* phvbit[3040], Flit[5], FlitOffset[480] */
            /* K/I = I */
            normalization_metadata_ip_df_action : 2; /* phvbit[3042], Flit[5], FlitOffset[482] */
            /* K/I = I */
            normalization_metadata_ip_options_action : 2; /* phvbit[3044], Flit[5], FlitOffset[484] */
            /* K/I = I */
            normalization_metadata_ip_new_tos_action : 2; /* phvbit[3046], Flit[5], FlitOffset[486] */
        };
    };
    /* FieldType = I */
    normalization_metadata_ip_payload_action : 2; /* phvbit[3048], Flit[5], FlitOffset[488] */
    /* FieldType = P */
    __pad_26 : 14; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    l4_metadata_profile_idx : 8; /* phvbit[3064], Flit[5], FlitOffset[504] */
    /* FieldType = P */
    __pad_48 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            inner_ipv4_valid : 1; /* phvbit[446], Flit[0], FlitOffset[446] */
            /* K/I = I */
            inner_ipv6_valid : 1; /* phvbit[447], Flit[0], FlitOffset[447] */
        };
    };
    /* FieldType = P */
    __pad_56 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[505], Flit[0], FlitOffset[505] */
    /* FieldType = P */
    __pad_58 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ipv4_totalLen : 16; /* phvbit[1360], Flit[2], FlitOffset[336] */
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit0_ebit31 : 32; /* phvbit[2112], Flit[4], FlitOffset[64] */
        /* FieldType = I */
        inner_ipv4_srcAddr : 32; /* phvbit[2112], Flit[4], FlitOffset[64] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_srcAddr_sbit32_ebit63 : 32; /* phvbit[2112], Flit[4], FlitOffset[64] */
        /* FieldType = I */
        inner_ipv4_dstAddr : 32; /* phvbit[2144], Flit[4], FlitOffset[96] */
    };
    /* FieldType = I */
    inner_ipv6_srcAddr_sbit64_ebit127 : 64; /* phvbit[2112], Flit[4], FlitOffset[64] */
    /* FieldType = I */
    inner_ipv6_dstAddr : 128; /* phvbit[2240], Flit[4], FlitOffset[192] */
    /* FieldType = P */
    __pad_384 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            l4_metadata_tcp_xmas_tree_packet_drop_action : 1; /* phvbit[3073], Flit[6], FlitOffset[1] */
            /* K/I = I */
            l4_metadata_ipv4_ping_of_death_drop_action : 1; /* phvbit[3074], Flit[6], FlitOffset[2] */
            /* K/I = I */
            l4_metadata_ipv4_fragment_too_small_drop_action : 1; /* phvbit[3075], Flit[6], FlitOffset[3] */
        };
    };
    /* FieldType = P */
    __pad_388 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            flow_lkp_metadata_ipv4_hlen : 4; /* phvbit[3352], Flit[6], FlitOffset[280] */
            /* K/I = I */
            flow_lkp_metadata_ipv4_flags : 3; /* phvbit[3356], Flit[6], FlitOffset[284] */
        };
    };
    /* FieldType = P */
    __pad_399 : 1; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    flow_lkp_metadata_ipv4_frag_offset : 13; /* phvbit[3360], Flit[6], FlitOffset[288] */
    /* FieldType = P */
    __pad_413 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    qos_metadata_dscp : 8; /* phvbit[3600], Flit[7], FlitOffset[16] */
    /* FieldType = I */
    l3_metadata_payload_length : 16; /* phvbit[3608], Flit[7], FlitOffset[24] */
    /* FieldType = I */
    control_metadata_packet_len : 16; /* phvbit[3624], Flit[7], FlitOffset[40] */
    __pad_to_512b : 104;
};

/* K + D fields */
struct l4_profile_l4_profile_d {

    ip_normalization_en : 1;
    tcp_normalization_en : 1;
    icmp_normalization_en : 1;
    tcp_split_handshake_detect_en : 1;
    tcp_split_handshake_action : 1;
    ip_ttl_change_detect_en : 1;
    tcp_non_syn_first_pkt_drop_action : 1;
    tcp_xmas_tree_packet_drop_action : 1;
    ipv4_ping_of_death_drop_action : 1;
    ipv4_fragment_drop_action : 1;
    ipv4_fragment_too_small_drop_action : 1;
    ip_res_action : 2;
    ip_df_action : 2;
    ip_options_action : 2;
    ip_new_tos_action : 2;
    ip_payload_action : 2;
    icmp_bad_request_action : 2;
    icmp_request_response_action : 2;
    icmp_redirect_action : 2;
    icmp_code_removal_action : 2;
    tcp_res_action : 2;
    tcp_nosyn_mss_action : 2;
    tcp_nosyn_ws_action : 2;
    tcp_urg_flag_noptr_action : 2;
    tcp_nourg_flag_ptr_action : 2;
    tcp_urg_flag_ptr_nopayload_action : 2;
    tcp_echots_nonzero_ack_zero_action : 2;
    tcp_rst_flag_datapresent_action : 2;
    tcp_datalen_gt_mss_action : 2;
    tcp_datalen_gt_ws_action : 2;
    tcp_noneg_ts_present_action : 2;
    tcp_neg_ts_not_present_action : 2;
    tcp_flags_combination : 2;
    tcp_flags_nonsyn_noack_action : 2;
};

struct l4_profile_d {
    struct l4_profile_l4_profile_d  l4_profile_d;
    __pad_to_512b : 455;
};



/* ASM Key Structure for p4-table 'input_properties_mac_vlan' */
/* P4-table 'input_properties_mac_vlan' ternary table.*/

/* K + I fields */
struct input_properties_mac_vlan_k {
    /* FieldType = I */
    capri_intrinsic_tm_iport : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = P */
    __pad_4 : 9; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_intrinsic_lif_sbit0_ebit2 : 3; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = K */
    ethernet_srcAddr : 48; /* phvbit[608], Flit[1], FlitOffset[96] */
    /* FieldType = I */
    capri_intrinsic_lif_sbit3_ebit10 : 8; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = K */
    vlan_tag_vid_sbit4_ebit11 : 8; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = P */
    __pad_80 : 32; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[505], Flit[0], FlitOffset[505] */
    /* FieldType = I */
    vlan_tag_vid_sbit0_ebit3 : 4; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = P */
    __pad_120 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_p4_intrinsic_frame_size : 14; /* phvbit[138], Flit[0], FlitOffset[138] */
    __pad_to_512b : 379;
};

/* K + D fields */
struct input_properties_mac_vlan_input_properties_mac_vlan_d {

    src_lif : 11;
    src_lif_check_en : 1;
    vrf : 16;
    dir : 1;
    flow_miss_action : 2;
    flow_miss_idx : 16;
    ipsg_enable : 1;
    dscp : 8;
    l4_profile_idx : 8;
    dst_lif : 11;
};

struct input_properties_mac_vlan_d {
    struct input_properties_mac_vlan_input_properties_mac_vlan_d  input_properties_mac_vlan_d;
    __pad_to_512b : 437;
};



/* ASM Key Structure for p4-table 'ddos_src_dst' */
/* P4-table 'ddos_src_dst' ternary table.*/

/* K + I fields */
struct ddos_src_dst_k {
    /* FieldType = P */
    __pad_0 : 128; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_src : 128; /* phvbit[2368], Flit[4], FlitOffset[320] */
    /* FieldType = I */
    tcp_flags : 8; /* phvbit[2768], Flit[5], FlitOffset[208] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dst : 128; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dport : 16; /* phvbit[2808], Flit[5], FlitOffset[248] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    __pad_to_512b : 80;
};

/* K + D fields */
struct ddos_src_dst_ddos_src_dst_hit_d {

    ddos_src_dst_base_policer_idx : 11;
};

struct ddos_src_dst_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ddos_src_dst_ddos_src_dst_hit_d  ddos_src_dst_hit_d;
    } u;
    __pad_to_512b : 493;
};



/* ASM Key Structure for p4-table 'flow_hash_overflow' */
/* P4-table 'flow_hash_overflow' is index table */

/* K + I fields */
struct flow_hash_overflow_k {
    /* FieldType = P */
    __pad_0 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    recirc_header_overflow_entry_index : 14; /* phvbit[514], Flit[1], FlitOffset[2] */
    /* FieldType = I */
    recirc_header_overflow_hash : 32; /* phvbit[528], Flit[1], FlitOffset[16] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_src : 128; /* phvbit[2368], Flit[4], FlitOffset[320] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_dst : 128; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_sport : 16; /* phvbit[2792], Flit[5], FlitOffset[232] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_dport : 16; /* phvbit[2808], Flit[5], FlitOffset[248] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_type : 4; /* phvbit[2824], Flit[5], FlitOffset[264] */
    /* FieldType = P */
    __pad_404 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    /* FieldType = I */
    control_metadata_flow_miss : 8; /* phvbit[2864], Flit[5], FlitOffset[304] */
    __pad_to_512b : 136;
};

/* K + D fields */
struct flow_hash_overflow_flow_hash_overflow_d {

    lkp_type : 4;
    lkp_vrf : 16;
    lkp_src : 128;
    lkp_dst : 128;
    lkp_proto : 8;
    lkp_sport : 16;
    lkp_dport : 16;
    entry_valid : 1;
    export_en : 1;
    flow_index : 20;
    hash1 : 11;
    hint1 : 14;
    hash2 : 11;
    hint2 : 14;
    hash3 : 11;
    hint3 : 14;
    hash4 : 11;
    hint4 : 14;
    hash5 : 11;
    hint5 : 14;
    hash6 : 11;
    hint6 : 14;
};

struct flow_hash_overflow_d {
    struct flow_hash_overflow_flow_hash_overflow_d  flow_hash_overflow_d;
    __pad_to_512b : 24;
};



/* ASM Key Structure for p4-table 'flow_state' */
/* P4-table 'flow_state' is index table */

/* K + I fields */
struct flow_state_k {
    /* FieldType = I */
    l4_metadata_tcp_data_len : 16; /* phvbit[3080], Flit[6], FlitOffset[8] */
    /* FieldType = I */
    l4_metadata_tcp_rcvr_win_sz : 32; /* phvbit[3096], Flit[6], FlitOffset[24] */
    /* FieldType = I */
    l4_metadata_tcp_mss : 16; /* phvbit[3128], Flit[6], FlitOffset[56] */
    /* FieldType = I */
    l4_metadata_tcp_timestamp_negotiated : 1; /* phvbit[3144], Flit[6], FlitOffset[72] */
    /* FieldType = P */
    __pad_65 : 8; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    flow_info_metadata_flow_role : 1; /* phvbit[3393], Flit[6], FlitOffset[321] */
    /* FieldType = P */
    __pad_74 : 18; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    flow_info_metadata_flow_state_index : 20; /* phvbit[3420], Flit[6], FlitOffset[348] */
    /* FieldType = P */
    __pad_112 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            tcp_option_timestamp_valid : 1; /* phvbit[435], Flit[0], FlitOffset[435] */
            /* K/I = I */
            tcp_valid : 1; /* phvbit[436], Flit[0], FlitOffset[436] */
        };
    };
    /* FieldType = P */
    __pad_117 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    tcp_seqNo : 32; /* phvbit[2696], Flit[5], FlitOffset[136] */
    /* FieldType = I */
    tcp_ackNo : 32; /* phvbit[2728], Flit[5], FlitOffset[168] */
    /* FieldType = I */
    tcp_flags : 8; /* phvbit[2768], Flit[5], FlitOffset[208] */
    /* FieldType = I */
    tcp_window : 16; /* phvbit[2776], Flit[5], FlitOffset[216] */
    /* FieldType = I */
    tcp_option_timestamp_prev_echo_ts : 32; /* phvbit[3008], Flit[5], FlitOffset[448] */
    /* FieldType = P */
    __pad_240 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            normalization_metadata_tcp_echots_nonzero_ack_zero_action : 2; /* phvbit[3050], Flit[5], FlitOffset[490] */
            /* K/I = I */
            normalization_metadata_tcp_datalen_gt_mss_action : 2; /* phvbit[3052], Flit[5], FlitOffset[492] */
            /* K/I = I */
            normalization_metadata_tcp_datalen_gt_ws_action : 2; /* phvbit[3054], Flit[5], FlitOffset[494] */
        };
    };
    /* FieldType = I */
    normalization_metadata_tcp_noneg_ts_present_action : 2; /* phvbit[3056], Flit[5], FlitOffset[496] */
    /* FieldType = P */
    __pad_250 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 256;
};

/* K + D fields */
struct flow_state_flow_state_info_d {

    iflow_tcp_seq_num : 32;
    iflow_tcp_ack_num : 32;
    iflow_tcp_win_sz : 16;
    iflow_tcp_win_scale : 4;
    iflow_tcp_mss : 16;
    iflow_tcp_state : 4;
    iflow_exceptions_seen : 16;
    rflow_tcp_seq_num : 32;
    rflow_tcp_ack_num : 32;
    rflow_tcp_win_sz : 16;
    rflow_tcp_win_scale : 4;
    rflow_tcp_mss : 16;
    rflow_tcp_state : 4;
    syn_cookie_delta : 32;
    rflow_exceptions_seen : 16;
    flow_rtt_seq_check_enabled : 1;
    iflow_rtt_in_progress : 1;
    iflow_rtt : 34;
    iflow_rtt_seq_no : 32;
    iflow_rtt_timestamp : 48;
    rflow_rtt_in_progress : 1;
    rflow_rtt : 34;
    rflow_rtt_seq_no : 32;
    rflow_rtt_timestamp : 48;
};

struct flow_state_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct flow_state_flow_state_info_d  flow_state_info_d;
    } u;
    __pad_to_512b : 1;
};



/* ASM Key Structure for p4-table 'drop_stats' */
/* P4-table 'drop_stats' ternary table.*/

/* K + I fields */
struct drop_stats_k {
    /* FieldType = K */
    control_metadata_drop_reason : 32; /* phvbit[2920], Flit[5], FlitOffset[360] */
    __pad_to_512b : 480;
};

/* K + D fields */
struct drop_stats_drop_stats_d {

    stats_idx : 5;
    drop_pkts : 16;
    mirror_session_id : 8;
};

struct drop_stats_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct drop_stats_drop_stats_d  drop_stats_d;
    } u;
    __pad_to_512b : 475;
};



/* ASM Key Structure for p4-table 'flow_info' */
/* P4-table 'flow_info' is index table */

/* K + I fields */
struct flow_info_k {
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            l4_metadata_tcp_non_syn_first_pkt_drop_action : 1; /* phvbit[3072], Flit[6], FlitOffset[0] */
            /* K/I = I */
            l4_metadata_ip_ttl_change_detect_en : 1; /* phvbit[3076], Flit[6], FlitOffset[4] */
        };
    };
    /* FieldType = P */
    __pad_1 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    flow_lkp_metadata_ip_ttl : 8; /* phvbit[3376], Flit[6], FlitOffset[304] */
    /* FieldType = I */
    flow_lkp_metadata_pkt_type : 8; /* phvbit[3384], Flit[6], FlitOffset[312] */
    /* FieldType = P */
    __pad_24 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    flow_info_metadata_flow_index : 20; /* phvbit[3396], Flit[6], FlitOffset[324] */
    /* FieldType = I */
    tcp_flags : 8; /* phvbit[2768], Flit[5], FlitOffset[208] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    /* FieldType = I */
    control_metadata_flow_miss_idx : 16; /* phvbit[3640], Flit[7], FlitOffset[56] */
    /* FieldType = I */
    control_metadata_flow_miss_action : 2; /* phvbit[3696], Flit[7], FlitOffset[112] */
    /* FieldType = P */
    __pad_98 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 408;
};

/* K + D fields */
struct flow_info_flow_hit_from_vm_bounce_d {

    src_lif : 11;
};
struct flow_info_flow_hit_drop_d {

    flow_index : 20;
    start_timestamp : 32;
};
struct flow_info_flow_hit_to_vm_bounce_d {

    dst_lif : 11;
};
struct flow_info_flow_info_d {

    lif : 16;
    multicast_en : 1;
    p4plus_app_id : 4;
    ingress_policer_index : 11;
    egress_policer_index : 11;
    ingress_mirror_session_id : 8;
    egress_mirror_session_id : 8;
    rewrite_index : 12;
    tunnel_rewrite_index : 10;
    tunnel_vnid : 24;
    tunnel_originate : 1;
    nat_ip : 128;
    nat_l4_port : 16;
    twice_nat_idx : 13;
    cos_en : 1;
    cos : 3;
    dscp_en : 1;
    dscp : 8;
    qid_en : 1;
    log_en : 1;
    mac_sa_rewrite : 1;
    mac_da_rewrite : 1;
    ttl_dec : 1;
    flow_conn_track : 1;
    flow_ts_negotiated : 1;
    flow_ttl : 8;
    flow_role : 1;
    flow_index : 20;
    flow_state_index : 20;
    start_timestamp : 32;
};

struct flow_info_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct flow_info_flow_hit_from_vm_bounce_d  flow_hit_from_vm_bounce_d;
        struct flow_info_flow_hit_drop_d  flow_hit_drop_d;
        struct flow_info_flow_hit_to_vm_bounce_d  flow_hit_to_vm_bounce_d;
        struct flow_info_flow_info_d  flow_info_d;
    } u;
    __pad_to_512b : 140;
};



/* ASM Key Structure for p4-table 'input_properties' */
/* P4-table 'input_properties' is hash table with over flow tcam */

/* K + I fields */
struct input_properties_k {
    /* FieldType = I */
    capri_intrinsic_tm_iport : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = P */
    __pad_4 : 9; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    capri_intrinsic_lif_sbit0_ebit2 : 3; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = P */
    __pad_16 : 48; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    capri_intrinsic_lif_sbit3_ebit10 : 8; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = K */
    vlan_tag_vid_sbit4_ebit11 : 8; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = K */
    tunnel_metadata_tunnel_type : 8; /* phvbit[1416], Flit[2], FlitOffset[392] */
    /* FieldType = K */
    tunnel_metadata_tunnel_vni : 24; /* phvbit[1488], Flit[2], FlitOffset[464] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[505], Flit[0], FlitOffset[505] */
    /* FieldType = I */
    vlan_tag_vid_sbit0_ebit3 : 4; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = P */
    __pad_120 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_p4_intrinsic_frame_size : 14; /* phvbit[138], Flit[0], FlitOffset[138] */
    __pad_to_512b : 379;
};

/* K + D fields */
struct input_properties_input_properties_d {

    vrf : 16;
    dir : 1;
    flow_miss_action : 2;
    flow_miss_idx : 16;
    ipsg_enable : 1;
    dscp : 8;
    l4_profile_idx : 8;
    dst_lif : 11;
};

struct input_properties_d {
    struct input_properties_input_properties_d  input_properties_d;
    __pad_to_512b : 449;
};



/* ASM Key Structure for p4-table 'ingress_policer' */
/* P4-table 'ingress_policer' is index table */

/* K + I fields */
struct ingress_policer_k {
    /* FieldType = P */
    __pad_0 : 5; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    policer_metadata_ingress_policer_index : 11; /* phvbit[3453], Flit[6], FlitOffset[381] */
    /* FieldType = I */
    policer_metadata_ingress_policer_color : 1; /* phvbit[3480], Flit[6], FlitOffset[408] */
    /* FieldType = P */
    __pad_17 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 488;
};

/* K + D fields */




/* ASM Key Structure for p4-table 'input_mapping_tunneled' */
/* P4-table 'input_mapping_tunneled' ternary table.*/

/* K + I fields */
struct input_mapping_tunneled_k {
    /* FieldType = P */
    __pad_0 : 80; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = K */
        ipv6_dstAddr_sbit0_ebit31 : 32; /* phvbit[1216], Flit[2], FlitOffset[192] */
        /* FieldType = K */
        ipv4_dstAddr : 32; /* phvbit[1216], Flit[2], FlitOffset[192] */
    };
    /* FieldType = K */
    ipv6_dstAddr_sbit32_ebit127 : 96; /* phvbit[1216], Flit[2], FlitOffset[192] */
    /* FieldType = K */
    tunnel_metadata_tunnel_type : 8; /* phvbit[1416], Flit[2], FlitOffset[392] */
    /* FieldType = I */
    inner_ipv4_valid : 1; /* phvbit[446], Flit[0], FlitOffset[446] */
    /* FieldType = I */
    inner_ipv6_valid : 1; /* phvbit[447], Flit[0], FlitOffset[447] */
    /* FieldType = I */
    mpls_0_valid : 1; /* phvbit[457], Flit[0], FlitOffset[457] */
    /* FieldType = I */
    ipv6_valid : 1; /* phvbit[478], Flit[0], FlitOffset[478] */
    /* FieldType = I */
    ipv4_valid : 1; /* phvbit[479], Flit[0], FlitOffset[479] */
    /* FieldType = P */
    __pad_221 : 35; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    vxlan_gpe_flags : 8; /* phvbit[1424], Flit[2], FlitOffset[400] */
    /* FieldType = I */
    inner_ethernet_dstAddr : 48; /* phvbit[1584], Flit[3], FlitOffset[48] */
    /* FieldType = I */
    inner_ethernet_srcAddr : 48; /* phvbit[1632], Flit[3], FlitOffset[96] */
    /* FieldType = I */
    inner_ethernet_etherType : 16; /* phvbit[1680], Flit[3], FlitOffset[144] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            inner_ipv6_version : 4; /* phvbit[2048], Flit[4], FlitOffset[0] */
            /* Padded to align with unionized p4field */
            _pad_inner_ipv6_version : 4;
        };
        struct {
            /* K/I = I */
            inner_ipv4_version : 4; /* phvbit[2048], Flit[4], FlitOffset[0] */
            /* K/I = I */
            inner_ipv4_ihl : 4; /* phvbit[2052], Flit[4], FlitOffset[4] */
        };
    };
    /* FieldType = P */
    __pad_380 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    union { /* Sourced from field/hdr union */
        /* FieldType = P */
        __pad_384 : 16; /* phvbit[0], Flit[0], FlitOffset[0] */
        /* FieldType = I */
        inner_ipv4_totalLen : 16; /* phvbit[2064], Flit[4], FlitOffset[16] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_payloadLen_sbit0_ebit7 : 8; /* phvbit[2080], Flit[4], FlitOffset[32] */
        struct {
            /* K/I = I */
            inner_ipv4_flags : 3; /* phvbit[2080], Flit[4], FlitOffset[32] */
            /* K/I = I */
            inner_ipv4_fragOffset_sbit0_ebit4 : 5; /* phvbit[2083], Flit[4], FlitOffset[35] */
        };
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_payloadLen_sbit8_ebit15 : 8; /* phvbit[2080], Flit[4], FlitOffset[32] */
        /* FieldType = I */
        inner_ipv4_fragOffset_sbit5_ebit12 : 8; /* phvbit[2083], Flit[4], FlitOffset[35] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_nextHdr : 8; /* phvbit[2096], Flit[4], FlitOffset[48] */
        /* FieldType = I */
        inner_ipv4_ttl : 8; /* phvbit[2096], Flit[4], FlitOffset[48] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        inner_ipv6_hopLimit : 8; /* phvbit[2104], Flit[4], FlitOffset[56] */
        /* FieldType = I */
        inner_ipv4_protocol : 8; /* phvbit[2104], Flit[4], FlitOffset[56] */
    };
    /* FieldType = I */
    l3_metadata_inner_ipv4_option_seen : 8; /* phvbit[2688], Flit[5], FlitOffset[128] */
    /* FieldType = I */
    tcp_dataOffset : 4; /* phvbit[2760], Flit[5], FlitOffset[200] */
    /* FieldType = P */
    __pad_444 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 60;
};

/* K + D fields */

struct input_mapping_tunneled_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    __pad_to_512b : 504;
};



/* ASM Key Structure for p4-table 'input_mapping_native' */
/* P4-table 'input_mapping_native' ternary table.*/

/* K + I fields */
struct input_mapping_native_k {
    /* FieldType = I */
    ipv6_version : 4; /* phvbit[1024], Flit[2], FlitOffset[0] */
    /* FieldType = P */
    __pad_4 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ipv6_payloadLen : 16; /* phvbit[1056], Flit[2], FlitOffset[32] */
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_nextHdr : 8; /* phvbit[1072], Flit[2], FlitOffset[48] */
        /* FieldType = I */
        ipv4_ttl : 8; /* phvbit[1072], Flit[2], FlitOffset[48] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_hopLimit : 8; /* phvbit[1080], Flit[2], FlitOffset[56] */
        /* FieldType = I */
        ipv4_protocol : 8; /* phvbit[1080], Flit[2], FlitOffset[56] */
    };
    union { /* Sourced from field/hdr union */
        /* FieldType = I */
        ipv6_srcAddr_sbit0_ebit31 : 32; /* phvbit[1088], Flit[2], FlitOffset[64] */
        /* FieldType = I */
        ipv4_srcAddr : 32; /* phvbit[1088], Flit[2], FlitOffset[64] */
    };
    /* FieldType = I */
    ipv6_srcAddr_sbit32_ebit39 : 8; /* phvbit[1088], Flit[2], FlitOffset[64] */
    union { /* Sourced from field/hdr union */
        /* FieldType = K */
        ipv6_dstAddr_sbit0_ebit31 : 32; /* phvbit[1216], Flit[2], FlitOffset[192] */
        /* FieldType = K */
        ipv4_dstAddr : 32; /* phvbit[1216], Flit[2], FlitOffset[192] */
    };
    /* FieldType = K */
    ipv6_dstAddr_sbit32_ebit127 : 96; /* phvbit[1216], Flit[2], FlitOffset[192] */
    /* FieldType = K */
    tunnel_metadata_tunnel_type : 8; /* phvbit[1416], Flit[2], FlitOffset[392] */
    /* FieldType = I */
    inner_ipv4_valid : 1; /* phvbit[446], Flit[0], FlitOffset[446] */
    /* FieldType = I */
    inner_ipv6_valid : 1; /* phvbit[447], Flit[0], FlitOffset[447] */
    /* FieldType = I */
    mpls_0_valid : 1; /* phvbit[457], Flit[0], FlitOffset[457] */
    /* FieldType = I */
    ipv6_valid : 1; /* phvbit[478], Flit[0], FlitOffset[478] */
    /* FieldType = I */
    ipv4_valid : 1; /* phvbit[479], Flit[0], FlitOffset[479] */
    /* FieldType = P */
    __pad_221 : 3; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ipv4_totalLen : 16; /* phvbit[1360], Flit[2], FlitOffset[336] */
    union { /* Sourced from field/hdr union */
        struct {
            /* K/I = I */
            ipv4_flags : 3; /* phvbit[1376], Flit[2], FlitOffset[352] */
            /* K/I = I */
            ipv4_fragOffset_sbit0_ebit4 : 5; /* phvbit[1379], Flit[2], FlitOffset[355] */
        };
    };
    /* FieldType = I */
    ipv4_fragOffset_sbit5_ebit12 : 8; /* phvbit[1379], Flit[2], FlitOffset[355] */
    /* FieldType = I */
    ipv4_version : 4; /* phvbit[1344], Flit[2], FlitOffset[320] */
    /* FieldType = I */
    ipv4_ihl : 4; /* phvbit[1348], Flit[2], FlitOffset[324] */
    /* FieldType = I */
    ipv6_srcAddr_sbit40_ebit127 : 88; /* phvbit[1088], Flit[2], FlitOffset[64] */
    /* FieldType = I */
    ethernet_dstAddr : 48; /* phvbit[560], Flit[1], FlitOffset[48] */
    /* FieldType = I */
    ethernet_srcAddr : 48; /* phvbit[608], Flit[1], FlitOffset[96] */
    /* FieldType = I */
    ethernet_etherType : 16; /* phvbit[656], Flit[1], FlitOffset[144] */
    /* FieldType = I */
    udp_srcPort : 16; /* phvbit[1512], Flit[2], FlitOffset[488] */
    /* FieldType = I */
    udp_dstPort : 16; /* phvbit[1536], Flit[3], FlitOffset[0] */
    /* FieldType = I */
    tcp_dataOffset : 4; /* phvbit[2760], Flit[5], FlitOffset[200] */
    /* FieldType = P */
    __pad_500 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    __pad_to_512b : 8;
};

/* K + D fields */

struct input_mapping_native_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    __pad_to_512b : 504;
};



/* ASM Key Structure for p4-table 'ipsg' */
/* P4-table 'ipsg' ternary table.*/

/* K + I fields */
struct ipsg_k {
    /* FieldType = K */
    flow_lkp_metadata_lkp_src : 128; /* phvbit[2368], Flit[4], FlitOffset[320] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = I */
    flow_lkp_metadata_lkp_type : 4; /* phvbit[2824], Flit[5], FlitOffset[264] */
    /* FieldType = P */
    __pad_148 : 5; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[505], Flit[0], FlitOffset[505] */
    /* FieldType = P */
    __pad_154 : 6; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    ethernet_srcAddr : 48; /* phvbit[608], Flit[1], FlitOffset[96] */
    /* FieldType = P */
    __pad_208 : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    vlan_tag_vid : 12; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = I */
    control_metadata_ipsg_enable : 1; /* phvbit[3672], Flit[7], FlitOffset[88] */
    /* FieldType = P */
    __pad_225 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    control_metadata_src_lif : 16; /* phvbit[3680], Flit[7], FlitOffset[96] */
    __pad_to_512b : 264;
};

/* K + D fields */
struct ipsg_ipsg_hit_d {

    lif : 11;
    mac : 48;
    vlan_valid : 1;
    vlan_id : 12;
};

struct ipsg_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ipsg_ipsg_hit_d  ipsg_hit_d;
    } u;
    __pad_to_512b : 432;
};



/* ASM Key Structure for p4-table 'flow_stats' */
/* P4-table 'flow_stats' is index table */

/* K + I fields */
struct flow_stats_k {
    /* FieldType = P */
    __pad_0 : 12; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    flow_info_metadata_flow_index : 20; /* phvbit[3396], Flit[6], FlitOffset[324] */
    /* FieldType = I */
    capri_intrinsic_drop : 1; /* phvbit[120], Flit[0], FlitOffset[120] */
    /* FieldType = P */
    __pad_33 : 7; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    control_metadata_drop_reason : 32; /* phvbit[2920], Flit[5], FlitOffset[360] */
    /* FieldType = I */
    control_metadata_packet_len : 16; /* phvbit[3624], Flit[7], FlitOffset[40] */
    __pad_to_512b : 424;
};

/* K + D fields */
struct flow_stats_flow_stats_d {

    last_seen_timestamp : 32;
    permit_packets : 4;
    permit_bytes : 18;
    drop_packets : 4;
    drop_bytes : 18;
    drop_reason : 32;
    drop_count_map : 8;
    drop_count1 : 8;
    drop_count2 : 8;
    drop_count3 : 8;
    drop_count4 : 8;
    drop_count5 : 8;
    drop_count6 : 8;
    drop_count7 : 8;
    drop_count8 : 8;
    flow_agg_index1 : 16;
    flow_agg_index2 : 16;
    burst_start_timestamp : 48;
    burst_max_timestamp : 48;
    micro_burst_cycles : 32;
    allowed_bytes : 40;
    max_allowed_bytes : 40;
    burst_exceed_bytes : 40;
    burst_exceed_count : 32;
};

struct flow_stats_d {
    struct flow_stats_flow_stats_d  flow_stats_d;
    __pad_to_512b : 20;
};



/* ASM Key Structure for p4-table 'ingress_policer_action' */
/* P4-table 'ingress_policer_action' is index table */

/* K + I fields */
struct ingress_policer_action_k {
    /* FieldType = P */
    __pad_0 : 5; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    policer_metadata_ingress_policer_index : 11; /* phvbit[3453], Flit[6], FlitOffset[381] */
    /* FieldType = I */
    control_metadata_drop_reason : 32; /* phvbit[2920], Flit[5], FlitOffset[360] */
    /* FieldType = I */
    control_metadata_packet_len : 16; /* phvbit[3624], Flit[7], FlitOffset[40] */
    __pad_to_512b : 448;
};

/* K + D fields */
struct ingress_policer_action_ingress_policer_action_d {

    cos_en : 1;
    cos : 3;
    dscp_en : 1;
    dscp : 8;
    permitted_packets : 4;
    permitted_bytes : 18;
    denied_packets : 4;
    denied_bytes : 18;
};

struct ingress_policer_action_d {
    struct ingress_policer_action_ingress_policer_action_d  ingress_policer_action_d;
    __pad_to_512b : 455;
};



/* ASM Key Structure for p4-table 'ddos_service' */
/* P4-table 'ddos_service' ternary table.*/

/* K + I fields */
struct ddos_service_k {
    /* FieldType = I */
    tcp_flags : 8; /* phvbit[2768], Flit[5], FlitOffset[208] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dst : 128; /* phvbit[2560], Flit[5], FlitOffset[0] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_dport : 16; /* phvbit[2808], Flit[5], FlitOffset[248] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_vrf : 16; /* phvbit[2832], Flit[5], FlitOffset[272] */
    /* FieldType = K */
    flow_lkp_metadata_lkp_proto : 8; /* phvbit[2848], Flit[5], FlitOffset[288] */
    __pad_to_512b : 336;
};

/* K + D fields */
struct ddos_service_ddos_service_hit_d {

    ddos_service_base_policer_idx : 10;
};

struct ddos_service_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
                   /* Answer depends on understanding bit layout/endianess.
                    * For now assuming action-pc in bitlayout comes before 
                    * action-data 
                    */
    union {
        struct ddos_service_ddos_service_hit_d  ddos_service_hit_d;
    } u;
    __pad_to_512b : 494;
};



/* ASM Key Structure for p4-table 'input_properties_otcam' */
/* P4-table 'input_properties_otcam' ternary table.*/

/* K + I fields */
struct input_properties_otcam_k {
    /* FieldType = I */
    capri_intrinsic_tm_iport : 4; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = P */
    __pad_4 : 9; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    capri_intrinsic_lif_sbit0_ebit2 : 3; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = P */
    __pad_16 : 48; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = K */
    capri_intrinsic_lif_sbit3_ebit10 : 8; /* phvbit[13], Flit[0], FlitOffset[13] */
    /* FieldType = K */
    vlan_tag_vid_sbit4_ebit11 : 8; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = K */
    tunnel_metadata_tunnel_type : 8; /* phvbit[1416], Flit[2], FlitOffset[392] */
    /* FieldType = K */
    tunnel_metadata_tunnel_vni : 24; /* phvbit[1488], Flit[2], FlitOffset[464] */
    /* FieldType = I */
    vlan_tag_valid : 1; /* phvbit[505], Flit[0], FlitOffset[505] */
    /* FieldType = I */
    vlan_tag_vid_sbit0_ebit3 : 4; /* phvbit[676], Flit[1], FlitOffset[164] */
    /* FieldType = P */
    __pad_120 : 2; /* phvbit[0], Flit[0], FlitOffset[0] */
    /* FieldType = I */
    capri_p4_intrinsic_frame_size : 14; /* phvbit[138], Flit[0], FlitOffset[138] */
    __pad_to_512b : 379;
};

/* K + D fields */
struct input_properties_otcam_input_properties_otcam_d {

    vrf : 16;
    dir : 1;
    flow_miss_action : 2;
    flow_miss_idx : 16;
    ipsg_enable : 1;
    dscp : 8;
    l4_profile_idx : 8;
    dst_lif : 11;
};

struct input_properties_otcam_d {
    struct input_properties_otcam_input_properties_otcam_d  input_properties_otcam_d;
    __pad_to_512b : 449;
};




