struct phv_ {
/* --------------- Phv Flit 7 ------------ */ 

    _flit_3_pad__712_0 : 512; // 3456, Crossed Flits(from flit 6 to 7). Split into two fields
/* --------------- Phv Flit 6 ------------ */ 

    nat_metadata_twice_nat_ip : 128; // 3072
    nat_metadata_twice_nat_l4_port : 16; // 3200
    nat_metadata_update_checksum : 1; // 3216
    copp_metadata_policer_color : 2; // 3217
    ddos_metadata_ddos_src_vf_policer_idx_index_align_pad_147_3 : 3; // 3219
    ddos_metadata_ddos_src_vf_policer_idx : 10; // 3222
    ddos_metadata_ddos_src_vf_policer_color : 2; // 3232
    ddos_metadata_ddos_service_policer_color : 2; // 3234
    ddos_metadata_ddos_src_dst_policer_color : 2; // 3236
    tunnel_metadata_inner_ip_proto_byte_align_pad_166_2 : 2; // 3238
    tunnel_metadata_inner_ip_proto : 8; // 3240
    policer_metadata_egress_policer_color : 1; // 3248
    _flit_3_b_pad__7 : 7; // 3249
    _flit_3_pad__712_1 : 200; // 3256  Crossed Flits (from flit 6 to 7). Split into two fields
/* --------------- Phv Flit 5 ------------ */ 

    union { /* Header Union */
        struct {
            inner_ipv6_srcAddr : 128; // 2560
            inner_ipv6_dstAddr : 128; // 2688
        };
        struct {
            inner_ipv4_ttl : 8; // 2560
            inner_ipv4_protocol : 8; // 2568
            inner_ipv4_hdrChecksum : 16; // 2576
            inner_ipv4_srcAddr : 32; // 2592
            inner_ipv4_dstAddr : 32; // 2624
            _padinner_ipv4 : 160; // 
        };
    };
    union { /* Header Union */
        struct {
            inner_udp_srcPort : 16; // 2816
            inner_udp_dstPort : 16; // 2832
            inner_udp_len : 16; // 2848
            inner_udp_checksum : 16; // 2864
        };
        // skip header icmp - Nothing in PHV
        struct {
            tcp_srcPort : 16; // 2816
            tcp_dstPort : 16; // 2832
            tcp_flags : 8; // 2848
            _padtcp : 24; // 
        };
    };
    _flit_2_pad__192 : 192; // 2880
/* --------------- Phv Flit 4 ------------ */ 

    union { /* Header Union */
        struct {
            vxlan_gpe_flags : 8; // 2048
            vxlan_gpe_reserved : 16; // 2056
            vxlan_gpe_next_proto : 8; // 2072
            vxlan_gpe_vni : 24; // 2080
            vxlan_gpe_reserved2 : 8; // 2104
        };
        struct {
            genv_ver : 2; // 2048
            genv_optLen : 6; // 2050
            genv_oam : 1; // 2056
            genv_critical : 1; // 2057
            genv_reserved : 6; // 2058
            genv_protoType : 16; // 2064
            genv_vni : 24; // 2080
            genv_reserved2 : 8; // 2104
        };
        struct {
            nvgre_tni : 24; // 2048
            nvgre_flow_id : 8; // 2072
            _padnvgre : 32; // 
        };
        struct {
            vxlan_flags : 8; // 2048
            vxlan_reserved : 24; // 2056
            vxlan_vni : 24; // 2080
            vxlan_reserved2 : 8; // 2104
        };
    };
    udp_srcPort : 16; // 2112
    udp_dstPort : 16; // 2128
    udp_len : 16; // 2144
    udp_checksum : 16; // 2160
    roce_bth_opCode : 8; // 2176
    roce_bth_destQP : 24; // 2184
    mpls_0_label : 20; // 2208
    mpls_0_exp : 3; // 2228
    mpls_0_bos : 1; // 2231
    mpls_0_ttl : 8; // 2232
    mpls_1_label : 20; // 2240
    mpls_1_exp : 3; // 2260
    mpls_1_bos : 1; // 2263
    mpls_1_ttl : 8; // 2264
    mpls_2_label : 20; // 2272
    mpls_2_exp : 3; // 2292
    mpls_2_bos : 1; // 2295
    mpls_2_ttl : 8; // 2296
    inner_ethernet_dstAddr : 48; // 2304
    inner_ethernet_srcAddr : 48; // 2352
    inner_ethernet_etherType : 16; // 2400
    union { /* Header Union */
        struct {
            inner_ipv6_version : 4; // 2416
            inner_ipv6_trafficClass : 8; // 2420
            inner_ipv6_flowLabel : 20; // 2428
            inner_ipv6_payloadLen : 16; // 2448
            inner_ipv6_nextHdr : 8; // 2464
            inner_ipv6_hopLimit : 8; // 2472
        };
        struct {
            inner_ipv4_version : 4; // 2416
            inner_ipv4_ihl : 4; // 2420
            inner_ipv4_diffserv : 8; // 2424
            inner_ipv4_totalLen : 16; // 2432
            inner_ipv4_identification : 16; // 2448
            inner_ipv4_flags : 3; // 2464
            inner_ipv4_fragOffset : 13; // 2467
        };
    };
    flit_2_phv_flit_pad_560_80 : 80; // 2480
/* --------------- Phv Flit 3 ------------ */ 

    ethernet_etherType : 16; // 1536
    vlan_tag_pcp : 3; // 1552
    vlan_tag_dei : 1; // 1555
    vlan_tag_vid : 12; // 1556
    vlan_tag_etherType : 16; // 1568
    union { /* Header Union */
        struct {
            ipv6_version : 4; // 1584
            ipv6_trafficClass : 8; // 1588
            ipv6_flowLabel : 20; // 1596
            ipv6_payloadLen : 16; // 1616
            ipv6_nextHdr : 8; // 1632
            ipv6_hopLimit : 8; // 1640
            ipv6_srcAddr : 128; // 1648
            ipv6_dstAddr : 128; // 1776
        };
        struct {
            ipv4_version : 4; // 1584
            ipv4_ihl : 4; // 1588
            ipv4_diffserv : 8; // 1592
            ipv4_totalLen : 16; // 1600
            ipv4_identification : 16; // 1616
            ipv4_flags : 3; // 1632
            ipv4_fragOffset : 13; // 1635
            ipv4_ttl : 8; // 1648
            ipv4_protocol : 8; // 1656
            ipv4_hdrChecksum : 16; // 1664
            ipv4_srcAddr : 32; // 1680
            ipv4_dstAddr : 32; // 1712
            _padipv4 : 160; // 
        };
    };
    gre_C : 1; // 1904
    gre_R : 1; // 1905
    gre_K : 1; // 1906
    gre_S : 1; // 1907
    gre_s : 1; // 1908
    gre_recurse : 3; // 1909
    gre_flags : 5; // 1912
    gre_ver : 3; // 1917
    gre_proto : 16; // 1920
    erspan_t3_header_version : 4; // 1936
    erspan_t3_header_vlan : 12; // 1940
    erspan_t3_header_priority : 6; // 1952
    erspan_t3_header_span_id : 10; // 1958
    erspan_t3_header_timestamp : 32; // 1968
    erspan_t3_header_sgt : 16; // 2000
    erspan_t3_header_ft_d_other : 16; // 2016
    _flit_1_pad__16 : 16; // 2032
/* --------------- Phv Flit 2 ------------ */ 

    union {
        nat_metadata_nat_ip : 128; // 1024
        capri_i2e_metadata__i2e_meta__nat_metadata_nat_ip : 128; // 1024
    };
    union {
        nat_metadata_nat_l4_port : 16; // 1152
        capri_i2e_metadata__i2e_meta__nat_metadata_nat_l4_port : 16; // 1152
    };
    union {
        nat_metadata_twice_nat_idx : 16; // 1168
        capri_i2e_metadata__i2e_meta__nat_metadata_twice_nat_idx : 16; // 1168
    };
    union {
        capri_i2e_metadata__i2e_meta__copp_metadata_policer_index : 8; // 1184
        copp_metadata_policer_index : 8; // 1184
    };
    union {
        capri_i2e_metadata__i2e_meta__ddos_metadata_ddos_service_policer_idx : 16; // 1192
        ddos_metadata_ddos_service_policer_idx : 16; // 1192
    };
    union {
        ddos_metadata_ddos_src_dst_policer_idx : 16; // 1208
        capri_i2e_metadata__i2e_meta__ddos_metadata_ddos_src_dst_policer_idx : 16; // 1208
    };
    union {
        flow_lkp_metadata_lkp_proto : 8; // 1224
        capri_i2e_metadata__i2e_meta__flow_lkp_metadata_lkp_proto : 8; // 1224
    };
    union {
        capri_i2e_metadata__i2e_meta__flow_lkp_metadata_pkt_type : 8; // 1232
        flow_lkp_metadata_pkt_type : 8; // 1232
    };
    union {
        tunnel_metadata_tunnel_originate : 8; // 1240
        capri_i2e_metadata__i2e_meta__tunnel_metadata_tunnel_originate : 8; // 1240
    };
    union {
        capri_i2e_metadata__i2e_meta__tunnel_metadata_tunnel_terminate : 8; // 1248
        tunnel_metadata_tunnel_terminate : 8; // 1248
    };
    union {
        policer_metadata_egress_policer_index : 16; // 1256
        capri_i2e_metadata__i2e_meta__policer_metadata_egress_policer_index : 16; // 1256
    };
    union {
        rewrite_metadata_rewrite_index : 16; // 1272
        capri_i2e_metadata__i2e_meta__rewrite_metadata_rewrite_index : 16; // 1272
    };
    union {
        rewrite_metadata_tunnel_rewrite_index : 16; // 1288
        capri_i2e_metadata__i2e_meta__rewrite_metadata_tunnel_rewrite_index : 16; // 1288
    };
    union {
        rewrite_metadata_tunnel_vnid : 24; // 1304
        capri_i2e_metadata__i2e_meta__rewrite_metadata_tunnel_vnid : 24; // 1304
    };
    union {
        rewrite_metadata_mac_sa_rewrite : 8; // 1328
        capri_i2e_metadata__i2e_meta__rewrite_metadata_mac_sa_rewrite : 8; // 1328
    };
    union {
        rewrite_metadata_mac_da_rewrite : 8; // 1336
        capri_i2e_metadata__i2e_meta__rewrite_metadata_mac_da_rewrite : 8; // 1336
    };
    union {
        capri_i2e_metadata__i2e_meta__rewrite_metadata_ttl_dec : 8; // 1344
        rewrite_metadata_ttl_dec : 8; // 1344
    };
    union {
        capri_i2e_metadata__i2e_meta__qos_metadata_cos : 8; // 1352
        qos_metadata_cos : 8; // 1352
    };
    union {
        capri_i2e_metadata__i2e_meta__qos_metadata_dscp_en : 8; // 1360
        qos_metadata_dscp_en : 8; // 1360
    };
    union {
        qos_metadata_dscp : 8; // 1368
        capri_i2e_metadata__i2e_meta__qos_metadata_dscp : 8; // 1368
    };
    union {
        l3_metadata_payload_length : 16; // 1376
        capri_i2e_metadata__i2e_meta__l3_metadata_payload_length : 16; // 1376
    };
    union {
        capri_i2e_metadata__i2e_meta__control_metadata_packet_len : 16; // 1392
        control_metadata_packet_len : 16; // 1392
    };
    union {
        control_metadata_bypass : 8; // 1408
        capri_i2e_metadata__i2e_meta__control_metadata_bypass : 8; // 1408
    };
    union {
        control_metadata_flow_miss : 8; // 1416
        capri_i2e_metadata__i2e_meta__control_metadata_flow_miss : 8; // 1416
    };
    union {
        capri_i2e_metadata__i2e_meta__control_metadata_src_lif : 16; // 1424
        control_metadata_src_lif : 16; // 1424
    };
    ethernet_dstAddr : 48; // 1440
    ethernet_srcAddr : 48; // 1488
/* --------------- Phv Flit 1 ------------ */ 

    union {
        rewrite_metadata_entropy_hash : 16; // 512
        capri_i2e_metadata__i2e_meta__rewrite_metadata_entropy_hash : 16; // 512
    };
    union {
        capri_i2e_metadata__i2e_meta__control_metadata_egress_mirror_session_id : 8; // 528
        control_metadata_egress_mirror_session_id : 8; // 528
    };
    capri_rxdma_intrinsic_qid : 24; // 536
    capri_rxdma_intrinsic_qtype : 3; // 560
    capri_rxdma_intrinsic_qstate_addr : 34; // 563
    capri_rxdma_intrinsic_dma_cmd_ptr : 6; // 597
    capri_rxdma_intrinsic_rx_splitter_offset : 10; // 603
    capri_rxdma_intrinsic__padding : 3; // 613
    p4_to_p4plus_ipsec_p4plus_app_id : 4; // 616
    p4_to_p4plus_ipsec_pad : 4; // 620
    p4_to_p4plus_ipsec_ipsec_payload_start : 16; // 624
    p4_to_p4plus_ipsec_ipsec_payload_end : 16; // 640
    p4_to_p4plus_ipsec_l4_protocol : 8; // 656
    p4_to_p4plus_ipsec_ip_hdr_size : 8; // 664
    p4_to_p4plus_ipsec_seqNo : 32; // 672
    p4_to_p4plus_roce_p4plus_app_id : 4; // 704
    p4_to_p4plus_roce_flags : 4; // 708
    p4_to_p4plus_roce_rdma_hdr_len : 8; // 712
    p4_to_p4plus_roce_opcode : 8; // 720
    p4_to_p4plus_roce_raw_flags : 16; // 728
    p4_to_p4plus_roce_mac_da_inner : 48; // 744
    p4_to_p4plus_roce_mac_sa_inner : 48; // 792
    p4_to_p4plus_roce_payload_len : 16; // 840
    tm_replication_data_qtype : 3; // 856
    tm_replication_data_lif : 11; // 859
    tm_replication_data_tunnel_rewrite_index : 10; // 870
    tm_replication_data_qid_or_vnid : 24; // 880
    tm_replication_data_pad_1 : 4; // 904
    tm_replication_data_rewrite_index : 12; // 908
    _flit_0_pad__104 : 104; // 920
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
    nat_metadata_update_inner_checksum : 1; // 160
    control_metadata_cpu_copy : 1; // 161
    control_metadata_egress_ddos_src_vf_policer_drop : 1; // 162
    control_metadata_egress_ddos_service_policer_drop : 1; // 163
    control_metadata_egress_ddos_src_dst_policer_drop : 1; // 164
    pad_before_hv_224_ : 219; // 165
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
    icmp_echo_valid : 1; // 415
    tcp_option_eol_valid : 1; // 416
    tcp_option_nop_valid : 1; // 417
    tcp_option_mss_valid : 1; // 418
    tcp_option_ws_valid : 1; // 419
    tcp_option_sack_perm_valid : 1; // 420
    tcp_option_one_sack_valid : 1; // 421
    tcp_option_three_sack_valid : 1; // 422
    tcp_option_two_sack_valid : 1; // 423
    tcp_option_four_sack_valid : 1; // 424
    tcp_option_timestamp_valid : 1; // 425
    tcp_valid : 1; // 426
    icmp_valid : 1; // 427
    inner_udp_valid : 1; // 428
    inner_ipv4_option_EOL_valid : 1; // 429
    inner_ipv4_option_NOP_valid : 1; // 430
    inner_ipv4_option_security_valid : 1; // 431
    inner_ipv4_option_lsr_valid : 1; // 432
    inner_ipv4_option_rr_valid : 1; // 433
    inner_ipv4_option_ssr_valid : 1; // 434
    inner_ipv4_option_timestamp_valid : 1; // 435
    inner_ipv4_valid : 1; // 436
    inner_ipv6_valid : 1; // 437
    inner_ethernet_valid : 1; // 438
    mpls_2_valid : 1; // 439
    mpls_1_valid : 1; // 440
    mpls_0_valid : 1; // 441
    roce_bth_valid : 1; // 442
    udp_valid : 1; // 443
    vxlan_valid : 1; // 444
    genv_valid : 1; // 445
    vxlan_gpe_valid : 1; // 446
    nvgre_valid : 1; // 447
    flit_0_hv_64 : 1; // 448
    flit_0_hv_65 : 1; // 449
    flit_0_hv_66 : 1; // 450
    flit_0_hv_67 : 1; // 451
    flit_0_hv_68 : 1; // 452
    flit_0_hv_69 : 1; // 453
    flit_0_hv_70 : 1; // 454
    flit_0_hv_71 : 1; // 455
    flit_0_hv_72 : 1; // 456
    flit_0_hv_73 : 1; // 457
    flit_0_hv_74 : 1; // 458
    erspan_t3_header_valid : 1; // 459
    gre_valid : 1; // 460
    ah_valid : 1; // 461
    esp_valid : 1; // 462
    ipv4_option_EOL_valid : 1; // 463
    ipv4_option_NOP_valid : 1; // 464
    ipv4_option_security_valid : 1; // 465
    ipv4_option_lsr_valid : 1; // 466
    ipv4_option_rr_valid : 1; // 467
    ipv4_option_ssr_valid : 1; // 468
    ipv4_option_timestamp_valid : 1; // 469
    v6_generic_valid : 1; // 470
    v6_frag_valid : 1; // 471
    v6_ah_esp_valid : 1; // 472
    ipv4_valid : 1; // 473
    ipv6_valid : 1; // 474
    vlan_tag_valid : 1; // 475
    snap_header_valid : 1; // 476
    llc_header_valid : 1; // 477
    ethernet_valid : 1; // 478
    capri_i2e_metadata_valid : 1; // 479
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
    flit_0_hv_121 : 1; // 505
    flit_0_hv_122 : 1; // 506
    tm_replication_data_valid : 1; // 507
    p4_to_p4plus_roce_valid : 1; // 508
    p4_to_p4plus_ipsec_valid : 1; // 509
    capri_rxdma_intrinsic_valid : 1; // 510
    capri_intrinsic_valid : 1; // 511
};
