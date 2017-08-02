struct phv_ {
/* --------------- Phv Flit 7 ------------ */ 

    qos_metadata_cos : 8; // 3584
    qos_metadata_dscp_en : 8; // 3592
    qos_metadata_dscp : 8; // 3600
    l3_metadata_payload_length : 16; // 3608
    control_metadata_packet_len : 16; // 3624
    control_metadata_flow_miss_idx : 16; // 3640
    control_metadata_egress_mirror_session_id : 8; // 3656
    control_metadata_bypass : 8; // 3664
    control_metadata_ipsg_enable : 1; // 3672
    control_metadata_src_lif_index_align_pad_601_7 : 7; // 3673
    control_metadata_src_lif : 16; // 3680
    control_metadata_flow_miss_action : 2; // 3696
    _flit_3_b_pad__6 : 6; // 3698
    _flit_3_pad__392 : 392; // 3704
/* --------------- Phv Flit 6 ------------ */ 

    l4_metadata_tcp_non_syn_first_pkt_drop_action : 1; // 3072
    l4_metadata_tcp_xmas_tree_packet_drop_action : 1; // 3073
    l4_metadata_ipv4_ping_of_death_drop_action : 1; // 3074
    l4_metadata_ipv4_fragment_too_small_drop_action : 1; // 3075
    l4_metadata_ip_ttl_change_detect_en : 1; // 3076
    l4_metadata_tcp_data_len_byte_align_pad_5_3 : 3; // 3077
    l4_metadata_tcp_data_len : 16; // 3080
    l4_metadata_tcp_rcvr_win_sz : 32; // 3096
    l4_metadata_tcp_mss : 16; // 3128
    l4_metadata_tcp_timestamp_negotiated : 1; // 3144
    nat_metadata_nat_ip_byte_align_pad_73_7 : 7; // 3145
    nat_metadata_nat_ip : 128; // 3152
    nat_metadata_nat_l4_port : 16; // 3280
    nat_metadata_twice_nat_idx : 16; // 3296
    copp_metadata_policer_index : 8; // 3312
    ddos_metadata_ddos_service_policer_idx : 16; // 3320
    ddos_metadata_ddos_src_dst_policer_idx : 16; // 3336
    flow_lkp_metadata_ipv4_hlen : 4; // 3352
    flow_lkp_metadata_ipv4_flags : 3; // 3356
    flow_lkp_metadata_ipv4_frag_offset_byte_align_pad_287_1 : 1; // 3359
    flow_lkp_metadata_ipv4_frag_offset : 13; // 3360
    flow_lkp_metadata_ip_ttl_byte_align_pad_301_3 : 3; // 3373
    flow_lkp_metadata_ip_ttl : 8; // 3376
    flow_lkp_metadata_pkt_type : 8; // 3384
    flow_lkp_metadata_lkp_dir : 1; // 3392
    flow_info_metadata_flow_role : 1; // 3393
    flow_info_metadata_flow_index_index_align_pad_322_2 : 2; // 3394
    flow_info_metadata_flow_index : 20; // 3396
    flow_info_metadata_flow_state_index_index_align_pad_344_4 : 4; // 3416
    flow_info_metadata_flow_state_index : 20; // 3420
    tunnel_metadata_tunnel_originate : 8; // 3440
    policer_metadata_ingress_policer_index_index_align_pad_376_5 : 5; // 3448
    policer_metadata_ingress_policer_index : 11; // 3453
    policer_metadata_egress_policer_index : 16; // 3464
    policer_metadata_ingress_policer_color : 1; // 3480
    rewrite_metadata_rewrite_index_byte_align_pad_409_7 : 7; // 3481
    rewrite_metadata_rewrite_index : 16; // 3488
    rewrite_metadata_tunnel_rewrite_index : 16; // 3504
    rewrite_metadata_tunnel_vnid : 24; // 3520
    rewrite_metadata_entropy_hash : 16; // 3544
    rewrite_metadata_mac_sa_rewrite : 8; // 3560
    rewrite_metadata_mac_da_rewrite : 8; // 3568
    rewrite_metadata_ttl_dec : 8; // 3576
/* --------------- Phv Flit 5 ------------ */ 

    flow_lkp_metadata_lkp_dst : 128; // 2560
    l3_metadata_inner_ipv4_option_seen : 8; // 2688
    union { /* Header Union */
        // skip header inner_udp - Nothing in PHV
        // skip header icmp - Nothing in PHV
        struct {
            tcp_seqNo : 32; // 2696
            tcp_ackNo : 32; // 2728
            tcp_dataOffset : 4; // 2760
            tcp_res : 4; // 2764
            tcp_flags : 8; // 2768
            tcp_window : 16; // 2776
        };
    };
    flow_lkp_metadata_lkp_sport : 16; // 2792
    flow_lkp_metadata_lkp_dport : 16; // 2808
    flow_lkp_metadata_lkp_type : 4; // 2824
    flow_lkp_metadata_lkp_vrf_byte_align_pad_780_4 : 4; // 2828
    flow_lkp_metadata_lkp_vrf : 16; // 2832
    flow_lkp_metadata_lkp_proto : 8; // 2848
    tunnel_metadata_tunnel_terminate : 8; // 2856
    control_metadata_flow_miss : 8; // 2864
    flow_lkp_metadata_lkp_dstMacAddr : 48; // 2872
    control_metadata_drop_reason : 32; // 2920
    l3_metadata_ipv4_frag : 1; // 2952
    tcp_option_timestamp_optType_byte_align_pad_905_7 : 7; // 2953
    tcp_option_timestamp_optType : 8; // 2960
    tcp_option_timestamp_optLength : 8; // 2968
    tcp_option_timestamp_ts : 32; // 2976
    tcp_option_timestamp_prev_echo_ts : 32; // 3008
    normalization_metadata_ip_res_action : 2; // 3040
    normalization_metadata_ip_df_action : 2; // 3042
    normalization_metadata_ip_options_action : 2; // 3044
    normalization_metadata_ip_new_tos_action : 2; // 3046
    normalization_metadata_ip_payload_action : 2; // 3048
    normalization_metadata_tcp_echots_nonzero_ack_zero_action : 2; // 3050
    normalization_metadata_tcp_datalen_gt_mss_action : 2; // 3052
    normalization_metadata_tcp_datalen_gt_ws_action : 2; // 3054
    normalization_metadata_tcp_noneg_ts_present_action : 2; // 3056
    l4_metadata_profile_idx_byte_align_pad_1010_6 : 6; // 3058
    l4_metadata_profile_idx : 8; // 3064
/* --------------- Phv Flit 4 ------------ */ 

    union { /* Header Union */
        struct {
            inner_ipv6_version : 4; // 2048
            inner_ipv6_trafficClass : 8; // 2052
            inner_ipv6_flowLabel : 20; // 2060
            inner_ipv6_payloadLen : 16; // 2080
            inner_ipv6_nextHdr : 8; // 2096
            inner_ipv6_hopLimit : 8; // 2104
            inner_ipv6_srcAddr : 128; // 2112
            inner_ipv6_dstAddr : 128; // 2240
        };
        struct {
            inner_ipv4_version : 4; // 2048
            inner_ipv4_ihl : 4; // 2052
            inner_ipv4_diffserv : 8; // 2056
            inner_ipv4_totalLen : 16; // 2064
            inner_ipv4_flags : 3; // 2080
            inner_ipv4_fragOffset : 13; // 2083
            inner_ipv4_ttl : 8; // 2096
            inner_ipv4_protocol : 8; // 2104
            inner_ipv4_srcAddr : 32; // 2112
            inner_ipv4_dstAddr : 32; // 2144
        };
    };
    flow_lkp_metadata_lkp_src : 128; // 2368
    flit_2_phv_flit_pad_576_64 : 64; // 2496
/* --------------- Phv Flit 3 ------------ */ 

    udp_dstPort : 16; // 1536
    udp_len : 16; // 1552
    udp_checksum : 16; // 1568
    inner_ethernet_dstAddr : 48; // 1584
    inner_ethernet_srcAddr : 48; // 1632
    inner_ethernet_etherType : 16; // 1680
    _flit_1_pad__352 : 352; // 1696
/* --------------- Phv Flit 2 ------------ */ 

    ipv6_version : 4; // 1024
    ipv6_trafficClass : 8; // 1028
    ipv6_flowLabel : 20; // 1036
    union {
        ipv4_identification : 16; // 1056
        ipv6_payloadLen : 16; // 1056
    };
    union {
        ipv6_nextHdr : 8; // 1072
        ipv4_ttl : 8; // 1072
    };
    union {
        ipv4_protocol : 8; // 1080
        ipv6_hopLimit : 8; // 1080
    };
    union {
        ipv4_srcAddr : 32; // 1088
        ipv6_srcAddr : 128; // 1088
    };
    union {
        ipv6_dstAddr : 128; // 1216
        ipv4_dstAddr : 32; // 1216
    };
    ipv4_version : 4; // 1344
    ipv4_ihl : 4; // 1348
    ipv4_diffserv : 8; // 1352
    ipv4_totalLen : 16; // 1360
    ipv4_flags : 3; // 1376
    ipv4_fragOffset : 13; // 1379
    ipv4_hdrChecksum : 16; // 1392
    l3_metadata_ipv4_option_seen : 8; // 1408
    tunnel_metadata_tunnel_type : 8; // 1416
    union { /* Header Union */
        struct {
            vxlan_gpe_flags : 8; // 1424
            vxlan_gpe_reserved : 16; // 1432
            vxlan_gpe_next_proto : 8; // 1448
            vxlan_gpe_vni : 24; // 1456
            vxlan_gpe_reserved2 : 8; // 1480
        };
        // skip header genv - Nothing in PHV
        // skip header nvgre - Nothing in PHV
        struct {
            vxlan_vni : 24; // 1424
        };
    };
    tunnel_metadata_tunnel_vni : 24; // 1488
    udp_srcPort : 16; // 1512
    flit_1_phv_flit_pad_520_8 : 8; // 1528
/* --------------- Phv Flit 1 ------------ */ 

    recirc_header_reason : 2; // 512
    recirc_header_overflow_entry_index : 14; // 514
    recirc_header_overflow_hash : 32; // 528
    ethernet_dstAddr : 48; // 560
    ethernet_srcAddr : 48; // 608
    ethernet_etherType : 16; // 656
    vlan_tag_pcp : 3; // 672
    vlan_tag_dei : 1; // 675
    vlan_tag_vid : 12; // 676
    _flit_0_pad__336 : 336; // 688
/* --------------- Phv Flit 0 ------------ */ 

    capri_intrinsic_tm_iport : 4; // 0
    capri_intrinsic_tm_oport : 4; // 4
    capri_intrinsic_tm_iq : 5; // 8
    capri_intrinsic_lif : 11; // 13
    capri_intrinsic_timestamp : 48; // 24
    capri_intrinsic_tm_span_session : 8; // 72
    capri_intrinsic_error_bits : 4; // 80
    capri_intrinsic_tm_instance_type : 4; // 84
    capri_intrinsic_tm_replicate_ptr : 16; // 88
    capri_intrinsic_bypass : 1; // 104
    capri_intrinsic_tm_replicate_en : 1; // 105
    capri_intrinsic_tm_q_depth : 14; // 106
    capri_intrinsic_drop : 1; // 120
    capri_intrinsic_hw_error : 1; // 121
    capri_intrinsic_tm_cpu : 1; // 122
    capri_intrinsic_tm_oq : 5; // 123
    capri_intrinsic_debug_trace : 1; // 128
    capri_intrinsic__padding : 7; // 129
    capri_p4_intrinsic_no_data : 1; // 136
    capri_p4_intrinsic_recirc : 1; // 137
    capri_p4_intrinsic_frame_size : 14; // 138
    capri_p4_intrinsic_recirc_count : 3; // 152
    capri_p4_intrinsic__padding : 5; // 155
    normalization_metadata_icmp_request_response_action : 2; // 160
    normalization_metadata_icmp_bad_request_action : 2; // 162
    normalization_metadata_icmp_code_removal_action : 2; // 164
    normalization_metadata_icmp_redirect_action : 2; // 166
    normalization_metadata_tcp_res_action : 2; // 168
    normalization_metadata_tcp_nosyn_mss_action : 2; // 170
    normalization_metadata_tcp_nosyn_ws_action : 2; // 172
    normalization_metadata_tcp_urg_flag_noptr_action : 2; // 174
    normalization_metadata_tcp_nourg_flag_ptr_action : 2; // 176
    normalization_metadata_tcp_urg_flag_ptr_nopayload_action : 2; // 178
    normalization_metadata_tcp_rst_flag_datapresent_action : 2; // 180
    normalization_metadata_tcp_neg_ts_not_present_action : 2; // 182
    normalization_metadata_tcp_flags_combination : 2; // 184
    normalization_metadata_tcp_flags_nonsyn_noack_action : 2; // 186
    l4_metadata_flow_conn_track : 1; // 188
    l4_metadata_ip_normalization_en : 1; // 189
    l4_metadata_tcp_normalization_en : 1; // 190
    l4_metadata_icmp_normalization_en : 1; // 191
    l4_metadata_ipv4_fragment_drop_action : 1; // 192
    l4_metadata_tcp_split_handshake_detect_en : 1; // 193
    l4_metadata_tcp_split_handshake_action : 1; // 194
    l4_metadata_tcp_split_handshake_detected : 1; // 195
    l4_metadata_invalid_tcp_seq_num : 1; // 196
    l4_metadata_invalid_tcp_ack_num : 1; // 197
    flow_lkp_metadata_lkp_srcMacAddr : 48; // 198
    flow_lkp_metadata_ip_version : 4; // 246
    flow_info_metadata_flow_ttl : 8; // 250
    flow_info_metadata_flow_ttl_change_detected : 1; // 258
    qos_metadata_cos_en : 1; // 259
    control_metadata_qid : 24; // 260
    control_metadata_p4plus_app_id : 4; // 284
    pad_before_hv_224_ : 96; // 288
    flit_0_hv_0 : 1; // 384
    flit_0_hv_1 : 1; // 385
    flit_0_hv_2 : 1; // 386
    flit_0_hv_3 : 1; // 387
    flit_0_hv_4 : 1; // 388
    flit_0_hv_5 : 1; // 389
    flit_0_hv_6 : 1; // 390
    flit_0_hv_7 : 1; // 391
    flit_0_hv_8 : 1; // 392
    flit_0_hv_9 : 1; // 393
    flit_0_hv_10 : 1; // 394
    flit_0_hv_11 : 1; // 395
    flit_0_hv_12 : 1; // 396
    flit_0_hv_13 : 1; // 397
    flit_0_hv_14 : 1; // 398
    flit_0_hv_15 : 1; // 399
    flit_0_hv_16 : 1; // 400
    flit_0_hv_17 : 1; // 401
    flit_0_hv_18 : 1; // 402
    flit_0_hv_19 : 1; // 403
    flit_0_hv_20 : 1; // 404
    flit_0_hv_21 : 1; // 405
    flit_0_hv_22 : 1; // 406
    flit_0_hv_23 : 1; // 407
    flit_0_hv_24 : 1; // 408
    flit_0_hv_25 : 1; // 409
    flit_0_hv_26 : 1; // 410
    flit_0_hv_27 : 1; // 411
    flit_0_hv_28 : 1; // 412
    flit_0_hv_29 : 1; // 413
    flit_0_hv_30 : 1; // 414
    flit_0_hv_31 : 1; // 415
    flit_0_hv_32 : 1; // 416
    flit_0_hv_33 : 1; // 417
    flit_0_hv_34 : 1; // 418
    flit_0_hv_35 : 1; // 419
    flit_0_hv_36 : 1; // 420
    flit_0_hv_37 : 1; // 421
    flit_0_hv_38 : 1; // 422
    flit_0_hv_39 : 1; // 423
    flit_0_hv_40 : 1; // 424
    icmp_echo_valid : 1; // 425
    tcp_option_eol_valid : 1; // 426
    tcp_option_nop_valid : 1; // 427
    tcp_option_mss_valid : 1; // 428
    tcp_option_ws_valid : 1; // 429
    tcp_option_sack_perm_valid : 1; // 430
    tcp_option_one_sack_valid : 1; // 431
    tcp_option_three_sack_valid : 1; // 432
    tcp_option_two_sack_valid : 1; // 433
    tcp_option_four_sack_valid : 1; // 434
    tcp_option_timestamp_valid : 1; // 435
    tcp_valid : 1; // 436
    icmp_valid : 1; // 437
    inner_udp_valid : 1; // 438
    inner_ipv4_option_EOL_valid : 1; // 439
    inner_ipv4_option_NOP_valid : 1; // 440
    inner_ipv4_option_security_valid : 1; // 441
    inner_ipv4_option_lsr_valid : 1; // 442
    inner_ipv4_option_rr_valid : 1; // 443
    inner_ipv4_option_ssr_valid : 1; // 444
    inner_ipv4_option_timestamp_valid : 1; // 445
    inner_ipv4_valid : 1; // 446
    inner_ipv6_valid : 1; // 447
    flit_0_hv_64 : 1; // 448
    flit_0_hv_65 : 1; // 449
    flit_0_hv_66 : 1; // 450
    flit_0_hv_67 : 1; // 451
    flit_0_hv_68 : 1; // 452
    flit_0_hv_69 : 1; // 453
    inner_ethernet_valid : 1; // 454
    mpls_2_valid : 1; // 455
    mpls_1_valid : 1; // 456
    mpls_0_valid : 1; // 457
    roce_bth_valid : 1; // 458
    udp_valid : 1; // 459
    vxlan_valid : 1; // 460
    genv_valid : 1; // 461
    vxlan_gpe_valid : 1; // 462
    nvgre_valid : 1; // 463
    erspan_t3_header_valid : 1; // 464
    gre_valid : 1; // 465
    ah_valid : 1; // 466
    esp_valid : 1; // 467
    ipv4_option_EOL_valid : 1; // 468
    ipv4_option_NOP_valid : 1; // 469
    ipv4_option_security_valid : 1; // 470
    ipv4_option_lsr_valid : 1; // 471
    ipv4_option_rr_valid : 1; // 472
    ipv4_option_ssr_valid : 1; // 473
    ipv4_option_timestamp_valid : 1; // 474
    v6_generic_valid : 1; // 475
    v6_frag_valid : 1; // 476
    v6_ah_esp_valid : 1; // 477
    ipv6_valid : 1; // 478
    ipv4_valid : 1; // 479
    flit_0_hv_96 : 1; // 480
    flit_0_hv_97 : 1; // 481
    flit_0_hv_98 : 1; // 482
    flit_0_hv_99 : 1; // 483
    flit_0_hv_100 : 1; // 484
    flit_0_hv_101 : 1; // 485
    flit_0_hv_102 : 1; // 486
    flit_0_hv_103 : 1; // 487
    flit_0_hv_104 : 1; // 488
    flit_0_hv_105 : 1; // 489
    flit_0_hv_106 : 1; // 490
    flit_0_hv_107 : 1; // 491
    flit_0_hv_108 : 1; // 492
    flit_0_hv_109 : 1; // 493
    flit_0_hv_110 : 1; // 494
    flit_0_hv_111 : 1; // 495
    flit_0_hv_112 : 1; // 496
    flit_0_hv_113 : 1; // 497
    flit_0_hv_114 : 1; // 498
    flit_0_hv_115 : 1; // 499
    flit_0_hv_116 : 1; // 500
    flit_0_hv_117 : 1; // 501
    flit_0_hv_118 : 1; // 502
    flit_0_hv_119 : 1; // 503
    flit_0_hv_120 : 1; // 504
    vlan_tag_valid : 1; // 505
    snap_header_valid : 1; // 506
    llc_header_valid : 1; // 507
    ethernet_valid : 1; // 508
    capri_i2e_metadata_valid : 1; // 509
    recirc_header_valid : 1; // 510
    capri_intrinsic_valid : 1; // 511
};
