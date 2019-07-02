route_result_handler:
    seq             c1, res_reg[ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS:\
                                ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS], 1
    bcf             [c1], route_result_vpc
    seq             c1, res_reg[ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS:\
                                ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS], 1
    bcf             [c1], route_result_st
    nop
route_result_nexthop_group:
    phvwr           p.session_info_hint_nexthop_idx, res_reg[19:0]
    // Set Tx:src_ip to 01 for NH_TYPE=IP
    phvwr           p.session_info_hint_tx_rewrite_flags_src_ip, 1
    // Set Rx:dst_ip to 01 for NH_TYPE=IP
    phvwr           p.session_info_hint_rx_rewrite_flags_dst_ip, 1
    // Set Rx:smac to 1 for NH_TYPE=IP
    phvwr.e         p.session_info_hint_rx_rewrite_flags_smac, 1
    // Disable Peer VNET Route lookup
    phvwr           p.txdma_predicate_lpm1_enable, FALSE

route_result_vpc:
    // Set Tx: Encap for NH_TYPE=VNET
    phvwr.e         p.session_info_hint_tx_rewrite_flags_encap, 1
    phvwr           p.rx_to_tx_hdr_vpc_id, res_reg[7:0]

route_result_st:
    phvwr           p.txdma_control_svc_id, res_reg[9:0]
    // Rewrite Tx: dip only for NH_TYPE=ST
    phvwr           p.session_info_hint_tx_rewrite_flags_dst_ip, 1
    // Set Tx: Encap for NH_TYPE=ST
    phvwr           p.session_info_hint_tx_rewrite_flags_encap, 1
    // For NH_TYPE=ST, set the Tx: src_ip flag to 11 (rewrite CA6)
    phvwr           p.session_info_hint_tx_rewrite_flags_src_ip, 3
    // Set Rx:src_ip to 10 (blind 64 mapping) for NH_TYPE=ST
    phvwr           p.session_info_hint_rx_rewrite_flags_src_ip, 2
    // For NH_TYPE=ST, set the Rx: dst_ip flag to 11 (64 mapping)
    phvwr           p.session_info_hint_rx_rewrite_flags_dst_ip, 3
    // Enable 4-6 lookup
    phvwr.e         p.txdma_control_st_enable, TRUE
    // Disable Peer VNET Route lookup
    phvwr           p.txdma_predicate_lpm1_enable, FALSE
